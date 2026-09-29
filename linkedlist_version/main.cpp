// =============================================================================
// FILE: main.cpp   (LINKED LIST VERSION)
// ROLE: The user interface. Shows the menu, reads a choice, calls LinkedList.
//
// DESIGN POINT TO PRESENT: compare this file with array_version/main.cpp. Only
// the type name changed - ArrayList became LinkedList. Every method call is
// identical, because both classes expose the same six operations. That is
// ENCAPSULATION: the interface hides whether the data lives in an array or in a
// chain of nodes.
// =============================================================================

#include "LinkedList.h"
#include <iostream>

namespace {

// -----------------------------------------------------------------------------
// [1] THE THREE DATA FILES
//
// "../" means "one folder up". The CSV files live in the repository root while
// this program runs from inside linkedlist_version, hence the ../ prefix.
//
// IF THE DEMO FAILS TO LOAD: the program was started from the wrong folder.
// -----------------------------------------------------------------------------
const char* DATASETS[3] = {
    "../dataset1 facility_a.csv",
    "../dataset2 facility_b.csv",
    "../dataset3_facility_c.csv"
};

// -----------------------------------------------------------------------------
// [2] printMenu - draw the option list each time round the loop.
// -----------------------------------------------------------------------------
void printMenu() {
    std::cout << "\n========================================\n";
    std::cout << " Hospital Patient Management (Linked List)\n";
    std::cout << "========================================\n";
    std::cout << "1. Load datasets\n";
    std::cout << "2. View patient records\n";
    std::cout << "3. Demographic and billing categorization\n";
    std::cout << "4. Sorting experiment (insertion sort)\n";
    std::cout << "5. Searching experiment (unsorted vs sorted scan)\n";
    std::cout << "6. Run full demo\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter choice: ";
}

// -----------------------------------------------------------------------------
// [3] loadAll - read all three facilities into ONE list.
// loadFromCSV appends, so after three calls the chain holds 600 nodes.
// -----------------------------------------------------------------------------
void loadAll(LinkedList& patients) {
    for (int i = 0; i < 3; i++) {
        patients.loadFromCSV(DATASETS[i]);
    }
}

// -----------------------------------------------------------------------------
// [4] runDemo - the scripted sequence for the presentation / screenshots.
//
// ORDER MATTERS HERE: sortBy("id") runs last so that the following
// searchBy("id=PT1001") finds the chain already sorted and can therefore
// demonstrate the early-exit scan rather than refusing.
// -----------------------------------------------------------------------------
void runDemo(LinkedList& patients) {
    if (patients.count() == 0) {
        loadAll(patients);              // only load if nothing is there yet
    }
    patients.categorizeByAgeGroup();    // demographics + billing
    std::cout << "\n";
    patients.sortBy("age");             // three sorting experiments
    patients.sortBy("cost");
    patients.sortBy("id");              // leaves the chain sorted by id
    std::cout << "\n";
    patients.searchBy("id=PT1001");     // single match: unsorted vs sorted
    std::cout << "\n";
    patients.searchBy("caretype=Emergency");   // many matches
}

}  // namespace

// =============================================================================
// [5] main - the menu loop
// =============================================================================
int main() {
    // Creating this object runs the constructor (an empty chain). When main
    // ends, the destructor runs automatically and deletes every node.
    LinkedList patients;
    int choice = -1;

    while (choice != 0) {
        printMenu();

        // --- Input validation --------------------------------------------
        // If the user types letters, cin fails. We must clear() the error flag
        // and ignore() the bad text, otherwise the loop would spin forever.
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Please enter a number.\n";
            continue;
        }
        // Discard the leftover newline so the getline() calls below work.
        std::cin.ignore(10000, '\n');

        char input[128];
        switch (choice) {
            case 1:
                loadAll(patients);
                break;
            case 2:
                patients.displayTable();
                break;
            case 3:
                patients.categorizeByAgeGroup();
                break;
            case 4:
                std::cout << "Sort by (id / age / cost / duration / caretype): ";
                std::cin.getline(input, sizeof(input));
                patients.sortBy(input);
                break;
            case 5:
                std::cout << "Search (field=value, e.g. id=PT1001, age=42, caretype=Emergency): ";
                std::cin.getline(input, sizeof(input));
                patients.searchBy(input);
                break;
            case 6:
                runDemo(patients);
                break;
            case 0:
                std::cout << "Goodbye.\n";
                break;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }

    return 0;
}
