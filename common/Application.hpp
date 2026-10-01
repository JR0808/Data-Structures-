#ifndef DSTR_APPLICATION_HPP
#define DSTR_APPLICATION_HPP

#include "Reports.hpp"
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>

namespace healthcare {

constexpr int BENCHMARK_TRIALS = 9;
constexpr std::uint64_t SEARCH_BATCH_SIZE = 256;
constexpr std::size_t PATH_CAPACITY = 1024;
const char* const DATASET_NAMES[FACILITY_COUNT] = {
    "dataset1 facility_a.csv", "dataset2 facility_b.csv", "dataset3_facility_c.csv"
};
const char* const CSV_HEADER = "PatientID,Age,CareType,LengthOfStay,BaseCostPerHour,DaysVisitsPerYear";

inline void trimInput(char* text) {
    char* start = text;
    while (std::isspace(static_cast<unsigned char>(*start))) ++start;
    std::size_t length = std::strlen(start);
    while (length > 0 && std::isspace(static_cast<unsigned char>(start[length - 1]))) --length;
    std::memmove(text, start, length);
    text[length] = '\0';
}

inline void buildPath(char* output, std::size_t capacity, const char* directory, const char* name) {
    const int length = std::snprintf(output, capacity, "%s/%s", directory, name);
    if (length < 0 || static_cast<std::size_t>(length) >= capacity) throw std::runtime_error("Dataset path is too long.");
}

inline void loadError(const char* path, std::size_t line, const char* detail) {
    char message[PATH_CAPACITY + 256]{};
    std::snprintf(message, sizeof(message), "%s, line %zu: %s", path, line, detail);
    throw std::runtime_error(message);
}

inline bool readCsvLine(std::ifstream& input, char* line, std::size_t capacity, const char* path, std::size_t number) {
    input.getline(line, static_cast<std::streamsize>(capacity));
    if (input.bad()) loadError(path, number, "Unable to read the file.");
    if (input.eof() && input.gcount() == 0) return false;
    if (input.fail()) loadError(path, number, "CSV line exceeds the supported length.");
    trimInput(line);
    return true;
}

template <typename Container>
void loadAll(Container& records, const char* directory) {
    Container replacement;
    for (int facility = 0; facility < FACILITY_COUNT; ++facility) {
        char path[PATH_CAPACITY]{};
        buildPath(path, sizeof(path), directory, DATASET_NAMES[facility]);
        std::ifstream input(path);
        if (!input.is_open()) loadError(path, 0, "Cannot open dataset. Previous records were retained.");
        char line[1024]{};
        if (!readCsvLine(input, line, sizeof(line), path, 1)) loadError(path, 1, "Missing CSV header.");
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(line);
        if (std::strlen(line) >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
            std::memmove(line, line + 3, std::strlen(line + 3) + 1);
            trimInput(line);
        }
        if (std::strcmp(line, CSV_HEADER) != 0) loadError(path, 1, "Unexpected CSV header or column order.");
        std::size_t number = 1;
        while (readCsvLine(input, line, sizeof(line), path, ++number)) {
            if (*line == '\0') continue;
            Patient patient;
            char error[192]{};
            if (!parsePatient(line, facility, patient, error, sizeof(error))) loadError(path, number, error);
            if (replacement.containsId(patient.PatientID)) loadError(path, number, "Duplicate PatientID across the input datasets.");
            replacement.append(patient);
        }
    }
    static_cast<void>(summarize(replacement));
    records.swap(replacement);
}

inline bool hasDatasets(const char* directory) {
    for (const char* name : DATASET_NAMES) {
        char path[PATH_CAPACITY]{};
        buildPath(path, sizeof(path), directory, name);
        std::ifstream input(path);
        if (!input.is_open()) return false;
    }
    return true;
}

inline bool parentDirectory(char* path) {
    char* lastSeparator = nullptr;
    for (char* cursor = path; *cursor != '\0'; ++cursor) {
        if (*cursor != '/' && *cursor != '\\') continue;
        lastSeparator = cursor;
    }
    if (lastSeparator == nullptr) return false;
    *lastSeparator = '\0';
    return *path != '\0';
}

inline void chooseDirectory(char* directory, std::size_t capacity, const char* executable, const char* supplied) {
    if (supplied != nullptr) {
        const int length = std::snprintf(directory, capacity, "%s", supplied);
        if (length < 0 || static_cast<std::size_t>(length) >= capacity) throw std::runtime_error("Dataset directory is too long.");
        return;
    }
    const char* const candidates[] = {".", ".."};
    for (const char* candidate : candidates) {
        if (!hasDatasets(candidate)) continue;
        std::snprintf(directory, capacity, "%s", candidate);
        return;
    }
    char executableDirectory[PATH_CAPACITY]{};
    const int length = std::snprintf(executableDirectory, sizeof(executableDirectory), "%s", executable);
    if (length >= 0 && static_cast<std::size_t>(length) < sizeof(executableDirectory)) {
        for (int level = 0; level < 2; ++level) {
            if (!parentDirectory(executableDirectory)) break;
            if (!hasDatasets(executableDirectory)) continue;
            std::snprintf(directory, capacity, "%s", executableDirectory);
            return;
        }
    }
    std::snprintf(directory, capacity, ".");
}

inline bool readInput(const char* prompt, char* value, std::size_t capacity) {
    for (;;) {
        std::cout << prompt << std::flush;
        if (std::cin.getline(value, static_cast<std::streamsize>(capacity))) {
            trimInput(value);
            return true;
        }
        if (std::cin.eof()) return false;
        if (std::cin.bad()) throw std::runtime_error("Console input failed.");
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Input is too long. Please try again.\n";
    }
}

inline bool readChoice(const char* prompt, int minimum, int maximum, int& choice) {
    char input[128]{};
    while (readInput(prompt, input, sizeof(input))) {
        if (parseInteger(input, minimum, maximum, choice)) return true;
        std::cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
    return false;
}

inline bool readScope(int& facility) {
    int choice = 0;
    if (!readChoice("Scope (0 = all, 1 = A, 2 = B, 3 = C): ", 0, 3, choice)) return false;
    facility = choice - 1;
    return true;
}

struct Timing {
    double median = 0;
    double minimum = 0;
    double maximum = 0;
};

inline Timing summarizeTimes(double (&samples)[BENCHMARK_TRIALS]) {
    for (int index = 1; index < BENCHMARK_TRIALS; ++index) {
        const double held = samples[index];
        int position = index;
        while (position > 0 && samples[position - 1] > held) {
            samples[position] = samples[position - 1];
            --position;
        }
        samples[position] = held;
    }
    return {samples[BENCHMARK_TRIALS / 2], samples[0], samples[BENCHMARK_TRIALS - 1]};
}

inline double elapsedMicroseconds(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point finish) {
    return std::chrono::duration<double, std::micro>(finish - start).count();
}

inline void printTimingHeader(bool sorting) {
    std::cout << std::left << std::setw(25) << "Operation" << std::right
              << std::setw(15) << "Median us" << std::setw(15) << "Min us" << std::setw(15) << "Max us"
              << std::setw(15) << "Comparisons" << std::setw(15) << (sorting ? "Movements" : "Matches") << '\n';
}

inline void printTimingRow(const char* operation, Timing timing, std::uint64_t comparisons, std::uint64_t other) {
    std::cout << std::fixed << std::setprecision(6) << std::left << std::setw(25) << operation << std::right
              << std::setw(15) << timing.median << std::setw(15) << timing.minimum << std::setw(15) << timing.maximum
              << std::setw(15) << comparisons << std::setw(15) << other << '\n';
}

inline void printMemoryHeader() {
    std::cout << std::left << std::setw(25) << "Operation" << std::right << std::setw(17) << "Data bytes"
              << std::setw(17) << "Aux heap bytes" << std::setw(22) << "Accounted peak bytes" << '\n';
}

inline void printMemoryRow(const char* operation, std::size_t dataBytes, std::size_t auxiliaryBytes, std::size_t peakBytes) {
    std::cout << std::left << std::setw(25) << operation << std::right << std::setw(17) << dataBytes
              << std::setw(17) << auxiliaryBytes << std::setw(22) << peakBytes << '\n';
}

inline void printBenchmarkNotes() {
    std::cout << "\nTiming: 9 trials, steady_clock, microseconds; median/min/max. Copying and output are excluded.\n"
              << "Search trials batch 256 identical queries; times are per-query means including batch-loop overhead.\n"
              << "Comparisons count calls to the record/query comparator, including unsuccessful comparisons.\n"
              << "Memory is a sizeof-based estimate: container object + allocated record/node storage.\n"
              << "Aux heap bytes count explicit algorithm buffers; stack, allocator metadata and process RSS are excluded.\n"
              << "Accounted peak is during the timed operation: canonical data + simultaneous working containers + buffers.\n"
              << "Snapshot/copy preparation and its temporary allocations are excluded from this operation peak.\n"
              << "Small timings depend on the compiler, machine, timer resolution and system load; compare repeated release runs.\n";
}

template <typename Container>
void verifySorted(const Container& records, SortKey key) {
    const Patient* previous = nullptr;
    records.forEach([&](const Patient& patient) {
        if (previous != nullptr && compareRecords(*previous, patient, key) > 0) throw std::logic_error("Sorting verification failed.");
        previous = &patient;
    });
}

template <typename Container>
void runSortExperiment(const Container& source, SortKey key, SortAlgorithm algorithm, int facility, bool display) {
    Container working;
    double samples[BENCHMARK_TRIALS]{};
    OperationCounts counts;
    for (double& sample : samples) {
        working.copyFrom(source, facility);
        const auto start = std::chrono::steady_clock::now();
        counts = working.sort(key, algorithm);
        const auto finish = std::chrono::steady_clock::now();
        sample = elapsedMicroseconds(start, finish);
        verifySorted(working, key);
    }
    const Timing timing = summarizeTimes(samples);
    std::cout << "\nSORT | " << Container::structureName() << " | Facility " << facilityName(facility)
              << " | Key " << sortKeyName(key) << " | Records " << working.count() << " | Input: CSV order\n";
    printTimingHeader(true);
    printTimingRow(algorithmName(algorithm), timing, counts.comparisons, counts.movements);
    printMemoryHeader();
    printMemoryRow(algorithmName(algorithm), working.storageBytes(), counts.auxiliaryBytes,
                   source.storageBytes() + working.storageBytes() + counts.auxiliaryBytes);
    std::cout << "Movements = " << Container::movementName() << ".\n";
    if (display) displayPatients(working);
}

struct SearchMeasurement {
    SearchResult result;
    Timing timing;
};

template <typename Container>
SearchMeasurement measureSearch(const Container& records, const Query& query, bool ordered) {
    double samples[BENCHMARK_TRIALS]{};
    SearchResult first;
    bool hasFirst = false;
    for (double& sample : samples) {
        SearchResult result;
        std::uint64_t comparisonGuard = 0;
        std::size_t matchGuard = 0;
        volatile std::uint64_t checksumGuard = 0;
        const auto start = std::chrono::steady_clock::now();
        for (std::uint64_t repeat = 0; repeat < SEARCH_BATCH_SIZE; ++repeat) {
            result = records.search(query, ordered);
            comparisonGuard += result.comparisons;
            matchGuard += result.matches;
            checksumGuard += result.checksum;
        }
        const auto finish = std::chrono::steady_clock::now();
        sample = elapsedMicroseconds(start, finish) / static_cast<double>(SEARCH_BATCH_SIZE);
        if (comparisonGuard != result.comparisons * SEARCH_BATCH_SIZE || matchGuard != result.matches * SEARCH_BATCH_SIZE
                || checksumGuard != result.checksum * SEARCH_BATCH_SIZE) throw std::logic_error("Search batch returned inconsistent results.");
        if (!hasFirst) {
            first = result;
            hasFirst = true;
            continue;
        }
        if (result.matches != first.matches || result.checksum != first.checksum || result.comparisons != first.comparisons) {
            throw std::logic_error("Search trials returned inconsistent results.");
        }
    }
    return {first, summarizeTimes(samples)};
}

template <typename Container>
void runSearchExperiment(const Container& source, const Query& query, const char* description, int facility, bool display) {
    Container unordered;
    unordered.copyFrom(source, facility);
    unordered.reverse();
    Container ordered;
    double preparationSamples[BENCHMARK_TRIALS]{};
    OperationCounts preparation;
    for (double& sample : preparationSamples) {
        ordered.copyFrom(unordered);
        const auto start = std::chrono::steady_clock::now();
        preparation = ordered.sort(query.key, SortAlgorithm::Merge);
        const auto finish = std::chrono::steady_clock::now();
        sample = elapsedMicroseconds(start, finish);
        verifySorted(ordered, query.key);
    }
    const Timing preparationTiming = summarizeTimes(preparationSamples);
    const SearchMeasurement baseline = measureSearch(unordered, query, false);
    const SearchMeasurement sorted = measureSearch(ordered, query, true);
    if (baseline.result.matches != sorted.result.matches || baseline.result.checksum != sorted.result.checksum) {
        throw std::logic_error("Sorted and unsorted search results disagree.");
    }
    std::cout << "\nSEARCH | " << Container::structureName() << " | Facility " << facilityName(facility)
              << " | " << description << " | Records " << unordered.count() << '\n';
    std::cout << "Baseline: reverse CSV order, with no ordering assumed. Sorted input: ascending query key.\n";
    printTimingHeader(true);
    printTimingRow("Merge preparation", preparationTiming, preparation.comparisons, preparation.movements);
    printTimingHeader(false);
    printTimingRow("Unsorted full scan", baseline.timing, baseline.result.comparisons, baseline.result.matches);
    printTimingRow("Sorted search", sorted.timing, sorted.result.comparisons, sorted.result.matches);
    const std::size_t simultaneousBytes = source.storageBytes() + unordered.storageBytes() + ordered.storageBytes();
    printMemoryHeader();
    printMemoryRow("Merge preparation", ordered.storageBytes(), preparation.auxiliaryBytes, simultaneousBytes + preparation.auxiliaryBytes);
    printMemoryRow("Unsorted full scan", unordered.storageBytes(), baseline.result.auxiliaryBytes, simultaneousBytes + baseline.result.auxiliaryBytes);
    printMemoryRow("Sorted search", ordered.storageBytes(), sorted.result.auxiliaryBytes, simultaneousBytes + sorted.result.auxiliaryBytes);
    std::cout << "Match-count/checksum agreement: PASS. Checksums: " << baseline.result.checksum << " / " << sorted.result.checksum << '\n'
              << "One-query sorted path = preparation + sorted search; later queries may reuse the sorted data.\n"
              << "Preparation movements = " << Container::movementName() << ".\n";
    if (display) displayMatches(ordered, query);
}

template <typename Container>
void runBenchmarks(const Container& records) {
    if (records.count() == 0) {
        std::cout << "No records loaded. Use option 1 first.\n";
        return;
    }
    printBenchmarkNotes();
    const SortKey keys[] = {SortKey::Age, SortKey::Duration, SortKey::Cost};
    const SortAlgorithm algorithms[] = {SortAlgorithm::Insertion, SortAlgorithm::Merge};
    const char* const searches[] = {"age=61-100", "caretype=Emergency", "duration>24"};
    for (int facility = -1; facility < FACILITY_COUNT; ++facility) {
        for (SortKey key : keys) {
            for (SortAlgorithm algorithm : algorithms) runSortExperiment(records, key, algorithm, facility, false);
        }
        for (const char* expression : searches) {
            Query query;
            char error[192]{};
            if (!parseQuery(expression, query, error, sizeof(error))) throw std::logic_error("Invalid built-in benchmark query.");
            runSearchExperiment(records, query, expression, facility, false);
        }
    }
}

template <typename Container>
bool promptSort(const Container& records) {
    if (records.count() == 0) {
        std::cout << "No records loaded. Use option 1 first.\n";
        return true;
    }
    char field[128]{};
    if (!readInput("Sort key (id / age / duration / cost / caretype): ", field, sizeof(field))) return false;
    SortKey key;
    if (!parseSortKey(field, key)) {
        std::cout << "Unknown sort key.\n";
        return true;
    }
    int algorithm = 0;
    int facility = -1;
    if (!readChoice("Algorithm (1 = insertion, 2 = merge, 3 = compare both): ", 1, 3, algorithm)) return false;
    if (!readScope(facility)) return false;
    printBenchmarkNotes();
    if (algorithm == 1 || algorithm == 3) runSortExperiment(records, key, SortAlgorithm::Insertion, facility, true);
    if (algorithm == 2 || algorithm == 3) runSortExperiment(records, key, SortAlgorithm::Merge, facility, true);
    return true;
}

template <typename Container>
bool promptSearch(const Container& records) {
    if (records.count() == 0) {
        std::cout << "No records loaded. Use option 1 first.\n";
        return true;
    }
    char expression[256]{};
    if (!readInput("Query (e.g. age=61-100, caretype=Emergency, duration>24, id=PT1001): ", expression, sizeof(expression))) return false;
    Query query;
    char error[192]{};
    if (!parseQuery(expression, query, error, sizeof(error))) {
        std::cout << "Invalid query: " << error << '\n';
        return true;
    }
    int facility = -1;
    if (!readScope(facility)) return false;
    printBenchmarkNotes();
    runSearchExperiment(records, query, expression, facility, true);
    return true;
}

template <typename Container>
void confirmLoad(const Container& records) {
    std::cout << "Loaded " << records.count() << " records. Reload replaces the previous dataset.\n";
    std::cout << std::fixed << std::setprecision(2);
    printFacilitySummary(summarize(records));
}

inline void printMenu() {
    std::cout << "\n1. Load / reload all three datasets\n2. Display patient records\n3. Demographic, billing and care analysis\n"
              << "4. Sort and compare algorithms\n5. Search unsorted vs sorted data\n6. Full analysis and benchmark demonstration\n0. Exit\n";
}

template <typename Container>
int runApplication(int argc, char* argv[]) {
    try {
        if (argc > 3) throw std::runtime_error("Usage: program [dataset-directory] [--analysis | --benchmark | --demo]");
        const char* supplied = nullptr;
        const char* mode = nullptr;
        for (int index = 1; index < argc; ++index) {
            if (std::strncmp(argv[index], "--", 2) == 0) {
                if (mode != nullptr) throw std::runtime_error("Provide only one output mode.");
                mode = argv[index];
                continue;
            }
            if (supplied != nullptr) throw std::runtime_error("Provide only one dataset directory.");
            supplied = argv[index];
        }
        if (mode != nullptr && std::strcmp(mode, "--analysis") != 0 && std::strcmp(mode, "--benchmark") != 0 && std::strcmp(mode, "--demo") != 0) {
            throw std::runtime_error("Unknown output mode. Use --analysis, --benchmark or --demo.");
        }
        char directory[PATH_CAPACITY]{};
        chooseDirectory(directory, sizeof(directory), argv[0], supplied);
        std::cout << "SMART HEALTHCARE | " << Container::structureName() << "\nDataset directory: " << directory << '\n';
        Container records;
        if (mode != nullptr) {
            loadAll(records, directory);
            confirmLoad(records);
            if (std::strcmp(mode, "--benchmark") != 0) displayAnalysis(records);
            if (std::strcmp(mode, "--analysis") != 0) runBenchmarks(records);
            return 0;
        }
        for (;;) {
            printMenu();
            int choice = 0;
            if (!readChoice("Choice: ", 0, 6, choice) || choice == 0) return 0;
            try {
                switch (choice) {
                    case 1: loadAll(records, directory); confirmLoad(records); break;
                    case 2: displayPatients(records); break;
                    case 3: displayAnalysis(records); break;
                    case 4: if (!promptSort(records)) return 0; break;
                    case 5: if (!promptSearch(records)) return 0; break;
                    case 6: displayAnalysis(records); runBenchmarks(records); break;
                    default: throw std::logic_error("Invalid menu choice.");
                }
            } catch (const std::runtime_error& error) {
                std::cerr << "Operation failed: " << error.what() << "\nPrevious records are unchanged.\n";
            }
        }
    } catch (const std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    } catch (const std::bad_alloc& error) {
        std::cerr << "Insufficient memory: " << error.what() << '\n';
        return 1;
    }
}

}

#endif
