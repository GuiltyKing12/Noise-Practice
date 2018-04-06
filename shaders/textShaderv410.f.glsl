#version 410

in vec2 vTexCoord;

uniform sampler2D tex;
uniform vec4 color;

out vec4 fragColorOut;

void main() {
    vec4 texel = texture(tex, vTexCoord);
    
    fragColorOut = vec4(1.0f, 1.0f, 1.0f, texel.r) * color;
}
