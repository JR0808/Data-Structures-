#include "LinkedList.hpp"

#include <cstring>

namespace {

void writeLink(NodeType*& link, NodeType* node, healthcare::OperationCounts& counts) {
    link = node;
    ++counts.movements;
}

//

NodeType* splitRun(NodeType* first, std::size_t width, healthcare::OperationCounts& counts) {
    if (first == nullptr) return nullptr;

    NodeType* last = first;
    std::size_t remaining = width - 1;
    while (remaining > 0 && last->next != nullptr) {
        last = last->next;
        --remaining;
    }

    NodeType* following = last->next;
    writeLink(last->next, nullptr, counts);
    return following;
}

//

NodeType* takeFirst(NodeType*& first) {
    NodeType* selected = first;
    first = first->next;
    return selected;
}

//

NodeType* takeEarlier(NodeType*& left, NodeType*& right, healthcare::SortKey key,
                      healthcare::OperationCounts& counts) {
    if (left == nullptr) return takeFirst(right);
    if (right == nullptr) return takeFirst(left);

    ++counts.comparisons;
    if (healthcare::compareRecords(left->data, right->data, key) <= 0) return takeFirst(left);

    return takeFirst(right);
}

}

//

LinkedList::~LinkedList() {
    clear();
}

//

void LinkedList::clear() noexcept {
    while (head != nullptr) {
        NodeType* removed = head;
        head = head->next;
        delete removed;
    }

    tail = nullptr;
    size = 0;
}

//

void LinkedList::append(const healthcare::Patient& patient) {
    NodeType* node = new NodeType{patient, nullptr};
    ++size;
    if (tail == nullptr) {
        head = node;
        tail = node;
        return;
    }

    tail->next = node;
    tail = node;
}

//

std::size_t LinkedList::count() const noexcept {
    return size;
}

//

std::size_t LinkedList::storageBytes() const noexcept {
    return sizeof(*this) + size * sizeof(NodeType);
}

//

void LinkedList::copyFrom(const LinkedList& source, int facility) {
    LinkedList replacement;
    for (const NodeType* current = source.head; current != nullptr; current = current->next) {
        if (facility >= 0 && current->data.Facility != facility) continue;

        replacement.append(current->data);
    }

    swap(replacement);
}

//

void LinkedList::swap(LinkedList& other) noexcept {
    NodeType* previousHead = head;
    NodeType* previousTail = tail;
    const std::size_t previousSize = size;
    head = other.head;
    tail = other.tail;
    size = other.size;
    other.head = previousHead;
    other.tail = previousTail;
    other.size = previousSize;
}

//

void LinkedList::reverse() noexcept {
    NodeType* previous = nullptr;
    NodeType* current = head;
    tail = head;
    while (current != nullptr) {
        NodeType* next = current->next;
        current->next = previous;
        previous = current;
        current = next;
    }

    head = previous;
}

//

healthcare::OperationCounts LinkedList::sort(healthcare::SortKey key, healthcare::SortAlgorithm algorithm) {
    if (size < 2) return {};
    if (algorithm == healthcare::SortAlgorithm::Insertion) return insertionSort(key);

    return mergeSort(key);
}

//

healthcare::OperationCounts LinkedList::insertionSort(healthcare::SortKey key) {
    healthcare::OperationCounts counts;
    NodeType* current = head;
    writeLink(head, nullptr, counts);
    writeLink(tail, nullptr, counts);

    while (current != nullptr) {
        NodeType* next = current->next;
        if (head == nullptr) {
            writeLink(current->next, nullptr, counts);
            writeLink(head, current, counts);
            writeLink(tail, current, counts);
            current = next;
            continue;
        }

        ++counts.comparisons;
        if (healthcare::compareRecords(current->data, head->data, key) < 0) {
            writeLink(current->next, head, counts);
            writeLink(head, current, counts);
            current = next;
            continue;
        }

        NodeType* previous = head;
        while (previous->next != nullptr) {
            ++counts.comparisons;
            if (healthcare::compareRecords(current->data, previous->next->data, key) < 0) break;

            previous = previous->next;
        }

        writeLink(current->next, previous->next, counts);
        writeLink(previous->next, current, counts);
        if (current->next == nullptr) writeLink(tail, current, counts);

        current = next;
    }

    return counts;
}

//

void LinkedList::appendSortedNode(NodeType* node, healthcare::OperationCounts& counts) {
    if (tail == nullptr) {
        writeLink(head, node, counts);
        writeLink(tail, node, counts);
        return;
    }

    writeLink(tail->next, node, counts);
    writeLink(tail, node, counts);
}

//

healthcare::OperationCounts LinkedList::mergeSort(healthcare::SortKey key) {
    healthcare::OperationCounts counts;
    std::size_t width = 1;
    while (width < size) {
        NodeType* current = head;
        writeLink(head, nullptr, counts);
        writeLink(tail, nullptr, counts);
        while (current != nullptr) {
            NodeType* left = current;
            NodeType* right = splitRun(left, width, counts);
            current = splitRun(right, width, counts);
            while (left != nullptr || right != nullptr) {
                appendSortedNode(takeEarlier(left, right, key, counts), counts);
            }
        }

        writeLink(tail->next, nullptr, counts);
        if (width > size / 2) break;

        width *= 2;
    }

    return counts;
}

//

healthcare::SearchResult LinkedList::search(const healthcare::Query& query, bool ordered) const {
    healthcare::SearchResult result;
    for (const NodeType* current = head; current != nullptr; current = current->next) {
        ++result.comparisons;
        const int order = healthcare::compareRecordToQuery(current->data, query);
        if (ordered && order > 0) break;
        if (order != 0) continue;

        ++result.matches;
        result.checksum += healthcare::recordFingerprint(current->data);
    }

    return result;
}

//

bool LinkedList::containsId(const char* patientId) const {
    for (const NodeType* current = head; current != nullptr; current = current->next) {
        if (std::strcmp(current->data.PatientID, patientId) != 0) continue;

        return true;
    }

    return false;
}

//

const char* LinkedList::structureName() noexcept {
    return "Singly linked list";
}

//

const char* LinkedList::movementName() noexcept {
    return "node link writes";
}
