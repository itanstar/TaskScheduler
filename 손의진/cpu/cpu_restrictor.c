#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <libnotify/notify.h>
#include <string.h>

// CPU 사용량 제한 함수
void limit_cpu_usage(int pid, double limit)
{
    if (kill(pid, 0) == -1)
    {
        // printf("⚠️ 프로세스 PID=%d가 존재하지 않습니다. 제한 적용을 건너뜁니다.\n", pid);
        return;
    }

    char command[256];
    sn // printf(command, sizeof(command), "cpulimit -p %d -l %.0f &", pid, limit);
        system(command);
    // printf("프로세스 PID=%d의 CPU 사용량을 %.0f%%로 제한했습니다.\n", pid, limit);

    notify_init("CPU Limit");
    NotifyNotification *notification = notify_notification_new(
        "⚠️ CPU 제한 적용",
        "최대 CPU 사용 프로세스가 제한되었습니다.",
        NULL);
    notify_notification_show(notification, NULL);
    g_object_unref(G_OBJECT(notification));
    notify_uninit();
}

// CPU 사용량 모니터링 및 제한 제어
void monitor_and_control_high_cpu_usage(int shmid, double threshold, double cpu_limit)
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

        double usage = *shared_usage;
        // printf("현재 CPU 사용량: %.2f%%\n", usage);

        // 임계값 초과 처리
        if (usage > threshold)
        {
            // printf("⚠️ 경고: CPU 사용량이 %.2f%%를 초과했습니다!\n", threshold);

            notify_init("CPU Monitor");
            NotifyNotification *notification = notify_notification_new(
                "⚠️ CPU 경고",
                "CPU 사용량이 임계값을 초과했습니다!",
                NULL);
            notify_notification_show(notification, NULL);
            g_object_unref(G_OBJECT(notification));
            notify_uninit();

            // 가장 CPU를 많이 사용하는 프로세스 찾기
            FILE *fp = popen("top -bn1 | sed -n '8p' | awk '{print $1, $9, $12}'", "r");
            if (fp == NULL)
            {
                perror("popen 실패");
                shmdt(shared_usage);
                exit(1);
            }

            int pid;
            double proc_cpu;
            char command[256];
            if (fgets(command, sizeof(command), fp) != NULL)
            {
                if (sscanf(command, "%d %lf %[^\n]", &pid, &proc_cpu, command) == 3)
                {
                    if (kill(pid, 0) == -1)
                    {
                        // printf("⚠️ PID=%d가 유효하지 않습니다.\n", pid);
                    }
                    else
                    {
                        // printf("최대 사용 프로세스: PID=%d, COMMAND=%s, CPU=%.2f%%\n", pid, command, proc_cpu);
                        limit_cpu_usage(pid, cpu_limit);

                        // 30분 동안 제한 유지
                        // printf("30분 동안 제한을 유지합니다...\n");
                        sleep(1800);
                    }
                }
                else
                {
                    // printf("⚠️ 프로세스 정보를 읽는 데 실패했습니다.\n");
                }
            }
            pclose(fp);
        }

        // 공유 메모리 해제
        shmdt(shared_usage);
        sleep(3); // 3초마다 확인
    }
}

int main(int argc, char *argv[])
{
    // 기본 설정
    double threshold = 80.0; // CPU 사용량 경고 임계값(%)
    double cpu_limit = 50.0; // 제한할 CPU 사용량(%)

    // 명령행 인자를 통해 설정 변경 가능
    if (argc > 1)
    {
        threshold = atof(argv[1]);
    }
    if (argc > 2)
    {
        cpu_limit = atof(argv[2]);
    }

    // 공유 메모리 연결
    key_t key = ftok("cpu_usage", 65);
    int shmid = shmget(key, sizeof(double), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget 실패");
        exit(1);
    }

    // printf("CPU 사용량 모니터링 시작... 임계값: %.2f%%, 제한: %.2f%%\n", threshold, cpu_limit);

    monitor_and_control_high_cpu_usage(shmid, threshold, cpu_limit);

    return 0;
}
