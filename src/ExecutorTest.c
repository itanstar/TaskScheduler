#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_LEN 512

int main(int argc, char *argv[])
{
	char echo[MAX_LEN];

	sprintf(echo, "%s %s; exec bash", argv[1], argv[2]);
	execlp("gnome-terminal", "gnome-terminal", "--", "bash", "-c", echo, NULL);

	return 0;
}