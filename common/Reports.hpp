#ifndef DSTR_REPORTS_HPP
#define DSTR_REPORTS_HPP

#include "Patient.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace healthcare {

struct Aggregate {
    std::size_t count = 0;
    double cost = 0.0;
    double hours = 0.0;

    void add(const Patient& patient) {
        ++count;
        cost += costOf(patient);
        hours += patient.VisitDurationHours;
        if (!std::isfinite(cost) || !std::isfinite(hours)) throw std::overflow_error("Summary exceeds numeric capacity");
    }
};

struct ReportData {
    Aggregate facilities[FACILITY_COUNT + 1]{};
    Aggregate ages[FACILITY_COUNT + 1][AGE_GROUP_COUNT]{};
    Aggregate care[FACILITY_COUNT + 1][CARE_TYPE_COUNT]{};
    Aggregate details[FACILITY_COUNT + 1][AGE_GROUP_COUNT][CARE_TYPE_COUNT]{};
};

template <typename Container>
ReportData summarize(const Container& patients) {
    ReportData report;
    patients.forEach([&](const Patient& patient) {
        const int group = ageGroupOf(patient.Age);
        const int care = careTypeOf(patient.CareType);
        const int scopes[] = {0, patient.Facility + 1};
        for (const int scope : scopes) {
            report.facilities[scope].add(patient);
            report.ages[scope][group].add(patient);
            report.care[scope][care].add(patient);
            report.details[scope][group][care].add(patient);
        }
    });
    return report;
}

inline void printAverage(double total, std::size_t count, int width) {
    if (count == 0) {
        std::cout << std::setw(width) << "n/a";
        return;
    }
    std::cout << std::setw(width) << total / static_cast<double>(count);
}

inline void printPatientHeader() {
    std::cout << "| Facility | PatientID   | Age | AgeBand | CareType         | Hours   | RateMYR  | Visits | CostMYR       |\n";
}

inline void printPatient(const Patient& patient) {
    std::cout << std::fixed << std::setprecision(2)
              << "| " << std::left << std::setw(8) << facilityName(patient.Facility)
              << " | " << std::setw(11) << patient.PatientID
              << " | " << std::right << std::setw(3) << patient.Age
              << " | " << std::setw(7) << ageGroupName(ageGroupOf(patient.Age))
              << " | " << std::left << std::setw(16) << patient.CareType
              << " | " << std::right << std::setw(7) << patient.VisitDurationHours
              << " | " << std::setw(8) << patient.BaseCostPerHour
              << " | " << std::setw(6) << patient.DaysVisitsPerYear
              << " | " << std::setw(13) << costOf(patient) << " |\n";
}

template <typename Container>
void displayPatients(const Container& patients, int facility = -1) {
    if (patients.count() == 0) {
        std::cout << "No records loaded yet.\n";
        return;
    }
    printPatientHeader();
    Aggregate total;
    patients.forEach([&](const Patient& patient) {
        if (facility >= 0 && patient.Facility != facility) return;
        printPatient(patient);
        total.add(patient);
    });
    std::cout << "Records: " << total.count << " | Total cost (MYR): " << std::fixed << std::setprecision(2) << total.cost << '\n';
}

inline void printFacilitySummary(const ReportData& report) {
    std::cout << "\nFACILITY SUMMARY\n";
    std::cout << "| Facility | Patients | TotalCostMYR  | AvgCostMYR    | TotalHours    | MeanHours     |\n";
    for (int scope = 0; scope <= FACILITY_COUNT; ++scope) {
        const Aggregate& summary = report.facilities[scope];
        std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                  << " | " << std::right << std::setw(8) << summary.count
                  << " | " << std::setw(13) << summary.cost << " | ";
        printAverage(summary.cost, summary.count, 13);
        std::cout << " | " << std::setw(13) << summary.hours << " | ";
        printAverage(summary.hours, summary.count, 13);
        std::cout << " |\n";
    }
}

inline void printAgeSummary(const ReportData& report) {
    std::cout << "\nAGE GROUP SUMMARY\n";
    std::cout << "| Facility | AgeBand | Patients | TotalCostMYR  | AvgCostMYR    | TotalHours    | MeanHours     |\n";
    for (int scope = 0; scope <= FACILITY_COUNT; ++scope) {
        for (int age = 0; age < AGE_GROUP_COUNT; ++age) {
            const Aggregate& summary = report.ages[scope][age];
            std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                      << " | " << std::setw(7) << ageGroupName(age)
                      << " | " << std::right << std::setw(8) << summary.count
                      << " | " << std::setw(13) << summary.cost << " | ";
            printAverage(summary.cost, summary.count, 13);
            std::cout << " | " << std::setw(13) << summary.hours << " | ";
            printAverage(summary.hours, summary.count, 13);
            std::cout << " |\n";
        }
    }
}

inline void printCareSummary(const ReportData& report) {
    std::cout << "\nCARE TYPE SUMMARY\n";
    std::cout << "| Facility | CareType         | Patients | TotalCostMYR  | AvgCostMYR    | MeanHours     |\n";
    for (int scope = 0; scope <= FACILITY_COUNT; ++scope) {
        for (int care = 0; care < CARE_TYPE_COUNT; ++care) {
            const Aggregate& summary = report.care[scope][care];
            std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                      << " | " << std::setw(16) << careTypeName(care)
                      << " | " << std::right << std::setw(8) << summary.count
                      << " | " << std::setw(13) << summary.cost << " | ";
            printAverage(summary.cost, summary.count, 13);
            std::cout << " | ";
            printAverage(summary.hours, summary.count, 13);
            std::cout << " |\n";
        }
    }
}

