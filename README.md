# DSTR healthcare data-structure comparison

CT077-3-2-DSTR Task 1 compares a manually implemented dynamic array with a manually implemented singly linked list. Each container has a separate C++17 console program. Both programs process the same three facility datasets, calculate healthcare summaries, and compare sorting and searching operations.

## Files and responsibilities

```text
dataset1 facility_a.csv
dataset2 facility_b.csv
dataset3_facility_c.csv
common/
  Patient.hpp
  Patient.cpp
  Reports.hpp
  Application.hpp
array_version/
  main.cpp
  ArrayList.hpp
  ArrayList.cpp
linkedlist_version/
  main.cpp
  LinkedList.hpp
  LinkedList.cpp
  NodeType.hpp
```

The common layer owns patient fields, validation, query rules, report calculations, menu interaction, and experiment orchestration. Each container owns its storage, traversal, sorting, searching, and cleanup. Neither container imports the other container. The two executable entry points select their own container and use the same common application.

Patient storage and sorting/searching algorithms are implemented directly. The programs do not use `std::vector`, `std::list`, `std::map`, `std::array`, or `std::sort`. Standard streams, clocks, numeric/text helpers, and an owning smart pointer for temporary array memory support these implementations.

## Build

Use a C++17 compiler. The shared `common/Patient.cpp` must be compiled into each executable.

From `array_version`, using GCC or MinGW:

```text
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic main.cpp ArrayList.cpp ../common/Patient.cpp -o array_program.exe
```

From `linkedlist_version`:

```text
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic main.cpp LinkedList.cpp ../common/Patient.cpp -o linkedlist_program.exe
```

From a Visual Studio Developer Command Prompt, the corresponding MSVC commands are:

```text
cl /std:c++17 /EHsc /O2 /W4 /D_CRT_SECURE_NO_WARNINGS main.cpp ArrayList.cpp ..\common\Patient.cpp /Fe:array_program.exe
cl /std:c++17 /EHsc /O2 /W4 /D_CRT_SECURE_NO_WARNINGS main.cpp LinkedList.cpp ..\common\Patient.cpp /Fe:linkedlist_program.exe
```

Run each MSVC command from its respective container folder. GCC also builds the programs on Linux/WSL; use an output name without `.exe` and run it with `./`.

## Run

The program accepts one optional dataset directory and one optional noninteractive mode, in either order. With no mode, it opens the menu. If the directory is omitted, it searches the current folder, its parent, and directories obtained from the executable path. Supply the directory explicitly if automatic discovery does not locate the files.

Examples from `array_version`, where `..` contains the datasets:

```text
array_program.exe ..
array_program.exe .. --analysis
array_program.exe .. --benchmark
array_program.exe .. --demo
```

Use `linkedlist_program.exe` with the same arguments for the linked-list program. Quote a dataset directory containing spaces. `--analysis` loads the data and prints healthcare summaries, `--benchmark` runs the experiment matrix, and `--demo` combines the analysis and experiments.

| Menu option | Operation |
|---|---|
| 1 | Load all three datasets. |
| 2 | Display patient records and calculated costs. |
| 3 | Display facility, age-group, and care-type analysis. |
| 4 | Run a sorting experiment. |
| 5 | Run a searching experiment. |
| 6 | Run the full analysis and benchmark demo on the loaded records. |
| 0 | Exit and release container storage. |

In interactive mode, use option 1 before the analysis or experiments. Successful reloads replace the current dataset; selecting Load again does not double the records. A failed load retains the previous records. Invalid menu values receive an error, and end of input exits the program. Noninteractive modes load the datasets automatically.

## Data and calculations

Each supplied CSV has 200 patient records, so the complete input contains 600 records. The six input columns are:

```text
PatientID,Age,CareType,LengthOfStay,BaseCostPerHour,DaysVisitsPerYear
```

The program retains facility identity separately from the CSV fields. `LengthOfStay` is stored as `VisitDurationHours`. Input validation checks the header and column order, complete numeric values, ages from 0 through 100, known care types, bounded text, exactly six fields, and unique patient IDs across all files. The supplied input uses simple comma-separated fields without quoted commas.

```text
Patient cost (MYR) = VisitDurationHours × BaseCostPerHour × DaysVisitsPerYear
Group average cost (MYR) = Sum of patient costs ÷ Patient-record count
```

The implementation rounds each calculated patient cost to the nearest cent before totals, averages, sorting, and cost searches. This currency policy aligns exact-cost searches with the displayed patient cost. Group averages are displayed to two decimal places. Document this rounding policy when comparing results with calculations using unrounded intermediate values.

| Inclusive ages | Assignment group |
|---|---|
| 0–17 | Pediatrics and Adolescents |
| 18–25 | Young Adults / University Students |
| 26–45 | Working Adults, Early Career |
| 46–60 | Working Adults, Late Career |
| 61–100 | Senior Citizens / Geriatric Care |

Console tables show all facilities combined and each of A, B, and C separately. They cover facility totals, age groups, care types, care types within age groups, most requested care, highest billing age groups, and highest patient traffic. Cost tables display MYR. Empty groups show zero counts and `n/a` averages; tied care types are shown separately.

Traffic and preferred care use patient-record counts. They are not weighted by `DaysVisitsPerYear`. Duration summaries use the recorded hours per visit; annual cost uses the formula above. The datasets do not measure waiting time, staffing capacity, or bed occupancy. Any operational recommendations need to acknowledge those limits.

