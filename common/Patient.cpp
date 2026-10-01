#include "Patient.hpp"
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <limits>

namespace healthcare {
namespace {

const char* const AGE_NAMES[AGE_GROUP_COUNT] = {
    "0-17", "18-25", "26-45", "46-60", "61-100"
};
const char* const CARE_NAMES[CARE_TYPE_COUNT] = {
    "Emergency", "Inpatient", "Outpatient", "Vaccination", "Rehabilitation", "Routine Checkup"
};

bool reject(char* error, std::size_t size, const char* message) {
    std::snprintf(error, size, "%s", message);
    return false;
}

bool copyTrimmed(char* destination, std::size_t capacity, const char* begin, std::size_t length) {
    while (length > 0 && std::isspace(static_cast<unsigned char>(*begin))) {
        ++begin;
        --length;
    }
    while (length > 0 && std::isspace(static_cast<unsigned char>(begin[length - 1]))) --length;
    if (length >= capacity) return false;
    std::memcpy(destination, begin, length);
    destination[length] = '\0';
    return true;
}

int compareText(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0') {
        const int first = std::tolower(static_cast<unsigned char>(*left));
        const int second = std::tolower(static_cast<unsigned char>(*right));
        if (first != second) return first < second ? -1 : 1;
        ++left;
        ++right;
    }
    if (*left == *right) return 0;
    return *left == '\0' ? -1 : 1;
}

bool parseDecimal(const char* text, double& value) {
    if (*text == '\0') return false;
    errno = 0;
    char* end = nullptr;
    const double parsed = std::strtod(text, &end);
    if (errno == ERANGE || end == text || *end != '\0' || !std::isfinite(parsed) || parsed < 0) return false;
    value = parsed;
    return true;
}

double numericValue(const Patient& patient, SortKey key) {
    switch (key) {
        case SortKey::Age: return patient.Age;
        case SortKey::Cost: return costOf(patient);
        case SortKey::Duration: return patient.VisitDurationHours;
        default: throw std::logic_error("Numeric value requested for a text field");
    }
}

bool splitFields(const char* line, char (&fields)[6][64]) {
    const char* start = line;
    for (std::size_t column = 0; column < 6; ++column) {
        const char* comma = std::strchr(start, ',');
        if (column < 5 && comma == nullptr) return false;
        if (column == 5 && comma != nullptr) return false;
        const std::size_t length = comma == nullptr ? std::strlen(start) : static_cast<std::size_t>(comma - start);
        if (!copyTrimmed(fields[column], sizeof(fields[column]), start, length)) return false;
        if (comma == nullptr) break;
        start = comma + 1;
    }
    return true;
}

bool parseAgeRange(const char* text, Query& query) {
    const char* dash = std::strchr(text, '-');
    if (dash == nullptr) return false;
    char first[32]{};
    char last[32]{};
    if (!copyTrimmed(first, sizeof(first), text, static_cast<std::size_t>(dash - text))) return false;
    if (!copyTrimmed(last, sizeof(last), dash + 1, std::strlen(dash + 1))) return false;
    int minimum = 0;
    int maximum = 0;
    if (!parseInteger(first, 0, 100, minimum) || !parseInteger(last, 0, 100, maximum) || minimum > maximum) return false;
    query.relation = Relation::InclusiveRange;
    query.lower = minimum;
    query.upper = maximum;
    return true;
}

}

double costOf(const Patient& patient) {
    const double cost = patient.VisitDurationHours * patient.BaseCostPerHour * patient.DaysVisitsPerYear;
    if (!std::isfinite(cost) || cost > std::numeric_limits<double>::max() / 100.0) return cost;
    return std::round(cost * 100.0) / 100.0;
}

int ageGroupOf(int age) {
    if (age < 0 || age > 100) throw std::out_of_range("Patient age must be between 0 and 100");
    if (age <= 17) return 0;
    if (age <= 25) return 1;
    if (age <= 45) return 2;
    if (age <= 60) return 3;
    return 4;
}

int careTypeOf(const char* careType) {
    for (int index = 0; index < CARE_TYPE_COUNT; ++index) {
        if (compareText(careType, CARE_NAMES[index]) != 0) continue;
        return index;
    }
    throw std::invalid_argument("Unknown care type");
}

const char* ageGroupName(int group) {
    return AGE_NAMES[group];
}

const char* careTypeName(int careType) {
    return CARE_NAMES[careType];
}

const char* facilityName(int facility) {
    if (facility == -1) return "All";
    if (facility == 0) return "A";
    if (facility == 1) return "B";
    if (facility == 2) return "C";
    throw std::out_of_range("Invalid facility");
}

const char* sortKeyName(SortKey key) {
    switch (key) {
        case SortKey::Id: return "id";
        case SortKey::Age: return "age";
        case SortKey::Cost: return "cost";
        case SortKey::Duration: return "duration";
        case SortKey::CareType: return "caretype";
    }
    throw std::logic_error("Invalid sort key");
}

const char* algorithmName(SortAlgorithm algorithm) {
    return algorithm == SortAlgorithm::Insertion ? "Insertion" : "Merge";
}

int compareRecords(const Patient& left, const Patient& right, SortKey key) {
    if (key == SortKey::Id) return compareText(left.PatientID, right.PatientID);
    if (key == SortKey::CareType) return compareText(left.CareType, right.CareType);
    const double first = numericValue(left, key);
    const double second = numericValue(right, key);
    if (first == second) return 0;
    return first < second ? -1 : 1;
}

