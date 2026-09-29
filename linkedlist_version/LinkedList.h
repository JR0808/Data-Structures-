// =============================================================================
// FILE: LinkedList.h   (LINKED LIST VERSION)
// ROLE: Declares the self-built singly linked list that stores the patients.
//
// COMPARE DIRECTLY WITH ArrayList.h WHEN PRESENTING:
//
//                      ArrayList                LinkedList
//   storage            one contiguous block     scattered nodes + pointers
//   member variables   patients, size, capacity head, tail, size
//   reach record i     patients[i]        O(1)  walk i next hops      O(n)
//   growing            allocate + copy all      just link a new node  O(1)
//   insert in middle   shift everything         rewire two pointers
//   binary search      possible                 IMPOSSIBLE (no random access)
//
// NOTE: there is no `capacity` here. A linked list has no fixed size to outgrow,
// so grow() does not exist. Memory is requested one node at a time.
// =============================================================================

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "NodeType.h"

class LinkedList {
public:
    // -------------------------------------------------------------------------
    // [A] LIFECYCLE
    // -------------------------------------------------------------------------
    LinkedList();     // [A.1] start with an empty chain
    ~LinkedList();    // [A.2] walk the chain and delete EVERY node

    // -------------------------------------------------------------------------
    // [B] THE SIX REQUIRED OPERATIONS - same names as the array version
    // -------------------------------------------------------------------------
    void loadFromCSV(const char* filename);   // [B.1] file  -> list of nodes
    void categorizeByAgeGroup();              // [B.2] demographic + billing summary
    double calculateCost(Patient patient);    // [B.3] the billing formula
    void sortBy(const char* criteria);        // [B.4] insertion sort by RELINKING
    void searchBy(const char* criteria);      // [B.5] linear vs early-exit scan
    void displayTable();                      // [B.6] formatted console table

    // -------------------------------------------------------------------------
    // [C] SMALL PUBLIC EXTRA
    // -------------------------------------------------------------------------
    int count() const;    // [C.1] lets main() ask "is anything loaded yet?"

private:
    // -------------------------------------------------------------------------
    // [D] THE DATA STRUCTURE ITSELF - three variables are the whole list
    // -------------------------------------------------------------------------
    NodeType* head;   // [D.1] first node; 0 means the list is empty
    NodeType* tail;   // [D.2] last node; kept so append() is O(1) not O(n)
    int size;         // [D.3] running count, so count() need not walk the chain

    // WHY tail EXISTS: without it, adding a patient would mean walking all the
    // way to the end first. With 600 records that turns loading from O(n) into
    // O(n^2). Storing the last address is a cheap, standard optimisation.

    // -------------------------------------------------------------------------
    // [E] PRIVATE HELPERS
    // -------------------------------------------------------------------------
    void append(const Patient& record);          // [E.1] link a new node at the end
    bool isSortedBy(const char* criteria) const; // [E.2] guard for the sorted scan
};

#endif
