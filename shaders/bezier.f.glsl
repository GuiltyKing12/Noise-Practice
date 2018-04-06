#version 410 core
#define PI 3.14159265

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
    vec4 patchDistance;
} vectorsIn;

uniform int numoctaves;
uniform float persistenceset;
uniform vec3 mixColor;

in vec4 GVertex;

const int p[] = int[512] ( 151,160,137,91,90,15,                 // Hash lookup table as defined by Ken Perlin.  This is a randomly
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,    // arranged array of all numbers from 0-255 inclusive.
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180, 151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
);

out vec4 fragColorOut;

uniform int useOutline;

float amplify(float d, float scale, float offset) {
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

int inc(int num) {
    num++;
    //if (repeat > 0) num %= repeat;
    
    return num;
}

float grad(int hash, float x, float y, float z)
{
    switch(hash & 0xF)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x;
        case 0xD: return -y + z;
        case 0xE: return  y - x;
        case 0xF: return -y - z;
        default: return 0; // never happens
    }
}

float perlin(float x, float y, float z) {
    int xi = int(floor(x)) & 255; // Calculate the "unit cube" that the point asked will be located in
    int yi = int(floor(y)) & 255; // The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
    int zi = int(floor(z)) & 255; // plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
    float xf = x-floor(x);
    float yf = y-floor(y);
    float zf = z-floor(z);
    
    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);
    
    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = p[p[p[    xi ]+    yi ]+    zi ];
    aba = p[p[p[    xi ]+inc(yi)]+    zi ];
    aab = p[p[p[    xi ]+    yi ]+inc(zi)];
    abb = p[p[p[    xi ]+inc(yi)]+inc(zi)];
    baa = p[p[p[inc(xi)]+    yi ]+    zi ];
    bba = p[p[p[inc(xi)]+inc(yi)]+    zi ];
    bab = p[p[p[inc(xi)]+    yi ]+inc(zi)];
    bbb = p[p[p[inc(xi)]+inc(yi)]+inc(zi)];
    
    float x1, x2, y1, y2;
    
    x1 = mix(grad(aaa, xf,   yf, zf),
             grad(baa, xf-1, yf, zf),
             u);
    
    x2 = mix(grad(aba, xf,   yf-1, zf),
             grad(bba, xf-1, yf-1, zf),
             u);
    
    y1 = mix(x1, x2, v);
    
    x1 = mix(grad(aab, xf  , yf  , zf-1),
             grad(bab, xf-1, yf  , zf-1),
             u);
    
    x2 = mix(grad(abb, xf,   yf-1, zf-1),
             grad(bbb, xf-1, yf-1, zf-1),
             u);
    
    y2 = mix(x1, x2, v);
    
    return (mix(y1, y2, w)+1) / 2;
}

float OctavePerlin(float x, float y, float z, int octaves, float persistence) {
    float total = 0;
    float frequency = 1;
    float amplitude = 1;
    float maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
    for(int i=0;i<octaves;i++) {
        total += perlin(x * frequency, y * frequency, z * frequency) * amplitude;
        
        maxValue += amplitude;
        
        amplitude *= persistence;
        frequency *= 2;
    }
    
    return total/maxValue;
}

float stripes(float x, float f) {
    float t = .5 + .5 * sin(f * PI * x);
    return t * t - .5;
}

float marbled(vec3 v) {
    return stripes(v.x + 2 * OctavePerlin(v.x, v.y, v.z, numoctaves, persistenceset), 1.6);
}

void main() {
    float marble = marbled(GVertex.xyz);
    
    vec3 perlinColor = vec3(marble);
    
    vec3 ambient = light.La * Ka;
    
    vec3 normal = vectorsIn.normal;
    if( !gl_FrontFacing )
        normal = -normal;
    
    vec3 halfwayVec = normalize( vectorsIn.light + vectorsIn.camera );
    
    float sDotN = max( dot(vectorsIn.light, normal), 0.0 );
    vec3 diffuse = light.Ld * Kd * sDotN;
    diffuse = mix(diffuse, mixColor, perlinColor);
    
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
        spec = light.Ls * pow( max( dot(normal, halfwayVec), 0.0 ), 4*shininess );
    
    // distance from triangle edge
    float d1 = min( min( vectorsIn.altitudes.x, vectorsIn.altitudes.y ), vectorsIn.altitudes.z );
    // distance from patch edge
    float d2 = min( min( min( vectorsIn.patchDistance.x, vectorsIn.patchDistance.y ), vectorsIn.patchDistance.z ), vectorsIn.patchDistance.w );
    
    vec4 phongColor = vec4(ambient+diffuse+spec, 1);
    
    d1 = 1 - amplify(d1, 250, -0.5);
    d2 = amplify(d2, 100, -0.5);
    fragColorOut = d2 * phongColor + d1 * d2 * vec4(1,1,1,1);
    fragColorOut.a = 1;
    
    if( useOutline == 0 )
        fragColorOut = phongColor;
    
    if( !gl_FrontFacing )
        fragColorOut.rgb = fragColorOut.bgr;
    
}
