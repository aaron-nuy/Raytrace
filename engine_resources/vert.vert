#version 410 core

layout (location = 0) in vec2 aPos;

out vec2 vPosition;

void main() {
	vPosition = aPos;

	gl_Position = vec4(vPosition,0,1);
}