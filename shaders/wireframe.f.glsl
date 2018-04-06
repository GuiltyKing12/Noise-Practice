#version 410

layout(std140) uniform LightInfo {
    vec3 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
} light;

layout(std140) uniform MaterialInfo {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float shininess;
};

in Vectors {
    vec3 light;
    vec3 camera;
    vec3 normal;
    vec3 altitudes;
} vectorsIn;

out vec4 fragColorOut;

uniform int useOutline;

void main() {
    vec3 ambient = light.La * Ka;
    
    vec3 normal = vectorsIn.normal;
    if( !gl_FrontFacing )
        normal = -normal;
    
    vec3 halfwayVec = normalize( vectorsIn.light + vectorsIn.camera );
    
    float sDotN = max( dot(vectorsIn.light, normal), 0.0 );
    vec3 diffuse = light.Ld * Kd * sDotN;
    
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
        spec = light.Ls * Ks * pow( max( dot(normal, halfwayVec), 0.0 ), 4*shininess );
    
    float d = min( vectorsIn.altitudes.x, vectorsIn.altitudes.y );
    d = min( d, vectorsIn.altitudes.z );
    
    float mixVal = smoothstep( .03, .05, d );
    
    vec4 phongColor = vec4(ambient+diffuse+spec, 1);
    vec4 outlineColor = vec4(0,0,1,1);
    
    if( useOutline == 0 )
        mixVal = 1;
    
    fragColorOut = mix( outlineColor, phongColor, mixVal );
    
    if( !gl_FrontFacing )
        fragColorOut.rgb = fragColorOut.bgr;
}
