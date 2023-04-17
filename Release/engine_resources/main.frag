#version 410 core

in vec2 vPosition;
uniform sampler2D current;
uniform sampler2D prev;
uniform float aspec;
uniform float counter;

void main() {

	vec2 coords = vPosition / 2.0 + 0.5;
	gl_FragColor = texture( current , coords);
	//gl_FragColor = vec4(0,0,0,1);
}

