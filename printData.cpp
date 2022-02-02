// Arquivo com definicao de printData

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


void printData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data){
    using namespace std;

    SetThreadPriority(GetCurrentThread(), -1);
    looptimer th_timer(sample_time);
    
    const long long int exec_time_micros = exec_time*MILLION;
    const long long int t_begin = th_timer.micro_now();
    do{
        th_timer.tik();
        // É NECESSÁRIO POR unique_lock em um contexto "{}"!!!
        { 
            // sessao critica:
            unique_lock<mutex> lck(mtx);
            cout << "t: " << shared_data << " s\n";
            // fim da sessao critica
        }
        th_timer.tak(); // faz esperar até passar o tempo de "sample_time"
    } while (th_timer.micro_now() - t_begin <= exec_time_micros);
}