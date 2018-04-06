#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

uniform mat4 modelviewMatrix;
uniform mat4 normalMatrix;
uniform float time;

in VectorData {
    vec3 position;
    vec3 light;
    vec3 camera;
} vectorsIn[];

out Vectors {
    vec3 light;
    vec3 camera;
    vec3 normal;
    vec3 altitudes;
} vectorsOut;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 4.0f;
    vec3 direction = normal * ((sin(time) + 1.0f) / 2.0f) * magnitude;
    return position;// + vec4(direction, 0.0f);
}

void main() {
    
    vec3 a = vectorsIn[1].position - vectorsIn[0].position;
    vec3 b = vectorsIn[2].position - vectorsIn[0].position;
    vec3 c = vectorsIn[1].position - vectorsIn[2].position;
    vec3 normal = normalize( cross(a, b) );
    
    normal = transpose(inverse(mat3(modelviewMatrix)))*normal;
    
    float lenA = length(c);
    float lenB = length(b);
    float lenC = length(a);
    
    float beta = acos( (lenA*lenA + lenC*lenC - lenB*lenB) / (2*lenA*lenC) );
    float ha = lenC * sin(beta);
    
    float gamma = acos( (lenB*lenB + lenA*lenA - lenC*lenC) / (2*lenB*lenA) );
    float hb = lenA * sin(gamma);
    
    float alpha = acos( (lenC*lenC + lenB*lenB - lenA*lenA) / (2*lenC*lenB) );
    float hc = lenB * sin(alpha);
    
    gl_Position = explode(gl_in[0].gl_Position, -normal);
    vectorsOut.light = vectorsIn[0].light;
    vectorsOut.camera = vectorsIn[0].camera;
    vectorsOut.normal = normal;
    vectorsOut.altitudes = vec3(ha, 0, 0);
    EmitVertex();
    
    gl_Position = explode(gl_in[1].gl_Position, -normal);
    vectorsOut.light = vectorsIn[1].light;
    vectorsOut.camera = vectorsIn[1].camera;
    vectorsOut.normal = normal;
    vectorsOut.altitudes = vec3(0, hb, 0);
    EmitVertex();
    
    gl_Position = explode(gl_in[2].gl_Position, -normal);
    vectorsOut.light = vectorsIn[2].light;
    vectorsOut.camera = vectorsIn[2].camera;
    vectorsOut.normal = normal;
    vectorsOut.altitudes = vec3(0, 0, hc);
    EmitVertex();
    
    EndPrimitive();
}
