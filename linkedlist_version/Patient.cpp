// =============================================================================
// FILE: Patient.cpp   (LINKED LIST VERSION)
// ROLE: Implements the Patient default constructor.
// =============================================================================

#include "Patient.h"

// -----------------------------------------------------------------------------
// [1.1] Patient::Patient - zero out a fresh record.
//
// TALKING POINT: C++ does NOT clear memory for you. Every time append() runs
// `new NodeType`, the Patient inside it is blanked by this constructor before
// the real CSV values are copied in.
//
// Complexity: O(1).
// -----------------------------------------------------------------------------
Patient::Patient() {
    PatientID[0] = '\0';         // empty string marker
    Age = 0;
    CareType[0] = '\0';          // empty string marker
    VisitDurationHours = 0.0;
    BaseCostPerHour = 0.0;
    DaysVisitsPerYear = 0;
}
