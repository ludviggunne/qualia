#version 450 core

in vec2 vTexCoord;

uniform sampler2D uSampler;

layout(location = 0) out vec4 fColor;

void main() {

    fColor = texture(uSampler, vTexCoord);
}