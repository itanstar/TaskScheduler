#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("파일 열기 실패");
        return EXIT_FAILURE;
    }

    char line[256];
    long total_memory = 0;
    long free_memory = 0;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "MemTotal: %ld kB", &total_memory) == 1) {
            // 총 메모리
        } else if (sscanf(line, "MemAvailable: %ld kB", &free_memory) == 1) {
            // 사용 가능한 메모리
        }
    }

    fclose(file);

    printf("총 메모리: %.2f MB\n", total_memory / 1024.0);
    printf("사용 가능한 메모리: %.2f MB\n", free_memory / 1024.0);
    printf("사용 중인 메모리: %.2f MB\n", (total_memory - free_memory) / 1024.0);

    return 0;
}
