struct Material
{
    vec3 albedo;
    float roughness;
    vec3 emissive;
    float intensity;
    vec3 absorption;
    float ior;
    float metallic;
    float transmission;
};

struct LinearBVHNode
{
    vec4 bMin;
    vec4 bMax;
    int primitiveOffset;
    int secondChildOffset;
    int n_Primitives;
    int axis;
};

struct Primitive
{
    int id;
    int type;
    vec3 position;
    float radius;
    vec3 dimensions;
    Material mat;
};

struct Light
{
    int id;
    vec3 le;
};

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct Payload
{
    float t; // distance from origin to intersection point along direction 
    vec3 position;
    vec3 normal;
    bool fromInside;
    Material mat;
    int primID;
};