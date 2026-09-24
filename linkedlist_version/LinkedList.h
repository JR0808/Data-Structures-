#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "NodeType.h"

class LinkedList {
public:
    LinkedList();
    ~LinkedList();

    void loadFromCSV(const char* filename);
    void categorizeByAgeGroup();
    double calculateCost(Patient patient);
    void sortBy(const char* criteria);
    void searchBy(const char* criteria);
    void displayTable();

    int count() const;

private:
    NodeType* head;
    NodeType* tail;
    int size;

    void append(const Patient& record);
    bool isSortedBy(const char* criteria) const;
};

#endif
