// =============================================================================
// FILE: NodeType.h   (LINKED LIST VERSION)
// ROLE: One link in the chain. This file IS the linked list concept.
//
// THE SINGLE MOST IMPORTANT DIAGRAM IN THE PRESENTATION:
//
//   head
//    |
//    v
//   +---------+      +---------+      +---------+
//   | PT1001  |      | PT1002  |      | PT1003  |
//   | data    |      | data    |      | data    |
//   | next  --+----->| next  --+----->| next  --+----> 0  (nullptr = the end)
//   +---------+      +---------+      +---------+
//                                        ^
//                                       tail
//
// WHAT TO SAY:
//   Each node holds ONE patient plus the ADDRESS of the next node. The nodes are
//   scattered anywhere in memory; the `next` pointers are what put them in order.
//
//   "SINGLY" linked means there is only a forward pointer. You can move from a
//   node to the one after it, but never backwards, and you cannot jump straight
//   to node 300 - you must walk from head, step by step.
//
// CONSEQUENCE (this is the exam answer):
//   No random access  ->  NO BINARY SEARCH is possible on a linked list.
//   But inserting only rewires two pointers  ->  no element shifting, ever.
// =============================================================================

#ifndef NODETYPE_H
#define NODETYPE_H

#include "Patient.h"

struct NodeType {
    Patient data;      // [1] the payload - one patient record
    NodeType* next;    // [2] the link - address of the following node, or 0
};

#endif
