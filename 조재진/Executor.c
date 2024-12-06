#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARGC 8
#define MAX_ARGV_SIZE 256

int main(int argc, char* argv[]) {
	char parameters[MAX_ARGC][MAX_ARGV_SIZE];
	char* exec_params[MAX_ARGC];
	int current = 0;

	char* token = strtok(argv[2], " ");
	while (token != NULL && current < MAX_ARGC - 1) {
		strcpy(parameters[current], token);
		exec_params[current] = parameters[current];
		current++;
		token = strtok(NULL, " ");
	}
	exec_params[current] = NULL;

	execv(argv[1], exec_params);

	perror("execv");
	return 1;
}
