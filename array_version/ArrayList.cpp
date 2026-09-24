#include "ArrayList.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <chrono>

namespace {

const int FIELD_MAX = 64;
const int COLUMNS = 6;

void trimCopy(char* dest, int destSize, const char* src, int length) {
    while (length > 0 && (*src == ' ' || *src == '\t')) {
        src++;
        length--;
    }
    while (length > 0) {
        char last = src[length - 1];
        if (last == ' ' || last == '\t' || last == '\r' || last == '\n') {
            length--;
        } else {
            break;
        }
    }
    int copied = length < destSize - 1 ? length : destSize - 1;
    for (int i = 0; i < copied; i++) {
        dest[i] = src[i];
    }
    dest[copied] = '\0';
}

void toLower(char* text) {
    for (char* c = text; *c != '\0'; c++) {
        *c = static_cast<char>(std::tolower(static_cast<unsigned char>(*c)));
    }
}

void toUpper(char* text) {
    for (char* c = text; *c != '\0'; c++) {
        *c = static_cast<char>(std::toupper(static_cast<unsigned char>(*c)));
    }
}

bool equalsIgnoreCase(const char* a, const char* b) {
    while (*a != '\0' && *b != '\0') {
        if (std::tolower(static_cast<unsigned char>(*a)) != std::tolower(static_cast<unsigned char>(*b))) {
            return false;
        }
        a++;
        b++;
    }
    return *a == *b;
}

bool splitRow(const char* line, char fields[COLUMNS][FIELD_MAX]) {
    int index = 0;
    const char* start = line;
    for (const char* cursor = line; index < COLUMNS; cursor++) {
        if (*cursor == ',' || *cursor == '\0') {
            trimCopy(fields[index], FIELD_MAX, start, static_cast<int>(cursor - start));
            index++;
            if (*cursor == '\0') {
                break;
            }
            start = cursor + 1;
        }
    }
    return index == COLUMNS;
}

// CSV column order: PatientID,Age,CareType,LengthOfStay,BaseCostPerHour,DaysVisitsPerYear
bool parseRow(const char* line, Patient& record) {
    char fields[COLUMNS][FIELD_MAX];
    if (!splitRow(line, fields)) {
        return false;
    }
    trimCopy(record.PatientID, sizeof(record.PatientID), fields[0], static_cast<int>(std::strlen(fields[0])));
    record.Age = std::atoi(fields[1]);
    trimCopy(record.CareType, sizeof(record.CareType), fields[2], static_cast<int>(std::strlen(fields[2])));
    record.VisitDurationHours = std::atof(fields[3]);
    record.BaseCostPerHour = std::atof(fields[4]);
    record.DaysVisitsPerYear = std::atoi(fields[5]);
    return record.PatientID[0] != '\0';
}

double costOf(const Patient& record) {
    return record.VisitDurationHours * record.BaseCostPerHour * record.DaysVisitsPerYear;
}

const char* ageGroupOf(int age) {
    if (age <= 17) {
        return "Child";
    }
    if (age <= 35) {
        return "Young Adult";
    }
    if (age <= 59) {
        return "Adult";
    }
    return "Senior";
}

bool validSortCriteria(const char* criteria) {
    return std::strcmp(criteria, "id") == 0
        || std::strcmp(criteria, "age") == 0
        || std::strcmp(criteria, "cost") == 0
        || std::strcmp(criteria, "duration") == 0
        || std::strcmp(criteria, "caretype") == 0;
}

bool lessThan(const Patient& a, const Patient& b, const char* criteria) {
    if (std::strcmp(criteria, "age") == 0) {
        return a.Age < b.Age;
    }
    if (std::strcmp(criteria, "cost") == 0) {
        return costOf(a) < costOf(b);
    }
    if (std::strcmp(criteria, "duration") == 0) {
        return a.VisitDurationHours < b.VisitDurationHours;
    }
    if (std::strcmp(criteria, "caretype") == 0) {
        return std::strcmp(a.CareType, b.CareType) < 0;
    }
    return std::strcmp(a.PatientID, b.PatientID) < 0;
}

bool matches(const Patient& record, const char* field, const char* value) {
    if (std::strcmp(field, "id") == 0) {
        return std::strcmp(record.PatientID, value) == 0;
    }
    if (std::strcmp(field, "age") == 0) {
        return record.Age == std::atoi(value);
    }
    if (std::strcmp(field, "caretype") == 0) {
        return equalsIgnoreCase(record.CareType, value);
    }
    return false;
}

bool splitCriteria(const char* criteria, char* field, int fieldSize, char* value, int valueSize) {
    const char* separator = std::strchr(criteria, '=');
    if (separator == 0) {
        return false;
    }
    trimCopy(field, fieldSize, criteria, static_cast<int>(separator - criteria));
    trimCopy(value, valueSize, separator + 1, static_cast<int>(std::strlen(separator + 1)));
    toLower(field);
    return field[0] != '\0' && value[0] != '\0';
}

void printSeparator() {
    std::cout << "+------+------------+-----+-------------+------------------+--------+---------+--------+-------------+\n";
}

void printTableHeader() {
    printSeparator();
    std::cout << "| " << std::left << std::setw(4) << "No"
              << " | " << std::setw(10) << "PatientID"
              << " | " << std::right << std::setw(3) << "Age"
              << " | " << std::left << std::setw(11) << "AgeGroup"
              << " | " << std::setw(16) << "CareType"
              << " | " << std::right << std::setw(6) << "Hours"
              << " | " << std::setw(7) << "Rate"
              << " | " << std::setw(6) << "Visits"
              << " | " << std::setw(11) << "TotalCost"
              << " |\n";
    printSeparator();
}

void printPatientRow(int number, const Patient& record) {
    std::cout << "| " << std::left << std::setw(4) << number
              << " | " << std::setw(10) << record.PatientID
              << " | " << std::right << std::setw(3) << record.Age
              << " | " << std::left << std::setw(11) << ageGroupOf(record.Age)
              << " | " << std::setw(16) << record.CareType
              << " | " << std::right << std::fixed << std::setprecision(1) << std::setw(6) << record.VisitDurationHours
              << " | " << std::setprecision(2) << std::setw(7) << record.BaseCostPerHour
              << " | " << std::setw(6) << record.DaysVisitsPerYear
              << " | " << std::setw(11) << costOf(record)
              << " |\n";
}

double elapsedMs(const std::chrono::high_resolution_clock::time_point& start,
                 const std::chrono::high_resolution_clock::time_point& stop) {
    return std::chrono::duration<double, std::milli>(stop - start).count();
}

}  // namespace

