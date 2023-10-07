#pragma once

#include "primitives.h"

// Differentiate between nodes and leaves of BVH tree
enum node_t { PARENT, LEAF };

struct BVH_Node 
{
    node_t type;
    int primitiveOffset; // 'Pointer' to the primitive in the corresponding list of primitives
    BVH_Node* left;
    BVH_Node* right;
    AABB bbox; 
};

struct alignas(16) LinearBVH_Node
{
    alignas(16) glm::vec4 bMin = glm::vec4();
    alignas(16) glm::vec4 bMax = glm::vec4();
    int primitiveOffset = 0;
    int secondChildOffset = 0;
    int primitiveCount = 0;
    int axis = 0;
};

bool box_compare(Primitive a, Primitive b, int axis);
bool box_x_compare(Primitive a, Primitive b);
bool box_y_compare(Primitive a, Primitive b);
bool box_z_compare(Primitive a, Primitive b);

class BVH
{
public:
    BVH() {};
    BVH(std::vector<Primitive> primitives);
    ~BVH();
    int CountNodes(BVH_Node* root);
    void RebuildBVH(const std::vector<Primitive>& primitives);
public:
    BVH_Node* bvh_root = nullptr;
    LinearBVH_Node* flat_root = nullptr;
    bool b_Rebuilt = false;
private:
    BVH_Node* RecursiveBuild(std::vector<Primitive>& primitives, int start, int end);
    void DeleteBVHTree(BVH_Node* node);
    int FlattenBVHTree(LinearBVH_Node* flatten, BVH_Node* root, int* offset, int depth);
};

