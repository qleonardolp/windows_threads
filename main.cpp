// C/C++ threads/mutex practice | @qleonardolp, 2022
// The 'brabest'!

#ifdef _WIN32
#include "QpcLoopTimer.h" // ja inclui <windows.h>
#else
#include <windows.h>
#endif
#include "SharedStructs.h" // ja inclui <stdio.h> / <thread> / <mutex> / <vector>
#include <condition_variable>
#include <processthreadsapi.h>
#include <iostream>
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
void readDataStrct(const ThrdStruct &data_struct);
void writeDataStrct(const ThrdStruct &data_struct);
void printDataStrct(const ThrdStruct &data_struct);

int main(int argc, char* argv[]) 
{
    QueryPerformanceCounter(&StartingTime);
    QueryPerformanceFrequency(&Frequency);
    auto qpcFrequency = double(Frequency.QuadPart);

    int duration;
    if (argc >= 2){
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

    ThrdStruct w_struct, r_struct, p_struct;

    w_struct.exectime_ = duration;
    w_struct.sampletime_ = WRT_TS;
    w_struct.data_ = &clock_value;
    w_struct.mtx_ = &imuMutex;

    r_struct.exectime_ = duration;
    r_struct.sampletime_ = RED_TS;
    r_struct.data_ = &clock_value;
    r_struct.mtx_ = &imuMutex;

    p_struct.exectime_ = duration;
    p_struct.sampletime_ = PRT_TS;
    p_struct.data_ = &clock_value;
    p_struct.mtx_ = &imuMutex;


    // thread thWriting(writeData,  duration, WRT_TS, std::ref(imuMutex), std::ref(clock_value));
    // thread thReading(readData,   duration, RED_TS, std::ref(imuMutex), std::ref(clock_value));
    // thread thPrinting(printData, duration, PRT_TS, std::ref(imuMutex), std::ref(clock_value));
    thread thWriting(writeDataStrct, std::ref(w_struct));
    thread thReading(readDataStrct, std::ref(r_struct));
    thread thPrinting(printDataStrct, std::ref(p_struct));

    thWriting.join();
    thReading.join();
    thPrinting.join();

    QueryPerformanceCounter(&EndingTime);
    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
    auto qpcElapsedSeconds = double(ElapsedMicroseconds.QuadPart)/qpcFrequency;
    cout << "Elapsed time [s]: " << qpcElapsedSeconds << endl;
    return 0;
}