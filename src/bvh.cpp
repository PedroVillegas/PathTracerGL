#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

#include "primitives.h"
#include "scene.h"
#include "bvh.h"

bool box_compare(Primitive a, Primitive b, int axis)
{
    AABB b0;
    AABB b1; 

    a.BoundingBox(&b0);
    b.BoundingBox(&b1);

    return b0.bMin[axis] < b1.bMin[axis];
}

bool box_x_compare(Primitive a, Primitive b)
{
    return box_compare(a, b, 0);
}

bool box_y_compare(Primitive a, Primitive b)
{
    return box_compare(a, b, 1);
}

bool box_z_compare(Primitive a, Primitive b)
{
    return box_compare(a, b, 2);
}

BVH::BVH(std::vector<Primitive>& primitives)
{
    if (primitives.size() == 0)
        return;
        
    int nodeCount = 0;
    bvh_root = RecursiveBuild(primitives, 0, primitives.size(), &nodeCount);
    totalNodes = nodeCount;
    LinearBVH_Node* flatten = new LinearBVH_Node[totalNodes];

    int offset = 0;
    FlattenBVHTree(flatten, bvh_root, &offset);
    flat_root = flatten;
}

BVH::~BVH()
{
    DeleteBVHTree(bvh_root);
    delete[] flat_root;
}

void BVH::DeleteBVHTree(BVH_Node* node)
{
    if (node == nullptr)
        return; 
    
    DeleteBVHTree(node->left);
    DeleteBVHTree(node->right);

    delete node;
}

void BVH::RebuildBVH(std::vector<Primitive>& primitives)
{
    DeleteBVHTree(bvh_root);
    delete[] flat_root;

    int nodeCount = 0;
    bvh_root = RecursiveBuild(primitives, 0, primitives.size(), &nodeCount);
    totalNodes = nodeCount;
    LinearBVH_Node* flatten = new LinearBVH_Node[totalNodes];

    int offset = 0;
    FlattenBVHTree(flatten, bvh_root, &offset);
    flat_root = flatten;
    b_Rebuilt = true;
}

BVH_Node* BVH::RecursiveBuild(std::vector<Primitive>& primitives, size_t start, size_t end, int* nodeCount)
{
    BVH_Node* node = new BVH_Node();
    (*nodeCount)++;

    // Determine the tightest bounding box to encapsulate all remaining primitives
    AABB bounds;
    AABB primitiveBounds;
    for (size_t i = start; i < end; ++i)
    {   
        primitives[i].BoundingBox(&primitiveBounds);
        bounds = Union(bounds, primitiveBounds);
    }

    size_t primitivesCount = end - start;

    if (primitivesCount == 1)
    {
        // Node is a leaf
        node->type = node_t::LEAF;
        node->primitiveOffset = (int) start;
        node->left = node->right = nullptr;

        AABB bbox;
        primitives[start].BoundingBox(&bbox);
        node->bbox = bbox;
    }
    else
    {
        // Node is not a leaf so we must decide on an axis to split along
        // In this case, we want to split along the longest axis
        int axis = bounds.LongestAxis();
        //std::cout << "Split along axis " << axis << std::endl;
        node->axis = axis;

        auto comparator =   axis == 0 ? box_x_compare :
                            axis == 1 ? box_y_compare :
                            box_z_compare;

        std::sort(primitives.begin() + start, primitives.begin() + end, comparator);

        node->type = node_t::PARENT;
        node->primitiveOffset = -1;

        size_t mid = start + primitivesCount / 2;
        
        node->left = RecursiveBuild(primitives, start, mid, nodeCount);
        node->right = RecursiveBuild(primitives, mid, end, nodeCount);
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

int BVH::FlattenBVHTree(LinearBVH_Node* flatten, BVH_Node* node, int* offset)
{
    LinearBVH_Node* linearNode = &flatten[*offset];
    linearNode->bMin = glm::vec4(node->bbox.bMin, -1);
    linearNode->bMax = glm::vec4(node->bbox.bMax, -1);
    int myOffset = (*offset)++;
    if (node->type == node_t::LEAF)
    {
        linearNode->primitiveOffset = node->primitiveOffset; 
		linearNode->primitiveCount = 1;
	}
	else
	{  
        linearNode->axis = node->axis;
		linearNode->primitiveCount = 0;
		int firstChildOffset = FlattenBVHTree(flatten, node->left, offset);
		linearNode->bMin.w = float(firstChildOffset);
		linearNode->secondChildOffset = FlattenBVHTree(flatten, node->right, offset);
		linearNode->bMax.w = float(linearNode->secondChildOffset);

	}
	return myOffset;
}
