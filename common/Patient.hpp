#ifndef DSTR_PATIENT_HPP
#define DSTR_PATIENT_HPP

#include <cstddef>
#include <cstdint>

namespace healthcare {

constexpr int FACILITY_COUNT = 3;
constexpr int AGE_GROUP_COUNT = 5;
constexpr int CARE_TYPE_COUNT = 6;

enum class SortKey { Id, Age, Cost, Duration, CareType };
enum class SortAlgorithm { Insertion, Merge };
enum class Relation { Equal, InclusiveRange, Greater, GreaterEqual };

struct Patient {
    char PatientID[32]{};
    int Age = 0;
    char CareType[32]{};
    double VisitDurationHours = 0.0;
    double BaseCostPerHour = 0.0;
    int DaysVisitsPerYear = 0;
    int Facility = 0;
};

struct Query {
    SortKey key = SortKey::Id;
    Relation relation = Relation::Equal;
    double lower = 0.0;
    double upper = 0.0;
    char text[32]{};
};

struct OperationCounts {
    std::uint64_t comparisons = 0;
    std::uint64_t movements = 0;
    std::size_t auxiliaryBytes = 0;
};

struct SearchResult {
    std::size_t matches = 0;
    std::uint64_t comparisons = 0;
    std::uint64_t checksum = 0;
    std::size_t auxiliaryBytes = 0;
};

double costOf(const Patient& patient);
int ageGroupOf(int age);
int careTypeOf(const char* careType);
const char* ageGroupName(int group);
const char* careTypeName(int careType);
const char* facilityName(int facility);
const char* sortKeyName(SortKey key);
const char* algorithmName(SortAlgorithm algorithm);
int compareRecords(const Patient& left, const Patient& right, SortKey key);
int compareRecordToQuery(const Patient& patient, const Query& query);
std::uint64_t recordFingerprint(const Patient& patient);
bool parseSortKey(const char* text, SortKey& key);
bool parseQuery(const char* text, Query& query, char* error, std::size_t errorSize);
bool parseInteger(const char* text, int minimum, int maximum, int& value);
bool parsePatient(const char* line, int facility, Patient& patient, char* error, std::size_t errorSize);

}

#endif
