#include "primitives.h"

std::ostream& operator<<(std::ostream& os, const AABB& aabb)
{
    os << "Max Bound: " << aabb.bMax.x << ", " << aabb.bMax.y << ", " << aabb.bMax.z << "\n"
       << "Min Bound: " << aabb.bMin.x << ", " << aabb.bMin.y << ", " << aabb.bMin.z;
    return os;
}

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
    float pad = 0.0f;
    switch (type)
    {
        case PRIM_SPHERE:
            out->bMax = position + (radius + pad);
            out->bMin = position - (radius + pad);
            break;
        case PRIM_AABB:
            glm::vec3 a = dimensions * 0.5f;
            out->bMax = a + pad;
            out->bMin = -(a + pad);

            if (rotation != glm::mat4(1.0f))
            {
                glm::vec3 min = glm::vec3(FLT_MAX);
                glm::vec3 max = glm::vec3(FLT_MIN);

                for (int i = 0; i < 2; i++) {
                    for (int j = 0; j < 2; j++) {
                        for (int k = 0; k < 2; k++) {
                            auto x = i*out->bMax.x + (1-i)*out->bMin.x;
                            auto y = j*out->bMax.y + (1-j)*out->bMin.y;
                            auto z = k*out->bMax.z + (1-k)*out->bMin.z;

                            glm::vec3 tester = glm::mat3(rotation) * glm::vec3(x, y, z);

                            for (int c = 0; c < 3; c++) {
                                min[c] = glm::min(min[c], tester[c]);
                                max[c] = glm::max(max[c], tester[c]);
                            }
                        }
                    }
                }
                out->bMax = position + max;
                out->bMin = position + min;
                break;
            }
            out->bMax += position;
            out->bMin += position;
            break;
    }
    
}
