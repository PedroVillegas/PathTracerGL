#pragma once

#include "primitives.h"
#include <vector>

// Differentiate between nodes and leaves of BVH tree
enum node_t { PARENT, LEAF };

struct BVH_Node 
{
    node_t type;
    int primitiveOffset; // 'Pointer' to the primitive in the corresponding list of primitives
    int axis;
    BVH_Node* left;
    BVH_Node* right;
    AABB bbox;
};

struct LinearBVH_Node
{
    glm::vec4 bMin = glm::vec4();
    glm::vec4 bMax = glm::vec4();
    int primitiveOffset = 0;
    int secondChildOffset = 0;
    int primitiveCount = 0;
    int axis = -1;
};

struct BVHPrimitiveInfo;

class BVH
{
public:
    BVH() {};
    BVH(std::vector<Primitive>& primitives);
    ~BVH();
    void RebuildBVH(std::vector<Primitive>& primitives);

public:
    BVH_Node* bvh_root = nullptr;
    LinearBVH_Node* flat_root = nullptr;
    bool b_Rebuilt = false;
    int totalNodes = 0;
    std::vector<int> primitivesIndexBuffer;

private:
    BVH_Node* RecursiveBuild(
        std::vector<BVHPrimitiveInfo>& primitiveInfo, size_t start, size_t end, int* nodeCount);
    void DeleteBVHTree(BVH_Node* node);
    int FlattenBVHTree(LinearBVH_Node* flatten, BVH_Node* root, int* offset);
};

