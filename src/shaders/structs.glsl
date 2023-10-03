struct Material
{
    vec3 albedo;                    // Base Colour
    float specularChance;           // How reflective it is
    vec3 emissive;                  // How emissive it is
    float emissiveStrength;         // How strong it emits
    vec3 absorption;                // Absorption for beer's law
    float refractionChance;         // How transparent it is
    float ior;                      // Index of Refraction - how refractive it is
    float metallic;                 // A material is either metallic or it's not
    float roughness;                // How rough the object is
    float pad;
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