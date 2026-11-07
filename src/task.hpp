#ifndef TASK_HPP
#define TASK_HPP

#include <string>

struct Task {
    std::string id;
    int arrival_time;
    int burst_time;
    int priority;
    int start_time = 0;
    int completion_time = 0;
    int waiting_time = 0;
    int turnaround_time = 0;
};

#endif
