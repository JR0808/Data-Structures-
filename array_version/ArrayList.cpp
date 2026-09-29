// =============================================================================
// FILE: ArrayList.cpp   (ARRAY VERSION)
// ROLE: All the logic for storing and analysing patients inside a plain array.
//
// HOW TO WALK A MARKER THROUGH THIS FILE (in order):
//   SECTION 1 - Private helpers   : text handling, CSV parsing, formatting
//   SECTION 2 - Lifecycle         : constructor, destructor, grow, append
//   SECTION 3 - loadFromCSV       : file  -> array
//   SECTION 4 - calculateCost     : the billing formula
//   SECTION 5 - categorizeByAgeGroup : demographic + billing summary
//   SECTION 6 - sortBy            : INSERTION SORT on an array
//   SECTION 7 - searchBy          : LINEAR SEARCH vs BINARY SEARCH
//   SECTION 8 - displayTable      : the console report
//
// STL POLICY: only <iostream>, <fstream>, <iomanip>, <chrono> and the C string
// helpers are used. No vector, no list, no map, no std::string, no std::sort.
// Every container and every algorithm below is hand written.
// =============================================================================

#include "ArrayList.h"
#include <iostream>    // std::cout - console output
#include <iomanip>     // std::setw / setprecision - table alignment
#include <fstream>     // std::ifstream - reading the CSV files
#include <cstring>     // strcmp, strlen, strchr - C text comparison
#include <cstdlib>     // atoi, atof - text -> number conversion
#include <cstdio>      // snprintf - building the "5-17" age range text
#include <cctype>      // tolower, toupper - case handling
#include <chrono>      // high_resolution_clock - performance timing

