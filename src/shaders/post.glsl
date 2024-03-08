#version 460 core

out vec4 colour;

uniform vec2 u_Resolution;
uniform sampler2D u_PT_Texture;
uniform sampler3D u_TonyMcMapfaceLUT;
uniform int u_Tonemap;
uniform int u_EnableCrosshair;

#define sat(x) clamp(x, 0., 1.)
#define ENABLE_TONEMAP 1

float Luminance(vec3 c)
{
    return 0.212671 * c.x + 0.715160 * c.y + 0.072169 * c.z;
}

vec3 LessThan(vec3 f, float value)
{
    return vec3(
        (f.x < value) ? 1.0 : 0.0,
        (f.y < value) ? 1.0 : 0.0,
        (f.z < value) ? 1.0 : 0.0);
}

vec3 LinearToSRGB(vec3 rgb)
{
    rgb = clamp(rgb, 0.0, 1.0);
    
    return mix(
        pow(rgb, vec3(0.4545454545)) * 1.055 - 0.055,
        rgb * 12.92,
        LessThan(rgb, 0.0031308)
    );
}

vec3 SRGBToLinear(vec3 rgb)
{   
    rgb = clamp(rgb, 0.0, 1.0);
    
    return mix(
        pow(((rgb + 0.055) / 1.055), vec3(2.2)),
        rgb / 12.92,
        LessThan(rgb, 0.04045)
	);
}

vec3 TonyMcMapface(vec3 stimulus) {
    // Apply a non-linear transform that the LUT is encoded with.
    const vec3 encoded = stimulus / (stimulus + 1.0);

    // Align the encoded range to texel centers.
    const float LUT_DIMS = 48.0;
    vec3 uv = encoded * ((LUT_DIMS - 1.0) / LUT_DIMS) + 0.5 / LUT_DIMS;

    return texture3D(u_TonyMcMapfaceLUT, uv).rbg;
}

// Sources:
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3 ACESInputMat = mat3
(
    vec3(0.59719, 0.35458, 0.04823),
    vec3(0.07600, 0.90834, 0.01566),
    vec3(0.02840, 0.13383, 0.83777)
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat = mat3
(
    vec3(1.60475, -0.53108, -0.07367),
    vec3(-0.10208, 1.10813, -0.00605),
    vec3(-0.00327, -0.07276, 1.07602)
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = color * ACESInputMat;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = color * ACESOutputMat;

    // Clamp to [0, 1]
    color = clamp(color, 0.0, 1.0);

    return color;
}

// ACES tone mapping curve fit to go from HDR to SDR.
//https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm(vec3 color)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 jodieReinhardTonemap(vec3 c){
    // From: https://www.shadertoy.com/view/tdSXzD
    // float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    float l = Luminance(c);
    vec3 tc = c / (c + 1.0);
    return mix(c / (l + 1.0), tc, tc);
}

vec3 agxDefaultContrastApprox(vec3 x) {
  vec3 x2 = x * x;
  vec3 x4 = x2 * x2;
  
  return + 15.5     * x4 * x2
         - 40.14    * x4 * x
         + 31.96    * x4
         - 6.868    * x2 * x
         + 0.4298   * x2
         + 0.1191   * x
         - 0.00232;
}

vec3 agx(vec3 val) {
  const mat3 agx_mat = mat3(
    0.842479062253094, 0.0423282422610123, 0.0423756549057051,
    0.0784335999999992,  0.878468636469772,  0.0784336,
    0.0792237451477643, 0.0791661274605434, 0.879142973793104);
    
  const float min_ev = -12.47393f;
  const float max_ev = 4.026069f;

  // Input transform
  val = agx_mat * val;
  
  // Log2 space encoding
  val = clamp(log2(val), min_ev, max_ev);
  val = (val - min_ev) / (max_ev - min_ev);
  
  // Apply sigmoid function approximation
  val = agxDefaultContrastApprox(val);

  return val;
}

vec3 agxEotf(vec3 val) {
    const mat3 agx_mat_inv = mat3(
        1.19687900512017, -0.0528968517574562, -0.0529716355144438,
        -0.0980208811401368, 1.15190312990417, -0.0980434501171241,
        -0.0990297440797205, -0.0989611768448433, 1.15107367264116);
        
    // Undo input transform
    val = agx_mat_inv * val;

    return val;
}

vec3 agxLook(vec3 val, int type) {
    float luma = Luminance(val);

    vec3 offset = vec3(0.0);
    vec3 slope;
    vec3 power;
    float sat;

    if (type == 0)
    {
        // Default
        slope = vec3(1.0);
        power = vec3(1.0);
        sat = 1.0;
    }
    else if (type == 1)
    {
        // Punchy
        slope = vec3(1.0);
        power = vec3(1.35, 1.35, 1.35);
        sat = 1.4;
    }
    
    // ASC CDL
    val = pow(val * slope + offset, power);
    return luma + sat * (val - luma);
}

// Boolean like functions as described here:
// http://theorangeduck.com/page/avoiding-shader-conditionals

// return 1 if x > y, 0 otherwise.
float gt(float x, float y)
{
    return max(sign(x-y), 0.0);
}

// x and y must be either 0 or 1.
float and(float x, float y)
{
	return x * y;
}

// x and y must be 0 or 1.
float or(float x, float y)
{
    return min(x + y, 1.0);
}

// x must be 0 or 1
float not_(float x)
{
    return 1.0 - x;
}


// https://64.github.io/tonemapping/#aces
void main()
{
    const vec4 crosshairsColor = vec4(vec3(0.7), 1.0);
    const float thickness = 1.0;
    const float length = 5.0;

    vec2 uv = (gl_FragCoord.xy / u_Resolution);
    vec3 hdrCol = texture(u_PT_Texture, uv).rgb;

    // Map HDR to SDR 
    vec3 tonemapped;
    switch (u_Tonemap)
    {
        case 0:
            tonemapped = jodieReinhardTonemap(hdrCol);
            break;
        case 1:
            tonemapped = ACESFilm(hdrCol);
            break;
        case 2:
            tonemapped = ACESFitted(hdrCol);
            break;
        case 3:
        default:
            tonemapped = TonyMcMapface(hdrCol);
            break;
        case 4:
            tonemapped = agx(hdrCol);
            tonemapped = agxLook(tonemapped, 1);
            tonemapped = agxEotf(tonemapped);
            break;
    }
    // Apply gamma correction
#if ENABLE_TONEMAP
    vec3 final = LinearToSRGB(tonemapped);
#else
    vec3 final = LinearToSRGB(hdrCol);
#endif

    // vignetting
//    final *= 0.5 + 0.5*pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.1 );

    vec2 centre = u_Resolution * vec2(0.5);
    vec2 d = abs(centre - gl_FragCoord.xy);
    
    float crosshairMask = or(and(gt(thickness, d.x), gt(length, d.y)),
                             and(gt(thickness, d.y), gt(length, d.x)));
    
    float backgroundMask = not_(crosshairMask);
    
    if (u_EnableCrosshair == 1)
        colour = crosshairMask * crosshairsColor + backgroundMask * vec4(final, 1.0);
    else
        colour = vec4(final, 1.0);

    // colour = vec4(final, 1.0);
}