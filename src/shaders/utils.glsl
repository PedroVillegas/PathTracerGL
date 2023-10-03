#define INF     3.402823466e+38
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
    //  See : http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
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
    vec3 orientedNormal = normalize(hitpos - position) * radius;
    vec3 point = SampleHemisphereUniform(orientedNormal);
    pdf = 1.0 / (2.0 * PI);

    return point;
}
vec3 sampleSphere(vec3 position, float radius, out float pdf) {
    float u = Randf01();
    float v = Randf01();

    pdf = 1.0 / (4.0 * PI * radius * radius);
    float theta = acos(1.0 - 2.0 * u);
    float phi = 2.0 * PI * v;
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    float sinTheta = sin(theta);
    vec3 r = radius * vec3(cosPhi*sinTheta, cos(theta), sinPhi*sinTheta);
    // normal = normalize(r);
    return position + r;
}

vec3 SamplePlane(vec3 position, vec3 dimensions, out float pdf)
{
    vec3 bmin = position - dimensions * 0.5;
    vec3 bmax = position + dimensions * 0.5;

    vec3 up = vec3(0.0, 0.0, 1.0);
    vec3 right = vec3(1.0, 0.0, 0.0); 
    // vec3 up = bmax.x - bmin.x;
    // vec3 right = bmax.z - bmin.z;
    //vec3 normal = normalize(cross(right, up));

    pdf = 1.0 / (length(right) * length(up));

    // Use bmin as lower left corner
    return bmin + Randf01() * right + Randf01() * up;
}

vec3 SamplePointOnPrimitive(Primitive primitive, inout float pdf, vec3 hitpos)
{
    switch (primitive.type)
    {
        case 0: // Sphere
            return sampleSphere(primitive.position, primitive.radius, pdf);
        case 1: // AABB
            return SamplePlane(primitive.position, primitive.dimensions, pdf);
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

Ray TransformToWorldSpace(vec2 uv)
{
    // Local Space => World Space => View Space => Clip Space => NDC
    vec4 ndc = vec4(uv, -1.0, 1.0);
    vec4 clip_pos = Camera.InverseProjection * ndc;
    clip_pos.zw = vec2(-1.0, 0.0);

    // Ray direction in world space
    vec3 d = normalize(Camera.InverseView * clip_pos).xyz;

    Ray r = Ray(Camera.position, d);

    return r;
}