int compareRecordToQuery(const Patient& patient, const Query& query) {
    if (query.key == SortKey::Id) return compareText(patient.PatientID, query.text);
    if (query.key == SortKey::CareType) return compareText(patient.CareType, query.text);
    const double value = numericValue(patient, query.key);
    if (query.relation == Relation::Greater) return value <= query.lower ? -1 : 0;
    if (query.relation == Relation::GreaterEqual) return value < query.lower ? -1 : 0;
    if (value < query.lower) return -1;
    if (value > query.upper) return 1;
    return 0;
}

std::uint64_t recordFingerprint(const Patient& patient) {
    std::uint64_t fingerprint = 14695981039346656037ULL;
    for (const char* character = patient.PatientID; *character != '\0'; ++character) {
        fingerprint ^= static_cast<unsigned char>(*character);
        fingerprint *= 1099511628211ULL;
    }
    return fingerprint;
}

bool parseSortKey(const char* text, SortKey& key) {
    char field[32]{};
    if (!copyTrimmed(field, sizeof(field), text, std::strlen(text))) return false;
    const SortKey keys[] = {SortKey::Id, SortKey::Age, SortKey::Cost, SortKey::Duration, SortKey::CareType};
    for (const SortKey candidate : keys) {
        if (compareText(field, sortKeyName(candidate)) != 0) continue;
        key = candidate;
        return true;
    }
    return false;
}

bool parseInteger(const char* text, int minimum, int maximum, int& value) {
    if (*text == '\0') return false;
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || parsed < minimum || parsed > maximum) return false;
    value = static_cast<int>(parsed);
    return true;
}

bool parseQuery(const char* text, Query& query, char* error, std::size_t errorSize) {
    const char* operation = std::strpbrk(text, "=>");
    if (operation == nullptr) return reject(error, errorSize, "Use field=value, age=61-100, or duration>24.");
    char field[32]{};
    char value[96]{};
    if (!copyTrimmed(field, sizeof(field), text, static_cast<std::size_t>(operation - text))) return reject(error, errorSize, "Field is too long.");
    Query parsed;
    if (!parseSortKey(field, parsed.key)) return reject(error, errorSize, "Fields: id, age, caretype, duration, cost.");
    const char* valueStart = operation + 1;
    if (*operation == '>') {
        parsed.relation = Relation::Greater;
        if (*valueStart == '=') {
            parsed.relation = Relation::GreaterEqual;
            ++valueStart;
        }
    }
    if (!copyTrimmed(value, sizeof(value), valueStart, std::strlen(valueStart)) || *value == '\0') return reject(error, errorSize, "Provide a complete query value.");
    if (parsed.key == SortKey::Id || parsed.key == SortKey::CareType) {
        if (parsed.relation != Relation::Equal) return reject(error, errorSize, "Text searches support equality only.");
        if (!copyTrimmed(parsed.text, sizeof(parsed.text), value, std::strlen(value))) return reject(error, errorSize, "Search text is too long.");
        query = parsed;
        return true;
    }
    if (parsed.key == SortKey::Age && parsed.relation == Relation::Equal && std::strchr(value, '-') != nullptr) {
        if (!parseAgeRange(value, parsed)) return reject(error, errorSize, "Age range must be integers from 0 to 100, lower first.");
        query = parsed;
        return true;
    }
    if (parsed.key == SortKey::Age) {
        int age = 0;
        if (!parseInteger(value, 0, 100, age)) return reject(error, errorSize, "Age must be a complete integer from 0 to 100.");
        parsed.lower = age;
        parsed.upper = age;
        query = parsed;
        return true;
    }
    if (!parseDecimal(value, parsed.lower)) return reject(error, errorSize, "Use a finite, non-negative number with no trailing text.");
    parsed.upper = parsed.lower;
    query = parsed;
    return true;
}

bool parsePatient(const char* line, int facility, Patient& patient, char* error, std::size_t errorSize) {
    char fields[6][64]{};
    if (!splitFields(line, fields)) return reject(error, errorSize, "Expected exactly six bounded CSV fields.");
    Patient parsed;
    parsed.Facility = facility;
    if (std::strlen(fields[0]) < 3 || std::strlen(fields[0]) >= sizeof(parsed.PatientID)) return reject(error, errorSize, "Invalid PatientID length.");
    if (fields[0][0] != 'P' || fields[0][1] != 'T') return reject(error, errorSize, "PatientID must start with PT and contain digits.");
    for (const char* digit = fields[0] + 2; *digit != '\0'; ++digit) {
        if (std::isdigit(static_cast<unsigned char>(*digit))) continue;
        return reject(error, errorSize, "PatientID suffix must contain digits only.");
    }
    std::strcpy(parsed.PatientID, fields[0]);
    if (!parseInteger(fields[1], 0, 100, parsed.Age)) return reject(error, errorSize, "Age must be an integer between 0 and 100.");
    int careIndex = -1;
    for (int index = 0; index < CARE_TYPE_COUNT; ++index) {
        if (compareText(fields[2], CARE_NAMES[index]) != 0) continue;
        careIndex = index;
        break;
    }
    if (careIndex < 0) return reject(error, errorSize, "Unknown CareType.");
    std::strcpy(parsed.CareType, CARE_NAMES[careIndex]);
    if (!parseDecimal(fields[3], parsed.VisitDurationHours)) return reject(error, errorSize, "Invalid LengthOfStay.");
    if (!parseDecimal(fields[4], parsed.BaseCostPerHour)) return reject(error, errorSize, "Invalid BaseCostPerHour.");
    if (!parseInteger(fields[5], 0, INT_MAX, parsed.DaysVisitsPerYear)) return reject(error, errorSize, "Invalid DaysVisitsPerYear.");
    if (!std::isfinite(costOf(parsed))) return reject(error, errorSize, "Calculated cost overflows the numeric range.");
    patient = parsed;
    return true;
}

}
