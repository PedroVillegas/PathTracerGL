#include "primitives.h"


AABB Union(const AABB& b0, const AABB& b1)
{
    // Compute the bbox of two bboxes
    AABB newBounds;
    newBounds.bMin = glm::min(b0.bMin, b1.bMin);
    newBounds.bMax = glm::max(b0.bMax, b1.bMax);
    return newBounds;
}

AABB Union(const AABB& b, const glm::vec3& p)
{
    AABB newBounds;
    newBounds.bMin = glm::min(b.bMin, p);
    newBounds.bMax = glm::max(b.bMax, p);
    return newBounds;
}

void Primitive::BoundingBox(AABB* out)
{
    float pad = 0.01f;
    switch (type)
    {
        case 0: // sphere
            out->bMax = position + (radius + pad);
            out->bMin = position - (radius + pad);
            break;
        case 1:
            out->bMax = position + (dimensions * 0.5f) + pad;
            out->bMin = position - (dimensions * 0.5f) - pad;
            break;
    }
    
}
