#ifndef DSTR_ARRAY_LIST_HPP
#define DSTR_ARRAY_LIST_HPP

#include "../common/Patient.hpp"

#include <cstddef>

class ArrayList {
public:
    ArrayList() noexcept;
    ~ArrayList();
    ArrayList(const ArrayList&) = delete;
    ArrayList& operator=(const ArrayList&) = delete;

    void clear() noexcept;
    void append(const healthcare::Patient& record);
    std::size_t count() const noexcept;
    std::size_t storageBytes() const noexcept;
    void copyFrom(const ArrayList& source, int facility = -1);
    void swap(ArrayList& other) noexcept;
    void reverse() noexcept;
    healthcare::OperationCounts sort(healthcare::SortKey key, healthcare::SortAlgorithm algorithm);
    healthcare::SearchResult search(const healthcare::Query& query, bool ordered) const;
    bool containsId(const char* patientId) const;

    template <typename Visitor>
    void forEach(Visitor visitor) const {
        for (std::size_t index = 0; index < size; ++index) {
            visitor(static_cast<const healthcare::Patient&>(patients[index]));
        }
    }

    static const char* structureName() noexcept;
    static const char* movementName() noexcept;

private:
    healthcare::Patient* patients = nullptr;
    std::size_t size = 0;
    std::size_t capacity = 0;

    void grow();
    healthcare::OperationCounts insertionSort(healthcare::SortKey key);
    healthcare::OperationCounts mergeSort(healthcare::SortKey key);
};

#endif
