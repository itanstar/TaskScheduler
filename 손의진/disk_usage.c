#include <stdio.h>
#include <sys/statvfs.h>

int main() {
    struct statvfs stat;

    // 루트 파일 시스템의 정보를 가져옵니다.
    if (statvfs("/", &stat) != 0) {
        perror("statvfs 실패");
        return 1;
    }

    unsigned long total = stat.f_blocks * stat.f_frsize; // 전체 용량
    unsigned long free = stat.f_bfree * stat.f_frsize;   // 남은 용량
    unsigned long used = total - free;                    // 사용 중인 용량

    printf("총 저장공간: %.2f GB\n", total / (1024.0 * 1024.0 * 1024.0));
    printf("사용 중인 저장공간: %.2f GB\n", used / (1024.0 * 1024.0 * 1024.0));
    printf("남은 저장공간: %.2f GB\n", free / (1024.0 * 1024.0 * 1024.0));

    return 0;
}
