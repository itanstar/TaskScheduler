#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

// 프로세스 종료 함수 (RAM 제한)
void kill_high_memory_process(int pid) {
    char command[256];
    snprintf(command, sizeof(command), "kill -9 %d", pid);
    system(command);
    printf("프로세스 PID=%d을(를) 종료하여 RAM 사용량을 제한했습니다.\n", pid);
}

// 공유 메모리에서 RAM 사용량 모니터링 및 제어
void monitor_and_control_high_ram_usage(int shmid, double threshold) {
    while (1) {
        // 공유 메모리 연결
        double *shared_usage = (double *)shmat(shmid, NULL, 0);
        if (shared_usage == (double *)-1) {
            perror("shmat 실패");
            exit(1);
        }

        double usage = *shared_usage; // 공유 메모리에서 RAM 사용량 읽기
        printf("현재 RAM 사용량: %.2f%%\n", usage);

        // 임계값 초과 시 처리
        if (usage > threshold) {
            printf("⚠️ 경고: RAM 사용량이 %.2f%%를 초과했습니다!\n", threshold);

            // 가장 RAM을 많이 사용하는 프로세스 확인
            FILE *fp = popen("ps -eo pid,%mem,comm --sort=-%mem | head -n 2 | tail -n 1", "r");
            if (fp == NULL) {
                perror("popen 실패");
                shmdt(shared_usage);
                exit(1);
            }

            int pid;
            double proc_mem;
            char command[256];
            fscanf(fp, "%d %lf %s", &pid, &proc_mem, command);
            pclose(fp);

            printf("최대 RAM 사용 프로세스: PID=%d, COMMAND=%s, MEM=%.2f%%\n", pid, command, proc_mem);

            // RAM 사용량 제한 (프로세스 종료)
            kill_high_memory_process(pid);

            // 제한 후 30분 대기
            printf("30분 동안 제한 상태를 유지합니다...\n");
            sleep(1800); // 30분 = 1800초
        }

        // 공유 메모리 해제
        shmdt(shared_usage);

        sleep(3); // 3초마다 확인
    }
}

int main(int argc, char *argv[]) {
    // 기본 설정
    double threshold = 80.0;  // RAM 사용량 경고 임계값(%)

    // 명령행 인자를 통해 설정 변경 가능
    if (argc > 1) {
        threshold = atof(argv[1]);
    }

    // 공유 메모리 연결
    key_t key = ftok("ram_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget 실패");
        exit(1);
    }

    printf("RAM 사용량 모니터링 시작... 임계값: %.2f%%\n", threshold);

    monitor_and_control_high_ram_usage(shmid, threshold);

    return 0;
}
