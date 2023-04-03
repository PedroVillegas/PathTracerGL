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

void main()
{
    vec2 uv = (gl_FragCoord.xy / u_Resolution);
    vec3 hdrCol = texture(u_PT_Texture, uv).rgb;
    // Map HDR to SDR 
    vec3 mapped = ACESFilm(hdrCol);
    // Apply gamma correction
    mapped = LinearToSRGB(mapped);
    colour = vec4(mapped, 1.0);
}