// Arquivo com definicao de printData

#ifdef _WIN32
#include "QpcLoopTimer.h" // ja inclui <windows.h>
#else
#include <windows.h>
#endif
#include "SharedStructs.h" // ja inclui <stdio.h> / <thread> / <mutex> / <vector>
#include <processthreadsapi.h>
#include <iostream>


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


void printDataStrct(const ThrdStruct &data_struct){
    using namespace std;

    SetThreadPriority(GetCurrentThread(), data_struct.param00_);
    looptimer th_timer(data_struct.sampletime_);
    
    const long long int exec_time_micros = data_struct.exectime_*MILLION;
    const long long int t_begin = th_timer.micro_now();
    do{
        th_timer.tik();
        // É NECESSÁRIO POR unique_lock em um contexto "{}"!!!
        { 
            // sessao critica:
            unique_lock<mutex> lck( *(data_struct.mtx_) );
            cout << "t: " << *(data_struct.data_) << " s\n";
            // fim da sessao critica
        }
        th_timer.tak(); // faz esperar até passar o tempo de "sample_time"
    } while (th_timer.micro_now() - t_begin <= exec_time_micros);
}
