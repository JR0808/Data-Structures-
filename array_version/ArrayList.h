#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include "Patient.h"

class ArrayList {
public:
    ArrayList();
    ~ArrayList();

    void loadFromCSV(const char* filename);
    void categorizeByAgeGroup();
    double calculateCost(Patient patient);
    void sortBy(const char* criteria);
    void searchBy(const char* criteria);
    void displayTable();

    int count() const;

private:
    Patient* patients;
    int size;
    int capacity;

    void grow();
    void append(const Patient& record);
    bool isSortedBy(const char* criteria) const;
};

#endif
