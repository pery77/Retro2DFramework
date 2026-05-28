#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D sceneTexture;
uniform sampler2D prevCascade;
uniform vec2 resolution;
uniform int baseSpacing;
uniform int baseRays;
uniform vec2 probeCount;
uniform int cascadeIndex;
uniform int cascadeCount;
uniform int skyEnabled;
uniform vec3 skyColor;
uniform float falloff;
uniform float lightRange;

out vec4 finalColor;

const float PI = 3.14159265359;
const float TAU = 6.28318530718;
const float INF = 100000000.0;

struct Raymarch2D {
    ivec2 cell;
    ivec2 step_dir;
    vec2 next_t;
    vec2 delta_t;
    float max_t;
};

int ray_group_count()
{
    return (baseRays + 3) / 4;
}

int pow2i(int value)
{
    return 1 << value;
}

int pow4i(int value)
{
    return 1 << (value * 2);
}

bool is_air(vec3 color)
{
    return all(greaterThan(color, vec3(0.995)));
}

bool is_occluder(vec3 color)
{
    return dot(color, color) < 0.0001;
}

vec3 sample_mask_cell(ivec2 cell)
{
    return texture(sceneTexture, (vec2(cell) + 0.5) / resolution).rgb;
}

vec2 ray_dir(int ray, int ray_count)
{
    float angle = (float(ray) + 0.5) * TAU / float(ray_count);
    return vec2(cos(angle), sin(angle));
}

vec3 sample_sky(vec2 dir)
{
    if (skyEnabled == 0) {
        return vec3(0.0);
    }

    float up = clamp(0.5 - 0.5 * dir.y, 0.0, 1.0);
    float horizon = exp(-abs(dir.y) * 4.0);
    vec3 horizon_color = mix(skyColor, vec3(dot(skyColor, vec3(0.3333))), 0.30) * 0.42;
    vec3 zenith_color = skyColor * 1.05;
    return mix(horizon_color, zenith_color, up * up) + horizon_color * horizon * 0.18;
}

Raymarch2D raymarch_make(vec2 origin, vec2 dir, float start_distance, float end_distance)
{
    Raymarch2D r;
    float start_bias = 0.001;
    vec2 start = origin + dir * (start_distance + start_bias);
    vec2 cell_min = floor(start);
    vec2 cell_max = cell_min + 1.0;

    r.cell = ivec2(cell_min);
    r.step_dir = ivec2(dir.x > 0.0 ? 1 : (dir.x < 0.0 ? -1 : 0),
                       dir.y > 0.0 ? 1 : (dir.y < 0.0 ? -1 : 0));
    r.next_t.x = abs(dir.x) < 0.00001 ? INF :
        (r.step_dir.x > 0 ? (cell_max.x - start.x) / dir.x : (start.x - cell_min.x) / -dir.x);
    r.next_t.y = abs(dir.y) < 0.00001 ? INF :
        (r.step_dir.y > 0 ? (cell_max.y - start.y) / dir.y : (start.y - cell_min.y) / -dir.y);
    r.delta_t = vec2(abs(dir.x) < 0.00001 ? INF : abs(1.0 / dir.x),
                     abs(dir.y) < 0.00001 ? INF : abs(1.0 / dir.y));
    r.max_t = max(end_distance - start_distance - start_bias, 0.0);
    return r;
}

bool raymarch_next(inout Raymarch2D r)
{
    float tx = r.next_t.x;
    float ty = r.next_t.y;
    float t = min(tx, ty);

    if (t > r.max_t) {
        return false;
    }

    if (abs(tx - ty) < 0.00001) {
        r.cell += r.step_dir;
        r.next_t += r.delta_t;
    } else if (tx < ty) {
        r.cell.x += r.step_dir.x;
        r.next_t.x += r.delta_t.x;
    } else {
        r.cell.y += r.step_dir.y;
        r.next_t.y += r.delta_t.y;
    }

    return true;
}

vec4 sample_hit_cell(ivec2 cell, vec2 origin)
{
    if (cell.x < 0 || cell.y < 0 || cell.x >= int(resolution.x) || cell.y >= int(resolution.y)) {
        return vec4(0.0);
    }

    vec3 scene = sample_mask_cell(cell);
    if (is_air(scene)) {
        return vec4(0.0);
    }

    if (is_occluder(scene)) {
        return vec4(0.0, 0.0, 0.0, 1.0);
    }

    float hit_distance = length(vec2(cell) + 0.5 - origin);
    float range = max(lightRange, 1.0);
    float attenuation = pow(clamp(1.0 - hit_distance / range, 0.0, 1.0), max(falloff, 0.05));
    return vec4(scene * attenuation, 1.0);
}

