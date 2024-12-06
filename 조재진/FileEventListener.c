#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define BUF_LEN 512
#define MAX_TASK 64
#define EXECUTE 1

typedef struct Task {
    char* filePath;
    char* event;
}Task;

int string_to_event(char*);
void watch_inotify(void*);

int main(int argc, char* argv[]) {
    if(argc != 3){
        printf("Usage: ./FileEventListener <filePath> <event>\n");
        exit(0);
    }
    char* filePath = argv[1];
    char* event = argv[2];
    if(string_to_event(argv[2]) == -1){
        printf("Invalid Event Input\n\n");
        exit(1);
    }
    
    Task t = {filePath, event};
    printf("Watch Inotify is Working\n\n");
    watch_inotify((void*)(&t));
    
    return 0;
}

int string_to_event(char* str){
    if(strcmp("IN_CREATE", str) == 0) return IN_CREATE;
    if(strcmp("IN_DELETE", str) == 0) return IN_DELETE;
    if(strcmp("IN_ATTRIB", str) == 0) return IN_ATTRIB;
    if(strcmp("IN_MODIFY", str) == 0) return IN_MODIFY;
    if(strcmp("IN_MOVED_TO", str) == 0) return IN_MOVED_TO;
    if(strcmp("IN_MOVED_FROM", str) == 0) return IN_MOVED_FROM;
    if(strcmp("IN_OPEN", str) == 0) return IN_OPEN;
    if(strcmp("IN_CLOSE", str) == 0) return IN_CLOSE;
    return -1;
}

void watch_inotify(void* args) {
    Task *t = (Task*)args;
    ssize_t len, i;
    char buf[BUF_LEN];

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        free(t);
        pthread_exit(NULL);
    }

    int event_const = string_to_event(t->event);
    int wd = inotify_add_watch(fd, t->filePath, event_const);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(fd);
        free(t);
        pthread_exit(NULL);
    }

    while (1) {
        len = read(fd, buf, BUF_LEN);
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            break;
        }

        for (i = 0; i < len; ) {

            struct inotify_event *event = (struct inotify_event *) &buf[i];
                
            if (event->mask & event_const) {
                if (strstr(event->name, ".goutputstream") == NULL) 
                    kill(getppid(), SIGUSR1);
            }
            i += sizeof(struct inotify_event) + event->len;
        }
        sleep(1);
    }

    close(wd);
    close(fd);
    free(t);
}