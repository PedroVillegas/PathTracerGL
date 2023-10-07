bool IntersectSphere(vec3 position, float radius, Ray ray, inout float tNear, inout float tFar);
bool IntersectAABB(vec3 position, vec3 dimensions, Ray ray, inout float tNear, inout float tFar);
bool Intersect(Ray ray, Primitive prim, inout Payload payload);
vec3 GetAABBNormal(vec3 position, vec3 dimensions, vec3 surfacePosition);

bool TracePrimsHit(Ray ray, inout Payload tempHit, float dist)
{
    tempHit.t = dist;
    for (int k = 0; k < Prims.n_Primitives; k++)
    {
        if (Intersect(ray, Prims.Primitives[k], tempHit))
        {
            tempHit.primID = Prims.Primitives[k].id;
            return true;
        }
    }
    return false;
}

// #define BVH_ENABLED

Payload TracePrims(Ray ray, float dist)
{
    Payload payload;
    payload.t = dist;

#ifndef BVH_ENABLED
    for (int k = 0; k < Prims.n_Primitives; k++)
    {
        if (Intersect(ray, Prims.Primitives[k], payload))
        {
            payload.primID = Prims.Primitives[k].id;
        }
    }
#endif
    return payload;
}

bool Intersect(Ray ray, Primitive prim, inout Payload payload)
{
    float tNear = FLT_MIN;
    float tFar = INF;
    switch (prim.type)
    {
        case 0: // Sphere
            if (IntersectSphere(prim.position, prim.radius, ray, tNear, tFar) && tFar > 0.001 && tNear < payload.t)
            {
                payload.t = tNear < 0 ? tFar : tNear;
                payload.position = ray.origin + ray.direction * payload.t;
                payload.mat = prim.mat; 
                payload.fromInside = payload.t == tFar;
                vec3 outward_normal = (payload.position - prim.position) / prim.radius;
                payload.normal = payload.fromInside ? -outward_normal : outward_normal;
                return true;
            }
            break;
        case 1: // AABB
            if (IntersectAABB(prim.position, prim.dimensions, ray, tNear, tFar) && tFar > 0.001 && tNear < payload.t)
            {
                payload.t = tNear < 0 ? tFar : tNear;
                payload.position = ray.origin + ray.direction * payload.t;
                payload.mat = prim.mat;
                vec3 outward_normal = GetAABBNormal(prim.position, prim.dimensions, payload.position);
                payload.fromInside = payload.t == tFar;
                payload.normal = payload.fromInside ? -outward_normal : outward_normal;
                return true;
            }
            break;
    }
    return false;
}

bool IntersectAABB(vec3 position, vec3 dimensions, Ray ray, inout float tNear, inout float tFar)
{
    vec3 inv_D = 1.0 / ray.direction;
    vec3 bmin = position - dimensions * 0.5;
    vec3 bmax = position + dimensions * 0.5;

    vec3 tLower = (bmin - ray.origin) * inv_D;
    vec3 tUpper = (bmax - ray.origin) * inv_D;

    vec3 tMins = min(tLower, tUpper);
    vec3 tMaxes = max(tLower, tUpper);

    tNear = max(tNear, max(tMins.x, max(tMins.y, tMins.z)));
    tFar = min(tFar, min(tMaxes.x, min(tMaxes.y, tMaxes.z)));

    return tNear <= tFar;
}

vec3 GetAABBNormal(vec3 position, vec3 dimensions, vec3 surfacePosition)
{
    vec3 bmin = position - dimensions * 0.5;
    vec3 bmax = position + dimensions * 0.5;

    vec3 halfSize = (bmax - bmin) * 0.5;
    vec3 centerSurface = surfacePosition - (bmax + bmin) * 0.5;
    
    vec3 normal = vec3(0.0);
    normal += vec3(sign(centerSurface.x), 0.0, 0.0) * step(abs(abs(centerSurface.x) - halfSize.x), EPSILON);
    normal += vec3(0.0, sign(centerSurface.y), 0.0) * step(abs(abs(centerSurface.y) - halfSize.y), EPSILON);
    normal += vec3(0.0, 0.0, sign(centerSurface.z)) * step(abs(abs(centerSurface.z) - halfSize.z), EPSILON);
    return normalize(normal);
}

bool IntersectSphere(vec3 position, float radius, Ray ray, inout float tNear, inout float tFar) 
{
    tNear = tFar = INF;

    float b = dot(ray.origin - position, ray.direction);
    float len = length(ray.origin - position);
    float c = len*len - radius*radius;
    float D = b*b - c;
    if(D < 0.0) {
        return false;
    }

    tNear = -b - sqrt(D);
    tFar = -b + sqrt(D);
    
    return tNear <= tFar;
}