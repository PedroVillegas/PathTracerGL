Ray RayGen(vec2 uv)
{
    // Local Space => World Space => View Space => Clip Space => NDC
    vec4 ndc = vec4(uv, -1.0, 1.0);
    vec4 clip_pos = Camera.InverseProjection * ndc;
    clip_pos.zw = vec2(-1.0, 0.0);

    // Ray direction in world space
    vec3 d = normalize(Camera.InverseView * clip_pos).xyz;

    Ray r = Ray(Camera.position, d);

    return r;
}