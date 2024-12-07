#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// 디스크 사용량 모니터링 함수
void monitor_disk_usage_from_shared_memory(int shmid, double threshold) {
    while (1) {
        // 공유 메모리 연결
        double *shared_usage = (double *)shmat(shmid, NULL, 0);
        if (shared_usage == (double *)-1) {
            perror("shmat 실패");
            exit(1);
        }

        // 디스크 사용량 확인
        double usage = *shared_usage;

        // 임계값 미만 시 경고
        if (usage < threshold) {
            //printf("⚠️ 경고: 디스크 사용량이 %.2f%%미만입니다! 현재 사용량: %.2f%%\n", threshold, usage);
        }

        // 공유 메모리 해제
        shmdt(shared_usage);

        sleep(3); // 3초마다 확인
    }
}

int main(int argc, char *argv[]) {
    // 기본 임계값 설정
    double threshold = 20.0; // 80% 임계값

    if (argc > 1) {
        threshold = atof(argv[1]); // 명령행 인자로 임계값 설정 가능
    }

    // 공유 메모리 연결
    key_t key = ftok("disk_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget 실패");
        exit(1);
    }

    //printf("디스크 사용량 모니터링 시작... 임계값: %.2f%%\n", threshold);

    monitor_disk_usage_from_shared_memory(shmid, threshold);

    return 0;
}
