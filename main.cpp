// C/C++ threads/mutex practice | @qleonardolp, 2022
// Essa ta braaaaba! com processthreadsapi ainda mais braba!!

#ifdef _WIN32
#include "QpcLoopTimer.h" // ja inclui <windows.h>
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <iostream>
#include <processthreadsapi.h>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <time.h>

#define MAIN_RUNTIME 60 // [s]
#define RED_TS 0.004    // [s]
#define WRT_TS 0.001    // [s]
#define PRT_TS 2.000    // [s]

LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;

using namespace std;
// using namespace std::chrono;

float clock_value(0);        // VAR GLOBAL
mutex imuMutex;              // VAR GLOBAL
condition_variable dataCond; // VAR GLOBAL


void  readData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data);
void writeData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data);
void printData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data);

int main(int argc, char* argv[]) 
{
    QueryPerformanceCounter(&StartingTime);
    QueryPerformanceFrequency(&Frequency);
    auto qpcFrequency = double(Frequency.QuadPart);

    int duration;
    if (argc > 0){
        duration = atoi(argv[1]);
    }
    else{
        duration = MAIN_RUNTIME;
    }

    DWORD dwPriority, dwError;
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)){
        dwError = GetLastError();
        printf("Error %lu! \n", dwError);
    } else{
        dwPriority = GetPriorityClass( GetCurrentProcess() );
        printf("Process priority class: %lu \n", dwPriority);
    }

    thread thWriting(writeData,  duration, WRT_TS, std::ref(imuMutex), std::ref(clock_value));
    thread thReading(readData,   duration, RED_TS, std::ref(imuMutex), std::ref(clock_value));
    thread thPrinting(printData, duration, PRT_TS, std::ref(imuMutex), std::ref(clock_value));

    thWriting.join();
    thReading.join();
    thPrinting.join();

    QueryPerformanceCounter(&EndingTime);
    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
    auto qpcElapsedSeconds = double(ElapsedMicroseconds.QuadPart)/qpcFrequency;
    cout << "Elapsed time [s]: " << qpcElapsedSeconds << endl;
    return 0;
}