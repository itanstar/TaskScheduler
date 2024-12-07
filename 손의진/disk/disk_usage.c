#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/statvfs.h>

// 디스크 사용량 계산 함수
void write_disk_usage_to_shared_memory(int shmid) {
    struct statvfs stat;
    double usage;

    // 공유 메모리 연결
    double *shared_usage = (double *)shmat(shmid, NULL, 0);
    if (shared_usage == (double *)-1) {
        perror("shmat 실패");
        exit(1);
    }

    // 파일 시스템 정보 가져오기
    if (statvfs("/", &stat) != 0) {
        perror("statvfs 실패");
        return;
    }

    unsigned long total = stat.f_blocks * stat.f_frsize; // 전체 디스크 크기
    unsigned long free = stat.f_bfree * stat.f_frsize;   // 사용 가능한 공간
    unsigned long used = total - free;                  // 사용된 공간

    usage = (double)used / total * 100.0; // 사용 비율(%)
    *shared_usage = usage; // 공유 메모리에 저장

    //printf("디스크 사용량 계산: %.2f%%\n", usage);

    // 공유 메모리 해제
    shmdt(shared_usage);
}

int main() {
    // 공유 메모리 생성
    key_t key = ftok("disk_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget 실패");
        exit(1);
    }

    // 주기적으로 디스크 사용량 계산 및 저장
    while (1) {
        write_disk_usage_to_shared_memory(shmid);
        sleep(10); // 3초마다 갱신
    }

    return 0;
}
