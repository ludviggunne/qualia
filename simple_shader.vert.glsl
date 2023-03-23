#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uView;

out vec2 vTexCoord;

void main() {

    gl_Position = uView * vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}