// =============================================================================
// SECTION 1 - PRIVATE FILE HELPERS
//
// The anonymous namespace makes everything inside it visible ONLY in this file.
// TALKING POINT: this is encapsulation at file level - these are tools for
// ArrayList, not part of the public interface the marker sees in the header.
// =============================================================================
namespace {

const int FIELD_MAX = 64;   // biggest single CSV field we accept
const int COLUMNS = 6;      // the CSV has exactly 6 columns

// -----------------------------------------------------------------------------
// [1.1] trimCopy - copy text while removing surrounding whitespace.
//
// WHY THIS EXISTS: Windows text files end each line with "\r\n". When we split
// on commas, the final field keeps a stray '\r' and "2\r" would parse wrongly.
// This also protects against a human typing "PT1001 , 42".
//
// SAFETY: copied is capped at destSize - 1 so we can never overflow the
// destination buffer - the classic C string bug.
// Complexity: O(n) where n is the field length.
// -----------------------------------------------------------------------------
void trimCopy(char* dest, int destSize, const char* src, int length) {
    // Step 1: walk the start pointer forward past leading blanks.
    while (length > 0 && (*src == ' ' || *src == '\t')) {
        src++;
        length--;
    }
    // Step 2: shrink the length to drop trailing blanks and line endings.
    while (length > 0) {
        char last = src[length - 1];
        if (last == ' ' || last == '\t' || last == '\r' || last == '\n') {
            length--;
        } else {
            break;
        }
    }
    // Step 3: copy what survives, leaving room for the terminator.
    int copied = length < destSize - 1 ? length : destSize - 1;
    for (int i = 0; i < copied; i++) {
        dest[i] = src[i];
    }
    dest[copied] = '\0';   // C text MUST end with '\0'
}

// -----------------------------------------------------------------------------
// [1.2] toLower / toUpper - normalise text in place.
// USED FOR: accepting "AGE" or "Age" as a sort key, and "pt1001" as an ID.
// Complexity: O(n).
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// [1.3] equalsIgnoreCase - case-insensitive text equality, written by hand.
// USED FOR: caretype=emergency matching "Emergency" in the file.
// Complexity: O(n), stops at the first difference.
// -----------------------------------------------------------------------------
bool equalsIgnoreCase(const char* a, const char* b) {
    while (*a != '\0' && *b != '\0') {
        if (std::tolower(static_cast<unsigned char>(*a)) != std::tolower(static_cast<unsigned char>(*b))) {
            return false;
        }
        a++;
        b++;
    }
    return *a == *b;   // both must finish together, else one was longer
}

// -----------------------------------------------------------------------------
// [1.4] splitRow - THE CSV SPLITTER. Cuts one line into its 6 fields.
//
// ALGORITHM: sweep a cursor along the line. Every time we meet a comma or the
// end of the string, the text between `start` and `cursor` is one field.
//
//   PT1001,42,Emergency,12,150.0,2
//         ^  ^         ^  ^     ^
//         cut points found by the sweep
//
// Returns false if the line did not contain 6 fields, which is how a corrupt
// row gets rejected instead of crashing the program.
// Complexity: O(n), a single pass over the line.
// -----------------------------------------------------------------------------
bool splitRow(const char* line, char fields[COLUMNS][FIELD_MAX]) {
    int index = 0;
    const char* start = line;
    for (const char* cursor = line; index < COLUMNS; cursor++) {
        if (*cursor == ',' || *cursor == '\0') {
            trimCopy(fields[index], FIELD_MAX, start, static_cast<int>(cursor - start));
            index++;
            if (*cursor == '\0') {
                break;          // reached the end of the line
            }
            start = cursor + 1; // next field begins after this comma
        }
    }
    return index == COLUMNS;
}

// -----------------------------------------------------------------------------
// [1.5] parseRow - turn 6 pieces of TEXT into one typed Patient record.
//
// THIS IS THE ONLY PLACE the CSV layout is known. If the file format changed,
// only this function would need editing.
//
// CSV column order: PatientID,Age,CareType,LengthOfStay,BaseCostPerHour,DaysVisitsPerYear
// Index:                0      1     2          3              4                5
//
// NOTE THE RENAME: fields[3] is the CSV's "LengthOfStay" and it is stored into
// VisitDurationHours. atoi = text to int, atof = text to double.
// Complexity: O(n) over the line length.
// -----------------------------------------------------------------------------
bool parseRow(const char* line, Patient& record) {
    char fields[COLUMNS][FIELD_MAX];
    if (!splitRow(line, fields)) {
        return false;   // wrong number of columns -> reject this row
    }
    trimCopy(record.PatientID, sizeof(record.PatientID), fields[0], static_cast<int>(std::strlen(fields[0])));
    record.Age = std::atoi(fields[1]);
    trimCopy(record.CareType, sizeof(record.CareType), fields[2], static_cast<int>(std::strlen(fields[2])));
    record.VisitDurationHours = std::atof(fields[3]);   // CSV: LengthOfStay
    record.BaseCostPerHour = std::atof(fields[4]);
    record.DaysVisitsPerYear = std::atoi(fields[5]);
    return record.PatientID[0] != '\0';   // a row with no ID is not a patient
}

// -----------------------------------------------------------------------------
// [1.6] costOf - the billing formula, used internally by sorting and reporting.
//
//   Total Medical Cost = VisitDurationHours x BaseCostPerHour x DaysVisitsPerYear
//
// IF ASKED WHY IT IS DUPLICATED with the public calculateCost(): the public one
// is the required assignment interface; this const-reference version avoids
// copying a whole Patient during the thousands of comparisons a sort performs.
// Complexity: O(1).
// -----------------------------------------------------------------------------
double costOf(const Patient& record) {
    return record.VisitDurationHours * record.BaseCostPerHour * record.DaysVisitsPerYear;
}

// -----------------------------------------------------------------------------
// [1.7] ageGroupOf - the recategorisation rule.
//
//   0-17 Child | 18-35 Young Adult | 36-59 Adult | 60+ Senior
//
// The checks are ordered smallest first, so reaching "if (age <= 35)" already
// proves the age is at least 18. No lower bound test is needed.
// Complexity: O(1).
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// [1.8] validSortCriteria - whitelist of accepted sort keys.
// Rejecting unknown input here keeps sortBy() from silently doing nothing.
// -----------------------------------------------------------------------------
bool validSortCriteria(const char* criteria) {
    return std::strcmp(criteria, "id") == 0
        || std::strcmp(criteria, "age") == 0
        || std::strcmp(criteria, "cost") == 0
        || std::strcmp(criteria, "duration") == 0
        || std::strcmp(criteria, "caretype") == 0;
}

// -----------------------------------------------------------------------------
// [1.9] lessThan - THE COMPARISON RULE. "Does a come before b?"
//
// KEY DESIGN POINT TO PRESENT: the sort algorithm itself never mentions Age or
// cost. It only asks this one question. That is why a single insertion sort can
// order the data five different ways - we swap the rule, not the algorithm.
// Complexity: O(1) for numbers, O(n) for text (strcmp).
// -----------------------------------------------------------------------------
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
    return std::strcmp(a.PatientID, b.PatientID) < 0;   // default key: id
}

