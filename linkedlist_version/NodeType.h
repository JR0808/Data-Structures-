#ifndef NODETYPE_H
#define NODETYPE_H

#include "Patient.h"

struct NodeType {
    Patient data;
    NodeType* next;
};

#endif
