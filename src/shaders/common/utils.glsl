#define INF             3.402823466e+38
#define FLT_MIN        -3.402823466e+38
#define PI              3.14159265358979323
#define GOLDEN_RATIO    1.61803398874989485    
#define ONE_PI          0.3183098861837907
#define EPSILON         1e-3

uint g_Seed;
vec2 uv;

uint GenerateSeed()
{
    return uint(uint(gl_FragCoord.x) * uint(1973) + uint(gl_FragCoord.y) * uint(9277) + uint(u_SampleIterations+1) * uint(26699)) | uint(1);
}

uint PCGHash()
{
    g_Seed = g_Seed * uint(747796405) + uint(2891336453);
    uint state = g_Seed;
    uint word = ((state >> ((state >> uint(28)) + uint(4))) ^ state) * uint(277803737);
    return (word >> uint(22)) ^ word;
}

float Randf01()
{
    if (u_UseBlueNoise == 1)
    {
        vec2 tc = (uv + 1.0) / 2.0;
        vec2 ts = textureSize(u_BlueNoise, 0);
        float blueNoise = texelFetch(u_BlueNoise, ivec2(mod(gl_FragCoord.x, ts.x), mod(gl_FragCoord.y, ts.y)), 0).r;
        return fract(float(PCGHash()) / float(uint(0xffffffff)) + blueNoise);
//        return fract(blueNoise + (GOLDEN_RATIO * u_SampleIterations));
    }
    return float(PCGHash()) / float(uint(0xffffffff));
}

float Randf()
{
    return float(PCGHash());
}

vec2 SampleUniformUnitCirle(float r_1, float r_2)
{
    float theta = r_1 * 2.0 * PI;
    float r = sqrt(r_2);
    return vec2(cos(theta), sin(theta)) * r;
}

vec3 SampleUniformUnitSphere(float u_1, float u_2)
{
    vec3 dir;
    float theta = u_1 * 2.0 * PI;
    float z = u_2 * 2.0 - 1.0;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(theta);
    float y = r * sin(theta);
    dir = vec3(x, y, z);

    return normalize(dir);
}

vec3 SampleHemisphereUniform(vec3 N)
{
    float u = Randf01();
    float v = Randf01();

    // vec3 B = normalize(cross(N, vec3(0.0, 1.0, 1.0)));
	// vec3 T = cross(B, N);

    vec3 p = SampleUniformUnitSphere(u, v);
    return p * sign(dot(p, N));
    // return vec3(cos(phi) * sin(theta), y, sin(phi) * sin(theta));
}


vec3 Ortho(vec3 v) 
{
    return abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0) : vec3(0.0, -v.z, v.y);
}

vec3 GetConeSample(vec3 dir, float extent) 
{
    // Formula 34 in GI Compendium
	dir = normalize(dir);
	vec3 o1 = normalize(Ortho(dir));
	vec3 o2 = normalize(cross(dir, o1));
	vec2 r =  vec2(Randf01(), Randf01());
	r.x=r.x*2.*PI;
	r.y=1.0-r.y*extent;
	float oneminus = sqrt(1.0-r.y*r.y);
	return cos(r.x)*oneminus*o1+sin(r.x)*oneminus*o2+r.y*dir;
}

vec3 SampleSphere(vec3 position, float radius, inout float pdf, vec3 hitpos)
{
    vec3 sampledPoint = position + SampleHemisphereUniform(normalize(hitpos - position)) * radius;
    pdf = 1.0 / (2.0 * PI * radius * radius); // 1.0 / Area

    return sampledPoint;
}

vec3 SampleCubeNew(vec3 position, vec3 dimensions, out float pdf, vec3 hitpos)
{
    float width  = dimensions.x;
    float height = dimensions.y;
    float depth  = dimensions.z;
    
    if (height == 0.0)
    {
        float area = width * depth;
        pdf = 1.0 / (area);
        float r_1 = Randf01() - 0.5;
        float r_2 = Randf01() - 0.5;
        return position + vec3(r_1 * width, 0.0, r_2 * depth);
    }

    float area = 2.0 * ((height * depth) + (height * width) + (width * depth));
    pdf = 1.0 / (area);

    float half_width  = width  * 0.5;
    float half_height = height * 0.5;
    float half_depth  = depth  * 0.5;
    vec3 normal;
    int i = 0;
    int face;

    do {
        face = int(mod(Randf(), 6.0)); // Returns [0,5] uniformly distributed
        switch (face)
        {
            case 0: // Bottom face
                normal = vec3(0.0, -half_height, 0.0);
                break;
            case 3: // Top face
                normal = vec3(0.0, half_height, 0.0);
                break;
            case 1: // Left face
                normal = vec3(-half_width, 0.0, 0.0);
                break;
            case 4: // Right face
                normal = vec3(half_width, 0.0, 0.0);
                break;
            case 2: // Back face
                normal = vec3(0.0, 0.0, -half_depth);
                break;
            case 5: // Front face
                normal = vec3(0.0, 0.0, half_depth);
                break;
        }
    } while (dot(normal, hitpos - position) <= 0.0 && i++ < 10);

    float r_1 = Randf01() - 0.5;
    float r_2 = Randf01() - 0.5;
    vec3 sampledPoint;

    switch (face % 3)
    {
        case 0: // Bottom or top face
            sampledPoint = normal + vec3(r_1 * width, 0.0, r_2 * depth);
            pdf = 1.0 / (width * depth);
            break;
        case 1: // Left or right face
            sampledPoint = normal + vec3(0.0, r_1 * height, r_2 * depth);
            pdf = 1.0 / (height * depth);
            break;
        case 2: // Back or front face
            sampledPoint = normal + vec3(r_1 * width, r_2 * height, 0.0);
            pdf = 1.0 / (width * height);
            break;
    }
    return position + sampledPoint;
}

