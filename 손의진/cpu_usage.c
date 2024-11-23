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

    double total, idle;
    double usage;

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
    usage = (1 - (double)idle / (double)total) * 100;
    printf("CPU usage : %f\n", usage);
    fclose(fp);

}

int main(int argc, char* argv[]) {
    while (1) {
        get_cpu_usage();
        sleep(3);
    }
    return 0;
}
