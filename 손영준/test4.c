#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

// 현재 시간을 포맷팅하는 함수
void print_current_time() {
    time_t t;
    struct tm *tm_info;
    char buffer[26];

    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", buffer);
}

// 프로세스 이름을 가져오는 함수
void get_process_name(int pid) {
    char path[256];
    char name[256];
    FILE *fp;

    // /proc/[pid]/comm 파일을 열어 프로세스 이름을 읽는다
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    fp = fopen(path, "r");
    if (fp != NULL) {
        fgets(name, sizeof(name), fp);
        fclose(fp);
        // newline 문자를 제거
        name[strcspn(name, "\n")] = '\0';
        printf("Process Name: %s\n", name);
    } else {
        printf("Unknown Process Name\n");
    }
}

int main() {
    int fd, wd;
    char buffer[BUF_LEN];

    // inotify 인스턴스 생성
    fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    // /proc 디렉토리 모니터링
    wd = inotify_add_watch(fd, "/proc", IN_CREATE | IN_DELETE);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Monitoring /proc for process creation and deletion...\n");

    while (1) {
        int length = read(fd, buffer, BUF_LEN);
        if (length == -1) {
            perror("read");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // 이벤트 처리
        for (int i = 0; i < length;) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            i += EVENT_SIZE + event->len;  // 이벤트 크기만큼 이동

            // 'event->len'이 0보다 클 때만 'name' 필드를 사용할 수 있음
            if (event->len > 0) {
                if (event->mask & IN_CREATE) {
                    // 새 파일이 생성되었을 때 (프로세스 생성)
                    int pid = atoi(event->name);  // /proc/[pid] 디렉토리 이름을 pid로 사용
                    print_current_time();
                    printf("New process created: /proc/%s\n", event->name);
                    get_process_name(pid);
                }

                if (event->mask & IN_DELETE) {
                    // 파일이 삭제되었을 때 (프로세스 종료)
                    int pid = atoi(event->name);  // /proc/[pid] 디렉토리 이름을 pid로 사용
                    print_current_time();
                    printf("Process terminated: /proc/%s\n", event->name);
                    get_process_name(pid);
                }
            }
        }
    }

    // inotify 리소스 해제
    close(fd);
    return 0;
}

