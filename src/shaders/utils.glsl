#define FLT_MAX     3.402823466e+38
#define FLT_MIN    -3.402823466e+38
#define PI          3.14159265358979323
#define EPSILON     1e-3

uint g_Seed = 0;

uint GenerateSeed()
{
    return uint(gl_FragCoord.x * 1973 + gl_FragCoord.y * 9277 + u_SampleIterations * 2699) | uint(1);
}

uint PCGHash()
{
    uint state = g_Seed;
    g_Seed *= 747796405 + 2891336453;
    uint word = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    return (word >> 22) ^ word;
}

float Randf01()
{
    return float(PCGHash()) / 4294967295.0;
}

vec2 UniformSampleUnitCircle(float r_1, float r_2)
{
    float theta = r_1 * 2.0 * PI;
    float r = sqrt(r_2);
    return vec2(cos(theta), sin(theta)) * r;
}

vec3 UniformSampleUnitSphere(float u_1, float u_2)
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

vec3 CosineSampleHemisphere(float r_1, float r_2, vec3 N)
{
    vec3 dir;
    vec3 B = normalize(cross(N, vec3(0.0, 1.0, 1.0)));
	vec3 T = cross(B, N);
	float r = sqrt(r_1);
    float phi = 2.0 * PI * r_2;
	vec3 x = r * cos(phi) * B; 
	vec3 y = r * sin(phi) * T;
	vec3 z = sqrt(1.0 - r*r) * N;
    dir = x + y + z;
    
    return normalize(dir);
}

vec3 NoTangentCosineHemisphere(float u_1, float u_2, vec3 N) 
{
    return normalize(N + UniformSampleUnitSphere(u_1, u_2));
}

Ray TransformToWorldSpace(vec2 uv)
{
    // Local Space => World Space => View Space => Clip Space => NDC
    vec4 ndc = vec4(uv, -1.0, 1.0);
    vec4 clip_pos = u_InverseProjection * ndc;
    clip_pos.zw = vec2(-1.0, 0.0);

    // Ray direction in world space
    vec3 d = normalize(u_InverseView * clip_pos).xyz;

    Ray r;

    r.origin = u_RayOrigin;
    r.direction = d;

    return r;
}