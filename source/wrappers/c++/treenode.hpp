#ifndef UDA_WRAPPERS_CPP_TREENODE_H
#define UDA_WRAPPERS_CPP_TREENODE_H

#include <string>
#include <vector>

#include <structures/genStructs.h>
#include <clientserver/export.h>

#include "scalar.hpp"
#include "vector.hpp"
#include "array.hpp"
#include "structdata.hpp"

namespace uda {

class LIBRARY_API TreeNode
{
public:
    TreeNode findStructureDefinition(const std::string& name);
    TreeNode findStructureComponent(const std::string& name);
    TreeNode parent();
    size_t numChildren();
    std::vector<uda::TreeNode> children();
    uda::TreeNode child(int num);
    void printNode();
    void printStructureNames();
    void printUserDefinedTypeTable();
    void printUserDefinedTypeTable(const std::string& name);

    int structureCount();
    std::vector<std::string> structureNames();
    std::vector<std::string> structureTypes();

    std::string name();

    int atomicCount();
    std::vector<std::string> atomicNames();
    std::vector<std::string> atomicTypes();
    std::vector<bool> atomicPointers();
    std::vector<size_t> atomicRank();
    std::vector<std::vector<size_t> > atomicShape();
    void * structureComponentData(const std::string& name);
    Scalar atomicScalar(const std::string& target);
    Vector atomicVector(const std::string& target);
    Array atomicArray(const std::string& target);

    StructData structData(const std::string& target);
    void * data();

    NTREE* node() { return node_; }

private:
    TreeNode(int handle, NTREE* node, NTREE* full_ntree)
        : handle_(handle), node_(node), full_ntree_(full_ntree)
    {}
    friend class Result;
    int handle_;
    NTREE* node_;
    NTREE* full_ntree_;
};


}

#endif // UDA_WRAPPERS_CPP_TREENODE_H
