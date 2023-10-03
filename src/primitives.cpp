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
    switch (type)
    {
        case 0: // sphere
            out->bMax = position + (radius + 0.05f);
            out->bMin = position - (radius + 0.05f);
            break;
        case 1:
            out->bMax = position + (dimensions * 0.5f);
            out->bMin = position - (dimensions * 0.5f);
            break;
    }
    
}
