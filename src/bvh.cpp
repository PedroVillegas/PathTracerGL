#include <glm/glm.hpp>
#include <vector>

#include "primitives.h"
#include "scene.h"
#include "bvh.h"

bool box_compare(Sphere a, Sphere b, int axis)
{
    AABB b0;
    AABB b1; 

    a.BoundingBox(&b0);
    b.BoundingBox(&b1);

    return b0.bMin[axis] < b1.bMin[axis];
}

bool box_x_compare(Sphere a, Sphere b)
{
    return box_compare(a, b, 0);
}

bool box_y_compare(Sphere a, Sphere b)
{
    return box_compare(a, b, 1);
}

bool box_z_compare(Sphere a, Sphere b)
{
    return box_compare(a, b, 2);
}

BVH::BVH(std::vector<Sphere> &spheres)
{
    bvh_root = RecursiveBuild(spheres, 0, spheres.size());
    int size = CountNodes(bvh_root);
    LinearBVH_Node* flatten = new LinearBVH_Node[size];

    int offset = 0;
    FlattenBVHTree(flatten, bvh_root, &offset, 0);
    flat_root = flatten;
}

BVH::~BVH()
{
    DeleteBVHTree(bvh_root);
    delete flat_root;
}

void BVH::DeleteBVHTree(BVH_Node* node)
{
    if (node == nullptr)
        return; 
    
    DeleteBVHTree(node->left);
    DeleteBVHTree(node->right);

    delete node;
}

void BVH::RebuildBVH(std::vector<Sphere> &spheres)
{
    DeleteBVHTree(bvh_root);
    delete flat_root;

    bvh_root = RecursiveBuild(spheres, 0, spheres.size());
    int size = CountNodes(bvh_root);
    LinearBVH_Node* flatten = new LinearBVH_Node[size];

    int offset = 0;
    FlattenBVHTree(flatten, bvh_root, &offset, 0);
    flat_root = flatten;
    b_Rebuilt = true;
}

BVH_Node* BVH::RecursiveBuild(std::vector<Sphere>& spheres, int start, int end)
{
    BVH_Node* node = new BVH_Node();

    // Determine the tightest bounding box to encapsulate all remaining primitives
    AABB bounds;
    AABB primitiveBounds;
    for (int i = start; i < end; ++i)
        spheres[i].BoundingBox(&primitiveBounds);
        bounds = Union(bounds, primitiveBounds);

    int primitivesCount = end - start;

    if (primitivesCount == 1)
    {
        // Node is a leaf
        node->type = node_t::LEAF;
        node->primitiveOffset = start;
        node->left = node->right = nullptr;

        AABB bbox;
        spheres[start].BoundingBox(&bbox);
        node->bbox = bbox;
    }
    else
    {
        // Node is not a leaf so we must decide on an axis to split along
        // In this case, we want to split along the longest axis
        int axis = bounds.LongestAxis();

        auto comparator =   axis == 0 ? box_x_compare :
                            axis == 1 ? box_y_compare :
                            box_z_compare;

        std::sort(spheres.begin() + start, spheres.begin() + end, comparator);

        node->type = node_t::PARENT;
        node->primitiveOffset = -1;

        int mid = start + primitivesCount / 2;
        
        node->left = RecursiveBuild(spheres, start, mid);
        node->right = RecursiveBuild(spheres, mid, end);
        node->bbox = Union(node->left->bbox, node->right->bbox);
    }
    return node;
}


int BVH::CountNodes(BVH_Node* node)
{
    int c = 1;
    if (node == nullptr)
        return 0;

    c += CountNodes(node->left);
    c += CountNodes(node->right);

    return c;
}

int BVH::FlattenBVHTree(LinearBVH_Node* flatten, BVH_Node* node, int* offset, int depth)
{
    LinearBVH_Node* linear = &flatten[*offset];
    linear->bMin = glm::vec4(node->bbox.bMin, -1);
    linear->bMax = glm::vec4(node->bbox.bMax, -1);
    linear->axis = depth;
    int myOffset = (*offset)++;
    if (node->type != node_t::PARENT)
    {
        linear->primitiveOffset = node->primitiveOffset; 
		linear->primitiveCount = 1;
	}
	else
	{  
		
		linear->primitiveCount = 0;
		int ost = FlattenBVHTree(flatten,node->left, offset, depth + 1);
		linear->bMin.w = ost;
		linear->secondChildOffset = FlattenBVHTree(flatten,node->right, offset, depth + 1);
		linear->bMax.w = linear->secondChildOffset;

	}
	return myOffset;
}
