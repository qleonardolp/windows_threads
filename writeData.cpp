// Arquivo com definicao de writeData

#ifdef _WIN32
#include "QpcLoopTimer.h" // ja inclui <windows.h>
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <time.h>
#include <processthreadsapi.h>


void writeData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data){
    using namespace std;

    SetThreadPriority(GetCurrentThread(), -1);
    looptimer writeTimer(sample_time);
    
    auto exec_time_micros = exec_time*MILLION;
    auto t_begin = writeTimer.micro_now();
    do{
        writeTimer.tik();
        {   // É NECESSÁRIO POR unique_lock em um contexto "{}"!!!
            // sessao critica:
            unique_lock<mutex> lck(mtx);
            shared_data = float(writeTimer.micro_now() - t_begin)/MILLION;
            // dataCond.notify_one();
            // dataCond.wait(lck);
            // fim da sessao critica
        }
        writeTimer.tak(); // faz esperar até passar o tempo de "sample_time"
    } while (writeTimer.micro_now() - t_begin <= exec_time_micros);
}