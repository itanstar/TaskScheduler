#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct cpu {
    int user;
    int nice;
    int system;
    int idle;
    int iowait;
    int irq;
    int softirq;
    int steal;
    int guest;
    int guest_nice;
} cpu;

// 공유 메모리에 CPU 사용량 저장
void write_cpu_usage_to_shared_memory(int shmid) {
    FILE *fp;
    cpu cpu_st;
    double total, idle, usage;

    // 공유 메모리 연결
    double *shared_usage = (double *)shmat(shmid, NULL, 0);
    if (shared_usage == (double *)-1) {
        perror("shmat 실패");
        exit(1);
    }

    // /proc/stat에서 CPU 사용량 읽기
    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    fscanf(fp, "cpu %d %d %d %d %d %d %d %d %d %d",
           &cpu_st.user, &cpu_st.nice, &cpu_st.system,
           &cpu_st.idle, &cpu_st.iowait, &cpu_st.irq,
           &cpu_st.softirq, &cpu_st.steal,
           &cpu_st.guest, &cpu_st.guest_nice);

    fclose(fp);

    total = cpu_st.user + cpu_st.nice + cpu_st.system + cpu_st.idle +
            cpu_st.iowait + cpu_st.irq + cpu_st.softirq + cpu_st.steal;
    idle = cpu_st.idle + cpu_st.iowait;

    usage = (1 - (double)idle / total) * 100;
    *shared_usage = usage; // 공유 메모리에 저장


    // 공유 메모리 해제
    shmdt(shared_usage);
}

int main() {
    // 공유 메모리 생성
    key_t key = ftok("cpu_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget 실패");
        exit(1);
    }

    while (1) {
        write_cpu_usage_to_shared_memory(shmid);
        sleep(3); // 3초마다 갱신
    }

    return 0;
}
