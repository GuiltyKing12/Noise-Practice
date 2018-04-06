#version 410

uniform mat4 MVP_Matrix;

layout(location=0) in vec3 vPos;

void main() {
    gl_Position = MVP_Matrix * vec4(vPos, 1);
}
