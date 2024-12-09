#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#define MAX_PIDS 32768

static time_t last_time = 0;
pid_t my_pid;

// current time
void print_current_time()
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buffer[26];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", buffer);
}

// ���μ��� �̸� ��������
void get_process_name(int pid, char *name, size_t size)
{
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE *fp = fopen(path, "r");
    if (fp)
    {
        if (fgets(name, size, fp))
        {
            name[strcspn(name, "\n")] = '\0';
        }
        fclose(fp);
    }
    else
    {
        strncpy(name, "Unknown", size - 1);
        name[size - 1] = '\0';
    }
}

// �ñ׳� ������
void send_signal_to_parent()
{

    time_t current_time = time(NULL);

    if (current_time - last_time < 3)
    { // 3초 제한
        // printf("Rate limit: Signal ignored\n");
        return;
    }

    pid_t parent_pid = getppid();

    if (parent_pid == my_pid)
    {
        // printf("Parent process is the same as the current process. No signal will be sent.\n");
        return;
    }

    last_time = current_time;

    // printf("Attempting to send signal to parent PID: %d\n", parent_pid);
    if (kill(parent_pid, SIGUSR1) == -1)
    {
        // perror("Failed to send signal to parent");
    }
}

// ���μ��� ����/���� ����
void scan_proc(int *last_pids, char **last_names, int target_pid, const char *target_name, int initialized, int detect_create, int detect_terminate)
{
    DIR *dir = opendir("/proc");
    if (!dir)
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int current_pids[MAX_PIDS] = {0};
    struct dirent *entry;

    // 현재 프로세스 목록 확인
    while ((entry = readdir(dir)) != NULL)
    {
        if (!isdigit(entry->d_name[0]))
            continue;

        int pid = atoi(entry->d_name);
        current_pids[pid] = 1;

        // 프로세스 생성 감지
        if (!last_pids[pid] && detect_create)
        {
            char process_name[256];
            get_process_name(pid, process_name, sizeof(process_name));

            if (initialized)
            {
                if ((target_pid != -1 && pid != target_pid) ||
                    (target_name && strcasecmp(process_name, target_name) != 0))
                {
                    continue;
                }
                // print_current_time();
                // printf("New process created: PID %d, Name: %s\n", pid, process_name);
                send_signal_to_parent();
            }

            // 프로세스 이름 저장
            strncpy(last_names[pid], process_name, 255);
            last_names[pid][255] = '\0';
        }
        else if (!last_pids[pid])
        {
            // 생성 감지는 하지 않지만 이름은 저장
            get_process_name(pid, last_names[pid], 256);
        }
    }
    closedir(dir);

    // 프로세스 종료 감지
    if (initialized && detect_terminate)
    {
        for (int pid = 0; pid < MAX_PIDS; pid++)
        {
            if (last_pids[pid] && !current_pids[pid])
            {
                if ((target_pid != -1 && pid != target_pid) ||
                    (target_name && strcasecmp(last_names[pid], target_name) != 0))
                {
                    continue;
                }
                // print_current_time();
                // printf("Process terminated: PID %d, Name: %s\n", pid, last_names[pid]);
                send_signal_to_parent();
            }
        }
    }

    // 현재 프로세스 목록 저장
    memcpy(last_pids, current_pids, sizeof(int) * MAX_PIDS);
}

int main(int argc, char *argv[])
{
    int target_pid = -1;
    const char *target_name = NULL;
    int detect_create = 1;    // ���μ��� ���� ����
    int detect_terminate = 1; // ���μ��� ���� ����

    my_pid = getpid();
    // printf("My PID : %d\n",my_pid);
    //  ���� ó��
    if (argc == 3)
    {
        if (strcasecmp(argv[1], "PID") == 0)
        {
            target_pid = atoi(argv[2]);
            if (target_pid <= 0)
            {
                // fprintf(stderr, "Invalid PID: %s\n", argv[2]);
                return EXIT_FAILURE;
            }
            // printf("Monitoring PID: %d\n", target_pid);
        }
        else if (strcasecmp(argv[1], "NAME") == 0)
        {
            target_name = argv[2];
            // printf("Monitoring process name: %s\n", target_name);
        }
        else
        {
            // fprintf(stderr, "Usage: %s [PID <number> | NAME <string> | ALL] [create | terminate | both]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    else if (argc == 4)
    {
        if (strcasecmp(argv[1], "PID") == 0)
        {
            target_pid = atoi(argv[2]);
            if (target_pid <= 0)
            {
                // fprintf(stderr, "Invalid PID: %s\n", argv[2]);
                return EXIT_FAILURE;
            }
            if (strcasecmp(argv[3], "terminate") == 0)
            {
                detect_create = 0;
            }
            else if (strcasecmp(argv[3], "create") == 0)
            {
                detect_terminate = 0;
            }
            else if (strcasecmp(argv[3], "both") == 0)
            {
                detect_create = 1;
                detect_terminate = 1;
            }
            else
            {
                // fprintf(stderr, "Invalid option for create/terminate. Use 'create', 'terminate', or 'both'.\n");
                return EXIT_FAILURE;
            }
            // printf("Monitoring PID: %d with %s detection\n", target_pid, argv[3]);
        }
        else if (strcasecmp(argv[1], "NAME") == 0)
        {
            target_name = argv[2];
            if (strcasecmp(argv[3], "terminate") == 0)
            {
                detect_create = 0;
            }
            else if (strcasecmp(argv[3], "create") == 0)
            {
                detect_terminate = 0;
            }
            else if (strcasecmp(argv[3], "both") == 0)
            {
                detect_create = 1;
                detect_terminate = 1;
            }
            else
            {
                // fprintf(stderr, "Invalid option for create/terminate. Use 'create', 'terminate', or 'both'.\n");
                return EXIT_FAILURE;
            }
            // printf("Monitoring process name: %s with %s detection\n", target_name, argv[3]);
        }
        else
        {
            // fprintf(stderr, "Usage: %s [PID <number> | NAME <string>] [create | terminate | both]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    else if (argc == 2 && strcasecmp(argv[1], "ALL") == 0)
    {
        // printf("Monitoring all processes.\n");
    }
    else
    {
        // fprintf(stderr, "Usage: %s [PID <number> | NAME <string> | ALL] [create | terminate | both]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // �޸� �Ҵ�
    int *last_pids = calloc(MAX_PIDS, sizeof(int));
    char **last_names = calloc(MAX_PIDS, sizeof(char *));
    if (!last_pids || !last_names)
    {
        // perror("Memory allocation failed");
        free(last_pids);
        free(last_names);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < MAX_PIDS; i++)
    {
        last_names[i] = calloc(256, sizeof(char));
        if (!last_names[i])
        {
            // perror("Memory allocation failed");
            for (int j = 0; j < i; j++)
                free(last_names[j]);
            free(last_names);
            free(last_pids);
            return EXIT_FAILURE;
        }
    }

    // ���� ������ ��� ���μ���
    scan_proc(last_pids, last_names, -1, NULL, 0, 1, 1);

    // printf("Monitoring process creation and termination...\n");
    while (1)
    {
        scan_proc(last_pids, last_names, target_pid, target_name, 1, detect_create, detect_terminate);
        usleep(500000); // 0.5�� ���
    }
}
