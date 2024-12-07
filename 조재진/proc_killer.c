#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <ctype.h>

// 프로세스 이름으로 종료
void kill_by_name(const char *name) {
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Failed to open /proc");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {  // 프로세스 PID 디렉토리인지 확인
            char comm_path[512];
            snprintf(comm_path, sizeof(comm_path), "/proc/%s/comm", entry->d_name);

            FILE *comm_file = fopen(comm_path, "r");
            if (comm_file) {
                char comm_name[256];
                if (fgets(comm_name, sizeof(comm_name), comm_file)) {
                    comm_name[strcspn(comm_name, "\n")] = 0;
                    if (strcmp(comm_name, name) == 0) {
                        pid_t pid = atoi(entry->d_name);
                        if (kill(pid, SIGKILL) == 0) {
                            printf("Killed process: %s (PID: %d)\n", comm_name, pid);
                        } else {
                            perror("Failed to kill process");
                        }
                    }
                }
                fclose(comm_file);
            }
        }
    }
    closedir(proc_dir);
}

// PID로 종료
void kill_by_pid(pid_t pid) {
    if (kill(pid, SIGKILL) == 0) {
        printf("Killed process with PID: %d\n", pid);
    } else {
        perror("Failed to kill process");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s NAME <name> or %s PID <pid>\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "NAME") == 0) {
        kill_by_name(argv[2]);
    } else if (strcmp(argv[1], "PID") == 0) {
        pid_t pid = atoi(argv[2]);
        if (pid > 0) {
            kill_by_pid(pid);
        } else {
            fprintf(stderr, "Invalid PID: %s\n", argv[2]);
        }
    } else {
        fprintf(stderr, "Invalid option: %s. Use NAME or PID.\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

