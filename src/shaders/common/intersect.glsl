bool IntersectAABB(vec3 position, vec3 dimensions, Ray ray, inout float tNear, inout float tFar)
{
    tNear = FLT_MIN;
    tFar = INF;

    vec3 inv_D = 1.0 / ray.direction;
    vec3 bmin = position - dimensions * 0.5;
    vec3 bmax = position + dimensions * 0.5;

    vec3 tLower = (bmin - ray.origin) * inv_D;
    vec3 tUpper = (bmax - ray.origin) * inv_D;

    vec3 tMins = min(tLower, tUpper);
    vec3 tMaxes = max(tLower, tUpper);

    tNear = max(tMins.x, max(tMins.y, tMins.z));
    tFar = min(tMaxes.x, min(tMaxes.y, tMaxes.z));

    return tNear <= tFar;
}

vec3 GetAABBNormal(vec3 position, vec3 dimensions, vec3 surfacePosition)
{
    vec3 bmin = position - dimensions * 0.5;
    vec3 bmax = position + dimensions * 0.5;

    vec3 halfSize = (bmax - bmin) * 0.5;
    vec3 centerSurface = surfacePosition - (bmax + bmin) * 0.5;
    
    vec3 normal = vec3(0.0);
    normal += vec3(sign(centerSurface.x), 0.0, 0.0) * step(abs(abs(centerSurface.x) - halfSize.x), EPS);
    normal += vec3(0.0, sign(centerSurface.y), 0.0) * step(abs(abs(centerSurface.y) - halfSize.y), EPS);
    normal += vec3(0.0, 0.0, sign(centerSurface.z)) * step(abs(abs(centerSurface.z) - halfSize.z), EPS);
    return normalize(normal);
}

bool IntersectSphere(vec3 position, float radius, Ray ray, inout float tNear, inout float tFar) 
{
    tNear = FLT_MIN;
    tFar = INF;

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

bool Slabs(in vec3 bMin, in vec3 bMax, in Ray ray, in float tNear, in float tFar, vec3 invDir) 
{
    vec3 tMin = (bMin - ray.origin) * invDir;
    vec3 tMax = (bMax - ray.origin) * invDir;

    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    tNear = max(t1.x, max(t1.y, t1.z));
    tFar = min(t2.x, min(t2.y, t2.z));

    return tNear <= tFar;
}

bool Intersect(Ray ray, Primitive prim, inout Payload payload)
{
    float tNear, tFar;
    switch (prim.type)
    {
        case 0: // Sphere
            // IntersectSphere tNear and tFar to be the intersection points of the ray and object
            // These intersections are only valid if the far one is in front of the camera and
            // the near one is in front of the closest object so far
            if (IntersectSphere(prim.position, prim.radius, ray, tNear, tFar) && tFar > EPS && tNear < payload.t)
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
            if (IntersectAABB(prim.position, prim.dimensions, ray, tNear, tFar) && tFar > EPS && tNear < payload.t)
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