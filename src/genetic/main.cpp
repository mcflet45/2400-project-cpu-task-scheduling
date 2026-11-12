#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <random>
#include <numeric>
#include "task.hpp"

using namespace std;
using namespace std::chrono;

// GA Hyperparameters (Documented as required)
const int POPULATION_SIZE = 100;
const int GENERATIONS = 200;
const double MUTATION_RATE = 0.05;
const unsigned int RANDOM_SEED = 42; // Fixed seed for reproducible benchmarks

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

// Evaluation structure to hold a chromosome and its performance metrics
struct Individual 
{
    vector<int> chromosome; // Permutation of task indices
    vector<Task> scheduled_tasks;
    int makespan = 0;
    double avg_wait = 0.0;
    double avg_turnaround = 0.0;
    double fitness = 0.0;
};

// Decode chromosome into a valid non-preemptive schedule respecting arrival times and CPU idle gaps
Individual decode_schedule(const vector<Task>& original_tasks, const vector<int>& chromosome) 
{
    vector<Task> tasks = original_tasks;
    vector<bool> completed(tasks.size(), false);
    vector<Task> scheduled_order;
    
    int current_time = 0;
    size_t completed_count = 0;

    while (completed_count < tasks.size()) 
    {
        int chosen_idx = -1;

        // Find the first task in chromosome order that has arrived and not completed
        for (size_t i = 0; i < chromosome.size(); ++i) 
        {
            int idx = chromosome[i];
            if (!completed[idx] && tasks[idx].arrival_time <= current_time) 
            {
                chosen_idx = idx;
                break;
            }
        }

        // If CPU is idle, advance time to the next available task arrival
        if (chosen_idx == -1) 
        {
            int next_arrival = 999999;
            for (size_t i = 0; i < tasks.size(); ++i) 
            {
                if (!completed[i] && tasks[i].arrival_time < next_arrival) 
                {
                    next_arrival = tasks[i].arrival_time;
                }
            }
            if (next_arrival == 999999) break;
            current_time = next_arrival;
            continue;
        }

        Task chosen = tasks[chosen_idx];
        chosen.start_time = current_time;
        chosen.completion_time = current_time + chosen.burst_time;
        chosen.turnaround_time = chosen.completion_time - chosen.arrival_time;
        chosen.waiting_time = chosen.turnaround_time - chosen.burst_time;

        current_time = chosen.completion_time;
        completed[chosen_idx] = true;
        completed_count++;
        scheduled_order.push_back(chosen);
    }

    Individual ind;
    ind.chromosome = chromosome;
    ind.scheduled_tasks = scheduled_order;

    double total_wait = 0.0;
    double total_turnaround = 0.0;
    int makespan = 0;

    for (const auto& t : scheduled_order) 
    {
        total_wait += t.waiting_time;
        total_turnaround += t.turnaround_time;
        if (t.completion_time > makespan) 
        {
            makespan = t.completion_time;
        }
    }

    ind.makespan = makespan;
    ind.avg_wait = scheduled_order.empty() ? 0.0 : total_wait / scheduled_order.size();
    ind.avg_turnaround = scheduled_order.empty() ? 0.0 : total_turnaround / scheduled_order.size();

    // Fitness function: Lower makespan and waiting time yield higher fitness
    ind.fitness = 1.0 / (1.0 + ind.makespan + ind.avg_wait);
    return ind;
}

// Order Crossover (OX1) for permutation representation
vector<int> crossover(const vector<int>& parent1, const vector<int>& parent2, mt19937& rng) 
{
    size_t size = parent1.size();
    if (size <= 2) return parent1;

    uniform_int_distribution<size_t> dist(0, size - 1);
    size_t start = dist(rng);
    size_t end = dist(rng);
    if (start > end) swap(start, end);

    vector<int> child(size, -1);
    for (size_t i = start; i <= end; ++i) 
    {
        child[i] = parent1[i];
    }

    size_t p2_idx = 0;
    for (size_t i = 0; i < size; ++i) 
    {
        size_t curr_idx = (end + 1 + i) % size;
        while (p2_idx < size && find(child.begin(), child.end(), parent2[p2_idx]) != child.end()) 
        {
            p2_idx++;
        }
        if (p2_idx < size) 
        {
            child[curr_idx] = parent2[p2_idx++];
        }
    }

    return child;
}

