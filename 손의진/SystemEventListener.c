#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

// CPU 사용량 계산 및 공유 메모리에 저장
void write_cpu_usage_to_shared_memory(int shmid) {
    FILE *fp;
    int total, idle, usage;
    int user, nice, system, iowait, irq, softirq, steal;

    double *shared_usage = (double *)shmat(shmid, NULL, 0);
    if (shared_usage == (double *)-1) {
        perror("shmat 실패");
        exit(1);
    }

    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    fscanf(fp, "cpu %d %d %d %d %d %d %d %d",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    fclose(fp);

    total = user + nice + system + idle + iowait + irq + softirq + steal;
    idle = idle + iowait;

    usage = (1 - (double)idle / total) * 100;
    *shared_usage = usage;

    shmdt(shared_usage);
}

// RAM 사용량 계산 및 공유 메모리에 저장
void write_ram_usage_to_shared_memory(int shmid) {
    FILE *fp;
    int total, available, free;
    double usage;

    double *shared_usage = (double *)shmat(shmid, NULL, 0);
    if (shared_usage == (double *)-1) {
        perror("shmat 실패");
        exit(1);
    }

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    fscanf(fp, "MemTotal: %d kB\nMemFree:%d kB\nMemAvailable: %d kB\n", &total, &free, &available);
    fclose(fp);

    usage = (((double)total - (double)available) / (double)total) * 100;
    *shared_usage = usage;

    shmdt(shared_usage);
}

// Disk 사용량 계산 및 공유 메모리에 저장
void write_disk_usage_to_shared_memory(int shmid) {
    FILE *fp;
    double total, used, usage;

    double *shared_usage = (double *)shmat(shmid, NULL, 0);
    if (shared_usage == (double *)-1) {
        perror("shmat 실패");
        exit(1);
    }

    fp = popen("df --total | tail -1 | awk '{print $2, $3}'", "r");
    if (fp == NULL) {
        perror("popen");
        return;
    }

    fscanf(fp, "%lf %lf", &total, &used);
    pclose(fp);

    usage = (used / total) * 100;
    *shared_usage = usage;

    shmdt(shared_usage);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        //printf("Usage: %s <RESOURCE> <MAX/MIN> <VALUE>\n", argv[0]);
        return 1;
    }

    char *resource = argv[1];
    char *operation = argv[2];
    int value = atoi(argv[3]);

    if (strcmp(operation, "MAX") != 0 && strcmp(operation, "MIN") != 0) {
        //printf("Error: Invalid operation. Only 'MAX' or 'MIN' is supported.\n");
        return 1;
    }

    if (value < 0 || value > 100) {
        //printf("Error: Value must be between 0 and 100.\n");
        return 1;
    }

    pid_t pid;
    key_t key;
    int shmid;

    if (strcmp(resource, "CPU") == 0) {
        key = ftok("cpu_usage", 65);
        shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
        if (shmid == -1) {
            perror("shmget 실패");
            exit(1);
        }

        pid = fork();
        if (pid == 0) {
            execl("./cpuAlarm", "./cpuAlarm", operation, argv[3], (char *)NULL);
            perror("execl 실패");
            exit(1);
        } else {
            while (1) {
                write_cpu_usage_to_shared_memory(shmid);
                sleep(3);
            }
        }
    } else if (strcmp(resource, "RAM") == 0) {
        key = ftok("ram_usage", 66);
        shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
        if (shmid == -1) {
            perror("shmget 실패");
            exit(1);
        }

        pid = fork();
        if (pid == 0) {
            execl("./ramAlarm", "./ramAlarm", operation, argv[3], (char *)NULL);
            perror("execl 실패");
            exit(1);
        } else {
            while (1) {
                write_ram_usage_to_shared_memory(shmid);
                sleep(3);
            }
        }
    } else if (strcmp(resource, "DISK") == 0) {
        key = ftok("disk_usage", 67);
        shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
        if (shmid == -1) {
            perror("shmget 실패");
            exit(1);
        }

        pid = fork();
        if (pid == 0) {
            execl("./diskAlarm", "./diskAlarm", operation, argv[3], (char *)NULL);
            perror("execl 실패");
            exit(1);
        } else {
            while (1) {
                write_disk_usage_to_shared_memory(shmid);
                sleep(3);
            }
        }
    } else {
        //printf("Error: Invalid resource. Only 'CPU', 'RAM', or 'DISK' is supported.\n");
        return 1;
    }

    return 0;
}
