vec3 Miss(vec3 V)
{
    if (V == Scene.SunDirection)
        return Scene.SunColour * SUN_INTENSITY;

    float a = 0.5 * (normalize(V).y + 1.0);
    vec3 gradient = mix(vec3(1.0), vec3(0.5,0.7,1.0), a);
    return mix(vec3(0.0), gradient, Scene.Day);
}