// Swap Mutation operator
void mutate(vector<int>& chrom, double mutation_rate, mt19937& rng) 
{
    uniform_real_distribution<double> dist_prob(0.0, 1.0);
    uniform_int_distribution<size_t> dist_idx(0, chrom.size() - 1);

    for (size_t i = 0; i < chrom.size(); ++i) 
    {
        if (dist_prob(rng) < mutation_rate) 
        {
            size_t swap_idx = dist_idx(rng);
            swap(chrom[i], chrom[swap_idx]);
        }
    }
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

    // Benchmark algorithm decision overhead using std::chrono
    auto start_clock = high_resolution_clock::now();

    mt19937 rng(RANDOM_SEED);
    vector<int> base_indices(tasks.size());
    iota(base_indices.begin(), base_indices.end(), 0);

    // Initialize population
    vector<Individual> population;
    population.reserve(POPULATION_SIZE);
    for (int i = 0; i < POPULATION_SIZE; ++i) 
    {
        vector<int> chrom = base_indices;
        shuffle(chrom.begin(), chrom.end(), rng);
        population.push_back(decode_schedule(tasks, chrom));
    }

    // Sort initial population by fitness descending
    sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
        return a.fitness > b.fitness;
    });

    Individual best_overall = population[0];

    // Evolution loop over generations
    for (int gen = 0; gen < GENERATIONS; ++gen) 
    {
        vector<Individual> new_generation;
        new_generation.reserve(POPULATION_SIZE);

        // Elitism: retain top 5%
        int elite_count = max(1, POPULATION_SIZE / 20);
        for (int i = 0; i < elite_count; ++i) 
        {
            new_generation.push_back(population[i]);
        }

        uniform_int_distribution<int> tourn_dist(0, POPULATION_SIZE / 2);

        while (new_generation.size() < (size_t)POPULATION_SIZE) 
        {
            // Tournament selection
            int p1_idx = tourn_dist(rng);
            int p2_idx = tourn_dist(rng);
            const vector<int>& parent1 = population[p1_idx].chromosome;
            const vector<int>& parent2 = population[p2_idx].chromosome;

            vector<int> child_chrom = crossover(parent1, parent2, rng);
            mutate(child_chrom, MUTATION_RATE, rng);

            Individual child = decode_schedule(tasks, child_chrom);
            new_generation.push_back(child);
        }

        population = std::move(new_generation);
        sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
            return a.fitness > b.fitness;
        });

        if (population[0].fitness > best_overall.fitness) 
        {
            best_overall = population[0];
        }
    }

    auto end_clock = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end_clock - start_clock);
    double overhead_ms = duration.count() / 1000.0;

    // Console output matching project formatting specifications
    cout << "--- Genetic Algorithm Results ---\n";
    cout << "Input Workload:       " << input_path << "\n";
    cout << "Tasks Scheduled:      " << best_overall.scheduled_tasks.size() << "\n";

    const auto& scheduled = best_overall.scheduled_tasks;
    cout << "Schedule Order:       ";
    if (scheduled.size() <= 15) 
    {
        for (size_t i = 0; i < scheduled.size(); ++i) 
        {
            cout << scheduled[i].id;
            if (i + 1 < scheduled.size()) cout << " -> ";
            else cout << "\n";
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
    cout << "Total Makespan:       " << best_overall.makespan << " units\n";
    cout << "Avg Waiting Time:     " << best_overall.avg_wait << " units\n";
    cout << "Avg Turnaround Time:  " << best_overall.avg_turnaround << " units\n";
    cout << "Scheduling Overhead:  " << overhead_ms << " ms\n";

    return 0;
}