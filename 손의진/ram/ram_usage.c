#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// RAM 사용량 계산 함수
void write_ram_usage_to_shared_memory(int shmid) {
    FILE *fp;
    unsigned long total, available, mem_free;
    double usage;

    // 공유 메모리 연결
    double *shared_usage = (double *)shmat(shmid, NULL, 0);
    if (shared_usage == (double *)-1) {
        perror("shmat 실패");
        exit(1);
    }

    // /proc/meminfo에서 RAM 데이터 읽기
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    fscanf(fp, "MemTotal: %lu kB\n", &total);
    fscanf(fp, "MemFree: %lu kB\n", &mem_free); // 읽어서 mem_free에 저장
    fscanf(fp, "MemAvailable: %lu kB\n", &available);

    fclose(fp);

    // 사용량 계산
    usage = (double)(total - available) / total * 100.0; // 사용 비율(%)
    *shared_usage = usage; // 공유 메모리에 저장


    // 공유 메모리 해제
    shmdt(shared_usage);
}

int main() {
    // 공유 메모리 생성
    key_t key = ftok("ram_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget 실패");
        exit(1);
    }

    // 주기적으로 RAM 사용량 계산 및 저장
    while (1) {
        write_ram_usage_to_shared_memory(shmid);
        sleep(3); // 3초마다 갱신
    }

    return 0;
}
