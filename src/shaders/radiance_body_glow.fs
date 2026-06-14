#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D lightTexture;
uniform vec2 resolution;
uniform float radius;

out vec4 finalColor;

const float PI2 = 6.28318530718;
const float BLUR_DIRECTIONS = 16.0;
const float BLUR_QUALITY = 4.0;

void main()
{
    vec2 uv = fragTexCoord;
    vec2 blur_radius = max(radius, 0.0) / resolution;
    vec3 blur = texture(lightTexture, uv).rgb;
    float samples = 1.0;

    for (float d = 0.0; d < PI2; d += PI2 / BLUR_DIRECTIONS) {
        vec2 direction = vec2(cos(d), sin(d));

        for (float i = 1.0 / BLUR_QUALITY; i <= 1.0; i += 1.0 / BLUR_QUALITY) {
            vec2 sample_uv = clamp(uv + direction * blur_radius * i, vec2(0.0), vec2(1.0));
            blur += texture(lightTexture, sample_uv).rgb;
            samples += 1.0;
        }
    }

    finalColor = vec4(blur / samples, 1.0);
}
