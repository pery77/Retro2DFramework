#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D noiseTexture;
uniform vec2 resolution = vec2(320.0, 180.0);
uniform float rnd;

out vec4 finalColor;

vec2 curve_uv(vec2 uv)
{
    vec2 centered = uv * 2.0 - 1.0;
    centered *= 1.0 + dot(centered, centered) * 0.045;
    return centered * 0.5 + 0.5;
}

float vignette(vec2 uv)
{
    vec2 edge = uv * (1.0 - uv.yx);
    return clamp(pow(edge.x * edge.y * 18.0, 0.28), 0.0, 1.0);
}

void main()
{
   //resolution = vec2(320.0, 180.0);
    vec2 uv = curve_uv(fragTexCoord);
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec2 chroma = vec2(0.75 / max(resolution.x, 1.0), 0.0);
    float red = texture(texture0, uv - chroma).r;
    float green = texture(texture0, uv).g;
    float blue = texture(texture0, uv + chroma).b;
    vec3 color = vec3(red, green, blue);

    float scanline = 0.92 + 0.08 * sin(uv.y * resolution.y * 3.14159265);
    vec3 noise = texture(noiseTexture, uv * 2.0 + vec2(rnd, rnd * 0.37)).rgb;

    color *= scanline;
    color += (noise - 0.5) * 0.045;
    color *= vignette(uv);
color = vec3(1,0,0);
    finalColor = vec4(color, 1.0) * fragColor;
}
