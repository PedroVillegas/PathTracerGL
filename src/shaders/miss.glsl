vec3 Miss(vec3 V)
{           
    float t = 0.5 * (V.y + 1.0);
    // vec3 horizonColour = 1.2 * vec3(1.00,0.90,0.83);
    vec3 horizonColour = 1.0 * vec3(0.86,0.98,1.00);
    vec3 skyColour = vec3(0.5,0.7,1.0);
    vec3 atmosphere;

    u_Day == 1  ? atmosphere = mix(horizonColour, skyColour, t)
                : atmosphere = vec3(0.0);

    return atmosphere;
}