vec3 SampleCube(vec3 position, vec3 dimensions, out float pdf)
{
    float width  = dimensions.x;
    float height = dimensions.y;
    float depth  = dimensions.z;
    
    if (height == 0.0)
    {
        float area = width * depth;
        pdf = 1.0 / (area);
        float r_1 = Randf01() - 0.5;
        float r_2 = Randf01() - 0.5;
        return position + vec3(r_1 * width, 0.0, r_2 * depth);
    }

    float area = 2.0 * ((height * depth) + (height * width) + (width * depth));
    pdf = 1.0 / (area);

    vec3 sampledPoint;
    int face = int(mod(Randf(), 6.0)); // Returns [0,5] uniformly distributed
    float r_1 = Randf01() - 0.5;
    float r_2 = Randf01() - 0.5;

    switch (face)
    {
        case 0:
            // Bottom face
            sampledPoint = vec3(r_1 * width, -height/2, r_2 * depth);
            break;
        case 1:
            // Top face
            sampledPoint = vec3(r_1 * width, +height/2, r_2 * depth);
            break;
        case 2:
            // Left face
            sampledPoint = vec3(-width/2, r_1 * height, r_2 * depth);
            break;
        case 3:
            // Right face
            sampledPoint = vec3(+width/2, r_1 * height, r_2 * depth);
            break;
        case 4:
            // Back face
            sampledPoint = vec3(r_1 * width, r_2 * height, -depth/2);
            break;
        case 5:
            // Front face
            sampledPoint = vec3(r_1 * width, r_2 * height, +depth/2);
            break;
    }

    return position + sampledPoint;
}

vec3 SamplePointOnPrimitive(Primitive primitive, inout float pdf, vec3 hitpos)
{
    switch (primitive.type)
    {
        case 0: // Sphere
            return SampleSphere(primitive.position, primitive.radius, pdf, hitpos);
        case 1: // AABB
            return SampleCubeNew(primitive.position, primitive.dimensions, pdf, hitpos);
    }
}

// vec3 SampleHemisphereCosine(float r_1, float r_2, vec3 N)
// {
//     vec3 dir;
//     vec3 B = normalize(cross(N, vec3(0.0, 1.0, 1.0)));
// 	vec3 T = cross(B, N);
// 	float r = sqrt(r_1);
//     float phi = 2.0 * PI * r_2;
// 	vec3 x = r * cos(phi) * B; 
// 	vec3 y = r * sin(phi) * T;
// 	vec3 z = sqrt(1.0 - r*r) * N;
//     dir = x + y + z;
    
//     return normalize(dir);
// }

vec3 SampleCosineHemisphere(float u_1, float u_2, vec3 N) 
{
    return normalize(N + SampleUniformUnitSphere(u_1, u_2));
}

float FresnelSchlick(float cosine_t, float n_1, float n_2)
{
    float r_0 = (n_1 - n_2) / (n_1 + n_2);
    r_0 = r_0 * r_0;
    return r_0 + (1.0 - r_0) * pow((1.0 - cosine_t), 5.0);
}

float FresnelSchlick(float cosine_t)
{
    return pow((1.0-cosine_t), 5.0);
}

float FresnelReflectAmount(float n1, float n2, vec3 N, vec3 V, float f_0, float f_90)
{
        // Schlick aproximation
        float r0 = (n1-n2) / (n1+n2);
        r0 *= r0;
        float cosX = -dot(N, V);
        if (n1 > n2)
        {
            float n = n1/n2;
            float sinT2 = n*n*(1.0-cosX*cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return f_90;
            cosX = sqrt(1.0-sinT2);
        }
        float x = 1.0-cosX;
        float ret = r0+(1.0-r0)*x*x*x*x*x;
 
        // adjust reflect multiplier for object reflectivity
        return mix(f_0, f_90, ret);
}
