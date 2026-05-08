#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D noiseTexture;
uniform float rnd;
uniform vec2 resolution;

out vec4 finalColor;

#define Pi  3.14159265359
#define Pi2 6.28318530718

#define blurDirections 16.0
#define blurQuality    4.0
#define blurSize       6.0

uniform vec2 virtualResolution;
float blurFactor = 0.9;
float blurBlink = 0.05;
float blurPower = 0.46;
float curvature = 0.8;
float vignetteForce = 0.3;
float mask = 0.06;

float noiseAmount = 0.025;

float scan_line_strength = -8.0;
float scan_line_amount = 1.0;
float aberation_amount = 0.35;
float pixel_strength = -2.0;

vec2 curve(vec2 uv, float c)
{
    if (c < 0.001) return uv;

    uv = (uv - 0.5) * 2.0;
    uv *= 1.1;
    uv.x *= 1.0 + pow((abs(uv.y) / 5.0 * c * 0.8), 2.0);
    uv.y *= 1.0 + pow((abs(uv.x) / 4.0 * c * 0.8), 2.0);
    uv = (uv / 2.0) + 0.5;
    uv = uv * 0.92 + 0.04;
    return uv;
}

float vignette(vec2 uv)
{
    uv *= 1.0 - uv.xy;
    if (uv.x < 0.0 || uv.y < 0.0) {
        return 0.0;
    }

    float vUvs = uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
    float vigF = abs(2000.0 * vUvs);
    float vig = abs(16.0 * vUvs);
    float vigIn = clamp(pow(vigF, 0.5), 0.0, 1.0);

    return clamp(pow(vig, vignetteForce * 0.5), 0.0, 1.0) * vigIn;
}

float border(vec2 uv)
{
    uv *= 1.0 - uv.xy;

    float r = 0.0;
    float f = 100.0;

    if (uv.y > 0.0 && uv.x > 0.0) return 0.0;
    if (uv.y < 0.0 && uv.x < 0.0) return clamp(abs((uv.y + uv.x) * f), 0.0, 1.0);
    if (uv.x < 0.0) r = clamp(abs(uv.x * f), 0.0, 1.0);
    if (uv.y < 0.0) r = clamp(abs(uv.y * f), 0.0, 1.0);

    return r;
}

vec3 fetchPixel(vec2 uv, vec2 off)
{
    vec2 pos = floor(uv * virtualResolution + off) / virtualResolution + vec2(0.5) / virtualResolution;

    if (max(abs(pos.x - 0.5), abs(pos.y - 0.5)) > 0.5) {
        return vec3(0.0, 0.0, 0.0);
    }

    return texture(texture0, pos).rgb;
}

vec2 dist(vec2 pos)
{
    pos = pos * virtualResolution;
    return -((pos - floor(pos)) - vec2(0.5));
}

float gaus(float pos, float scale)
{
    return exp2(scale * pos * pos);
}

vec3 horz3(vec2 pos, float off)
{
    vec3 b = fetchPixel(pos, vec2(-1.0, off));
    vec3 c = fetchPixel(pos, vec2(0.0, off));
    vec3 d = fetchPixel(pos, vec2(1.0, off));
    float dst = dist(pos).x;

    float scale = pixel_strength;
    float wb = gaus(dst - 1.0, scale);
    float wc = gaus(dst + 0.0, scale);
    float wd = gaus(dst + 1.0, scale);

    return (b * wb + c * wc + d * wd) / (wb + wc + wd);
}

float scan(vec2 pos, float off)
{
    float dst = dist(pos).y;
    return gaus(dst + off, scan_line_strength);
}

vec3 tri(vec2 pos)
{
    vec3 clr = fetchPixel(pos, vec2(0.0));

    if (scan_line_amount > 0.0) {
        vec3 a = horz3(pos, -1.0);
        vec3 b = horz3(pos, 0.0);
        vec3 c = horz3(pos, 1.0);

        float wa = scan(pos, -1.0);
        float wb = scan(pos, 0.0);
        float wc = scan(pos, 1.0);

        vec3 scanlines = a * wa + b * wb + c * wc;
        clr = mix(clr, scanlines, scan_line_amount);
    }

    return clr;
}

vec3 grille(vec2 uv)
{
    float dst = dist(uv).x;
    return mix(vec3(1.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), gaus(dst, -20.0));
}

void main()
{
    float rnd1 = rnd / 100.0;
    float rnd2 = fract(rnd);

    vec2 uv = curve(fragTexCoord, curvature);

    vec2 radius = blurSize * 2.0 / resolution;
    vec3 blur = vec3(0.0);

    for (float d = 0.0; d < Pi2; d += Pi2 / blurDirections) {
        for (float i = 1.0 / blurQuality; i <= 1.0; i += 1.0 / blurQuality) {
            blur += texture(texture0, uv + vec2(cos(d), sin(d)) * radius * i).rgb;
        }
    }

    blur /= (blurQuality * blurDirections) - blurDirections;
    blur = vec3(
        pow(blur.r, blurFactor + rnd1 * blurBlink),
        pow(blur.g, blurFactor + rnd2 * blurBlink),
        pow(blur.b, blurFactor + (rnd / 100.0 * blurBlink))
    );

    vec3 color = vec3(0.0);

    if (aberation_amount > 0.0) {
        float chromatic = aberation_amount * 2.0;
        vec2 chromatic_x = vec2(chromatic, 0.0) / virtualResolution.x;
        vec2 chromatic_y = vec2(0.0, chromatic / 2.0) / virtualResolution.y;
        float r = tri(uv - chromatic_x).r;
        float g = tri(uv + chromatic_y).g;
        float b = tri(uv + chromatic_x).b;
        color = vec3(r, g, b);
    } else {
        color = tri(uv);
    }

    color += blur * blurPower;

    vec2 noiseOffset = vec2(rnd1, rnd2);
    vec3 noise = texture(noiseTexture, ((uv + noiseOffset) * (rnd1 * 0.2 + 1.5))).rgb;

    color += noise * vec3(0.8, 0.7, 0.9) * noiseAmount;

    color *= mix(vec3(1.0), grille(uv), mask);
    color *= vec3(vignette(uv));
    color += mix(vec3(0.0), blur, border(uv)) * vec3(0.0078, 0.1412, 0.1569);

    finalColor = vec4(color, 1.0);
}
