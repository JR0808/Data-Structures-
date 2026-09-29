// =============================================================================
// FILE: Patient.h   (ARRAY VERSION)
// ROLE: Defines the record that one patient occupies.
//
// TALKING POINT: This is the "what" of the program - the data. ArrayList.h is
// the "where" - the container. Keeping them in separate files means the same
// Patient definition is reused unchanged by the linked list version.
//
// WHY char arrays instead of std::string:
//   The assignment forbids STL containers. A fixed char array stores its text
//   inside the struct itself, so every Patient is the same fixed size. That is
//   what lets us hold them in one contiguous block of memory (the array).
// =============================================================================

#ifndef PATIENT_H
#define PATIENT_H

// -----------------------------------------------------------------------------
// [1] THE PATIENT RECORD
// -----------------------------------------------------------------------------
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
// NOTE FOR THE DEMO: the CSV header says "LengthOfStay" but our field is called
// "VisitDurationHours". Same value, clearer name. The translation happens in
// one place only: parseRow() inside ArrayList.cpp.
// -----------------------------------------------------------------------------
struct Patient {
    char PatientID[32];          // e.g. "PT1001"
    int Age;                     // years, used for age-group categorisation
    char CareType[32];           // e.g. "Emergency", "Routine Checkup"
    double VisitDurationHours;   // hours per visit  (CSV: LengthOfStay)
    double BaseCostPerHour;      // charge rate per hour
    int DaysVisitsPerYear;       // how many visits in a year

    // [1.1] Default constructor - guarantees a blank Patient is never garbage.
    //       Important because `new Patient[capacity]` creates many at once.
    Patient();
};

#endif
