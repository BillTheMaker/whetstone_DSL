#pragma once
#include <string>

namespace parsed_python_module {

class WorkItem {
public:
    std::string job_id;
    int priority;
    std::string payload;

    WorkItem(std::string job_id, int priority, std::string payload);
};
class PriorityQueue {
public:
    std::vector<std::string> items;

    PriorityQueue();
    void enqueue(WorkItem item);
    WorkItem dequeue();
    WorkItem peek();
    int size();
    bool empty();
};

}  // namespace parsed_python_module

