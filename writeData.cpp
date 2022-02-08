// Arquivo com definicao de writeData

#ifdef _WIN32
#include "QpcLoopTimer.h" // ja inclui <windows.h>
#else
#include <windows.h>
#endif
#include "SharedStructs.h" // ja inclui <stdio.h> / <thread> / <mutex> / <vector>
#include <processthreadsapi.h>
#include <iostream>


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


void writeDataStrct(const ThrdStruct &data_struct){
    using namespace std;

    SetThreadPriority(GetCurrentThread(), -1);
    looptimer writeTimer(data_struct.sampletime_);
    
    auto exec_time_micros = data_struct.exectime_*MILLION;
    auto t_begin = writeTimer.micro_now();
    do{
        writeTimer.tik();
        {   // É NECESSÁRIO POR unique_lock em um contexto "{}"!!!
            // sessao critica:
            unique_lock<mutex> lck(*data_struct.mtx_);
            *data_struct.data_ = float(writeTimer.micro_now() - t_begin)/MILLION;
            // dataCond.notify_one();
            // dataCond.wait(lck);
            // fim da sessao critica
        }
        writeTimer.tak(); // faz esperar até passar o tempo de "sample_time"
    } while (writeTimer.micro_now() - t_begin <= exec_time_micros);
}