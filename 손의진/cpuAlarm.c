#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <libnotify/notify.h>

// 알림 표시 함수
void show_notification(const char *summary, const char *body)
{
    notify_init("CPU Monitor");
    NotifyNotification *notification = notify_notification_new(summary, body, NULL);
    notify_notification_show(notification, NULL);
    g_object_unref(G_OBJECT(notification));
    notify_uninit();
}

// 공유 메모리에서 CPU 사용량 모니터링
void monitor_cpu_usage(int shmid, double threshold, int is_max)
{
    while (1)
    {
        // 공유 메모리 연결
        double *shared_usage = (double *)shmat(shmid, NULL, 0);
        if (shared_usage == (double *)-1)
        {
            perror("shmat 실패");
            exit(1);
        }

        // CPU 사용량 확인
        double usage = *shared_usage;

        // 조건에 따라 알림 표시
        if ((is_max && usage > threshold) || (!is_max && usage < threshold))
        {
            char message[100];
            snprintf(message, sizeof(message), "현재 CPU 사용량: %.2f%%", usage);
            show_notification(
                is_max ? "⚠️ CPU 사용량 초과 경고" : "⚠️ CPU 낮은 사용량 경고",
                message);
            sleep(300); // 알림 후 300초 대기
        }

        // 공유 메모리 해제
        shmdt(shared_usage);

        sleep(3); // 3초마다 확인
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        // printf("Usage: %s <MAX/MIN> <THRESHOLD>\n", argv[0]);
        return 1;
    }

    char *operation = argv[1];
    double threshold = atof(argv[2]);

    int is_max = (strcmp(operation, "MAX") == 0);
    if (!is_max && strcmp(operation, "MIN") != 0)
    {
        // printf("Error: Invalid operation. Only 'MAX' or 'MIN' is supported.\n");
        return 1;
    }

    // 공유 메모리 연결
    key_t key = ftok("cpu_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666);
    if (shmid == -1)
    {
        perror("shmget 실패");
        exit(1);
    }

    monitor_cpu_usage(shmid, threshold, is_max);

    return 0;
}
