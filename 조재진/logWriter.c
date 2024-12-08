// logWriter.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage : ./logWriter <FileName> <Event>\n");
		exit(EXIT_FAILURE);
	}

	FILE *file = fopen("log.txt", "a");
	if (file == NULL)
	{
		perror("File Open Error");
		exit(EXIT_FAILURE);
	}

	time_t current_time = time(NULL);
	struct tm *time_info = localtime(&current_time);

	char buffer[20];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
	fprintf(file, "%s %s : %s\n", argv[1], argv[2], buffer);
	fclose(file);

	return 0;
}