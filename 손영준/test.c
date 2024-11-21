#include <stdio.h>
#include <stdlib.h>
#include <sys/fanotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <time.h>

// 로그 기록 함수
void log_event(const char *message) {
    FILE *log = fopen("process_log.txt", "a");
    if (log == NULL) {
        perror("fopen");
        return;
    }
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // 줄바꿈 제거
    fprintf(log, "[%s] %s\n", time_str, message);
    fclose(log);
}

int main() {
    // fanotify 파일 기술자 생성
    int fan_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_NOTIF, O_RDONLY);
    if (fan_fd < 0) {
        perror("fanotify_init");
        return EXIT_FAILURE;
    }

    // /proc 디렉토리 감시
    if (fanotify_mark(fan_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_CREATE | FAN_DELETE, AT_FDCWD, "/proc") < 0) {
        perror("fanotify_mark");
        close(fan_fd);
        return EXIT_FAILURE;
    }

    log_event("Started monitoring process events.");

    struct pollfd fds = { .fd = fan_fd, .events = POLLIN };

    // 이벤트 루프
    while (1) {
        int poll_res = poll(&fds, 1, -1); // 무기한 대기
        if (poll_res < 0) {
            perror("poll");
            break;
        }

        if (fds.revents & POLLIN) {
            // 이벤트 읽기
            char buf[4096];
            ssize_t len = read(fan_fd, buf, sizeof(buf));
            if (len <= 0) {
                perror("read");
                break;
            }

            // 이벤트 처리
            struct fanotify_event_metadata *metadata = (struct fanotify_event_metadata *)buf;
            while (FAN_EVENT_OK(metadata, len)) {
                if (metadata->mask & FAN_CREATE) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Process started: PID %d", metadata->pid);
                    log_event(msg);
                } else if (metadata->mask & FAN_DELETE) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Process terminated: PID %d", metadata->pid);
                    log_event(msg);
                }

                metadata = FAN_EVENT_NEXT(metadata, len);
            }
        }
    }

    close(fan_fd);
    return EXIT_SUCCESS;
}
