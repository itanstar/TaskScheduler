#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#define BUF_LEN 512

void change_permissions_in_directory(const char *dir_path, mode_t new_attr) {
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    char full_path[BUF_LEN];

    if (dir == NULL) {
        perror("Cannot Open DIR");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
		
		if (chmod(full_path, new_attr) != 0) {
            //perror("chmod");
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]){
	mode_t new_attr = 0;
	
	if(argc != 3){
		printf("Wront Input\n");
		exit(-1);
	}

	if(argv[2][0] == 'r') new_attr |= S_IRUSR;	 
	if(argv[2][1] == 'w') new_attr |= S_IWUSR;
	if(argv[2][2] == 'x') new_attr |= S_IXUSR;
	if(argv[2][3] == 'r') new_attr |= S_IRGRP;	 
	if(argv[2][4] == 'w') new_attr |= S_IWGRP;
	if(argv[2][5] == 'x') new_attr |= S_IXGRP;
	if(argv[2][6] == 'r') new_attr |= S_IROTH;	 
	if(argv[2][7] == 'w') new_attr |= S_IWOTH;
	if(argv[2][8] == 'x') new_attr |= S_IXOTH;

	if(strcmp(argv[1], ".") == 0){
		change_permissions_in_directory(".", new_attr);
	}
	else if (chmod(argv[1], new_attr) != 0) {
        //perror("chmod");
    }
	return 0;
}