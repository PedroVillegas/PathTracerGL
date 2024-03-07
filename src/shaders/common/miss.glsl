vec3 SunWithBloom(vec3 rayDir, vec3 sunDir) {
    const float sunSolidAngle = 0.53*PI/180.0;
    const float minSunCosTheta = cos(sunSolidAngle);

    float cosTheta = dot(rayDir, sunDir);
    if (cosTheta >= minSunCosTheta) return vec3(1.0);
    
    float offset = minSunCosTheta - cosTheta;
    float gaussianBloom = exp(-offset*50000.0)*0.5;
    float invBloom = 1.0/(0.02 + offset*300.0)*0.01;
    return vec3(gaussianBloom+invBloom);
}

vec3 Miss(vec3 V)
{
    vec3 lum;
    // Sun from https://www.shadertoy.com/view/slSXRW
    vec3 sunLum = SunWithBloom(V, Scene.SunDirection);
    // Use smoothstep to limit the effect, so it drops off to actual zero.
    sunLum = smoothstep(0.002, 1.0, sunLum);
    if (length(sunLum) > 0.0)
        sunLum *= Scene.SunColour * SUN_INTENSITY;

    float a = 0.5 * (normalize(V).y + 1.0);
    lum = mix(vec3(1.0), vec3(0.39, 0.57, 1.0), a) + sunLum;

    return mix(vec3(0.0), lum, Scene.Day);
}