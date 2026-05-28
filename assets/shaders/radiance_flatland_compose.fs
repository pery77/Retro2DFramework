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

const int MAX_RAY_GROUPS = 16;

int ray_group_count()
{
    return (baseRays + 3) / 4;
}

bool is_air(vec3 color)
{
    return all(greaterThan(color, vec3(0.995)));
}

bool is_occluder(vec3 color)
{
    return dot(color, color) < 0.0001;
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

void main()
{
    vec2 uv = fragTexCoord;
    vec4 scene = texture(texture0, uv);
    vec3 mask = texture(maskTexture, uv).rgb;
    vec2 pixel = uv * resolution;
    vec3 radiance = expose_radiance(sample_radiance_at_pixel(pixel));
    vec3 emissive = (!is_air(mask) && !is_occluder(mask)) ? mask : vec3(0.0);
    vec3 lit = scene.rgb * (vec3(ambient) + radiance);

    finalColor = vec4(max(lit, emissive), scene.a);
}
