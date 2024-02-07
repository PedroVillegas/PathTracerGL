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

bool box_compare(Primitive a, Primitive b, int axis);
bool box_x_compare(Primitive a, Primitive b);
bool box_y_compare(Primitive a, Primitive b);
bool box_z_compare(Primitive a, Primitive b);

class BVH
{
public:
    BVH() {};
    BVH(std::vector<Primitive>& primitives);
    ~BVH();
    int CountNodes(BVH_Node* root);
    void RebuildBVH(std::vector<Primitive>& primitives);
public:
    BVH_Node* bvh_root = nullptr;
    LinearBVH_Node* flat_root = nullptr;
    bool b_Rebuilt = false;
private:
    BVH_Node* RecursiveBuild(std::vector<Primitive>& primitives, size_t start, size_t end);
    void DeleteBVHTree(BVH_Node* node);
    int FlattenBVHTree(LinearBVH_Node* flatten, BVH_Node* root, int* offset);
};