ArrayList::ArrayList() {
    capacity = 64;
    size = 0;
    patients = new Patient[capacity];
}

ArrayList::~ArrayList() {
    delete[] patients;
    patients = 0;
    size = 0;
    capacity = 0;
}

int ArrayList::count() const {
    return size;
}

void ArrayList::grow() {
    int newCapacity = capacity * 2;
    Patient* bigger = new Patient[newCapacity];
    for (int i = 0; i < size; i++) {
        bigger[i] = patients[i];
    }
    delete[] patients;
    patients = bigger;
    capacity = newCapacity;
}

void ArrayList::append(const Patient& record) {
    if (size == capacity) {
        grow();
    }
    patients[size] = record;
    size++;
}

void ArrayList::loadFromCSV(const char* filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "Could not open " << filename << "\n";
        return;
    }

    char line[512];
    file.getline(line, sizeof(line));  // discard header row

    int loaded = 0;
    int skipped = 0;
    while (file.getline(line, sizeof(line))) {
        if (line[0] == '\0' || line[0] == '\r') {
            continue;
        }
        Patient record;
        if (parseRow(line, record)) {
            append(record);
            loaded++;
        } else {
            skipped++;
        }
    }

    std::cout << "Loaded " << loaded << " records from " << filename;
    if (skipped > 0) {
        std::cout << " (skipped " << skipped << " malformed)";
    }
    std::cout << ". Total in list: " << size << "\n";
}

