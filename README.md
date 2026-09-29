# Data-Structures-

CT077-3-2-DSTR Lab Work #1 — healthcare patient record analysis using **arrays** and **singly linked lists**.

Each version is a separate program. Both load the three facility CSV files from the repository root, recategorize patients by age group, compute billing, and run sorting and searching experiments with console tables.

Cost formula used throughout:

`Total Medical Cost = Visit Duration Hours × Base Cost Per Hour × Days Visits Per Year`

The CSV column is named `LengthOfStay`. Both programs store that value as `VisitDurationHours`.

## Layout

```
dataset1 facility_a.csv
dataset2 facility_b.csv
dataset3_facility_c.csv

array_version/
  main.cpp          menu
  ArrayList.h
  ArrayList.cpp     dynamic Patient array
  Patient.h
  Patient.cpp

linkedlist_version/
  main.cpp          menu
  LinkedList.h
  LinkedList.cpp    singly linked list (NodeType + next)
  NodeType.h
  Patient.h
  Patient.cpp
```

Run each program from its own folder. The CSV paths use `../`, so the three dataset files stay in the repository root.

## Compile and run (Windows)

Array version, from `array_version`:

```bat
g++ -std=c++17 -o array_program.exe main.cpp ArrayList.cpp Patient.cpp
array_program.exe
```

Linked list version, from `linkedlist_version`:

```bat
g++ -std=c++17 -o linkedlist_program.exe main.cpp LinkedList.cpp Patient.cpp
linkedlist_program.exe
```

## How the system runs

Start the program from its own folder. `main` creates one empty container (`ArrayList` or `LinkedList`) and loops on the menu until you enter `0`.

1. **Load datasets.** Reads the three CSV files in order and appends every valid row. The header is skipped. A bad row is counted and skipped. After all three files, the container holds the combined patient list (600 records when every row loads).

2. **View patient records.** Prints every stored patient in a table, including the computed cost, then prints the record count and the total cost. If nothing is loaded, it prints `No records loaded yet.`

3. **Demographic and billing categorization.** Walks the list once and buckets each patient by age:

   | Age | Group |
   |-----|--------|
   | 0–17 | Child |
   | 18–35 | Young Adult |
   | 36–59 | Adult |
   | 60+ | Senior |

   For each group it prints the count, share of patients, age range, total cost, and average cost, then the grand total.

4. **Sorting experiment.** Asks for one field: `id`, `age`, `cost`, `duration`, or `caretype`. Both versions use insertion sort and print the comparison count and the time in milliseconds. The array version moves patient records and reports how many shifts that took. The linked list version rewires `next` pointers and reports how many nodes were relinked. The list stays in that order for the next search.

5. **Searching experiment.** Asks for `field=value`. Searchable fields are `id`, `age`, and `caretype` (for example `id=PT1001`, `age=42`, `caretype=Emergency`). Both versions first scan every record and print the matches.

   The second pass depends on the container:

   - **Array:** if the list is already sorted by `id` or `age`, it also runs binary search and prints that comparison count. `caretype` is not binary-searched. If the array is not sorted by the chosen field, it asks you to sort first.
   - **Linked list:** if the chain is already sorted by `id` or `age`, it scans again but stops once the key has passed the target. It does not use binary search, because a singly linked list cannot jump to the middle.

6. **Run full demo.** If the list is empty, it loads all three files. Then it runs categorization, sorts by `age`, `cost`, and `id` (id last, so the list is sorted for the id search), and searches for `id=PT1001` and `caretype=Emergency`.

Enter `0` to exit. The container is destroyed when `main` ends: the array frees its block, and the linked list deletes every node.

## GenAI declaration

AI was used to identify C++ `<chrono>` timing keywords, console table formatting patterns, and to review menu structure. Algorithms, data-structure choices, and analysis logic were checked against the three CSV files and can be explained during evaluation.
