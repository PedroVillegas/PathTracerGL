#define STACK_SIZE 64

uniform int u_SampleIterations;
uniform int u_SamplesPerPixel;
uniform sampler2D u_AccumulationTexture;
uniform sampler2D u_BlueNoise;

uniform sampler2D u_EnvMapTex;
uniform sampler2D u_EnvMapCDFTex;
uniform float u_EnvMapTotalSum;
uniform float u_EnvMapRotation;

uniform vec2 u_Resolution;
uniform int u_BVHEnabled;
uniform int u_DebugBVHVisualisation;
uniform int u_TotalNodes;
uniform int u_UseBlueNoise;

layout (std140) uniform PrimsBlock
{
    int n_Lights;
    int n_Primitives;
    Light Lights[100];
    Primitive Primitives[100];
} Prims;

layout (std140) uniform SceneBlock
{
    vec3 SunDirection;
    float pad;
    vec3 SunColour;
    float pad1;
    int Depth;
    int SelectedPrimIdx;
    int Day;
} Scene;

layout (std140) uniform CameraBlock
{
    mat4 InverseProjection;
    mat4 InverseView;
    vec3 position;
    float pad;
    float aperture;
    float focalLength;
} Camera;

layout (std430) uniform BVH
{
    LinearBVHNode bvh[1000];
    int PrimitiveIndexBuffer[100];
} bvh;