## Sorting and searching

Both versions provide stable insertion sort and iterative merge sort. Supported sorting keys are `id`, `age`, `duration`, `cost`, and `caretype`, in ascending order. Equal keys retain their previous relative order.

Search examples:

```text
id=PT1001
age=42
age=61-100
caretype=Emergency
duration>24
duration>=24
cost>10000
```

An age range includes both endpoints. Text searches use case-insensitive equality. Numeric searches support equality and `>`/`>=` thresholds. Numeric values with trailing text, such as `age=42junk`, are rejected.

Unordered searching scans every record. Ordered array searching finds the start of the matching interval with a binary lower-bound search, then scans the complete matching range. Ordered linked-list searching traverses the sorted chain and can stop after passing the matching interval. Every method counts all matching records and accumulates the same patient-ID checksum; sorting preserves the whole record.

Binary lower-bound searching does not make returning multiple records constant-time: retrieving `k` matches requires processing those `k` records. For a greater-than query, the matching interval continues to the end of the sorted data.

## Experiment method

The full matrix runs separately for all 600 records and for each 200-record facility:

- Insertion sort and merge sort, each using age, duration, and total cost.
- Searches for `age=61-100`, `caretype=Emergency`, and `duration>24`, on an unordered baseline and a separately prepared sorted copy.

Timing uses `std::chrono::steady_clock`. Sorting and merge-sort preparation each use nine individual operation trials, restoring an equivalent fresh copy before every trial. Their median, minimum, and maximum describe those nine operation durations in microseconds.

Search timing uses nine batches of 256 identical queries against the same unchanged input snapshot. Each batch duration is divided by 256 to obtain its mean time per query. The displayed median, minimum, and maximum summarize those nine per-query batch means; they do not describe the distribution of individual query durations. Batching reduces the impact of timer resolution on very short searches. The measurement includes the batch loop and result accumulation used to keep the search results observable. Comparison counts and match counts remain per query, and the accumulated counts/checksums are checked for consistency after each batch.

Input loading, copying/resetting, order verification, and console printing occur outside the measured operation. Merge-sort preparation is timed and reported separately from searching; a first query using the sorted approach incurs preparation plus searching.

The unordered search baseline uses reversed CSV record order with no ordering assumed. Each sorted search uses ascending order for the query key. Match counts and order-independent patient-ID checksums must agree between the baseline and the ordered search. This checksum is a consistency check over patient IDs, not a cryptographic proof that all fields are correct.

Timings describe the machine, compiler, optimization settings, and execution conditions used for that run. These small datasets can produce noisy durations, so retain compiler/build details and the actual measured output when writing the report. Do not treat a single runtime ratio as a universal property of arrays or lists.

## Interpreting operation and memory counters

Comparisons count calls to the record or query comparison function. They are not CPU-instruction counts or individual character comparisons inside a string comparison.

| Metric | Array | Singly linked list |
|---|---|---|
| Container storage estimate | `sizeof(ArrayList) + capacity × sizeof(Patient)` | `sizeof(LinkedList) + count × sizeof(NodeType)` |
| Sorting movements | Patient-record assignments | Pointer-link writes |
| Merge-sort auxiliary heap workspace | `count × sizeof(Patient)` for two or more records | No additional node array |
| Insertion-sort/search auxiliary heap workspace | Zero | Zero |

Array insertion movements include assignment into the held record, shifts, and placement. Array merge movements include assignments during merge passes and any final copy back. Array and list movement counts represent different operations and must not be compared as equal units of work.

`Data bytes` estimates the working container's storage, including allocated but unused array capacity. `Aux heap bytes` includes only explicit dynamic workspace used by the measured algorithm.

`Accounted peak bytes` estimates memory present during the timed operation. Sorting includes the canonical dataset, the working container, and algorithm workspace. Search experiments include the canonical dataset, both unordered and ordered snapshots, and the workspace for the reported operation. The preparation row includes the merge buffer where applicable.

These estimates exclude transient allocations while loading or constructing/resetting snapshots outside the timed operation. They also exclude stack variables, allocator metadata, allocation bookkeeping/alignment overhead, executable/library memory, and process resident memory. The estimates are not the process's full-run peak memory usage. A zero auxiliary-heap result does not mean an algorithm uses no memory. Loading and copying are outside algorithm counters.

## Submission files

Use the assignment's naming and upload rules. The source ZIP should contain the `.cpp`, `.hpp`, and CSV/text files needed to build both programs, with the shared `common` directory retained. Include this guide as `README.txt` in the submission copy if only text files are allowed. Keep generated executables, object files, build folders, editor files, and unrelated artifacts out of that ZIP. Submit the required Word report and team MP4 separately.

Before packaging, extract a clean copy, build both executables from source, load all three datasets, and check the final reports and benchmark output. A working source package does not establish that the separate report, video, workload signatures, declarations, or Moodle submission are complete.

## AI assistance and assignment policy

AI assistance in this revision included code review, implementation/refactoring, and test creation/execution. Describing this revision as formatting-only or syntax-only assistance would be inaccurate.

The previously supplied assignment PDF contains restrictive AI-use rules, including restrictions on generated core solutions. The authoritative version and any lecturer permission remain to be confirmed. Before submitting, check the applicable policy, disclose the actual assistance truthfully, and complete any independently authored rework the lecturer requires. Every member must understand and be able to explain their own submitted contribution. A declaration alone does not establish permission to submit AI-assisted implementation.
