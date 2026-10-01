#ifndef DSTR_NODE_TYPE_HPP
#define DSTR_NODE_TYPE_HPP

#include "../common/Patient.hpp"

struct NodeType {
    healthcare::Patient data;
    NodeType* next = nullptr;
};

#endif
