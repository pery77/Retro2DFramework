#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D sceneTexture;
uniform sampler2D cascadeTexture;
uniform sampler2D maskTexture;
uniform vec2 resolution;
uniform int baseSpacing;
uniform int baseRays;
uniform vec2 probeCount;
uniform float intensity;
uniform float ambient;

out vec4 finalColor;

const int MAX_BASE_RAYS = 64;

vec3 tonemap_aces(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

bool is_air(vec3 color)
{
    return all(greaterThan(color, vec3(0.995)));
}

bool is_occluder(vec3 color)
{
    return dot(color, color) < 0.0001;
}

vec3 sample_probe(vec2 probe)
{
    vec2 extent = vec2(textureSize(cascadeTexture, 0));
    vec2 clamped_probe = clamp(probe, vec2(0.0), probeCount - vec2(1.0));
    int atlas_width = int(extent.x);
    int probe_x = int(clamped_probe.x);
    int probe_y = int(clamped_probe.y);
    int probe_index = probe_y * int(probeCount.x) + probe_x;
    vec3 radiance = vec3(0.0);
    float ray_count = float(min(baseRays, MAX_BASE_RAYS));

    for (int r = 0; r < MAX_BASE_RAYS; ++r) {
        if (r >= baseRays) {
            break;
        }
        int linear = probe_index * baseRays + r;
        vec2 coord = vec2(float(linear - (linear / atlas_width) * atlas_width), float(linear / atlas_width)) + 0.5;
        radiance += texture(cascadeTexture, coord / extent).rgb;
    }

    return radiance / ray_count;
}

vec3 sample_radiance(vec2 probe)
{
    return sample_probe(probe);
}

vec3 sample_radiance_at_pixel(vec2 pixel)
{
    vec2 probe = pixel / float(baseSpacing);
    vec2 p0 = floor(probe);
    vec2 f = smoothstep(vec2(0.0), vec2(1.0), fract(probe));
    vec3 r00 = sample_radiance(p0);
    vec3 r10 = sample_radiance(p0 + vec2(1.0, 0.0));
    vec3 r01 = sample_radiance(p0 + vec2(0.0, 1.0));
    vec3 r11 = sample_radiance(p0 + vec2(1.0, 1.0));
    return mix(mix(r00, r10, f.x), mix(r01, r11, f.x), f.y);
}

float filter_weight(vec2 pixel)
{
    vec3 mask = texture(maskTexture, clamp(pixel / resolution, vec2(0.0), vec2(1.0))).rgb;
    return is_occluder(mask) ? 0.0 : 1.0;
}

vec3 sample_filtered_radiance(vec2 pixel)
{
    vec2 radius = vec2(max(float(baseSpacing), 1.0));
    vec3 color = sample_radiance_at_pixel(pixel) * 0.36;
    float weight = 0.36;

    vec2 offsets[8] = vec2[8](
        vec2( 1.0,  0.0), vec2(-1.0,  0.0), vec2( 0.0,  1.0), vec2( 0.0, -1.0),
        vec2( 1.0,  1.0), vec2(-1.0,  1.0), vec2( 1.0, -1.0), vec2(-1.0, -1.0)
    );

    for (int i = 0; i < 8; ++i) {
        vec2 sample_pixel = pixel + offsets[i] * radius;
        float w = i < 4 ? 0.10 : 0.06;
        w *= filter_weight(sample_pixel);
        color += sample_radiance_at_pixel(sample_pixel) * w;
        weight += w;
    }

    return color / max(weight, 0.0001);
}

void main()
{
    vec2 uv = fragTexCoord;
    vec4 scene = texture(texture0, uv);
    vec3 mask = texture(maskTexture, uv).rgb;
    vec2 pixel = uv * resolution;
    vec3 radiance = sample_filtered_radiance(pixel);

    radiance = tonemap_aces(radiance * intensity);

    vec3 emissive = (!is_air(mask) && !is_occluder(mask)) ? mask : vec3(0.0);
    vec3 lit = scene.rgb * (ambient + radiance);
    finalColor = vec4(max(lit, emissive), scene.a);
}
