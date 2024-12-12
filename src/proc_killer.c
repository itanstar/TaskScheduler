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
                        } else {
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
    } else {
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "NAME") == 0) {
        kill_by_name(argv[2]);
    } else if (strcmp(argv[1], "PID") == 0) {
        pid_t pid = atoi(argv[2]);
        if (pid > 0) {
            kill_by_pid(pid);
        } else {
        }
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

