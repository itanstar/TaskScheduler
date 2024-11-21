#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void scan_proc() {
    DIR *dir;
    struct dirent *entry;
    static int last_pids[32768] = {0}; // 이전에 확인한 PID 상태
    int current_pids[32768] = {0};    // 현재 확인한 PID 상태

    dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // 현재 PID 상태 읽기
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            int pid = atoi(entry->d_name);
            current_pids[pid] = 1;
        }
    }
    closedir(dir);

    // PID 비교
    for (int pid = 0; pid < 32768; pid++) {
        if (current_pids[pid] && !last_pids[pid]) {
            printf("New process created: PID %d\n", pid);
        } else if (!current_pids[pid] && last_pids[pid]) {
            printf("Process terminated: PID %d\n", pid);
        }
    }

    // 현재 상태를 이전 상태로 복사
    memcpy(last_pids, current_pids, sizeof(last_pids));
}

int main() {
    while (1) {
        scan_proc();
        usleep(500000); // 0.5초 대기
    }
    return 0;
}

