#version 460 core

out vec4 colour;

uniform vec2 u_Resolution;
uniform sampler2D u_PT_Texture;

#define sat(x) clamp(x, 0., 1.)
#define AGX_LOOK 0

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
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
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
    
    // sRGB IEC 61966-2-1 2.2 Exponent Reference EOTF Display
    //val = pow(val, vec3(2.2));

    return val;
}

vec3 agxLook(vec3 val) {
    const vec3 lw = vec3(0.2126, 0.7152, 0.0722);
    float luma = dot(val, lw);
    
    // Default
    vec3 offset = vec3(0.0);
    vec3 slope = vec3(1.0);
    vec3 power = vec3(1.0);
    float sat = 1.0;
    
    #if AGX_LOOK == 1
    // Golden
    slope = vec3(1.0, 0.9, 0.5);
    power = vec3(0.8);
    sat = 0.8;
    #elif AGX_LOOK == 2
    // Punchy
    slope = vec3(1.0);
    power = vec3(1.35, 1.35, 1.35);
    sat = 1.4;
    #endif
    
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
    // vec3 mapped = jodieReinhardTonemap(hdrCol);
    // vec3 mapped = ACESFilm(hdrCol);

    // AgX
    // ->
    vec3 mapped = agx(hdrCol);
    mapped = agxLook(mapped);
    mapped = agxEotf(mapped);
    // <-

    // Apply gamma correction
    mapped = LinearToSRGB(mapped);
    // vignetting
    // mapped *= 0.5 + 0.5*pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.1 );

    vec2 centre = u_Resolution * vec2(0.5);
    vec2 d = abs(centre - gl_FragCoord.xy);
    
    float crosshairMask = or(and(gt(thickness, d.x), gt(length, d.y)),
                             and(gt(thickness, d.y), gt(length, d.x)));
    
    float backgroundMask = not_(crosshairMask);
    
    colour = crosshairMask * crosshairsColor + backgroundMask * vec4(mapped, 1.0); 

    // colour = vec4(mapped, 1.0);
}