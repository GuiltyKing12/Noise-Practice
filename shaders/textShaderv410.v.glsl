#version 410

layout(location=0)in vec4 coord;

out vec2 vTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4( coord.xy, 0.0f, 1.0f );
    vTexCoord = coord.zw;
}
