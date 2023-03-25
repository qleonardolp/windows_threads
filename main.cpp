// C/C++ threads/mutex practice | @qleonardolp, 2022
// The 'brabest'! ;)

#ifdef _WIN32
#include "QpcLoopTimer.h" // ja inclui <windows.h>
#else
#include <windows.h>
#endif
#include "SharedStructs.h" // ja inclui <stdio.h> / <thread> / <mutex> / <vector>
#include <processthreadsapi.h>
#include <iostream>

#define MAIN_RUNTIME 5 // [s]
// Tasks Sample Time:
#define RED_TS 0.050    // [s]
#define WRT_TS 0.010    // [s]
#define PRT_TS 2.000    // [s]


// Functions Prototypes; Thier definitions are on the proper files.
void  readData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data);
void writeData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data);
void printData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data);
void  readDataStrct(const ThrdStruct &data_struct);
void writeDataStrct(const ThrdStruct &data_struct);
void printDataStrct(const ThrdStruct &data_struct);

int main(int argc, char* argv[]) 
{
    using namespace std;
    // Starting program time counter:
    LARGE_INTEGER StartingTime, EndingTime;
    LARGE_INTEGER Frequency, ElapsedMicroseconds;
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
    if (!SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS)){
        dwError = GetLastError();
        printf("Error %lu! \n", dwError);
    } else{
        dwPriority = GetPriorityClass( GetCurrentProcess() );
        printf("Process priority class: %lu \n", dwPriority);
    }

    cout << "//////////////////////////////////////////\\/////////\\/"   << endl;
    cout << "// INTERFACE DE CONTROLE EXO-TAU  /       /\\     ////\\"   << endl;
    cout << "// EESC-USP                      / _____ ___  ___  //|"     << endl;
    cout << "// RehabLab                     /  | |  | . \\/   \\  /|"   << endl;
    cout << "// *Copyright 2021-2026* \\//// //  | |   \\ \\   |_|  /|"  << endl;
    cout << "//\\///////////////////////\\// //// \\_'_/\\_`_/__|   ///" << endl;
    cout << "///\\///////////////////////\\ //////////////////\\/////\\" << endl;

    mutex imu_mtx;
    float shared_data = 0;
    ThrdStruct w_struct, r_struct, p_struct;

    w_struct.exectime_ = duration;
    w_struct.param00_ =  THREAD_PRIORITY_ABOVE_NORMAL;
    w_struct.sampletime_ = WRT_TS;
    w_struct.data_ = &shared_data;
    w_struct.mtx_ = &imu_mtx;

    r_struct.exectime_ = duration;
    r_struct.param00_ =  THREAD_PRIORITY_NORMAL;
    r_struct.sampletime_ = RED_TS;
    r_struct.data_ = &shared_data;
    r_struct.mtx_ = &imu_mtx;

    p_struct.exectime_ = duration;
    p_struct.param00_ =  THREAD_PRIORITY_LOWEST;
    p_struct.sampletime_ = PRT_TS;
    p_struct.data_ = &shared_data;
    p_struct.mtx_ = &imu_mtx;

    thread thWriting(writeDataStrct, std::ref(w_struct));
    thread thReading(readDataStrct, std::ref(r_struct));
    thread thPrinting(printDataStrct, std::ref(p_struct));

    thWriting.join();
    thReading.join();
    thPrinting.join();

    // Finishing program time counter:
    QueryPerformanceCounter(&EndingTime);
    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
    auto qpcElapsedSeconds = double(ElapsedMicroseconds.QuadPart)/qpcFrequency;
    cout << "Elapsed time [s]: " << qpcElapsedSeconds << endl;
    return 0;
}