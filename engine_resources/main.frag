#version 430 core

in vec2 vPosition;
uniform sampler2D current;

out vec4 fragColor;

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec2 coords = vPosition / 2.0 + 0.5;

    vec3 mappedColor = ACESFilm(rawColor);
    vec3 finalColor = pow(mappedColor, vec3(1.0 / 2.2));

    fragColor = texture(current, coords);
}