// File for structs definitions:

#include <stdio.h>
#include <thread>
#include <mutex>
#include <vector>

typedef struct shared_struct {
    float sampletime_;
    int     exectime_;
    float       *data_;
    std::mutex   *mtx_;
} ThrdStruct;