#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <libnotify/notify.h>

// CPU 사용량 모니터링 및 알림
void monitor_cpu_usage_from_shared_memory(int shmid, double threshold) {
    while (1) {
        // 공유 메모리 연결
        double *shared_usage = (double *)shmat(shmid, NULL, 0);
        if (shared_usage == (double *)-1) {
            perror("shmat 실패");
            exit(1);
        }

        // CPU 사용량 확인
        double usage = *shared_usage;

        // 임계값 초과 시 알림
        if (usage > threshold) {
            printf("⚠️ 경고: CPU 사용량이 %.2f%%를 초과했습니다! 현재 사용량: %.2f%%\n", threshold, usage);

            // libnotify를 이용한 데스크탑 알림
            notify_init("CPU Monitor");
            NotifyNotification *notification = notify_notification_new(
                "⚠️ CPU 경고",
                "CPU 사용량이 임계값을 초과했습니다!",
                NULL // 아이콘 설정 (필요 시 파일 경로로 대체 가능)
            );
            notify_notification_show(notification, NULL);
            g_object_unref(G_OBJECT(notification));
            notify_uninit();

            sleep(500); // 알림 후 500초 대기
        }

        // 공유 메모리 해제
        shmdt(shared_usage);

        sleep(3); // 3초마다 확인
    }
}

int main(int argc, char *argv[]) {
    // 임계값 설정
    double threshold = 75.0;
    if (argc > 1) {
        threshold = atof(argv[1]); // 명령행 인자로 임계값 설정 가능
    }

    // 공유 메모리 연결
    key_t key = ftok("cpu_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget 실패");
        exit(1);
    }

    printf("CPU 사용량 모니터링 시작... 임계값: %.2f%%\n", threshold);

    monitor_cpu_usage_from_shared_memory(shmid, threshold);

    return 0;
}
