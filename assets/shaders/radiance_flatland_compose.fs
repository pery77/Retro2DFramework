#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D sceneTexture;
uniform sampler2D lightTexture;
uniform sampler2D bodyGlowTexture;
uniform sampler2D maskTexture;
uniform vec2 resolution;
uniform vec2 viewportResolution;
uniform vec2 maskOffset;
uniform float ambient;
uniform float edgeForce;
uniform float bodyForce;
uniform float bodyGlowStrength;

out vec4 finalColor;

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

vec2 viewport_uv_from_mask_pixel(vec2 pixel)
{
    vec2 view_pixel = pixel - maskOffset;
    vec2 half_texel = vec2(0.5) / viewportResolution;
    return clamp(view_pixel / viewportResolution, half_texel, vec2(1.0) - half_texel);
}

vec3 sample_light_at_pixel(vec2 pixel)
{
    return texture(lightTexture, viewport_uv_from_mask_pixel(pixel)).rgb;
}

vec3 sample_body_glow_at_pixel(vec2 pixel)
{
    return texture(bodyGlowTexture, viewport_uv_from_mask_pixel(pixel)).rgb;
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
        light += sample_light_at_pixel(pixel + offset) * w;
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
    vec3 rim_light = max(edge_light, radiance) * edge * edgeForce;
    vec3 body_glow = sample_body_glow_at_pixel(pixel) * bodyGlowStrength;
    vec3 body_light = (vec3(ambient) + body_glow) * bodyForce;
    return scene * (body_light + rim_light);
}

void main()
{
    vec2 uv = fragTexCoord;
    vec4 scene = texture(texture0, uv);
    vec2 pixel = uv * viewportResolution + maskOffset;
    vec3 mask = texture(maskTexture, pixel / resolution).rgb;
    vec3 radiance = texture(lightTexture, uv).rgb;
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
