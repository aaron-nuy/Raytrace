#version 430 core

in vec2 vPosition;
uniform sampler2D current;
uniform sampler2D prev;
uniform float aspec;
uniform float counter;

out vec4 fragColor;
void main() {

    vec2 coords = vPosition / 2.0 + 0.5;
    fragColor = texture(current, coords);
    //fragColor = vec4(0,0,0,1);
}

