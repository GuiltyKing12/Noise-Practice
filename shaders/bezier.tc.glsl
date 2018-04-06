#version 410 core

layout(vertices=16) out;

uniform vec4 tessLevelOuter;
uniform vec2 tessLevelInner;

void main() {
    // pass along the vertex position unmodified
    gl_out[ gl_InvocationID ].gl_Position = gl_in[ gl_InvocationID ].gl_Position;
    
    gl_TessLevelOuter[0] = float( tessLevelOuter.x );
    gl_TessLevelOuter[1] = float( tessLevelOuter.y );
    gl_TessLevelOuter[2] = float( tessLevelOuter.z );
    gl_TessLevelOuter[3] = float( tessLevelOuter.w );
    
    gl_TessLevelInner[0] = float( tessLevelInner.x );
    gl_TessLevelInner[1] = float( tessLevelInner.y );
}
