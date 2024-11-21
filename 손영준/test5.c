#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

void print_current_time() {
    time_t t;
    struct tm *tm_info;
    char buffer[26];

    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", buffer);
}

void get_process_name(int pid) {
    char path[256];
    char name[256];
    FILE *fp;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    fp = fopen(path, "r");
    if (fp != NULL) {
        fgets(name, sizeof(name), fp);
        fclose(fp);
        name[strcspn(name, "\n")] = '\0';
        printf("Process Name: %s\n", name);
    } else {
        printf("Unable to retrieve process name\n");
    }
}

int main() {
    int fd, wd;
    char buffer[BUF_LEN];

    fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

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

        for (int i = 0; i < length;) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            i += EVENT_SIZE + event->len;

            if (event->len > 0 && isdigit(event->name[0])) {
                int pid = atoi(event->name);

                if (event->mask & IN_CREATE) {
                    print_current_time();
                    printf("New process created: /proc/%s\n", event->name);
                    get_process_name(pid);
                }

                if (event->mask & IN_DELETE) {
                    print_current_time();
                    printf("Process terminated: /proc/%s\n", event->name);
                    get_process_name(pid);
                }
            }
        }
    }

    close(fd);
    return 0;
}

