#include "Patient.h"

Patient::Patient() {
    PatientID[0] = '\0';
    Age = 0;
    CareType[0] = '\0';
    VisitDurationHours = 0.0;
    BaseCostPerHour = 0.0;
    DaysVisitsPerYear = 0;
}
