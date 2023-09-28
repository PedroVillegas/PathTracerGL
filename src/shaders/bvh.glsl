bool RaySphereIntersect(Ray ray, Sphere sphere, out float tFar, out float tNear);
bool RayAABBIntersect(Ray ray, AABB aabb, out float tFar, out float tNear);
vec3 GetAABBNormal(AABB aabb, vec3 surfacePosition);
vec3 GetSphereNormal(Sphere sphere, vec3 surfacePosition);

bool Slabs(in vec3 bMin, in vec3 bMax, in Ray r, in float tNear, in float tFar) 
{
    vec3 inv_D =  1.0 / r.direction;

    vec3 tLower = (bMin - r.origin) * inv_D;
    vec3 tUpper = (bMax - r.origin) * inv_D;

    vec3 tMins = min(tLower, tUpper);
    vec3 tMaxes = max(tLower, tUpper);

    tNear = max(tNear, max(tMins.x, max(tMins.y, tMins.z)));
    tFar = min(tFar, min(tMaxes.x, min(tMaxes.y, tMaxes.z)));

    return tNear <= tFar;
}   

float BVHTraversal(in Ray r, float tNear, float tFar, inout Payload hitrec)
{
	// Stack traversal without pointer
	int stack[64];	
	int stackAddr = 0;	
	stack[stackAddr] = 0;

	float closestSoFar = tFar;

	while(stackAddr >= 0 && stackAddr < 64) 
	{
		LinearBVHNode node = bvh.bvh[stack[stackAddr]];
		stackAddr -= 1;

		if(Slabs(node.bMin.xyz, node.bMax.xyz, r, tNear, closestSoFar))
		{
			if(node.bMin.w == -1.0 || node.bMax.w == -1.0)
			{
				int i = node.primitiveOffset;
                Sphere sphere = Prims.Spheres[i];
                float t1, t2;
                if (RaySphereIntersect(r, sphere, t1, t2) && t1 > 0.01 && t2 < closestSoFar)
                {
                    hitrec.t = t1 < 0 ? t2 : t1;
                    hitrec.position = r.origin + r.direction * hitrec.t;
                    hitrec.mat = sphere.mat; 
                    hitrec.fromInside = hitrec.t == t2;
                    vec3 outward_normal = GetSphereNormal(sphere, hitrec.position);
                    hitrec.normal = hitrec.fromInside ? -outward_normal : outward_normal;
                    closestSoFar = hitrec.t;
                }
			}

			if(node.bMin.w != -1.0)
			{
				stackAddr += 1;
				stack[stackAddr] = int(node.bMin.w);
			}
			if(node.bMax.w != -1.0)
			{
				stackAddr += 1;
				stack[stackAddr] = int(node.bMax.w);
			}
		}
	}

	return closestSoFar;
}