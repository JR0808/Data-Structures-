// =============================================================================
// FILE: Patient.h   (LINKED LIST VERSION)
// ROLE: Defines the record that one patient occupies.
//
// TALKING POINT: this file is IDENTICAL to the array version's Patient.h. The
// data never changed - only the container around it did. That is the cleanest
// possible demonstration that the two programs differ purely in data structure.
// =============================================================================

#ifndef PATIENT_H
#define PATIENT_H

// -----------------------------------------------------------------------------
// [1] THE PATIENT RECORD
//
// Field mapping from the CSV file:
//
//   CSV column          ->  struct field           Type
//   ------------------      --------------------   -------
//   PatientID           ->  PatientID              text
//   Age                 ->  Age                    whole number
//   CareType            ->  CareType               text
//   LengthOfStay        ->  VisitDurationHours     decimal   <-- NAME CHANGES
//   BaseCostPerHour     ->  BaseCostPerHour        decimal
//   DaysVisitsPerYear   ->  DaysVisitsPerYear      whole number
//
// In this version a Patient is stored INSIDE a NodeType (see NodeType.h), which
// adds a `next` pointer so records can be chained together.
// -----------------------------------------------------------------------------
struct Patient {
    char PatientID[32];          // e.g. "PT1001"
    int Age;                     // years, used for age-group categorisation
    char CareType[32];           // e.g. "Emergency", "Routine Checkup"
    double VisitDurationHours;   // hours per visit  (CSV: LengthOfStay)
    double BaseCostPerHour;      // charge rate per hour
    int DaysVisitsPerYear;       // how many visits in a year

    // [1.1] Default constructor - guarantees a blank Patient is never garbage.
    Patient();
};

#endif