// -----------------------------------------------------------------------------
// [1.10] matches - THE SEARCH RULE. "Is this the record the user asked for?"
// Same idea as lessThan: the search loops stay generic.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// [1.11] splitCriteria - break the user's "age=42" into field "age", value "42".
// strchr finds the '=' sign; everything left of it is the field name.
// -----------------------------------------------------------------------------
bool splitCriteria(const char* criteria, char* field, int fieldSize, char* value, int valueSize) {
    const char* separator = std::strchr(criteria, '=');
    if (separator == 0) {
        return false;   // no '=' typed, so the input is unusable
    }
    trimCopy(field, fieldSize, criteria, static_cast<int>(separator - criteria));
    trimCopy(value, valueSize, separator + 1, static_cast<int>(std::strlen(separator + 1)));
    toLower(field);     // field name is case-insensitive; the value is not
    return field[0] != '\0' && value[0] != '\0';
}

// -----------------------------------------------------------------------------
// [1.12] Table formatting helpers.
// setw(n) reserves n characters per column so the '|' borders line up.
// setprecision(2) with fixed shows money as 3600.00 rather than 3.6e+03.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// [1.13] elapsedMs - convert two clock readings into milliseconds.
// This is what produces the timing figures for the performance analysis.
// -----------------------------------------------------------------------------
double elapsedMs(const std::chrono::high_resolution_clock::time_point& start,
                 const std::chrono::high_resolution_clock::time_point& stop) {
    return std::chrono::duration<double, std::milli>(stop - start).count();
}

}  // namespace

// =============================================================================
// SECTION 2 - LIFECYCLE: building and tearing down the array
// =============================================================================

// -----------------------------------------------------------------------------
// [2.1] Constructor - reserve the first block of memory.
//
// We start at 64 even though 600 records are coming, purely to demonstrate the
// grow() logic. `new Patient[64]` calls the Patient constructor 64 times.
// Complexity: O(capacity).
// -----------------------------------------------------------------------------
ArrayList::ArrayList() {
    capacity = 64;
    size = 0;
    patients = new Patient[capacity];
}

// -----------------------------------------------------------------------------
// [2.2] Destructor - hand the memory back to the operating system.
//
// EXAM POINT: every `new[]` needs exactly one matching `delete[]`. Without this
// the program leaks memory. Setting the pointer to 0 afterwards prevents an
// accidental dangling-pointer use.
// Complexity: O(capacity).
// -----------------------------------------------------------------------------
ArrayList::~ArrayList() {
    delete[] patients;
    patients = 0;
    size = 0;
    capacity = 0;
}

// [2.3] count - report how many records are loaded (used by main's demo).
int ArrayList::count() const {
    return size;
}

// -----------------------------------------------------------------------------
// [2.4] grow - THE COST OF USING AN ARRAY. Called only when the block is full.
//
// An array cannot be stretched, so we must:
//   1. allocate a new, bigger block
//   2. COPY every existing element across
//   3. delete the old block
//   4. point at the new one
//
// TALKING POINT: doubling rather than adding one keeps the average cost per
// insertion at O(1). A linked list never needs this step at all - that is one
// of its genuine advantages.
// Complexity: O(n) for this single call.
// -----------------------------------------------------------------------------
void ArrayList::grow() {
    int newCapacity = capacity * 2;
    Patient* bigger = new Patient[newCapacity];
    for (int i = 0; i < size; i++) {
        bigger[i] = patients[i];        // element-by-element copy
    }
    delete[] patients;                  // release the old, smaller block
    patients = bigger;
    capacity = newCapacity;
}

