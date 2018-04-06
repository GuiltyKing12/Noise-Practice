#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

layout(std140) uniform LightInfo {
    vec3 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
} light;

uniform mat4 modelviewMatrix;
uniform mat4 normalMatrix;

in vec3 TEPosition[];   // vertex position in camera coords
in vec3 TENormal[];     // vertex normal in camera coords
in vec4 TEPatchDistance[];  // patch u,v info
in vec4 TEVertex[];

out Vectors {
    vec3 light;
    vec3 camera;
    vec3 normal;
    vec3 altitudes;
    vec4 patchDistance;
} vectorsOut;

out vec4 GVertex;

void main() {
    
    vec3 a = TEPosition[1] - TEPosition[0];
    vec3 b = TEPosition[2] - TEPosition[0];
    vec3 c = TEPosition[1] - TEPosition[2];
    
    float lenA = length(c);
    float lenB = length(b);
    float lenC = length(a);
    
    float beta = acos( (lenA*lenA + lenC*lenC - lenB*lenB) / (2*lenA*lenC) );
    float ha = lenC * sin(beta);
    
    float gamma = acos( (lenB*lenB + lenA*lenA - lenC*lenC) / (2*lenB*lenA) );
    float hb = lenA * sin(gamma);
    
    float alpha = acos( (lenC*lenC + lenB*lenB - lenA*lenA) / (2*lenC*lenB) );
    float hc = lenB * sin(alpha);
    
    gl_Position = gl_in[0].gl_Position;
    GVertex = TEVertex[0];
    vectorsOut.light = normalize( light.position - TEPosition[0] );
    vectorsOut.camera = normalize( -TEPosition[0] );
    vectorsOut.normal = TENormal[0];
    vectorsOut.altitudes = vec3(ha, 0, 0);
    vectorsOut.patchDistance = TEPatchDistance[0];
    EmitVertex();
    
    gl_Position = gl_in[1].gl_Position;
    GVertex = TEVertex[1];
    vectorsOut.light = normalize( light.position - TEPosition[1] );
    vectorsOut.camera = normalize( -TEPosition[1] );
    vectorsOut.normal = TENormal[1];
    vectorsOut.altitudes = vec3(0, hb, 0);
    vectorsOut.patchDistance = TEPatchDistance[1];
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    GVertex = TEVertex[2];
    vectorsOut.light = normalize( light.position - TEPosition[2] );
    vectorsOut.camera = normalize( -TEPosition[2] );
    vectorsOut.normal = TENormal[2];
    vectorsOut.altitudes = vec3(0, 0, hc);
    vectorsOut.patchDistance = TEPatchDistance[2];
    EmitVertex();
    
    EndPrimitive();
}
