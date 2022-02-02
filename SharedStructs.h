// File for structs definitions:

#include <stdio.h>
#include <condition_variable>
#include <vector>
#include <thread>
#include <mutex>

struct read_struct 
{
    float taks_sample_time;
    int task_execution_time;
    std::reference_wrapper< float > task_shared_data;
    std::reference_wrapper< std::mutex > task_mtx;
    std::reference_wrapper< std::condition_variable > task_cv;
};