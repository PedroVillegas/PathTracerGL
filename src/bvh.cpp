#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

#include "primitives.h"
#include "scene.h"
#include "bvh.h"


struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(size_t primitiveNumber, const AABB& bounds)
        : primitiveNumber(primitiveNumber),
        bounds(bounds),
        centroid(.5f * bounds.bMin + .5f * bounds.bMax) {}
    size_t primitiveNumber;
    AABB bounds;
    glm::vec3 centroid;
};

BVH::BVH(std::vector<Primitive>& primitives)
{
    if (primitives.size() == 0)
        return;

    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
    for (size_t i = 0; i < primitives.size(); i++)
    {
        AABB bbox;
        primitives[i].BoundingBox(&bbox);
        primitiveInfo[i] = {i, bbox};
    }

    /*std::vector<Primitive> orderedPrims;
    orderedPrims.reserve(primitives.size());*/

    primitivesIndexBuffer.reserve(primitives.size());

    int nodeCount = 0;
    bvh_root = RecursiveBuild(primitiveInfo, 0, primitives.size(), &nodeCount);
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

    primitivesIndexBuffer.resize(0);

    if (primitives.size() == 0)
        return;

    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
    for (size_t i = 0; i < primitives.size(); i++)
    {
        AABB bbox;
        primitives[i].BoundingBox(&bbox);
        primitiveInfo[i] = {i, bbox};
    }

    /*std::vector<Primitive> orderedPrims;
    orderedPrims.reserve(primitives.size());*/

    //std::vector<int> primitivesIndexBuffer;
    primitivesIndexBuffer.reserve(primitives.size());

    int nodeCount = 0;
    bvh_root = RecursiveBuild(primitiveInfo, 0, primitives.size(), &nodeCount);
    totalNodes = nodeCount;

    /*primitives.swap(orderedPrims);
    primitiveInfo.resize(0);*/

    LinearBVH_Node* flatten = new LinearBVH_Node[totalNodes];
    int offset = 0;
    FlattenBVHTree(flatten, bvh_root, &offset);
    flat_root = flatten;
    b_Rebuilt = true;
}

BVH_Node* BVH::RecursiveBuild(
    std::vector<BVHPrimitiveInfo>& primitiveInfo, size_t start, size_t end, int* nodeCount)
{
    BVH_Node* node = new BVH_Node();
    (*nodeCount)++;

    // Determine the tightest bounding box to encapsulate all remaining primitives
    AABB bounds;
    for (size_t i = start; i < end; ++i)
        bounds = Union(bounds, primitiveInfo[i].bounds);

    size_t primitivesCount = end - start;

    if (primitivesCount == 1)
    {
        // Node is a leaf

        int firstPrimOffset = primitivesIndexBuffer.size();
        for (int i = start; i < end; ++i) 
        {
            int primNum = primitiveInfo[i].primitiveNumber;
            primitivesIndexBuffer.push_back(primNum);
        }

        node->type = node_t::LEAF;
        node->primitiveOffset = firstPrimOffset;
        node->left = node->right = nullptr;
        node->bbox = bounds;
    }
    else
    {
        // Node is not a leaf so we must decide on an axis to split along
        // In this case, we want to split along the longest axis
        
        // Compute bound of primitive centroids, choose split dimension _dim_
        AABB centroidBounds;
        for (int i = start; i < end; ++i)
            centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
        int axis = centroidBounds.LongestAxis();

        // Partition into equally sized subsets
        size_t mid = start + primitivesCount / 2;

        /*std::sort(
            primitiveInfo.begin() + start, primitiveInfo.begin() + end, 
            [axis](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b)
            {
                return a.centroid[axis] < b.centroid[axis];
            }
        );*/

        std::nth_element(
            &primitiveInfo[start],
            &primitiveInfo[mid],
            &primitiveInfo[end - 1] + 1,
            [axis](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b)
            {
                return a.centroid[axis] < b.centroid[axis];
            }
        );

        //std::cout << "Split along axis " << axis << std::endl;
        node->axis = axis;
        node->type = node_t::PARENT;
        node->primitiveOffset = -1;
        node->left = RecursiveBuild(primitiveInfo, start, mid, nodeCount);
        node->right = RecursiveBuild(primitiveInfo, mid, end, nodeCount);
        node->bbox = Union(node->left->bbox, node->right->bbox);
    }
    return node;
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
