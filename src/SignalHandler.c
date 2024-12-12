#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>

#define BUF_LEN 512
#define MAX_TASK_SIZE 64

//경로 변수
#define FILE_EVENT_LISTENER "./FileEventListener"
#define LOG_WRITER "./logWriter"
#define EXECUTOR "./Executor"
#define CHG_ATTR "./CHG_ATTR"

//자료형
typedef struct{
	char filePath[BUF_LEN];
	char event[BUF_LEN];
	char next_process[BUF_LEN];
	char next_filePath[BUF_LEN];
	char parameter[BUF_LEN];
}TaskList;

TaskList t_list[MAX_TASK_SIZE];
pid_t pid[MAX_TASK_SIZE];
int taskNum;

//시그널 이벤트 처리 부분
void sigusr1_handler(int signum, siginfo_t *info, void *context){
	for(int i = 0; i < taskNum; i++){
		if(pid[i] == info->si_pid){
			pid_t id = fork();
			if(id == -1){
				perror("fork");
				exit(-1);
			}
			if(id == 0){
				//다음 이벤트가 로그 작성일 때 -> 감지 파일과 감지 이벤트 전달
				if(strcmp(t_list[i].next_process, "LogWrite") == 0){
					execl(LOG_WRITER, LOG_WRITER, 
							t_list[i].filePath, t_list[i].event, NULL);
					perror("execl failed");
					exit(EXIT_FAILURE);
				}
				//다음 이벤트가 실행일 때 -> 실행할 파일과 매개변수 전달
				if(strcmp(t_list[i].next_process, "Execute") == 0){
					execl(EXECUTOR, EXECUTOR, 
							t_list[i].next_filePath, t_list[i].parameter, NULL);
					perror("execl failed");
					exit(EXIT_FAILURE);
				}
				//다음 이벤트가 권한 변경일 때 -> 대상 파일과 매개변수(rwxrwxrwx) 전달
				//대상 파일이 "." 일 경우 현재 폴더의 모든 파일의 권한 변경
				if(strcmp(t_list[i].next_process, "Change_Attr") == 0){
					execl(CHG_ATTR, CHG_ATTR, 
							t_list[i].next_filePath, t_list[i].parameter, NULL);
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
		//이벤트가 발생했을 때 처리할 다음 작업이 "Execute"거나 "Change_Attr"일 때
		if((strcmp(t_list[taskNum].next_process, "Execute") == 0) ||
				(strcmp(t_list[taskNum].next_process, "Change_Attr") == 0)){
			//대상 파일과 파라미터 입력 받음
			printf("Enter next filePath : ");
			scanf("%s", t_list[taskNum].next_filePath);
			printf("Enter parameter(s) : ");
			getchar();
			fgets(t_list[taskNum].parameter, BUF_LEN, stdin);
		}

		pid[taskNum] = fork();
		if(pid[taskNum] == -1){
			perror("fork");
			exit(-1);
		}
		
		if(pid[taskNum] == 0){
			//이벤트 리스너(감지 파일, 감지 이벤트) 실행
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