vec4 trace_ray(vec2 origin, vec2 dir, float start_distance, float end_distance)
{
    Raymarch2D rm = raymarch_make(origin, dir, start_distance, end_distance);

    for (int step = 0; step < 1024; ++step) {
        vec4 hit = sample_hit_cell(rm.cell, origin);
        if (hit.a > 0.0) {
            return hit;
        }

        if (abs(rm.next_t.x - rm.next_t.y) < 0.00001 && min(rm.next_t.x, rm.next_t.y) <= rm.max_t) {
            hit = sample_hit_cell(rm.cell + ivec2(rm.step_dir.x, 0), origin);
            if (hit.a > 0.0) {
                return hit;
            }

            hit = sample_hit_cell(rm.cell + ivec2(0, rm.step_dir.y), origin);
            if (hit.a > 0.0) {
                return hit;
            }
        }

        if ((rm.cell.x < 0 || rm.cell.y < 0 || rm.cell.x >= int(resolution.x) || rm.cell.y >= int(resolution.y)) ||
            !raymarch_next(rm)) {
            break;
        }
    }

    return vec4(0.0);
}

ivec2 cascade_probe_dims(int ci)
{
    return ivec2(max(int(probeCount.x) >> ci, 1),
                 max(int(probeCount.y) >> ci, 1));
}

int cascade_tile_count_x(int ci)
{
    return ray_group_count() * pow2i(ci);
}

vec2 atlas_coord(int ci, int ray_group, int probe_x, int probe_y)
{
    ivec2 dims = cascade_probe_dims(ci);
    int tiles_x = cascade_tile_count_x(ci);
    int max_group = ray_group_count() * pow4i(ci) - 1;
    int group = clamp(ray_group, 0, max_group);
    int tile_x = group - (group / tiles_x) * tiles_x;
    int tile_y = group / tiles_x;
    int px = clamp(probe_x, 0, dims.x - 1);
    int py = clamp(probe_y, 0, dims.y - 1);
    return vec2(float(tile_x * dims.x + px), float(tile_y * dims.y + py)) + 0.5;
}

vec4 sample_atlas(sampler2D tex, int ci, int ray_group, int probe_x, int probe_y)
{
    vec2 extent = vec2(textureSize(tex, 0));
    return texture(tex, atlas_coord(ci, ray_group, probe_x, probe_y) / extent);
}

vec4 sample_upper(int xi, int yi, int actual_ray)
{
    int upper_ci = cascadeIndex + 1;
    ivec2 upper_dims = cascade_probe_dims(upper_ci);
    vec2 upper_probe = vec2(float(xi), float(yi)) * 0.5 + 0.25;
    vec2 upper_max = max(vec2(0.5), vec2(upper_dims) - 0.5);
    vec2 sample_pos = clamp(upper_probe, vec2(0.5), upper_max);
    vec2 base = floor(sample_pos - 0.5);
    vec2 f = clamp(sample_pos - (base + 0.5), vec2(0.0), vec2(1.0));
    ivec2 p0 = ivec2(base);

    vec4 p00 = sample_atlas(prevCascade, upper_ci, actual_ray, p0.x,     p0.y);
    vec4 p10 = sample_atlas(prevCascade, upper_ci, actual_ray, p0.x + 1, p0.y);
    vec4 p01 = sample_atlas(prevCascade, upper_ci, actual_ray, p0.x,     p0.y + 1);
    vec4 p11 = sample_atlas(prevCascade, upper_ci, actual_ray, p0.x + 1, p0.y + 1);
    return mix(mix(p00, p10, f.x), mix(p01, p11, f.x), f.y);
}

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    int ci = cascadeIndex;
    ivec2 dims = cascade_probe_dims(ci);
    int tile_x = pixel.x / dims.x;
    int tile_y = pixel.y / dims.y;
    int xi = pixel.x - tile_x * dims.x;
    int yi = pixel.y - tile_y * dims.y;
    int tiles_x = cascade_tile_count_x(ci);
    int tiles_y = pow2i(ci);
    int ray_group = tile_y * tiles_x + tile_x;
    int stored_groups = ray_group_count() * pow4i(ci);
    int actual_ray_count = stored_groups * 4;

    if (tile_x < 0 || tile_y < 0 || tile_x >= tiles_x || tile_y >= tiles_y) {
        finalColor = vec4(0.0);
        return;
    }

    float spacing = float(baseSpacing * pow2i(ci));
    float interval_base = max(float(baseSpacing), 1.0);
    float interval_start = ci == 0 ? 0.0 : interval_base * (float(pow4i(ci)) - 1.0) / 3.0;
    float interval_end = interval_start + interval_base * float(pow4i(ci));
    vec2 origin = (vec2(float(xi), float(yi)) + 0.5) * spacing;
    vec4 sum = vec4(0.0);

    for (int ri = 0; ri < 4; ++ri) {
        int actual_ray = ray_group * 4 + ri;
        vec2 dir = ray_dir(actual_ray, actual_ray_count);
        vec4 ray = trace_ray(origin, dir, interval_start, interval_end);

        if (ray.a == 0.0) {
            if (ci == cascadeCount - 1) {
                ray = vec4(sample_sky(dir), 1.0);
            } else {
                ray = sample_upper(xi, yi, actual_ray);
            }
        }

        sum += ray;
    }

    finalColor = vec4(sum.rgb * 0.25, 1.0);
}
