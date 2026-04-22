# 🖥️ OS CPU Scheduling Simulator

A **production-quality, console-based CPU Scheduling Simulator** written in C.  
Simulates **6 scheduling algorithms** with step-by-step CPU execution, ASCII Gantt charts, formatted process tables, and a full comparison mode that highlights the best algorithm for your dataset.

---

## 📋 Table of Contents

- [Algorithms](#-algorithms)
- [Features](#-features)
- [Project Structure](#-project-structure)
- [Build & Run](#-build--run)
- [Sample Output](#-sample-output)
- [How It Works](#-how-it-works)
- [Concepts Covered](#-concepts-covered)
- [License](#-license)

---

## ⚙️ Algorithms

| # | Algorithm | Type |
|---|-----------|------|
| 1 | **FCFS** — First Come First Serve | Non-preemptive |
| 2 | **SJF** — Shortest Job First | Non-preemptive |
| 3 | **SRTF** — Shortest Remaining Time First | Preemptive |
| 4 | **Round Robin** — configurable time quantum | Preemptive |
| 5 | **Priority Scheduling** | Non-preemptive |
| 6 | **Priority Scheduling** | Preemptive |

---

## ✅ Features

- Step-by-step CPU execution simulation *(not just formula-based)*
- Idle CPU time handled correctly across all algorithms
- Proper arrival time handling and preemption logic
- **ASCII Gantt chart** rendered with timeline markers
- **Formatted results table** — Completion, Waiting, Turnaround times
- **Comparison mode** — runs all 6 algorithms and highlights the best (★)
- Input validation with friendly error messages
- Reuse process data without re-entering between algorithm runs
- Menu-driven loop — no program restart required

---

## 🗂️ Project Structure
```bash
os-cpu-scheduling-simulator/
├── main.c # Menu-driven entry point
├── scheduler.h # Structs, constants, function prototypes
├── scheduler.c # All 6 scheduling algorithm implementations
├── Makefile # Build configuration
└── README.md
```

### Key Functions

| Function | Description |
|----------|-------------|
| `inputProcesses()` | Reads N processes, validates input, sorts by arrival time |
| `resetProcesses()` | Clears runtime fields so data can be reused across algorithms |
| `copyProcesses()` | Deep-copies process array — originals are never mutated |
| `printGanttChart()` | Renders `\| P1 \| P2 \| IDLE \|` bar chart with timeline |
| `printTable()` | Formatted results table with all 5 output columns |
| `runAllAndCompare()` | Runs all 6 algorithms, prints comparison table with ★ winner |
| `fcfs()` | First Come First Serve |
| `sjf_non_preemptive()` | Shortest burst among arrived processes, no preemption |
| `sjf_preemptive()` | Tick-by-tick shortest remaining time (SRTF) |
| `round_robin()` | Circular queue with configurable quantum |
| `priority_non_preemptive()` | Lowest priority number wins, no preemption |
| `priority_preemptive()` | Lowest priority number wins, preempts on new arrival |

---

## 🚀 Build & Run

### Prerequisites
- GCC (or any C11-compatible compiler)
- Make *(optional)*

### With Make
```bash
git clone https://github.com/your-username/os-cpu-scheduling-simulator.git
cd os-cpu-scheduling-simulator
make        # compile
make run    # compile + run immediately
make debug  # compile with debug symbols (-g)
make clean  # remove build artefacts
```

### Manual Compilation
```bash
gcc -Wall -Wextra -O2 -std=c11 -o scheduler main.c scheduler.c
./scheduler
```

---

## 📊 Sample Output

### Input
```bash
Number of processes: 4

P1 → Arrival: 0 Burst: 8 Priority: 2
P2 → Arrival: 1 Burst: 4 Priority: 1
P3 → Arrival: 2 Burst: 9 Priority: 3
P4 → Arrival: 3 Burst: 5 Priority: 2
```

### FCFS Output
```bash
Gantt Chart:
──────────────────────────────────────────────────────────
| P1 | P2 | P3 | P4 |
──────────────────────────────────────────────────────────
0 8 12 21 26

┌──────┬─────────┬───────┬────────────┬─────────┬────────────────┐
│ PID │ Arrival │ Burst │ Completion │ Waiting │ Turnaround │
├──────┼─────────┼───────┼────────────┼─────────┼────────────────┤
│ P1 │ 0 │ 8 │ 8 │ 0 │ 8 │
│ P2 │ 1 │ 4 │ 12 │ 7 │ 11 │
│ P3 │ 2 │ 9 │ 21 │ 10 │ 19 │
│ P4 │ 3 │ 5 │ 26 │ 18 │ 23 │
└──────┴─────────┴───────┴────────────┴─────────┴────────────────┘
Average Waiting Time : 8.75
Average Turnaround Time : 15.25
```
### SRTF Output
```bash
Gantt Chart:
──────────────────────────────────────────────────────────────
| P1 | P2 | P4 | P1 | P3 |
──────────────────────────────────────────────────────────────────
0 1 5 10 17 26
```

### Comparison Mode
```bash
╔══════════════════════════════════════════════════════════════════╗
║ ALGORITHM COMPARISON SUMMARY ║
╠══════════════════════════════╦═══════════════╦═════════════════╣
║ Algorithm ║ Avg WT ║ Avg TAT ║
╠══════════════════════════════╬═══════════════╬═════════════════╣
║ FCFS ║ 8.75 ║ 15.25 ║
║ SJF Non-Preemptive ║ 7.75 ║ 14.25 ║
║ SRTF (SJF Preemptive) ║ 6.50 ║ 13.00 ║ ★ BEST
║ Round Robin (Q=2) ║ 12.75 ║ 19.25 ║
║ Priority Non-Preemptive ║ 7.75 ║ 14.25 ║
║ Priority Preemptive ║ 7.00 ║ 13.50 ║
╚══════════════════════════════╩═══════════════╩═════════════════╝

★ Best algorithm for this dataset: SRTF (SJF Preemptive)
(Lowest Average Waiting Time: 6.50)
```

---

## 🧠 How It Works

### Simulation Approach
All algorithms simulate CPU execution **tick-by-tick** or **event-by-event** rather than computing results from formulas alone. This means:
- The CPU advances through time units
- At each decision point, the scheduler selects the next process based on the algorithm's policy
- Preemption is detected the moment a higher-priority / shorter-remaining-time process arrives
- Idle periods are inserted whenever no process has arrived yet

### Data Flow
```bash
inputProcesses() → sorted Process[] → algorithm(copy) → GanttSegment]
→ printGanttChart()
→ printTable()
→ printAverages()
```

The original `Process[]` array is **never mutated**. Each algorithm receives a deep copy via `copyProcesses()` and a clean state via `resetProcesses()`, so you can run different algorithms on the same dataset without re-entering data.

### Round Robin Queue
The RR implementation uses a flat integer index array as a circular queue (`front` / `rear` pointers — no dynamic memory allocation). Newly arrived processes are enqueued **after** the current process uses its quantum but **before** the current process re-enters the back of the queue, matching standard RR semantics.

---

## 📚 Concepts Covered

| Concept | Implementation |
|---------|---------------|
| Process struct with runtime fields | `scheduler.h → Process` |
| Gantt chart segment struct | `scheduler.h → GanttSegment` |
| Circular queue (RR) | `scheduler.c → round_robin()` |
| Preemption logic | `sjf_preemptive()`, `priority_preemptive()` |
| Idle CPU gaps | All algorithms check for arrival gaps |
| qsort with comparators | `cmpArrival()` for arrival-order sorting |
| Modular C design | Separate `.h`, `.c`, `main.c` |
| No global variables | All state passed through parameters |

---

> Built as a systems programming portfolio project demonstrating OS scheduling concepts in standard C11.