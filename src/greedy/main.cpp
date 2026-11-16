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
vector<Task> loadTasks(const string& filepath) 
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
vector<Task> runGreedyScheduler(vector<Task> tasks) 
{
    vector<Task> scheduledOrder;

    // Track which tasks have finished
    vector<bool> completed(tasks.size(), false);
    
    int currentTime = 0;
    size_t completedCount = 0;
    int totalTasks = tasks.size();

    while (completedCount < tasks.size()) 
    {
        int bestIdx = -1;

        // Look for tasks that have arrived and pick the shortest burst time
        for (int i = 0; i < totalTasks; i++) 
        {
            if (!completed[i] && tasks[i].arrival_time <= currentTime) 
            {
                if (bestIdx == -1) 
                {
                    bestIdx = i;
                } 
                else if (tasks[i].burst_time < tasks[bestIdx].burst_time) 
                {
                    bestIdx = i;
                } 
                else if (tasks[i].burst_time == tasks[bestIdx].burst_time) 
                {
                    // Tie breaker: pick earlier arrival
                    if (tasks[i].arrival_time < tasks[bestIdx].arrival_time) 
                    {
                        bestIdx = i;
                    }
                }
            }
        }

        // If CPU is idle (no task has arrived yet), skip currentTime ahead to the next arrival
        if (bestIdx == -1) 
        {
            int nextArrival = 999999;
            for (int i = 0; i < totalTasks; i++) 
            {
                if (!completed[i] && tasks[i].arrival_time < nextArrival) 
                {
                    nextArrival = tasks[i].arrival_time;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        // Schedule the chosen task
        Task chosen = tasks[bestIdx];
        chosen.start_time = currentTime;
        chosen.completion_time = currentTime + chosen.burst_time;

        // Turnaround = completion - arrival; Waiting = turnaround - burst
        chosen.turnaround_time = chosen.completion_time - chosen.arrival_time;
        chosen.waiting_time = chosen.turnaround_time - chosen.burst_time;

        currentTime = chosen.completion_time;
        completed[bestIdx] = true;
        completedCount++;

        scheduledOrder.push_back(chosen);
    }

    return scheduledOrder;
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) 
    {
        cerr << "Usage: " << argv[0] << " <path_to_workload_file>\n";
        return 1;
    }

    string inputPath = argv[1];
    vector<Task> tasks = loadTasks(inputPath);

    if (tasks.empty()) 
    {
        cerr << "Error: No valid tasks loaded.\n";
        return 1;
    }

    // Benchmark the algorithm's decision overhead
    auto startClock = high_resolution_clock::now();
    vector<Task> scheduled = runGreedyScheduler(tasks);
    auto endClock = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(endClock - startClock);
    double overheadMs = duration.count() / 1000.0;

    // Compute evaluation metrics
    double totalWait = 0.0;
    double totalTurnaround = 0.0;
    int makespan = 0;

    for (int i = 0; i < (int)scheduled.size(); i++) 
    {
        totalWait += scheduled[i].waiting_time;
        totalTurnaround += scheduled[i].turnaround_time;
        if (scheduled[i].completion_time > makespan) 
        {
            makespan = scheduled[i].completion_time;
        }
    }

    double avgWait = totalWait / scheduled.size();
    double avgTurnaround = totalTurnaround / scheduled.size();

    // Console output for direct verification
    cout << "--- Greedy Algorithm (Shortest Job First) Results ---\n";
    cout << "Input Workload:       " << inputPath << "\n";
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
    cout << "Avg Waiting Time:     " << avgWait << " units\n";
    cout << "Avg Turnaround Time:  " << avgTurnaround << " units\n";
    cout << "Scheduling Overhead:  " << overheadMs << " ms\n";

    return 0;
}