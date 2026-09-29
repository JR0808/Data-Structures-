// =============================================================================
// FILE: ArrayList.h   (ARRAY VERSION)
// ROLE: Declares the container class that stores many Patient records in one
//       contiguous, dynamically grown array.
//
// THE CORE IDEA TO EXPLAIN:
//   All 600 patients sit side by side in one block of memory. Because every
//   Patient is the same size, the computer can jump straight to record number
//   i by arithmetic:  address = start + (i * sizeof(Patient)).
//   That single property is called RANDOM ACCESS and it is the reason the
//   array version can use BINARY SEARCH while the linked list cannot.
//
// COST OF THAT BENEFIT:
//   Inserting or removing in the middle forces every later element to shift
//   along, which is why our insertion sort reports ~94,000 shifts.
// =============================================================================

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include "Patient.h"

class ArrayList {
public:
    // -------------------------------------------------------------------------
    // [A] LIFECYCLE - creating and destroying the array
    // -------------------------------------------------------------------------
    ArrayList();     // [A.1] allocates the starting block
    ~ArrayList();    // [A.2] releases it again (prevents a memory leak)

    // -------------------------------------------------------------------------
    // [B] THE SIX REQUIRED OPERATIONS
    // -------------------------------------------------------------------------
    void loadFromCSV(const char* filename);   // [B.1] file  -> array
    void categorizeByAgeGroup();              // [B.2] demographic + billing summary
    double calculateCost(Patient patient);    // [B.3] the billing formula
    void sortBy(const char* criteria);        // [B.4] insertion sort
    void searchBy(const char* criteria);      // [B.5] linear search vs binary search
    void displayTable();                      // [B.6] formatted console table

    // -------------------------------------------------------------------------
    // [C] SMALL PUBLIC EXTRA
    // -------------------------------------------------------------------------
    int count() const;    // [C.1] lets main() ask "is anything loaded yet?"

private:
    // -------------------------------------------------------------------------
    // [D] THE DATA STRUCTURE ITSELF - three variables are the whole array list
    // -------------------------------------------------------------------------
    Patient* patients;   // [D.1] pointer to the first element of the block
    int size;            // [D.2] how many slots are actually USED
    int capacity;        // [D.3] how many slots EXIST before we must grow

    // NOTE: size vs capacity is a classic exam question. capacity is the size
    // of the rented room; size is how many people are currently inside it.

    // -------------------------------------------------------------------------
    // [E] PRIVATE HELPERS - internal plumbing, hidden from main()
    // -------------------------------------------------------------------------
    void grow();                                 // [E.1] double the block when full
    void append(const Patient& record);          // [E.2] add one record at the end
    bool isSortedBy(const char* criteria) const; // [E.3] guard for binary search
};

#endif
