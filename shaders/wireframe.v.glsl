#version 410

uniform mat4 modelviewMatrix;
uniform mat4 MVP_Matrix;

layout(std140) uniform LightInfo {
    vec3 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
} light;

layout(location=0) in vec3 vPos;

out VectorData {
    vec3 position;
    vec3 light;
    vec3 camera;
} vectorsOut;



void main() {
    gl_Position = MVP_Matrix * vec4(vPos, 1);
    
    vec4 eyeCoords = modelviewMatrix * vec4(vPos, 1);
    
    vec3 eyeCoord = vec3( eyeCoords / eyeCoords.w );
    
    vectorsOut.position = vPos;
    vectorsOut.light = normalize( light.position - eyeCoord );
    vectorsOut.camera = normalize( -eyeCoord );
}