double ArrayList::calculateCost(Patient patient) {
    return patient.VisitDurationHours * patient.BaseCostPerHour * patient.DaysVisitsPerYear;
}

void ArrayList::categorizeByAgeGroup() {
    if (size == 0) {
        std::cout << "No records loaded yet.\n";
        return;
    }

    const char* groups[4] = { "Child", "Young Adult", "Adult", "Senior" };
    int counts[4] = { 0, 0, 0, 0 };
    double totals[4] = { 0.0, 0.0, 0.0, 0.0 };
    int lowest[4] = { -1, -1, -1, -1 };
    int highest[4] = { -1, -1, -1, -1 };
    double grandTotal = 0.0;

    for (int i = 0; i < size; i++) {
        const char* group = ageGroupOf(patients[i].Age);
        int index = 0;
        for (int g = 0; g < 4; g++) {
            if (std::strcmp(group, groups[g]) == 0) {
                index = g;
            }
        }
        counts[index]++;
        double cost = calculateCost(patients[i]);
        totals[index] += cost;
        grandTotal += cost;
        if (lowest[index] < 0 || patients[i].Age < lowest[index]) {
            lowest[index] = patients[i].Age;
        }
        if (highest[index] < 0 || patients[i].Age > highest[index]) {
            highest[index] = patients[i].Age;
        }
    }

    std::cout << "\nDemographic and billing categorization (" << size << " patients)\n";
    std::cout << "+-------------+-------+--------+-----------+---------------+---------------+\n";
    std::cout << "| " << std::left << std::setw(11) << "AgeGroup"
              << " | " << std::right << std::setw(5) << "Count"
              << " | " << std::setw(6) << "Share"
              << " | " << std::setw(9) << "AgeRange"
              << " | " << std::setw(13) << "TotalCost"
              << " | " << std::setw(13) << "AvgCost"
              << " |\n";
    std::cout << "+-------------+-------+--------+-----------+---------------+---------------+\n";

    for (int g = 0; g < 4; g++) {
        double share = size > 0 ? (counts[g] * 100.0) / size : 0.0;
        double average = counts[g] > 0 ? totals[g] / counts[g] : 0.0;
        char range[16];
        if (counts[g] > 0) {
            std::snprintf(range, sizeof(range), "%d-%d", lowest[g], highest[g]);
        } else {
            std::snprintf(range, sizeof(range), "-");
        }
        std::cout << "| " << std::left << std::setw(11) << groups[g]
                  << " | " << std::right << std::setw(5) << counts[g]
                  << " | " << std::fixed << std::setprecision(1) << std::setw(5) << share << "%"
                  << " | " << std::setw(9) << range
                  << " | " << std::setprecision(2) << std::setw(13) << totals[g]
                  << " | " << std::setw(13) << average
                  << " |\n";
    }

    std::cout << "+-------------+-------+--------+-----------+---------------+---------------+\n";
    std::cout << "Total expenditure: " << std::fixed << std::setprecision(2) << grandTotal
              << "   Average per patient: " << (grandTotal / size) << "\n";
}

bool ArrayList::isSortedBy(const char* criteria) const {
    for (int i = 1; i < size; i++) {
        if (lessThan(patients[i], patients[i - 1], criteria)) {
            return false;
        }
    }
    return true;
}

