#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

#define MAX_PIDS 32768

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
void scan_proc(int *last_pids, char **last_names) {
    DIR *dir;
    struct dirent *entry;
    int current_pids[MAX_PIDS] = {0};

    dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        // 숫자로만 이루어진 이름인지 확인
        if (isdigit(entry->d_name[0])) {
            for (int i = 1; entry->d_name[i] != '\0'; i++) {
                if (!isdigit(entry->d_name[i])) {
                    continue;
                }
            }

            int pid = atoi(entry->d_name);
            current_pids[pid] = 1;

            // 새로운 프로세스 발견
            if (!last_pids[pid]) {
                char process_name[256];
                get_process_name(pid, process_name, sizeof(process_name));
                print_current_time();
                printf("New process created: PID %d, Name: %s\n", pid, process_name);

                // 이름 저장
                strncpy(last_names[pid], process_name, 255);
                last_names[pid][255] = '\0'; // NULL 종결
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

    // 상태 업데이트
    memcpy(last_pids, current_pids, sizeof(int) * MAX_PIDS);
}

int main() {
    int *last_pids = calloc(MAX_PIDS, sizeof(int)); // 힙에 동적 할당
    if (!last_pids) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }

    char **last_names = calloc(MAX_PIDS, sizeof(char *)); // 이름 배열 동적 할당
    if (!last_names) {
        perror("calloc failed");
        free(last_pids);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_PIDS; i++) {
        last_names[i] = calloc(256, sizeof(char)); // 각 이름 배열 초기화
        if (!last_names[i]) {
            perror("calloc failed");
            // 메모리 해제
            for (int j = 0; j < i; j++) {
                free(last_names[j]);
            }
            free(last_names);
            free(last_pids);
            exit(EXIT_FAILURE);
        }
    }

    printf("Monitoring process creation and termination...\n");
    while (1) {
        scan_proc(last_pids, last_names);
        usleep(500000); // 0.5초 대기
    }

    // 메모리 해제
    for (int i = 0; i < MAX_PIDS; i++) {
        free(last_names[i]);
    }
    free(last_names);
    free(last_pids);

    return 0;
}

