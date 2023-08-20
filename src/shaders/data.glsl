#include <structs.glsl>

uniform int u_SampleIterations;
uniform int u_SamplesPerPixel;
uniform int u_Depth;
uniform int u_Day;
uniform int u_SelectedObjIdx;
uniform vec3 u_LightDir;
uniform float u_FocalLength;
uniform float u_Aperture;
uniform vec2 u_Resolution;
uniform vec3 u_RayOrigin;
uniform vec3 u_ObjectCounts;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;
uniform mat4 u_ViewProjection;
uniform sampler2D u_AccumulationTexture;

layout (std140) uniform ObjectData
{
    int n_Spheres;
    Sphere Spheres[50];
    int n_AABBs;
    AABB aabbs[50];
    int n_Lights;
    Light Lights[50];
} objectData;

layout (std140) uniform BVH
{
    LinearBVHNode bvh[50];
} bvh;