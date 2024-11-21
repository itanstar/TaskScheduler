#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define LOG_FILE "process_events.log"

// 기존 PID를 저장하는 연결 리스트 노드
typedef struct PIDNode {
    int pid;
    struct PIDNode *next;
} PIDNode;

// PID 연결 리스트에서 PID 추가
void add_pid(PIDNode **head, int pid) {
    PIDNode *new_node = (PIDNode *)malloc(sizeof(PIDNode));
    new_node->pid = pid;
    new_node->next = *head;
    *head = new_node;
}

// PID 연결 리스트에서 PID 제거
void remove_pid(PIDNode **head, int pid) {
    PIDNode *current = *head;
    PIDNode *prev = NULL;
    while (current != NULL && current->pid != pid) {
        prev = current;
        current = current->next;
    }
    if (current == NULL) return; // PID가 리스트에 없음
    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    free(current);
}

// PID 연결 리스트에서 PID 존재 확인
int pid_exists(PIDNode *head, int pid) {
    while (head != NULL) {
        if (head->pid == pid) return 1;
        head = head->next;
    }
    return 0;
}

// PID 연결 리스트 정리
void free_pid_list(PIDNode *head) {
    PIDNode *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// 현재 시간을 문자열로 반환
void get_current_time(char *buffer, size_t size) {
    time_t raw_time;
    struct tm *time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info);
}

// 프로세스 이름 가져오기
void get_process_name(int pid, char *buffer, size_t size) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    FILE *cmd_file = fopen(path, "r");
    if (cmd_file == NULL) {
        snprintf(buffer, size, "Unknown");
        return;
    }

    if (fgets(buffer, size, cmd_file) == NULL) {
        snprintf(buffer, size, "Unknown");
    }

    fclose(cmd_file);
}

// 로그 작성
void log_event(const char *event, int pid) {
    char time_str[20];
    char process_name[256];
    get_current_time(time_str, sizeof(time_str));
    get_process_name(pid, process_name, sizeof(process_name));

    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("fopen");
        return;
    }
    fprintf(log_file, "[%s] PID %d: %s (%s)\n", time_str, pid, event, process_name);
    fclose(log_file);
}

// /proc 디렉토리를 스캔하여 현재 PID 목록 반환
void scan_proc(PIDNode **head) {
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    PIDNode *temp_head = NULL;

    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            int pid = atoi(entry->d_name);

            // 새로운 PID 감지
            if (!pid_exists(*head, pid)) {
                add_pid(&temp_head, pid);
                log_event("Process created", pid);
            } else {
                // 기존 PID 유지
                add_pid(&temp_head, pid);
            }
        }
    }
    closedir(proc_dir);

    // 종료된 PID 감지
    PIDNode *current = *head;
    while (current != NULL) {
        if (!pid_exists(temp_head, current->pid)) {
            log_event("Process terminated", current->pid);
        }
        current = current->next;
    }

    // 기존 리스트 대체
    free_pid_list(*head);
    *head = temp_head;
}

int main() {
    PIDNode *pid_list = NULL;

    // 초기 상태 스캔
    scan_proc(&pid_list);

    printf("Monitoring process creation and termination...\n");
    while (1) {
        scan_proc(&pid_list);
        sleep(2); // 2초마다 스캔
    }

    free_pid_list(pid_list);
    return 0;
}
