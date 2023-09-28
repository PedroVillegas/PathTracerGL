bool RaySphereIntersect(Ray ray, Sphere sphere, out float tNear, out float tFar);
bool RayAABBIntersect(Ray ray, AABB aabb, out float tNear, out float tFar);
vec3 GetAABBNormal(AABB aabb, vec3 surfacePosition);
vec3 GetSphereNormal(Sphere sphere, vec3 surfacePosition);

// #define BVH_ENABLED

Payload TraceRay(Ray ray)
{
    Payload hitrec;
    hitrec.t = FLT_MAX;
    float tNear = FLT_MIN;
    float tFar = FLT_MAX;

#ifdef BVH_ENABLED
    float closestSoFar = BVHTraversal(ray, tNear, tFar, hitrec);
    hitrec.t = closestSoFar;
#endif
#ifndef BVH_ENABLED
    for (int i = 0; i < Prims.n_Spheres; i++)
    {        
        Sphere sphere = Prims.Spheres[i];
        if (RaySphereIntersect(ray, sphere, tNear, tFar) && tFar > 0.001 && tNear < hitrec.t)
        {
            hitrec.t = tNear < 0 ? tFar : tNear;
            hitrec.position = ray.origin + ray.direction * hitrec.t;
            hitrec.mat = sphere.mat; 
            hitrec.fromInside = hitrec.t == tFar;
            vec3 outward_normal = GetSphereNormal(sphere, hitrec.position);
            hitrec.normal = hitrec.fromInside ? -outward_normal : outward_normal;
        }
    }
#endif

    for (int i = 0; i < Prims.n_AABBs; i++)
    {        
        AABB aabb = Prims.aabbs[i];
        if (RayAABBIntersect(ray, aabb, tNear, tFar) && tFar > 0.001 && tNear < hitrec.t)
        {
            hitrec.t = tNear < 0 ? tFar : tNear;
            hitrec.position = ray.origin + ray.direction * hitrec.t;
            hitrec.mat = aabb.mat;
            hitrec.normal = GetAABBNormal(aabb, hitrec.position);
            hitrec.fromInside = hitrec.t == tFar;
        }
    }
    return hitrec;
}

bool RaySphereIntersect(Ray ray, Sphere sphere, out float tNear, out float tFar)
{
    // tNear = tFar = FLT_MAX;

    float radius = sphere.radius;
    vec3 OC = ray.origin - sphere.position;
    vec3 V = -ray.direction;

    // Evaluate intersection points between ray and sphere
    // by solving quadratic equation
    float b = dot(OC, -V);
    float c = dot(OC, OC) - radius * radius;

    // Discriminant hit test (< 0 means no real solution)
    float discriminant = b * b - c;
    if (discriminant < 0.0)
        return false;
    
    discriminant = sqrt(discriminant);
    tNear = -b - discriminant;
    tFar = -b + discriminant;

    return tNear <= tFar;
}

bool RayAABBIntersect(Ray ray, AABB aabb, out float tNear, out float tFar)
{
    tNear = FLT_MIN;
    tFar = FLT_MAX;

    vec3 inv_D = 1.0 / ray.direction;
    vec3 bmin = aabb.position - aabb.dimensions * 0.5;
    vec3 bmax = aabb.position + aabb.dimensions * 0.5;

    vec3 tLower = (bmin - ray.origin) * inv_D;
    vec3 tUpper = (bmax - ray.origin) * inv_D;

    vec3 tMins = min(tLower, tUpper);
    vec3 tMaxes = max(tLower, tUpper);

    tNear = max(tNear, max(tMins.x, max(tMins.y, tMins.z)));
    tFar = min(tFar, min(tMaxes.x, min(tMaxes.y, tMaxes.z)));

    return tNear <= tFar;
}

vec3 GetAABBNormal(AABB aabb, vec3 surfacePosition)
{
    vec3 bmin = aabb.position - aabb.dimensions * 0.5;
    vec3 bmax = aabb.position + aabb.dimensions * 0.5;

    vec3 halfSize = (bmax - bmin) * 0.5;
    vec3 centerSurface = surfacePosition - (bmax + bmin) * 0.5;
    
    vec3 normal = vec3(0.0);
    normal += vec3(sign(centerSurface.x), 0.0, 0.0) * step(abs(abs(centerSurface.x) - halfSize.x), EPSILON);
    normal += vec3(0.0, sign(centerSurface.y), 0.0) * step(abs(abs(centerSurface.y) - halfSize.y), EPSILON);
    normal += vec3(0.0, 0.0, sign(centerSurface.z)) * step(abs(abs(centerSurface.z) - halfSize.z), EPSILON);
    return normalize(normal);
}

vec3 GetSphereNormal(Sphere sphere, vec3 surfacePosition)
{
    return (surfacePosition - sphere.position) / sphere.radius;
}