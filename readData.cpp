// Arquivo com definicao de das funcoes de "read"

#ifdef _WIN32
#include "QpcLoopTimer.h" // ja inclui <windows.h>
#else
#include <windows.h>
#endif
#include <iostream>
#include "SharedStructs.h" // ja inclui <stdio.h> / <thread> / <mutex> / <vector>
#include <processthreadsapi.h>
#include <Eigen/Core>
#include <Eigen/LU>

typedef Eigen::Matrix<float, 9, 9> StressMtx;

int GetRandom(int max){
    srand(time(NULL));
    return rand() % max;
}

StressMtx AddIdentityNoise(StressMtx in_mtx){
    return in_mtx += StressMtx::Identity()*float(GetRandom(100) / 50);
}

void readData(const int exec_time, const float sample_time, std::mutex &mtx, float &shared_data){
    using namespace std;
    using namespace Eigen;

    SetThreadPriority(GetCurrentThread(), -2);
    looptimer readTimer(sample_time);

    FILE* pLogFile;
    char filename[] = "ReadThread001Log.txt";
    pLogFile = fopen(filename, "w");
    if (pLogFile != NULL) fclose(pLogFile);

    // Stress test using Eigen:
    StressMtx stateSpaceMtx = StressMtx::Random();
    stateSpaceMtx += 0.5*StressMtx::Identity();

    auto exec_time_micros = exec_time*MILLION;
    auto t_begin = readTimer.micro_now();
    do{
        readTimer.tik();

        FullPivLU<StressMtx> Mtx2Inv(stateSpaceMtx);
        StressMtx MtxInverted = StressMtx::Identity();
        if (Mtx2Inv.isInvertible()){
            MtxInverted = Mtx2Inv.inverse();
        }
        stateSpaceMtx = AddIdentityNoise(stateSpaceMtx);

        pLogFile = fopen(filename, "a");
        if (pLogFile != NULL){
            unique_lock<mutex> lck(mtx); // É NECESSÁRIO POR unique_lock em um contexto "{}"!!!
            // dataCond.notify_one();
            fprintf(pLogFile, "Tempo: %.4f  M55: %.6f\n", shared_data, MtxInverted(4,4) );
            fclose(pLogFile);
        }
        readTimer.tak(); // faz esperar até passar o tempo de "sample_time"
    } while (readTimer.micro_now() - t_begin <= exec_time_micros);
}


void readDataStrct(const ThrdStruct &data_struct){
    using namespace std;
    using namespace Eigen;

    SetThreadPriority(GetCurrentThread(), -2);
    looptimer readTimer(data_struct.sampletime_);

    FILE* pLogFile;
    char filename[] = "ReadThread001Log.txt";
    pLogFile = fopen(filename, "w");
    if (pLogFile != NULL) fclose(pLogFile);

    // Stress test using Eigen:
    StressMtx stateSpaceMtx = StressMtx::Random();
    stateSpaceMtx += 0.5*StressMtx::Identity();

    auto exec_time_micros = data_struct.exectime_*MILLION;
    auto t_begin = readTimer.micro_now();
    do{
        readTimer.tik();

        FullPivLU<StressMtx> Mtx2Inv(stateSpaceMtx);
        StressMtx MtxInverted = StressMtx::Identity();
        if (Mtx2Inv.isInvertible()){
            MtxInverted = Mtx2Inv.inverse();
        }
        stateSpaceMtx = AddIdentityNoise(stateSpaceMtx);

        pLogFile = fopen(filename, "a");
        if (pLogFile != NULL){
            unique_lock<mutex> lck(*data_struct.mtx_); // É NECESSÁRIO POR unique_lock em um contexto "{}"!!!
            // dataCond.notify_one();
            fprintf(pLogFile, "Tempo: %.4f  M55: %.6f\n", *data_struct.data_, MtxInverted(4,4) );
            fclose(pLogFile);
        }
        readTimer.tak(); // faz esperar até passar o tempo de "sample_time"
    } while (readTimer.micro_now() - t_begin <= exec_time_micros);
}