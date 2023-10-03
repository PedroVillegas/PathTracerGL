bool IntersectSphere(vec3 position, float radius, Ray ray, inout float tNear, inout float tFar);
bool IntersectAABB(vec3 position, vec3 dimensions, Ray ray, inout float tNear, inout float tFar);
bool Intersect(Ray ray, Primitive prim, inout Payload payload);

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

bool TraverseBVH(in Ray r, inout Payload payload)
{
    bool hit = false;
    float tNear = INF;
    float tFar = INF;

	// Stack traversal without pointer
	int stack[64];	
	int stackAdrr = 0;	
	stack[stackAdrr] = 0;

	float closestSoFar = tFar;

	while (stackAdrr >= 0 && stackAdrr < 64) 
	{
		LinearBVHNode node = bvh.bvh[7];
        payload.position = Prims.Primitives[node.primitiveOffset].mat.albedo;
        // return true;
		stackAdrr -= 1;

		if (Slabs(node.bMin.xyz, node.bMax.xyz, r, tNear, closestSoFar))
		{
            return true;
			if (node.bMin.w == -1.0 || node.bMax.w == -1.0)
			{
                // Node is a leaf (No Children)
                if (node.n_Primitives > 0)
                {
                    int i = node.primitiveOffset;
                    Payload tempHit;
                    if (Intersect(r, Prims.Primitives[i], tempHit))
                    {
                        hit = true;
                        closestSoFar = tempHit.t;
                        payload = tempHit;
                    }
                }
			}

			if (node.bMin.w != -1.0)
			{
				stackAdrr += 1;
				stack[stackAdrr] = int(node.bMin.w);
			}
			if (node.bMax.w != -1.0)
			{
				stackAdrr += 1;
				stack[stackAdrr] = int(node.bMax.w);
			}
		}
	}
	return hit;
}