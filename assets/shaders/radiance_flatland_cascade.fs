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

out vec4 finalColor;

const float PI = 3.14159265359;

struct Raymarch2D {
    int x;
    int y;
    int sx;
    int sy;
    int ex;
    int ey;
    float tmx;
    float tmy;
    float tdx;
    float tdy;
};

float pow2i(int value)
{
    return exp2(float(value));
}

bool is_air(vec3 color)
{
    return all(greaterThan(color, vec3(0.995)));
}

Raymarch2D raymarch_make(vec2 a, vec2 b)
{
    Raymarch2D r;
    vec2 d = b - a;
    float len = max(length(d), 0.0001);
    vec2 dir = d / len;

    r.x = int(floor(a.x));
    r.y = int(floor(a.y));
    r.sx = a.x < b.x ? 1 : b.x < a.x ? -1 : 0;
    r.sy = a.y < b.y ? 1 : b.y < a.y ? -1 : 0;
    r.ex = int(floor(b.x)) + 2 * r.sx;
    r.ey = int(floor(b.y)) + 2 * r.sy;
    r.tmx = abs(dir.x) < 0.00001 ? 10000000.0 : ((float(r.x) + (r.sx > 0 ? 1.0 : 0.0)) - a.x) / dir.x;
    r.tmy = abs(dir.y) < 0.00001 ? 10000000.0 : ((float(r.y) + (r.sy > 0 ? 1.0 : 0.0)) - a.y) / dir.y;
    r.tdx = abs(dir.x) < 0.00001 ? 0.0 : abs(1.0 / dir.x);
    r.tdy = abs(dir.y) < 0.00001 ? 0.0 : abs(1.0 / dir.y);
    return r;
}

bool raymarch_next(inout Raymarch2D r)
{
    if (r.tmx < r.tmy) {
        r.tmx += r.tdx;
        r.x += r.sx;
        return r.x != r.ex;
    }

    r.tmy += r.tdy;
    r.y += r.sy;
    return r.y != r.ey;
}

vec3 sample_sky(float angle, float width)
{
    float horizon = 0.35 + 0.65 * max(sin(angle), 0.0);
    return skyColor * horizon * (skyEnabled != 0 ? 1.0 : 0.0);
}

vec4 sample_upper(int xi, int yi, int r, int rn, int n_x, int n_y)
{
    int xi2 = (xi + 1) / 2;
    int yi2 = (yi + 1) / 2;
    int r2 = r << 1;
    int rn2 = rn << 1;
    int n2x = max(n_x >> 1, 1);
    int n2y = max(n_y >> 1, 1);
    float tx = 0.75 - 0.5 * float(xi & 1);
    float ty = 0.75 - 0.5 * float(yi & 1);
    vec2 extent = vec2(textureSize(prevCascade, 0));
    vec4 upper = vec4(0.0);

    for (int ri = 0; ri < 2; ++ri) {
        vec2 p1 = vec2(float(clamp(xi2 - 1, 0, n2x - 1) * rn2 + r2 + ri) + 0.5, float(clamp(yi2 - 1, 0, n2y - 1)) + 0.5);
        vec2 p2 = vec2(float(clamp(xi2,     0, n2x - 1) * rn2 + r2 + ri) + 0.5, float(clamp(yi2 - 1, 0, n2y - 1)) + 0.5);
        vec2 p3 = vec2(float(clamp(xi2 - 1, 0, n2x - 1) * rn2 + r2 + ri) + 0.5, float(clamp(yi2,     0, n2y - 1)) + 0.5);
        vec2 p4 = vec2(float(clamp(xi2,     0, n2x - 1) * rn2 + r2 + ri) + 0.5, float(clamp(yi2,     0, n2y - 1)) + 0.5);
        vec4 a = mix(texture(prevCascade, p1 / extent), texture(prevCascade, p2 / extent), tx);
        vec4 b = mix(texture(prevCascade, p3 / extent), texture(prevCascade, p4 / extent), tx);
        upper += mix(a, b, ty) * 0.5;
    }

    return upper;
}

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    int ci = cascadeIndex;
    int ray_mult = 1 << ci;
    int probe_mult = 1 << ci;
    int n_x = int(probeCount.x) >> ci;
    int n_y = int(probeCount.y) >> ci;
    float d0 = float(baseSpacing);
    float d = d0 * float(probe_mult);
    int rn = baseRays * ray_mult;
    int xi = pixel.x / rn;
    int yi = pixel.y;
    int ray = pixel.x - xi * rn;

    if (xi < 0 || yi < 0 || xi >= n_x || yi >= n_y) {
        finalColor = vec4(0.0);
        return;
    }

    float l = 0.5 * d0;
    float ra = ci == 0 ? 0.0 : l * pow2i((ci - 1) * 2);
    float rb = l * pow2i(ci * 2);
    float angle_width = 2.0 * PI / float(rn);
    float angle = angle_width * (float(ray) + 0.5);
    vec2 dir = vec2(cos(angle), sin(angle));
    vec2 origin = vec2(float(xi), float(yi)) * d + vec2(d * 0.5);
    vec2 a = origin + dir * ra;
    vec2 b = origin + dir * rb;
    Raymarch2D rm = raymarch_make(a, b);
    vec4 col = vec4(0.0);

    for (int step = 0; step < 512; ++step) {
        if (!raymarch_next(rm)) {
            break;
        }
        if (rm.x < 0 || rm.y < 0 || rm.x >= int(resolution.x) || rm.y >= int(resolution.y)) {
            break;
        }

        vec3 scene = texture(sceneTexture, (vec2(float(rm.x), float(rm.y)) + 0.5) / resolution).rgb;
        if (!is_air(scene)) {
            col = vec4(scene, 1.0);
            break;
        }
    }

    if (col.a == 0.0) {
        if (ci == cascadeCount - 1) {
            col = vec4(sample_sky(angle, angle_width), skyEnabled != 0 ? 1.0 : 0.0);
        } else {
            col = sample_upper(xi, yi, ray, rn, n_x, n_y);
        }
    }

    finalColor = vec4(col.rgb, 1.0);
}
