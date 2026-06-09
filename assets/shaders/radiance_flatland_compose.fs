#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D sceneTexture;
uniform sampler2D cascadeTexture;
uniform sampler2D maskTexture;
uniform vec2 resolution;
uniform vec2 viewportResolution;
uniform vec2 maskOffset;
uniform int baseSpacing;
uniform int baseRays;
uniform vec2 probeCount;
uniform float intensity;
uniform float ambient;
uniform float edgeForce;
uniform float bodyForce;

out vec4 finalColor;

const int MAX_RAY_GROUPS = 16;

int ray_group_count()
{
    return (baseRays + 3) / 4;
}

bool is_air(vec3 color)
{
    return all(greaterThan(color, vec3(0.995)));
}

float color_luma(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

bool is_neutral(vec3 color)
{
    float lo = min(min(color.r, color.g), color.b);
    float hi = max(max(color.r, color.g), color.b);
    return hi - lo < 0.025;
}

float occluder_amount(vec3 color)
{
    if (!is_neutral(color)) {
        return 0.0;
    }

    return clamp(1.0 - color_luma(color), 0.0, 1.0);
}

bool is_occluder(vec3 color)
{
    return occluder_amount(color) > 0.015;
}

vec2 atlas_coord(int ray_group, int probe_x, int probe_y)
{
    int groups = ray_group_count();
    int px = clamp(probe_x, 0, int(probeCount.x) - 1);
    int py = clamp(probe_y, 0, int(probeCount.y) - 1);
    int group = clamp(ray_group, 0, groups - 1);
    return vec2(float(group * int(probeCount.x) + px), float(py)) + 0.5;
}

vec3 sample_probe(ivec2 probe)
{
    vec2 extent = vec2(textureSize(cascadeTexture, 0));
    vec3 radiance = vec3(0.0);
    int groups = ray_group_count();

    for (int g = 0; g < MAX_RAY_GROUPS; ++g) {
        if (g >= groups) {
            break;
        }

        radiance += texture(cascadeTexture, atlas_coord(g, probe.x, probe.y) / extent).rgb;
    }

    return radiance / max(float(groups), 1.0);
}

vec3 sample_radiance_at_pixel(vec2 pixel)
{
    float spacing = max(float(baseSpacing), 1.0);
    vec2 probe = pixel / spacing - 0.5;
    vec2 p0 = floor(probe);
    vec2 f = clamp(fract(probe), vec2(0.0), vec2(1.0));
    ivec2 ip0 = ivec2(p0);
    vec3 r00 = sample_probe(ip0);
    vec3 r10 = sample_probe(ip0 + ivec2(1, 0));
    vec3 r01 = sample_probe(ip0 + ivec2(0, 1));
    vec3 r11 = sample_probe(ip0 + ivec2(1, 1));
    return mix(mix(r00, r10, f.x), mix(r01, r11, f.x), f.y);
}

vec3 expose_radiance(vec3 color)
{
    vec3 lit = max(color, vec3(0.0)) * max(intensity, 0.0);
    float peak = max(max(lit.r, lit.g), lit.b);

    if (peak > 1.0) {
        lit /= peak;
    }

    return clamp(lit, vec3(0.0), vec3(1.0));
}

vec3 sample_mask_at_pixel(vec2 pixel)
{
    vec2 p = clamp(pixel, vec2(0.5), resolution - vec2(0.5));
    return texture(maskTexture, p / resolution).rgb;
}

void accumulate_edge_light(vec2 pixel, vec2 offset, inout vec3 light, inout float weight)
{
    vec3 mask = sample_mask_at_pixel(pixel + offset);
    float open = is_occluder(mask) ? 1.0 - occluder_amount(mask) : 1.0;

    if (open > 0.01) {
        float falloff = 1.0 / max(length(offset), 1.0);
        float w = open * falloff;
        light += sample_radiance_at_pixel(pixel + offset) * w;
        weight += w;
    }
}

vec3 shade_occluder(vec3 scene, vec2 pixel, vec3 mask, vec3 radiance)
{
    vec3 edge_light = vec3(0.0);
    float edge_weight = 0.0;

    accumulate_edge_light(pixel, vec2( 1.0,  0.0), edge_light, edge_weight);
    accumulate_edge_light(pixel, vec2(-1.0,  0.0), edge_light, edge_weight);
    accumulate_edge_light(pixel, vec2( 0.0,  1.0), edge_light, edge_weight);
    accumulate_edge_light(pixel, vec2( 0.0, -1.0), edge_light, edge_weight);
    accumulate_edge_light(pixel, vec2( 2.0,  0.0), edge_light, edge_weight);
    accumulate_edge_light(pixel, vec2(-2.0,  0.0), edge_light, edge_weight);
    accumulate_edge_light(pixel, vec2( 0.0,  2.0), edge_light, edge_weight);
    accumulate_edge_light(pixel, vec2( 0.0, -2.0), edge_light, edge_weight);

    if (edge_weight > 0.0) {
        edge_light /= edge_weight;
    }

    float edge = clamp(edge_weight * 0.45, 0.0, 1.0);
    vec3 rim_light =  max(edge_light, radiance ) * edge * edgeForce;
    vec3 body_light = vec3(ambient) * bodyForce;
    return scene * (body_light + rim_light);
}

void main()
{
    vec2 uv = fragTexCoord;
    vec4 scene = texture(texture0, uv);
    vec2 pixel = uv * viewportResolution + maskOffset;
    vec3 mask = texture(maskTexture, pixel / resolution).rgb;
    vec3 radiance = expose_radiance(sample_radiance_at_pixel(pixel));
    vec3 emissive = (!is_air(mask) && !is_occluder(mask)) ? mask : vec3(0.0);


    vec3 mult_lit = scene.rgb * (vec3(ambient) + radiance);

    float scene_luma = color_luma(scene.rgb);
    float shadow_amount = 1.0 - smoothstep(0.15, 0.475, scene_luma);
    vec3 fog_lift = radiance * shadow_amount * 0.35;

    vec3 lit = mult_lit + fog_lift;

    if (is_occluder(mask)) {
        lit = shade_occluder(scene.rgb, pixel, mask, radiance);
    }

    finalColor = vec4(clamp(max(lit, emissive), 0.0, 1.0), scene.a);

}
