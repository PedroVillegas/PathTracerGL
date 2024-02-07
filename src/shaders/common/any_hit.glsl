#include <common/intersect.glsl>

bool AnyHitBVHTraversal(in Ray r, float tNear, float tFar, inout Payload payload)
{
    bool hit = false;
    vec3 invDir = 1.0 / r.direction;
    int dirIsNeg[3] = { int(invDir.x < 0), int(invDir.y < 0), int(invDir.z < 0) };

	// PBRT v3 BVH Traversal
	int nodesToVisit[STACK_SIZE];	
	int toVisitOffset = 0;
    int currentNodeIndex = 0;

    tNear = FLT_MIN;

	while (true) 
	{
		LinearBVHNode node = bvh.bvh[currentNodeIndex];
        // Check if ray intersects the node's bounding box
		if (Slabs(node.bMin.xyz, node.bMax.xyz, r, tNear, tFar, invDir))
		{
			if (node.n_Primitives > 0)
			{
                int i = node.primitiveOffset;
                if (Intersect(r, Prims.Primitives[i], payload))
                {   
                    // payload returned with closest intersection point so far
                    hit = true;
                    payload.primID = Prims.Primitives[i].id;
                    return hit;
                }

                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
            else
            {
                if (dirIsNeg[node.axis] == 1)
                {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = int(node.bMax.w); // node.secondChildOffset;
                }
                else
                {
                    nodesToVisit[toVisitOffset++] = int(node.bMax.w); // node.secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        }
        else 
        {
            if (toVisitOffset == 0) break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
	}
    return hit;
}

bool AnyHit(Ray ray, inout Payload payload, float dist)
{
    payload.t = dist;
    float tNear = FLT_MIN;
    float tFar = INF;
    if (u_BVHEnabled == 1)
    {  
       return AnyHitBVHTraversal(ray, tNear, tFar, payload);
    }
    else
    {
        for (int k = 0; k < Prims.n_Primitives; k++)
        {
            if (Intersect(ray, Prims.Primitives[k], payload))
            {
                tempHit.primID = Prims.Primitives[k].id;
                return true;
            }
        }
        return false;
    }
}
