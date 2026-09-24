#include "ArrayList.h"
#include <iostream>

namespace {

// The CSV files live in the repository root, one level above this program.
const char* DATASETS[3] = {
    "../dataset1 facility_a.csv",
    "../dataset2 facility_b.csv",
    "../dataset3_facility_c.csv"
};

void printMenu() {
    std::cout << "\n========================================\n";
    std::cout << " Hospital Patient Management (Array)\n";
    std::cout << "========================================\n";
    std::cout << "1. Load datasets\n";
    std::cout << "2. View patient records\n";
    std::cout << "3. Demographic and billing categorization\n";
    std::cout << "4. Sorting experiment (insertion sort)\n";
    std::cout << "5. Searching experiment (linear vs binary)\n";
    std::cout << "6. Run full demo\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter choice: ";
}

void loadAll(ArrayList& patients) {
    for (int i = 0; i < 3; i++) {
        patients.loadFromCSV(DATASETS[i]);
    }
}

void runDemo(ArrayList& patients) {
    if (patients.count() == 0) {
        loadAll(patients);
    }
    patients.categorizeByAgeGroup();
    std::cout << "\n";
    patients.sortBy("age");
    patients.sortBy("cost");
    patients.sortBy("id");
    std::cout << "\n";
    patients.searchBy("id=PT1001");
    std::cout << "\n";
    patients.searchBy("caretype=Emergency");
}

}  // namespace

int main() {
    ArrayList patients;
    int choice = -1;

    while (choice != 0) {
        printMenu();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Please enter a number.\n";
            continue;
        }
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
