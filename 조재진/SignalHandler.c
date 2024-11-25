#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>

#define BUF_LEN 512
#define MAX_TASK_SIZE 64
#define FILE_EVENT_LISTENER "./FileEventListener"
#define LOG_WRITER "./logWriter"

typedef struct{
	char filePath[BUF_LEN];
	char event[BUF_LEN];
	char next_process[BUF_LEN];
	char next_filePath[BUF_LEN];
}TaskList;

int clear_input_buffer();

TaskList t_list[MAX_TASK_SIZE];
pid_t pid[MAX_TASK_SIZE];
int taskNum;


void sigusr1_handler(int signum, siginfo_t *info, void *context){
	for(int i = 0; i < taskNum; i++){
		if(pid[i] == info->si_pid){
			pid_t id = fork();
			if(id == -1){
				perror("fork");
				exit(-1);
			}
			if(id == 0){
				if(strcmp(t_list[i].next_process, "LOG_WRITE") == 0){
					execl(LOG_WRITER, LOG_WRITER, 
							t_list[i].filePath, t_list[i].event, NULL);
					perror("execl failed");
					exit(EXIT_FAILURE);
				}
			}
			return;
		}
	}
}

int main(void){
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigusr1_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

	for(taskNum = 0; taskNum < MAX_TASK_SIZE; taskNum++){
		printf("Enter file path : ");
		scanf("%s", t_list[taskNum].filePath);
		printf("Enter event : ");
		scanf("%s", t_list[taskNum].event);
		printf("Enter next Process : ");
		scanf("%s", t_list[taskNum].next_process);
		if(strcmp(t_list[taskNum].next_process, "LOG_WRITE") != 0){
			printf("Enter next filePath : ");
			scanf("%s", t_list[taskNum].next_filePath);
		}

		pid[taskNum] = fork();
		if(pid[taskNum] == -1){
			perror("fork");
			exit(-1);
		}
		
		if(pid[taskNum] == 0){
			execl(FILE_EVENT_LISTENER, FILE_EVENT_LISTENER, 
				t_list[taskNum].filePath, t_list[taskNum].event, NULL);
			perror("execl failed");
			exit(EXIT_FAILURE);
		}else{
			printf("%d Event Listener is added\n", taskNum+1);
			sleep(1);
		}
	}
}

int clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // 버퍼 비우기
    return 0;
}