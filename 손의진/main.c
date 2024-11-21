#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct cpu
{
    int user;
    int nice;
    int system;
    int idle;
    int iowait;
    int irq;
    int softirq;
    int steal;
    int guest;
    int guest_nice;
} cpu;


void get_cpu_usage() {
    FILE *fp;
    cpu cpu_st;

    double prev_usage, cur_usage;
    double total, idle;
    double usage;

    // 처음 CPU 사용량을 읽어옴
    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    fscanf(fp, "cpu %d %d %d %d %d %d %d %d %d %d", 
           &cpu_st.user, &cpu_st.nice, &cpu_st.system, 
           &cpu_st.idle, &cpu_st.iowait, &cpu_st.irq, 
           &cpu_st.softirq, &cpu_st.steal, 
           &cpu_st.guest, &cpu_st.guest_nice);
    
    total = cpu_st.user + cpu_st.nice + cpu_st.system + cpu_st.idle + cpu_st.irq + cpu_st.softirq + cpu_st.steal;
    idle = cpu_st.idle + cpu_st.iowait;
    prev_usage = (1 - (double)idle / (double)total) * 100;
    printf("prev_usage : %f\n", prev_usage);
    fclose(fp);

    // 잠시 대기
    sleep(3);

    // 다시 CPU 사용량을 읽어옴
    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    fscanf(fp, "cpu %d %d %d %d %d %d %d %d %d %d", 
           &cpu_st.user, &cpu_st.nice, &cpu_st.system, 
           &cpu_st.idle, &cpu_st.iowait, &cpu_st.irq, 
           &cpu_st.softirq, &cpu_st.steal, 
           &cpu_st.guest, &cpu_st.guest_nice);
           
    total = cpu_st.user + cpu_st.nice + cpu_st.system + cpu_st.idle + cpu_st.irq + cpu_st.softirq + cpu_st.steal;
    idle = cpu_st.idle + cpu_st.iowait;
    cur_usage = (1 - (double)idle / (double)total) * 100;
    printf("cur_usage : %f\n", cur_usage);
    fclose(fp);

    // CPU 사용량 계산
    usage = (prev_usage + cur_usage) / 2;

    printf("CPU Usage: %.2f%%\n", usage);
}

int main() {
    get_cpu_usage();
    return 0;
}
