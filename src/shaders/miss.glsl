vec3 calc(float x, vec3 a, vec3 b, vec3 c, vec3 d)
{
    // sin(1/x) suggested by Phillip Trudeau
    return (b - d) * sin(1. / (vec3(x) / c + 2. / radians(180.) - a)) + d;
}

vec3 Miss(vec3 V)
{           
    float alp = normalize(V.y) * 0.5 + 0.5;
    vec3 da = mix(vec3(1.0), vec3(82.0,102.0,130.0)/255.0, alp);
    return mix(vec3(0.0), da, Scene.Day);

    vec3 atmosphere;
    vec3 p_dark[4] = vec3[4](
        vec3(0.3720705374951474, 0.3037080684557225, 0.26548632969565816),
        vec3(0.446163834012046, 0.39405890487346595, 0.425676737673072),
        vec3(0.16514907579431481, 0.40461292460006665, 0.8799446225003938),
        vec3(-7.057075230154481e-17, -0.08647963850488945, -0.269042973306185)
    );

    vec3 p_bright[4] = vec3[4](
        vec3( 0.38976745480184677, 0.31560358280318124,  0.27932656874),
        vec3( 1.2874522895367628,  1.0100154283349794,   0.862325457544),
        vec3( 0.12605043174959588, 0.23134451619328716,  0.526179948359),
        vec3(-0.0929868539256387, -0.07334463258550537, -0.192877259333)
    );

    float uvx = V.x * 0.5 + 0.5;
    float night = .3 + .7 * sin(uvx * radians(60.) + (0. - 4.) * radians(30.));
    float x = mix(night, V.y, Scene.Day);

    vec3 a = mix(p_dark[0], p_bright[0], x);
    vec3 b = mix(p_dark[1], p_bright[1], x);
    vec3 c = mix(p_dark[2], p_bright[2], x);
    vec3 d = mix(p_dark[3], p_bright[3], x);

    if (dot(V, vec3(0, 1, 0)) > .005) 
    {
        atmosphere = calc(V.y, a, b, c, d);
    } 
    else 
    {
        atmosphere = mix(vec3(0.0), vec3(0.333), Scene.Day);
    }

    return atmosphere;
}