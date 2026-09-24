# Data-Structures-

CT077-3-2-DSTR Lab Work #1 — healthcare patient record analysis using **arrays** and **singly linked lists**.

## Programs

| File | Data structure |
|------|----------------|
| `array_implementation.cpp` | Fixed Patient arrays only |
| `linkedlist_implementation.cpp` | Singly linked list (`Node` + `next`) only |

Each program is standalone. They load the three facility CSV files, recategorize patients by age group, compute billing, and run sorting / searching experiments with console tables.

Cost formula used throughout:

`Total Medical Cost = Length of Stay × Base Cost Per Hour × Days Visits Per Year`

## Compile and run (Windows)

From this folder:

```bat
compile.bat
```

Or:

```bat
g++ -std=c++17 -o array_program.exe array_implementation.cpp
g++ -std=c++17 -o linkedlist_program.exe linkedlist_implementation.cpp
```

Interactive:

```bat
array_program.exe
linkedlist_program.exe
```

Print every required section once (useful for screenshots / the report):

```bat
array_program.exe --demo
linkedlist_program.exe --demo
```

CSV files must stay in the same folder as the `.exe` files.

## Menu (both programs)

1. Load datasets  
2. View patient records  
3. Demographic and billing categorization  
4. Healthcare expenditure and service analysis  
5. Sorting experiments (Insertion Sort)  
6. Searching experiments (unsorted linear vs sorted)  
7. Performance analysis  
8. Clinical insights and recommendations  
9. Run full demo  
0. Exit  

## GenAI declaration

AI was used to identify C++ `<chrono>` timing keywords, console table formatting patterns, and to review menu structure. Algorithms, data-structure choices, and analysis logic were checked against the three CSV files and can be explained during evaluation.
