#ifndef DSTR_LINKED_LIST_HPP
#define DSTR_LINKED_LIST_HPP

#include "NodeType.hpp"

class LinkedList {
public:
    LinkedList() noexcept = default;
    ~LinkedList();
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    void clear() noexcept;
    void append(const healthcare::Patient& patient);
    std::size_t count() const noexcept;
    std::size_t storageBytes() const noexcept;
    void copyFrom(const LinkedList& source, int facility = -1);
    void swap(LinkedList& other) noexcept;
    void reverse() noexcept;
    healthcare::OperationCounts sort(healthcare::SortKey key, healthcare::SortAlgorithm algorithm);
    healthcare::SearchResult search(const healthcare::Query& query, bool ordered) const;
    bool containsId(const char* patientId) const;

    template <typename Visitor>
    void forEach(Visitor visitor) const {
        for (const NodeType* current = head; current != nullptr; current = current->next) {
            visitor(current->data);
        }
    }

    static const char* structureName() noexcept;
    static const char* movementName() noexcept;

private:
    NodeType* head = nullptr;
    NodeType* tail = nullptr;
    std::size_t size = 0;

    healthcare::OperationCounts insertionSort(healthcare::SortKey key);
    healthcare::OperationCounts mergeSort(healthcare::SortKey key);
    void appendSortedNode(NodeType* node, healthcare::OperationCounts& counts);
};

#endif
