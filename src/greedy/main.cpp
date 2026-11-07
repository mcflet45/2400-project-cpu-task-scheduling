#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include "task.hpp"

using namespace std;
using namespace std::chrono;

// Read tasks line by line from the input text file
vector<Task> load_tasks(const string& filepath) 
{
    vector<Task> tasks;
    ifstream file(filepath);

    if (!file.is_open()) 
    {
        cerr << "Error: Could not open input file: " << filepath << "\n";
        return tasks;
    }

    string line;
    while (getline(file, line)) 
    {
        // Skip empty lines or comment headers
        if (line.empty() || line[0] == '#') 
        {
            continue;
        }
        // Expected format: TaskID ArrivalTime BurstTime Priority
        stringstream ss(line);
        Task t;
        if (ss >> t.id >> t.arrival_time >> t.burst_time >> t.priority) 
        {
            tasks.push_back(t);
        }
    }

    file.close();
    return tasks;
}

// Greedy Heuristic: Shortest Job First (Non-Preemptive)
vector<Task> run_greedy_scheduler(vector<Task> tasks) 
{
    vector<Task> scheduled_order;

    // Track which tasks have finished
    vector<bool> completed(tasks.size(), false);
    
    int current_time = 0;
    size_t completed_count = 0;
    int total_tasks = tasks.size();

    while (completed_count < tasks.size()) 
    {
        int best_idx = -1;

        // Look for tasks that have arrived and pick the shortest burst time
        for (int i = 0; i < total_tasks; i++) 
        {
            if (!completed[i] && tasks[i].arrival_time <= current_time) 
            {
                if (best_idx == -1) 
                {
                    best_idx = i;
                } 
                else if (tasks[i].burst_time < tasks[best_idx].burst_time) 
                {
                    best_idx = i;
                } 
                else if (tasks[i].burst_time == tasks[best_idx].burst_time) 
                {
                    // Tie breaker: pick earlier arrival
                    if (tasks[i].arrival_time < tasks[best_idx].arrival_time) 
                    {
                        best_idx = i;
                    }
                }
            }
        }

        // If CPU is idle (no task has arrived yet), skip current_time ahead to the next arrival
        if (best_idx == -1) 
        {
            int next_arrival = 999999;
            for (int i = 0; i < total_tasks; i++) 
            {
                if (!completed[i] && tasks[i].arrival_time < next_arrival) 
                {
                    next_arrival = tasks[i].arrival_time;
                }
            }
            current_time = next_arrival;
            continue;
        }

        // Schedule the chosen task
        Task chosen = tasks[best_idx];
        chosen.start_time = current_time;
        chosen.completion_time = current_time + chosen.burst_time;

        // Turnaround = completion - arrival; Waiting = turnaround - burst
        chosen.turnaround_time = chosen.completion_time - chosen.arrival_time;
        chosen.waiting_time = chosen.turnaround_time - chosen.burst_time;

        current_time = chosen.completion_time;
        completed[best_idx] = true;
        completed_count++;

        scheduled_order.push_back(chosen);
    }

    return scheduled_order;
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) 
    {
        cerr << "Usage: " << argv[0] << " <path_to_workload_file>\n";
        return 1;
    }

    string input_path = argv[1];
    vector<Task> tasks = load_tasks(input_path);

    if (tasks.empty()) 
    {
        cerr << "Error: No valid tasks loaded.\n";
        return 1;
    }

    // Benchmark the algorithm's decision overhead
    auto start_clock = high_resolution_clock::now();
    vector<Task> scheduled = run_greedy_scheduler(tasks);
    auto end_clock = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(end_clock - start_clock);
    double overhead_ms = duration.count() / 1000.0;

    // Compute evaluation metrics
    double total_wait = 0.0;
    double total_turnaround = 0.0;
    int makespan = 0;

    for (int i = 0; i < (int)scheduled.size(); i++) 
    {
        total_wait += scheduled[i].waiting_time;
        total_turnaround += scheduled[i].turnaround_time;
        if (scheduled[i].completion_time > makespan) 
        {
            makespan = scheduled[i].completion_time;
        }
    }

    double avg_wait = total_wait / scheduled.size();
    double avg_turnaround = total_turnaround / scheduled.size();

    // Console output for direct verification
    cout << "--- Greedy Algorithm (Shortest Job First) Results ---\n";
    cout << "Input Workload:       " << input_path << "\n";
    cout << "Tasks Scheduled:      " << scheduled.size() << "\n";

    cout << "Schedule Order:       ";
    if (scheduled.size() <= 15) 
    {
        for (int i = 0; i < (int)scheduled.size(); i++) 
        {
            cout << scheduled[i].id;
            if (i + 1 < (int)scheduled.size()) 
            {
                cout << " -> ";
            } 
            else 
            {
                cout << "\n";
            }
        }
    } 
    else 
    {
        cout << scheduled[0].id << " -> " 
             << scheduled[1].id << " -> " 
             << scheduled[2].id << " -> ... -> " 
             << scheduled[scheduled.size() - 2].id << " -> " 
             << scheduled[scheduled.size() - 1].id 
             << " (" << scheduled.size() << " tasks total)\n";
    }

    cout << fixed << setprecision(3);
    cout << "Total Makespan:       " << makespan << " units\n";
    cout << "Avg Waiting Time:     " << avg_wait << " units\n";
    cout << "Avg Turnaround Time:  " << avg_turnaround << " units\n";
    cout << "Scheduling Overhead:  " << overhead_ms << " ms\n";

    return 0;
}