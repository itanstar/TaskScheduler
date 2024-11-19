#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int main(int argc, char* argv[]){
	if(argc != 3){
		printf("Usage : ./logWriter FileName Motion\n");
		exit(0);
	}
 	
	FILE* file = fopen("log.txt", "a");
	if(file == NULL){
		printf("File Open Error\n");
		exit(-1);
	}

	time_t current_time = time(NULL);
    // struct tm 구조체로 변환
    struct tm *time_info = localtime(&current_time);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
    fprintf(file, "%s %s : %s\n", argv[1], argv[2], buffer);

    return 0;
}