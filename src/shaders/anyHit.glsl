bool AnyHit(Ray ray, float MaxDistance)
{
    Payload hitrec;
    hitrec.t = FLT_MAX;
    float tNear = FLT_MIN;
    float tFar = MaxDistance;

#ifdef BVH_ENABLED
    float closestSoFar = BVHTraversal(ray, tNear, tFar, hitrec);
    hitrec.t = closestSoFar;
#endif
#ifndef BVH_ENABLED
    for (int i = 0; i < Prims.n_Spheres; i++)
    {        
        Sphere sphere = Prims.Spheres[i];
        if (RaySphereIntersect(ray, sphere, tNear, tFar) && tFar > 0.001 && tNear < MaxDistance)
        {
            return true;
        }
    }
#endif

    for (int i = 0; i < Prims.n_AABBs; i++)
    {        
        AABB aabb = Prims.aabbs[i];
        if (RayAABBIntersect(ray, aabb, tNear, tFar) && tFar > 0.001 && tNear < MaxDistance)
        {
            return true;
        }
    }
    return false;
}