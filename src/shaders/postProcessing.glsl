#version 410 core

out vec4 colour;

uniform vec2 u_Resolution;
uniform sampler2D u_PT_Texture;

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

vec3 ACESApprox(vec3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
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
    // vec3 mapped = ACESApprox(hdrCol);
    vec3 mapped = ACESFilm(hdrCol);
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