# Comparative CPU Task Scheduling Optimization

## Project Overview
This project evaluates algorithmic trade-offs across CPU task scheduling paradigms under diverse workload scenarios. The objective is to analyze how different scheduling strategies balance CPU scheduling overhead against execution efficiency, specifically minimizing average waiting time, average turnaround time, and total makespan.

For the full theoretical background and research plan, see the [Project Proposal](docs/proposal.pdf).

- **Inputs:** Task sets defined by Task ID, Arrival Time, Burst/Execution Time, and Priority.
- **Outputs:** An ordered task execution schedule with computed completion, turnaround, and waiting times.
- **Optimization Criteria:** Minimizing average waiting time, average turnaround time, and algorithm decision overhead.

---

## Team Members & Responsibilities
- **Gabriel Holman:** Brute-Force Algorithm implementation.
- **Hunter Clan:** Genetic Algorithm implementation.
- **Caleb Fletcher:** Greedy Algorithm implementation.

---

## Algorithms

### 1. Brute Force
- **Approach:** Evaluates all possible valid execution sequences across the task set to guarantee the absolute globally optimal schedule.
- **Complexity:** Combinatorial explosion, requiring capped problem sizes during benchmarking.

### 2. Genetic Algorithm
- **Approach:** Represents candidate task orders as chromosomes within an evolving population. Applies selection, crossover, and mutation heuristics across successive generations to converge on near-optimal schedules without exhaustive search.

### 3. Greedy Algorithm
- **Approach:** Employs Non-Preemptive Shortest Job First (SJF). At each scheduling boundary, the scheduler dispatches the available arrived task with the minimal burst time (breaking ties by earliest arrival).

---

## Building and Running

### Prerequisites
- C++17 compliant compiler (`g++` or `clang++`)
- GNU `make`

### Build All Executables
- Compile all three algorithms into `bin/`:
```bash
make
```

- To remove compiled binaries:
```bash
make clean
```

### Execution Commands
- Run any individual algorithm by supplying a workload file path:
```bash
./bin/greedy tests/test_inputs/workload_1_baseline_4.txt
./bin/brute_force tests/test_inputs/workload_1_baseline_4.txt
./bin/genetic tests/test_inputs/workload_1_baseline_4.txt
```

---

## Example Input and Output

### Input (tests/test_inputs/workload_1_baseline_4.txt)
```text
# Workload 1: Baseline Workload
# Schema: ID ArrivalTime BurstTime Priority
T1 0 7 2
T2 2 4 1
T3 4 1 3
T4 5 4 2
```

### Output (./bin/greedy tests/test_inputs/workload_1_baseline_4.txt)
```text
--- Greedy Algorithm (Shortest Job First) Results ---
Input Workload:       tests/test_inputs/workload_1_baseline_4.txt
Tasks Scheduled:      4
Schedule Order:       T1 -> T3 -> T2 -> T4
Total Makespan:       16 units
Avg Waiting Time:     4.000 units
Avg Turnaround Time:  8.000 units
Scheduling Overhead:  0.001 ms
```

> **Note on Large Workloads:** For workloads exceeding 15 tasks (such as `workload_5_large_500.txt`), the console output automatically shortens the schedule sequence to keep logs concise while reporting full summary metrics.

---

## Experimental Results
*Work in progress.*

Benchmarking metrics and visualizations will be populated as full experimental trials are completed:
- **Raw benchmark metrics:** `results/raw_data/`
- **Visualizations & comparative plots:** `results/graphs/`

---

## Generative AI Statement
The authors acknowledge the utilization of Gemini, a language model developed by
Google, in the preparation of this assignment. The Gemini model was employed in the
following manner within this assignment: code assistance/review, repository setup assistance, research, brainstorming.