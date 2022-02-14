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
#include <unsupported/Eigen/MatrixFunctions>

typedef Eigen::Matrix<float, 9, 9> StressMtx;
#define Id3 Eigen::Matrix3f::Identity()

float controle_lpshap(const float input, const float smpl_time, float buffer[10]); // loop-shaping

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

    SetThreadPriority(GetCurrentThread(), data_struct.param00_);
    looptimer readTimer(data_struct.sampletime_);

    FILE* pLogFile;
    char filename[] = "ReadThread001Log.txt";
    pLogFile = fopen(filename, "w");
    if (pLogFile != NULL) fclose(pLogFile);

    float lp_buffer[10];
    for (size_t i = 0; i < 10; i++)
    {
        lp_buffer[i] = 0;
    }
    lp_buffer[9] = data_struct.sampletime_;

    // Stress test using Eigen:
    StressMtx stateSpaceMtx = StressMtx::Random();
    stateSpaceMtx += 0.5*StressMtx::Identity();

    auto exec_time_micros = data_struct.exectime_*MILLION;
    auto t_begin = readTimer.micro_now();
    do{
        readTimer.tik();

        int input = (readTimer.micro_now() - t_begin > 3*MILLION); // step input
        float control_u = controle_lpshap(lp_buffer[8], data_struct.sampletime_, lp_buffer);
        // pseudo dynamics:
        lp_buffer[8] = input*(1 - 0.72*lp_buffer[8] - 0.28*control_u);

        pLogFile = fopen(filename, "a");
        if (pLogFile != NULL){
            unique_lock<mutex> lck(*data_struct.mtx_); // É NECESSÁRIO POR unique_lock em um contexto "{}"!!!
            // dataCond.notify_one();
            fprintf(pLogFile, "Tempo: %.4f  M55: %.6f\n", *data_struct.data_, control_u );
            fclose(pLogFile);
        }
        readTimer.tak(); // faz esperar até passar o tempo de "sample_time"
    } while (readTimer.micro_now() - t_begin <= exec_time_micros);
}

float controle_lpshap(const float input, const float smpl_time, float buffer[10])
{
    using namespace Eigen;

    float a0 = 1;
    float b0 = 0.0/a0;
    float b1 = 10/a0;
    float b2 = 8.0/a0;
    float b3 = 1.6/a0;
    float a1 = 0.2/a0;
    float a2 = 0.0/a0;
    float a3 = 0.0/a0;

    // Def.:
    //        b0 s^3 + b1 s^2 + b2 s + b3
    // FT = --------------------------------, with a0 = 1!
    //        a0 s^3 + a1 s^2 + a2 s + a3
    // Forma Canonica Controlavel | (Ogata pg. 596):
    Matrix3f A; A << 0, 1, 0, 0, 0, 1, -a3, -a2, -a1;
    Vector3f B; B << 0, 0, 1;
    RowVector3f C; C << b3 - a3*b0, b2 - a2*b0, b1 - a1*b0;
    float D = b0;
    // Discretizacao 2 Ord:
    float Ts = buffer[9];
    Matrix3f Ak = Id3 + A*Ts + (A*Ts).pow(2)/2;
    Vector3f Bk = (Id3 + A*Ts/2 + (A*Ts).pow(2)/6)*B*Ts;

    Vector3f xk; xk << buffer[0], buffer[1], buffer[2];
    xk = Ak*xk + Bk*input;
    float yk = C*xk + D*input;
    buffer[0] = xk(0);
    buffer[1] = xk(1);
    buffer[2] = xk(2);

    //return input.mtr_rgtknee_tau + Jr*input.hum_rgtknee_acc + yk; // FF+FB
    return yk;
}