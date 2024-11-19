#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define BUF_LEN 512
#define MAX_TASK 64

struct task {
    char filePath[BUF_LEN];
    int event;
    char* nextTask;
};

typedef struct taskList {
    struct task t[MAX_TASK];
    int size;
} TaskList;

void addTask(TaskList*, struct task);
void do_next_task(char*, char*, char*);
void* watch_inotify(void*);

int main(void) {
    int t[MAX_TASK];
    TaskList t_list;
    pthread_t threads[MAX_TASK];
 
	char *logWriter = "/home/knu/SystemProgramming/logWriter";

    t_list.size = 0;

    addTask(&t_list, (struct task){"/home/knu/SystemProgramming/hello1", IN_OPEN, logWriter});
    addTask(&t_list, (struct task){"/home/knu/SystemProgramming/hello1", IN_CLOSE, logWriter});
    addTask(&t_list, (struct task){"/home/knu/SystemProgramming/HelloWorld", IN_OPEN, logWriter});
    addTask(&t_list, (struct task){"/home/knu/SystemProgramming/HelloWorld", IN_CLOSE, logWriter});

    for (int i = 0; i < t_list.size; i++) {
        struct task *task_copy = malloc(sizeof(struct task));
        if (task_copy == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        *task_copy = t_list.t[i];        

        t[i] = pthread_create(&threads[i], NULL, watch_inotify, (void *)task_copy);
        if (t[i]) {
            perror("Thread initiate");
            free(task_copy);
        }
    }
    
	while(1);

    return 0;
}

void addTask(TaskList* t_list, struct task t) {
    t_list->t[t_list->size] = t;
    t_list->size++;
}

void* watch_inotify(void* args) {
    struct task *t = (struct task*)args;
    ssize_t len, i;
    char buf[BUF_LEN];

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        free(t);
        pthread_exit(NULL);
    }

    int wd = inotify_add_watch(fd, t->filePath, t->event);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(fd);
        free(t);
        pthread_exit(NULL);
    }

    while (1) {
        pthread_testcancel();
        len = read(fd, buf, BUF_LEN);
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            break;
        }

        for (i = 0; i < len; ) {
            struct inotify_event *event = (struct inotify_event *) &buf[i];
            if (event->mask & IN_OPEN) {
                do_next_task(t->nextTask, t->filePath, "Opened");
            }
            if (event->mask & IN_CLOSE) {
                do_next_task(t->nextTask, t->filePath, "Closed");
            }

            i += sizeof(struct inotify_event) + event->len;
        }
        sleep(1);
    }

    close(wd);
    close(fd);
    free(t);
    pthread_exit(NULL);
}

void do_next_task(char* nextTask, char* filePath, char* args) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
    } else if (pid == 0) {
        if (execl(nextTask, "nextTask", filePath, args, (char *)NULL) == -1) {
            perror("execl");
            exit(EXIT_FAILURE);
        }
    }
}
