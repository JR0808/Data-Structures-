#include "ArrayList.hpp"

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

void mergeRange(const healthcare::Patient* source, healthcare::Patient* destination,
                std::size_t begin, std::size_t middle, std::size_t end,
                healthcare::SortKey key, healthcare::OperationCounts& counts) {
    std::size_t left = begin;
    std::size_t right = middle;
    std::size_t output = begin;

    while (left < middle && right < end) {
        ++counts.comparisons;
        if (healthcare::compareRecords(source[left], source[right], key) <= 0) {
            destination[output++] = source[left++];
            ++counts.movements;
            continue;
        }
        destination[output++] = source[right++];
        ++counts.movements;
    }

    while (left < middle) {
        destination[output++] = source[left++];
        ++counts.movements;
    }
    while (right < end) {
        destination[output++] = source[right++];
        ++counts.movements;
    }
}

}

ArrayList::ArrayList() noexcept = default;

ArrayList::~ArrayList() {
    clear();
}

void ArrayList::clear() noexcept {
    delete[] patients;
    patients = nullptr;
    size = 0;
    capacity = 0;
}

void ArrayList::grow() {
    constexpr std::size_t INITIAL_CAPACITY = 64;
    constexpr std::size_t MAX_CAPACITY = std::numeric_limits<std::size_t>::max() / sizeof(healthcare::Patient);
    if (capacity > MAX_CAPACITY / 2) {
        throw std::length_error("Array patient capacity exceeds the allocation limit");
    }

    const std::size_t newCapacity = capacity == 0 ? INITIAL_CAPACITY : capacity * 2;
    auto replacement = std::make_unique<healthcare::Patient[]>(newCapacity);
    for (std::size_t index = 0; index < size; ++index) {
        replacement[index] = patients[index];
    }
    delete[] patients;
    patients = replacement.release();
    capacity = newCapacity;
}

void ArrayList::append(const healthcare::Patient& record) {
    if (size < capacity) {
        patients[size++] = record;
        return;
    }

    const healthcare::Patient retainedRecord = record;
    grow();
    patients[size++] = retainedRecord;
}

std::size_t ArrayList::count() const noexcept {
    return size;
}

std::size_t ArrayList::storageBytes() const noexcept {
    return sizeof(*this) + capacity * sizeof(healthcare::Patient);
}

void ArrayList::copyFrom(const ArrayList& source, int facility) {
    ArrayList replacement;
    source.forEach([&](const healthcare::Patient& record) {
        if (facility >= 0 && record.Facility != facility) {
            return;
        }
        replacement.append(record);
    });
    swap(replacement);
}

void ArrayList::swap(ArrayList& other) noexcept {
    std::swap(patients, other.patients);
    std::swap(size, other.size);
    std::swap(capacity, other.capacity);
}

void ArrayList::reverse() noexcept {
    for (std::size_t index = 0; index < size / 2; ++index) {
        std::swap(patients[index], patients[size - 1 - index]);
    }
}

healthcare::OperationCounts ArrayList::insertionSort(healthcare::SortKey key) {
    healthcare::OperationCounts counts;
    for (std::size_t index = 1; index < size; ++index) {
        healthcare::Patient current;
        current = patients[index];
        ++counts.movements;
        std::size_t insertion = index;

        while (insertion > 0) {
            ++counts.comparisons;
            if (healthcare::compareRecords(current, patients[insertion - 1], key) >= 0) {
                break;
            }
            patients[insertion] = patients[insertion - 1];
            ++counts.movements;
            --insertion;
        }
        patients[insertion] = current;
        ++counts.movements;
    }
    return counts;
}

healthcare::OperationCounts ArrayList::mergeSort(healthcare::SortKey key) {
    healthcare::OperationCounts counts;
    if (size < 2) {
        return counts;
    }

    auto buffer = std::make_unique<healthcare::Patient[]>(size);
    counts.auxiliaryBytes = size * sizeof(healthcare::Patient);
    healthcare::Patient* source = patients;
    healthcare::Patient* destination = buffer.get();
    std::size_t runWidth = 1;

    while (runWidth < size) {
        std::size_t begin = 0;
        while (begin < size) {
            const std::size_t leftLength = runWidth < size - begin ? runWidth : size - begin;
            const std::size_t middle = begin + leftLength;
            const std::size_t rightLength = runWidth < size - middle ? runWidth : size - middle;
            const std::size_t end = middle + rightLength;
            mergeRange(source, destination, begin, middle, end, key, counts);
            begin = end;
        }
        std::swap(source, destination);
        if (runWidth >= size - runWidth) {
            break;
        }
        runWidth *= 2;
    }

    if (source == patients) {
        return counts;
    }
    for (std::size_t index = 0; index < size; ++index) {
        patients[index] = source[index];
        ++counts.movements;
    }
    return counts;
}

healthcare::OperationCounts ArrayList::sort(healthcare::SortKey key, healthcare::SortAlgorithm algorithm) {
    if (algorithm == healthcare::SortAlgorithm::Insertion) {
        return insertionSort(key);
    }
    return mergeSort(key);
}

healthcare::SearchResult ArrayList::search(const healthcare::Query& query, bool ordered) const {
    healthcare::SearchResult result;
    if (!ordered) {
        forEach([&](const healthcare::Patient& record) {
            ++result.comparisons;
            if (healthcare::compareRecordToQuery(record, query) != 0) {
                return;
            }
            ++result.matches;
            result.checksum += healthcare::recordFingerprint(record);
        });
        return result;
    }

    std::size_t lower = 0;
    std::size_t upper = size;
    while (lower < upper) {
        const std::size_t middle = lower + (upper - lower) / 2;
        ++result.comparisons;
        if (healthcare::compareRecordToQuery(patients[middle], query) < 0) {
            lower = middle + 1;
            continue;
        }
        upper = middle;
    }

    for (std::size_t index = lower; index < size; ++index) {
        ++result.comparisons;
        if (healthcare::compareRecordToQuery(patients[index], query) != 0) {
            break;
        }
        ++result.matches;
        result.checksum += healthcare::recordFingerprint(patients[index]);
    }
    return result;
}

bool ArrayList::containsId(const char* patientId) const {
    for (std::size_t index = 0; index < size; ++index) {
        if (std::strcmp(patients[index].PatientID, patientId) != 0) {
            continue;
        }
        return true;
    }
    return false;
}

const char* ArrayList::structureName() noexcept {
    return "Array";
}

const char* ArrayList::movementName() noexcept {
    return "record assignments";
}