inline void printAgeCareDetails(const ReportData& report) {
    std::cout << "\nCARE TYPE WITHIN AGE GROUP\n";
    std::cout << "| Facility | AgeBand | CareType         | Patients | TotalCostMYR  | AvgCostMYR    |\n";
    for (int scope = 0; scope <= FACILITY_COUNT; ++scope) {
        for (int age = 0; age < AGE_GROUP_COUNT; ++age) {
            for (int care = 0; care < CARE_TYPE_COUNT; ++care) {
                const Aggregate& summary = report.details[scope][age][care];
                if (summary.count == 0) continue;
                std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                          << " | " << std::setw(7) << ageGroupName(age)
                          << " | " << std::setw(16) << careTypeName(care)
                          << " | " << std::right << std::setw(8) << summary.count
                          << " | " << std::setw(13) << summary.cost << " | ";
                printAverage(summary.cost, summary.count, 13);
                std::cout << " |\n";
            }
        }
    }
}

inline void printPreferredCare(const ReportData& report) {
    std::cout << "\nMOST REQUESTED CARE BY AGE GROUP\n";
    std::cout << "Patient-record counts define demand; each tied care type gets a row.\n";
    std::cout << "| Facility | AgeBand | CareType         | Patients |\n";
    for (int scope = 0; scope <= FACILITY_COUNT; ++scope) {
        for (int age = 0; age < AGE_GROUP_COUNT; ++age) {
            std::size_t maximum = 0;
            for (const Aggregate& care : report.details[scope][age]) {
                if (care.count <= maximum) continue;
                maximum = care.count;
            }
            if (maximum == 0) {
                std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                          << " | " << std::setw(7) << ageGroupName(age)
                          << " | " << std::setw(16) << "n/a"
                          << " | " << std::right << std::setw(8) << 0 << " |\n";
                continue;
            }
            for (int care = 0; care < CARE_TYPE_COUNT; ++care) {
                if (report.details[scope][age][care].count != maximum) continue;
                std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                          << " | " << std::setw(7) << ageGroupName(age)
                          << " | " << std::setw(16) << careTypeName(care)
                          << " | " << std::right << std::setw(8) << maximum << " |\n";
            }
        }
    }
}

inline void printLeaders(const ReportData& report) {
    std::cout << "\nHIGHEST BILLING AGE GROUPS\n";
    std::cout << "| Facility | AgeBand | TotalCostMYR  |\n";
    for (int scope = 0; scope <= FACILITY_COUNT; ++scope) {
        if (report.facilities[scope].count == 0) continue;
        double maximum = 0.0;
        for (const Aggregate& age : report.ages[scope]) {
            if (age.cost <= maximum) continue;
            maximum = age.cost;
        }
        for (int age = 0; age < AGE_GROUP_COUNT; ++age) {
            const Aggregate& group = report.ages[scope][age];
            if (group.count == 0 || std::fabs(group.cost - maximum) > 0.000001) continue;
            std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                      << " | " << std::setw(7) << ageGroupName(age)
                      << " | " << std::right << std::setw(13) << group.cost << " |\n";
        }
    }
    std::cout << "\nHIGHEST PATIENT TRAFFIC\n";
    std::cout << "| Facility | CareType         | Patients |\n";
    for (int scope = 0; scope <= FACILITY_COUNT; ++scope) {
        std::size_t maximum = 0;
        for (const Aggregate& care : report.care[scope]) {
            if (care.count <= maximum) continue;
            maximum = care.count;
        }
        if (maximum == 0) continue;
        for (int care = 0; care < CARE_TYPE_COUNT; ++care) {
            if (report.care[scope][care].count != maximum) continue;
            std::cout << "| " << std::left << std::setw(8) << facilityName(scope - 1)
                      << " | " << std::setw(16) << careTypeName(care)
                      << " | " << std::right << std::setw(8) << maximum << " |\n";
        }
    }
}

template <typename Container>
void displayAnalysis(const Container& patients) {
    if (patients.count() == 0) {
        std::cout << "No records loaded yet.\n";
        return;
    }
    const ReportData report = summarize(patients);
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n" << Container::structureName() << " healthcare analysis\n";
    printFacilitySummary(report);
    printAgeSummary(report);
    printCareSummary(report);
    printAgeCareDetails(report);
    printPreferredCare(report);
    printLeaders(report);
    std::cout << "\nAge bands: 0-17 Pediatrics/Adolescents; 18-25 Young Adults/Students;\n"
              << "26-45 Working Adults (Early Career); 46-60 Working Adults (Late Career); 61-100 Seniors.\n"
              << "Cost (MYR) = hours per visit x MYR/hour x DaysVisitsPerYear.\n"
              << "Duration means hours per recorded visit; traffic means patient-record count.\n"
              << "These data do not measure waiting time, staffing capacity, or bed occupancy.\n";
}

template <typename Container>
void displayMatches(const Container& patients, const Query& query) {
    std::size_t count = 0;
    printPatientHeader();
    patients.forEach([&](const Patient& patient) {
        if (compareRecordToQuery(patient, query) != 0) return;
        printPatient(patient);
        ++count;
    });
    std::cout << "Matched records: " << count << '\n';
}

}

#endif