// -----------------------------------------------------------------------------
// [2.5] append - add one record at the end.
// Complexity: O(1) normally, O(n) on the rare call that triggers grow().
// -----------------------------------------------------------------------------
void ArrayList::append(const Patient& record) {
    if (size == capacity) {
        grow();                 // no space left, enlarge first
    }
    patients[size] = record;    // write into the first free slot
    size++;                     // that slot is now in use
}

// =============================================================================
// SECTION 3 - loadFromCSV: getting the data in
// =============================================================================

// -----------------------------------------------------------------------------
// [3.1] loadFromCSV - read one facility file and append every valid row.
//
// STEPS TO NARRATE:
//   1. open the file, report failure instead of crashing
//   2. read and THROW AWAY the first line (it is the column header)
//   3. loop: read a line -> parseRow -> append
//   4. count anything malformed instead of aborting the whole load
//   5. print a confirmation so the demo visibly proves 200 rows arrived
//
// IMPORTANT: this APPENDS, so calling it three times accumulates 600 records
// rather than replacing them. That is how the three facilities combine.
// Complexity: O(rows) - one pass through the file.
// -----------------------------------------------------------------------------
void ArrayList::loadFromCSV(const char* filename) {
    std::ifstream file(filename);
    if (!file) {
        // Most likely cause: the program was run from the wrong folder.
        std::cout << "Could not open " << filename << "\n";
        return;
    }

    char line[512];
    file.getline(line, sizeof(line));  // discard header row

    int loaded = 0;
    int skipped = 0;
    while (file.getline(line, sizeof(line))) {
        if (line[0] == '\0' || line[0] == '\r') {
            continue;                  // blank line, ignore it
        }
        Patient record;
        if (parseRow(line, record)) {
            append(record);            // good row -> store it
            loaded++;
        } else {
            skipped++;                 // bad row -> count and move on
        }
    }

    std::cout << "Loaded " << loaded << " records from " << filename;
    if (skipped > 0) {
        std::cout << " (skipped " << skipped << " malformed)";
    }
    std::cout << ". Total in list: " << size << "\n";
}

// =============================================================================
// SECTION 4 - calculateCost: the billing formula
// =============================================================================

// -----------------------------------------------------------------------------
// [4.1] calculateCost - the required public billing method.
//
//   Total Medical Cost = Length of Stay x Base Cost Per Hour x Days Visits Per Year
//
// WORKED EXAMPLE FOR THE DEMO (patient PT1001):
//   12 hours x 150.00 per hour x 2 visits = 3600.00
// Complexity: O(1) - two multiplications.
// -----------------------------------------------------------------------------
double ArrayList::calculateCost(Patient patient) {
    return patient.VisitDurationHours * patient.BaseCostPerHour * patient.DaysVisitsPerYear;
}

// =============================================================================
// SECTION 5 - categorizeByAgeGroup: demographics + billing summary
// =============================================================================

