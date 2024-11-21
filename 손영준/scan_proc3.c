#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

#define MAX_PIDS 32768 // 최대 PID 수

// 현재 시간을 출력하는 함수
void print_current_time() {
    time_t t;
    struct tm *tm_info;
    char buffer[26];

    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", buffer);
}

// 프로세스 이름 가져오기
void get_process_name(int pid, char *name, size_t size) {
    char path[256];
    FILE *fp;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    fp = fopen(path, "r");
    if (fp) {
        fgets(name, size, fp);
        fclose(fp);
        name[strcspn(name, "\n")] = '\0'; // 개행 문자 제거
    } else {
        strncpy(name, "Unknown", size);
    }
}

// /proc 디렉토리를 스캔하여 프로세스 시작/종료 감지
void scan_proc(int *last_pids, char last_names[MAX_PIDS][256]) {
    DIR *dir;
    struct dirent *entry;
    int current_pids[MAX_PIDS] = {0};

    dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // 현재 /proc 디렉토리를 읽어 PID 상태 업데이트
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            int pid = atoi(entry->d_name);
            current_pids[pid] = 1;

            // 새로운 프로세스 발견
            if (!last_pids[pid]) {
                char process_name[256];
                get_process_name(pid, process_name, sizeof(process_name));
                print_current_time();
                printf("New process created: PID %d, Name: %s\n", pid, process_name);

                // 이름 저장
                strncpy(last_names[pid], process_name, sizeof(last_names[pid]));
            }
        }
    }
    closedir(dir);

    // 종료된 프로세스 확인
    for (int pid = 0; pid < MAX_PIDS; pid++) {
        if (last_pids[pid] && !current_pids[pid]) {
            print_current_time();
            printf("Process terminated: PID %d, Name: %s\n", pid, last_names[pid]);
        }
    }

    // 현재 PID 상태와 이름을 이전 상태로 업데이트
    memcpy(last_pids, current_pids, sizeof(int) * MAX_PIDS);
}

int main() {
    int last_pids[MAX_PIDS] = {0};
    char last_names[MAX_PIDS][256] = {0}; // 이전 프로세스 이름 저장용

    printf("Monitoring process creation and termination...\n");
    while (1) {
        scan_proc(last_pids, last_names);
        usleep(500000); // 0.5초 대기
    }

    return 0;
}

