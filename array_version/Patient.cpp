// =============================================================================
// FILE: Patient.cpp   (ARRAY VERSION)
// ROLE: Implements the Patient default constructor.
// =============================================================================

#include "Patient.h"

// -----------------------------------------------------------------------------
// [1.1] Patient::Patient - zero out a fresh record.
//
// TALKING POINT: C++ does NOT clear memory for you. Without this constructor a
// new Patient would contain whatever bytes were previously at that address, so
// an unloaded slot could print random ages or costs. Setting PatientID[0] to
// '\0' makes the text an empty string immediately.
//
// Complexity: O(1) - a fixed number of assignments.
// -----------------------------------------------------------------------------
Patient::Patient() {
    PatientID[0] = '\0';         // empty string marker
    Age = 0;
    CareType[0] = '\0';          // empty string marker
    VisitDurationHours = 0.0;
    BaseCostPerHour = 0.0;
    DaysVisitsPerYear = 0;
}
