#ifndef PATIENT_H
#define PATIENT_H

// CSV column LengthOfStay maps to VisitDurationHours.
struct Patient {
    char PatientID[32];
    int Age;
    char CareType[32];
    double VisitDurationHours;
    double BaseCostPerHour;
    int DaysVisitsPerYear;

    Patient();
};

#endif
