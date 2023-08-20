bool RaySphereIntersect(Ray ray, Sphere sphere, out float t1, out float t2);
bool RayAABBIntersect(Ray ray, AABB aabb, out float t1, out float t2);
vec3 GetAABBNormal(AABB aabb, vec3 surfacePosition);
vec3 GetSphereNormal(Sphere sphere, vec3 surfacePosition);

Payload TraceRay(Ray ray)
{
    Payload hitrec;
    hitrec.t = FLT_MAX;
    float t1; // Near plane
    float t2;

    for (int i = 0; i < objectData.n_Spheres; i++)
    {        
        Sphere sphere = objectData.Spheres[i];
        if (RaySphereIntersect(ray, sphere, t1, t2) && t2 > 0.01 && t1 < hitrec.t)
        {
            hitrec.t = t1 < 0 ? t2 : t1;
            hitrec.position = ray.origin + ray.direction * hitrec.t;
            hitrec.mat = sphere.mat; 
            hitrec.fromInside = hitrec.t == t2;
            vec3 outward_normal = GetSphereNormal(sphere, hitrec.position);
            hitrec.normal = hitrec.fromInside ? -outward_normal : outward_normal;
        }
    }

    for (int i = 0; i < objectData.n_AABBs; i++)
    {        
        AABB aabb = objectData.aabbs[i];
        if (RayAABBIntersect(ray, aabb, t1, t2) && t2 > 0.01 && t1 < hitrec.t)
        {
            hitrec.t = t1 < 0 ? t2 : t1;
            hitrec.position = ray.origin + ray.direction * hitrec.t;
            hitrec.mat = aabb.mat;
            hitrec.normal = GetAABBNormal(aabb, hitrec.position);
            hitrec.fromInside = hitrec.t == t2;
        }
    }

    return hitrec;
}

bool RaySphereIntersect(Ray ray, Sphere sphere, out float t1, out float t2)
{
    t1 = FLT_MAX;
    t2 = FLT_MAX;

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
    t1 = -b - discriminant;
    t2 = -b + discriminant;

    return  t1 <= t2;
}

bool RayAABBIntersect(Ray ray, AABB aabb, out float t1, out float t2)
{
    t1 = FLT_MIN;
    t2 = FLT_MAX;

    vec3 inv_D =  1.0 / ray.direction;
    vec3 bmin = aabb.position - aabb.dimensions * 0.5;
    vec3 bmax = aabb.position + aabb.dimensions * 0.5;

    vec3 tLower = (bmin - ray.origin) * inv_D;
    vec3 tUpper = (bmax - ray.origin) * inv_D;

    vec3 tMins = min(tLower, tUpper);
    vec3 tMaxes = max(tLower, tUpper);

    t1 = max(t1, max(tMins.x, max(tMins.y, tMins.z)));
    t2 = min(t2, min(tMaxes.x, min(tMaxes.y, tMaxes.z)));

    return t1 <= t2;
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