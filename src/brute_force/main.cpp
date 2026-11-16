#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <numeric>
#include "task.hpp"
 
using namespace std;
 
struct SimulationResult {
    vector<Task> tasks;
    int makespan;
    double avg_waiting_time;
    double avg_turnaround_time;
};
 
SimulationResult simulate_schedule(const vector<Task>& original_tasks, const vector<int>& perm) {
    int current_time = 0;
    double total_waiting = 0;
    double total_turnaround = 0;
    int max_completion = 0;
 
    vector<Task> scheduled_tasks;
    scheduled_tasks.reserve(perm.size());
 
    for (int idx : perm) {
        Task t = original_tasks[idx];
        if (current_time < t.arrival_time) {
            current_time = t.arrival_time;
        }
        t.start_time = current_time;
        t.completion_time = t.start_time + t.burst_time;
        t.turnaround_time = t.completion_time - t.arrival_time;
        t.waiting_time = t.turnaround_time - t.burst_time;
 
        current_time = t.completion_time;
        if (t.completion_time > max_completion) {
            max_completion = t.completion_time;
        }
 
        total_waiting += t.waiting_time;
        total_turnaround += t.turnaround_time;
        scheduled_tasks.push_back(t);
    }
 
    SimulationResult res;
    res.tasks = scheduled_tasks;
    res.makespan = max_completion;
    res.avg_waiting_time = total_waiting / original_tasks.size();
    res.avg_turnaround_time = total_turnaround / original_tasks.size();
    return res;
}
 
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <workload_filepath>\n";
        return 1;
    }
 
    string filepath = argv[1];
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Could not open workload file: " << filepath << "\n";
        return 1;
    }
 
    vector<Task> tasks;
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        stringstream ss(line);
        string task_id;
        int arrival, burst, priority;
        if (ss >> task_id >> arrival >> burst >> priority) {
            Task t;
            t.id = task_id;
            t.arrival_time = arrival;
            t.burst_time = burst;
            t.priority = priority;
            t.start_time = 0;
            t.completion_time = 0;
            t.waiting_time = 0;
            t.turnaround_time = 0;
            tasks.push_back(t);
        }
    }
    file.close();
 
    size_t n = tasks.size();
    if (n == 0) {
        cerr << "Error: No tasks found in workload file.\n";
        return 1;
    }
 
    if (n > 15) {
        cerr << "Error: Brute force algorithm is unfeasible for N = " << n 
<< " due to combinatorial explosion (N!). Exiting cleanly.\n";
        return 1;
    }
 
    vector<int> perm(n);
    iota(perm.begin(), perm.end(), 0);
 
    auto start_time_chrono = chrono::high_resolution_clock::now();
 
    SimulationResult best_result;
    bool baseline_set = false;
 
    do {
        SimulationResult current_result = simulate_schedule(tasks, perm);
        if (!baseline_set || current_result.avg_waiting_time < best_result.avg_waiting_time) {
            best_result = current_result;
            baseline_set = true;
        }
    } while (next_permutation(perm.begin(), perm.end()));
 
    auto end_time_chrono = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> overhead = end_time_chrono - start_time_chrono;
 
    cout << "--- Brute Force Results ---\n";
    cout << "Input Workload:       " << filepath << "\n";
    cout << "Tasks Scheduled:      " << n << "\n";
    cout << "Schedule Order:       ";
    if (n <= 15) {
        for (size_t i = 0; i < best_result.tasks.size(); ++i) {
            cout << best_result.tasks[i].id;
            if (i + 1 < best_result.tasks.size()) cout << " -> ";
        }
        cout << "\n";
    } else {
        cout << best_result.tasks[0].id << " -> " 
<< best_result.tasks[1].id << " -> " 
<< best_result.tasks[2].id << " -> ... -> " 
<< best_result.tasks[n - 2].id << " -> " 
<< best_result.tasks[n - 1].id << " (" << n << " tasks total)\n";
    }
 
    cout << fixed << setprecision(3);
    cout << "Total Makespan:       " << best_result.makespan << " units\n";
    cout << "Avg Waiting Time:     " << best_result.avg_waiting_time << " units\n";
    cout << "Avg Turnaround Time:  " << best_result.avg_turnaround_time << " units\n";
    cout << "Scheduling Overhead:  " << overhead.count() << " ms\n";
 
    return 0;
}