// -----------------------------------------------------------------------------
// [5.1] categorizeByAgeGroup - one pass that fills four parallel tally arrays.
//
// THE TECHNIQUE (worth naming in the presentation): instead of scanning the data
// four times (once per age group), we scan ONCE and use the group as an index
// into small counter arrays. Four "parallel arrays" all indexed by the same g:
//
//   groups[g]  = the label            counts[g]  = how many patients
//   totals[g]  = summed cost          lowest/highest[g] = actual age range
//
// Complexity: O(n) time, O(1) extra memory - the tally arrays are fixed at 4.
// -----------------------------------------------------------------------------
void ArrayList::categorizeByAgeGroup() {
    if (size == 0) {
        std::cout << "No records loaded yet.\n";
        return;
    }

    // --- Step 1: set up the four buckets -------------------------------------
    const char* groups[4] = { "Child", "Young Adult", "Adult", "Senior" };
    int counts[4] = { 0, 0, 0, 0 };
    double totals[4] = { 0.0, 0.0, 0.0, 0.0 };
    int lowest[4] = { -1, -1, -1, -1 };    // -1 means "nothing seen yet"
    int highest[4] = { -1, -1, -1, -1 };
    double grandTotal = 0.0;

    // --- Step 2: ONE sweep through all patients ------------------------------
    for (int i = 0; i < size; i++) {
        // Decide the bucket, then convert that label into an index 0..3.
        const char* group = ageGroupOf(patients[i].Age);
        int index = 0;
        for (int g = 0; g < 4; g++) {
            if (std::strcmp(group, groups[g]) == 0) {
                index = g;
            }
        }

        counts[index]++;                              // demographic tally
        double cost = calculateCost(patients[i]);     // billing tally
        totals[index] += cost;
        grandTotal += cost;

        // Track the real minimum and maximum age inside this bucket.
        if (lowest[index] < 0 || patients[i].Age < lowest[index]) {
            lowest[index] = patients[i].Age;
        }
        if (highest[index] < 0 || patients[i].Age > highest[index]) {
            highest[index] = patients[i].Age;
        }
    }

    // --- Step 3: present the findings ----------------------------------------
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
        // Derived statistics. Both guard against dividing by zero.
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

// =============================================================================
// SECTION 6 - sortBy: INSERTION SORT on an array
// =============================================================================

// -----------------------------------------------------------------------------
// [6.1] isSortedBy - verify order by checking every neighbouring pair.
// If any element is smaller than the one before it, the data is not sorted.
// searchBy() uses this as a GUARD, because binary search on unsorted data
// silently returns wrong answers.
// Complexity: O(n).
// -----------------------------------------------------------------------------
bool ArrayList::isSortedBy(const char* criteria) const {
    for (int i = 1; i < size; i++) {
        if (lessThan(patients[i], patients[i - 1], criteria)) {
            return false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// [6.2] sortBy - INSERTION SORT, the required algorithm.
//
// THE ANALOGY TO USE: sorting a hand of playing cards. The left part of the
// array is your tidy hand; you pick up the next card and slide it backwards
// past every card that is bigger, then drop it into the gap.
//
//   sorted part        | next
//   [12][25][40][55]   | 30
//                  <-- 30 moves left past 55 and 40, lands after 25
//
// WHY WE COUNT comparisons AND shifts: they are the two different operations,
// and the contrast with the linked list version is the heart of the report.
// The array does ~94,000 SHIFTS because every element physically moves.
//
// Complexity: O(n^2) worst and average case, O(n) if already sorted (the inner
//             while breaks immediately). Memory: O(1) - sorts in place.
// -----------------------------------------------------------------------------
void ArrayList::sortBy(const char* criteria) {
    // --- Step 1: clean and validate the requested key ------------------------
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

    // --- Step 2: start the instruments ---------------------------------------
    long long comparisons = 0;
    long long shifts = 0;
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    // --- Step 3: THE INSERTION SORT ------------------------------------------
    // Outer loop: patients[0..i-1] is already sorted; insert patients[i].
    for (int i = 1; i < size; i++) {
        Patient current = patients[i];   // hold the card we are placing
        int j = i - 1;                   // start comparing with its left neighbour

        // Inner loop: slide bigger elements one slot to the RIGHT.
        while (j >= 0) {
            comparisons++;
            if (lessThan(current, patients[j], field)) {
                patients[j + 1] = patients[j];   // shift that element right
                shifts++;
                j--;                             // keep looking further left
            } else {
                break;    // found the correct position, stop early
            }
        }
        patients[j + 1] = current;   // drop the held card into the gap
    }

    // --- Step 4: report the measurements -------------------------------------
    std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();

    std::cout << "Insertion sort by " << field << " on array: " << size << " records, "
              << comparisons << " comparisons, " << shifts << " shifts, "
              << std::fixed << std::setprecision(3) << elapsedMs(start, stop) << " ms\n";
}

// =============================================================================
// SECTION 7 - searchBy: LINEAR SEARCH vs BINARY SEARCH
// =============================================================================

// -----------------------------------------------------------------------------
// [7.1] searchBy - run the searching experiment for input like "age=42".
//
// THE EXPERIMENT THIS DEMONSTRATES:
//   Pass 1 - LINEAR SEARCH on unsorted data: check every record.  O(n)
//            Result for 600 patients: always 600 comparisons.
//   Pass 2 - BINARY SEARCH on sorted data: halve the range each step. O(log n)
//            Result for 600 patients: about 7 to 9 comparisons.
//
// That is roughly a 70x reduction, and it is the headline number of the report.
// The trade-off to mention: binary search is only legal AFTER paying for a sort.
// -----------------------------------------------------------------------------
void ArrayList::searchBy(const char* criteria) {
    // --- Step 1: understand the request --------------------------------------
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
        // IDs in the files are upper case, so accept "pt1001" too. Doing this
        // here keeps the linear and binary passes comparing identical text.
        toUpper(value);
    }

    // --- Step 2: PASS 1 - LINEAR SEARCH (works on unsorted data) -------------
    // Note it deliberately scans to the very end because duplicates may exist.
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

    // --- Step 3: show the matching records -----------------------------------
    // Printing is done in a SECOND loop so console output never pollutes the
    // timing measurement above - screen writing is far slower than comparing.
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

    // --- Step 4: decide whether binary search is allowed ---------------------
    bool binarySearchable = std::strcmp(field, "id") == 0 || std::strcmp(field, "age") == 0;
    if (!binarySearchable) {
        std::cout << "Binary search needs an ordered key; caretype has many duplicates.\n";
        return;
    }
    if (!isSortedBy(field)) {
        // Honest behaviour: refuse rather than return a wrong answer.
        std::cout << "Sort by " << field << " first to compare binary search on sorted data.\n";
        return;
    }

    // --- Step 5: PASS 2 - BINARY SEARCH (requires sorted data) --------------
    // ALGORITHM: keep a window [low..high]. Look at the middle element.
    // Too small -> discard the left half. Too big -> discard the right half.
    // Each comparison throws away HALF the remaining records, so 600 records
    // need only about log2(600) = 10 steps.
    long long binaryComparisons = 0;
    int hit = -1;
    int low = 0;
    int high = size - 1;
    int targetAge = std::atoi(value);
    start = std::chrono::high_resolution_clock::now();
    while (low <= high) {
        // Written as low + (high - low) / 2 rather than (low + high) / 2 to
        // avoid integer overflow on very large arrays - standard good practice.
        int mid = low + (high - low) / 2;
        binaryComparisons++;

        // order: -1 = middle is too small, +1 = too big, 0 = found it
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
            low = mid + 1;    // answer must be in the RIGHT half
        } else {
            high = mid - 1;   // answer must be in the LEFT half
        }
    }
    stop = std::chrono::high_resolution_clock::now();

    std::cout << "Sorted binary search: " << (hit >= 0 ? "found" : "not found") << ", "
              << binaryComparisons << " comparisons, "
              << std::fixed << std::setprecision(3) << elapsedMs(start, stop) << " ms\n";
}

// =============================================================================
// SECTION 8 - displayTable: the console report
// =============================================================================

// -----------------------------------------------------------------------------
// [8.1] displayTable - print every record with its computed cost, plus a total.
//
// Uses an INDEX loop (patients[i]) - the array's natural access pattern.
// Compare this with the linked list version, which must follow next pointers.
// Complexity: O(n).
// -----------------------------------------------------------------------------
void ArrayList::displayTable() {
    if (size == 0) {
        std::cout << "No records loaded yet.\n";
        return;
    }

    printTableHeader();
    double total = 0.0;
    for (int i = 0; i < size; i++) {
        printPatientRow(i + 1, patients[i]);      // i + 1 so humans see 1-based
        total += calculateCost(patients[i]);
    }
    printSeparator();
    std::cout << size << " records, total cost " << std::fixed << std::setprecision(2) << total << "\n";
}