void ArrayList::sortBy(const char* criteria) {
    char field[FIELD_MAX];
    trimCopy(field, sizeof(field), criteria, static_cast<int>(std::strlen(criteria)));
    toLower(field);

    if (!validSortCriteria(field)) {
        std::cout << "Sort criteria must be one of: id, age, cost, duration, caretype\n";
        return;
    }
    if (size < 2) {
        std::cout << "Need at least two records to sort.\n";
        return;
    }

    long long comparisons = 0;
    long long shifts = 0;
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    for (int i = 1; i < size; i++) {
        Patient current = patients[i];
        int j = i - 1;
        while (j >= 0) {
            comparisons++;
            if (lessThan(current, patients[j], field)) {
                patients[j + 1] = patients[j];
                shifts++;
                j--;
            } else {
                break;
            }
        }
        patients[j + 1] = current;
    }

    std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();

    std::cout << "Insertion sort by " << field << " on array: " << size << " records, "
              << comparisons << " comparisons, " << shifts << " shifts, "
              << std::fixed << std::setprecision(3) << elapsedMs(start, stop) << " ms\n";
}

void ArrayList::searchBy(const char* criteria) {
    char field[FIELD_MAX];
    char value[FIELD_MAX];
    if (!splitCriteria(criteria, field, sizeof(field), value, sizeof(value))) {
        std::cout << "Use field=value, for example id=PT1001, age=42 or caretype=Emergency\n";
        return;
    }
    if (std::strcmp(field, "id") != 0 && std::strcmp(field, "age") != 0 && std::strcmp(field, "caretype") != 0) {
        std::cout << "Searchable fields: id, age, caretype\n";
        return;
    }
    if (size == 0) {
        std::cout << "No records loaded yet.\n";
        return;
    }
    if (std::strcmp(field, "id") == 0) {
        toUpper(value);
    }

    long long linearComparisons = 0;
    int found = 0;
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < size; i++) {
        linearComparisons++;
        if (matches(patients[i], field, value)) {
            found++;
        }
    }
    std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();

    if (found == 0) {
        std::cout << "No patient matched " << field << "=" << value << "\n";
    } else {
        printTableHeader();
        int shown = 0;
        for (int i = 0; i < size; i++) {
            if (matches(patients[i], field, value)) {
                shown++;
                printPatientRow(shown, patients[i]);
            }
        }
        printSeparator();
    }

    std::cout << "Unsorted linear search: " << found << " match(es), "
              << linearComparisons << " comparisons, "
              << std::fixed << std::setprecision(3) << elapsedMs(start, stop) << " ms\n";

    bool binarySearchable = std::strcmp(field, "id") == 0 || std::strcmp(field, "age") == 0;
    if (!binarySearchable) {
        std::cout << "Binary search needs an ordered key; caretype has many duplicates.\n";
        return;
    }
    if (!isSortedBy(field)) {
        std::cout << "Sort by " << field << " first to compare binary search on sorted data.\n";
        return;
    }

    long long binaryComparisons = 0;
    int hit = -1;
    int low = 0;
    int high = size - 1;
    int targetAge = std::atoi(value);
    start = std::chrono::high_resolution_clock::now();
    while (low <= high) {
        int mid = low + (high - low) / 2;
        binaryComparisons++;
        int order;
        if (std::strcmp(field, "age") == 0) {
            order = patients[mid].Age < targetAge ? -1 : (patients[mid].Age > targetAge ? 1 : 0);
        } else {
            int raw = std::strcmp(patients[mid].PatientID, value);
            order = raw < 0 ? -1 : (raw > 0 ? 1 : 0);
        }
        if (order == 0) {
            hit = mid;
            break;
        }
        if (order < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    stop = std::chrono::high_resolution_clock::now();

    std::cout << "Sorted binary search: " << (hit >= 0 ? "found" : "not found") << ", "
              << binaryComparisons << " comparisons, "
              << std::fixed << std::setprecision(3) << elapsedMs(start, stop) << " ms\n";
}

void ArrayList::displayTable() {
    if (size == 0) {
        std::cout << "No records loaded yet.\n";
        return;
    }

    printTableHeader();
    double total = 0.0;
    for (int i = 0; i < size; i++) {
        printPatientRow(i + 1, patients[i]);
        total += calculateCost(patients[i]);
    }
    printSeparator();
    std::cout << size << " records, total cost " << std::fixed << std::setprecision(2) << total << "\n";
}
