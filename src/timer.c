#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

typedef struct {
    time_t target_time;    
    int duration;          
    pid_t target_pid;      
    int signal_number;     
    bool use_target_time;  
} TimerConfig;

// 시그널 전송 함수
void send_timer_signal(pid_t pid, int signum) 
{
	kill(pid, signum);
}

// 특정 시각이 되었는지 확인하는 함수
bool check_target_time(time_t target_time) 
{
    time_t current_time = time(NULL);
    return current_time >= target_time;
}

// 특정 시간이 경과했는지 확인하는 함수
bool check_duration(time_t start_time, int duration) 
{
    time_t current_time = time(NULL);
    return (current_time - start_time) >= duration;
}

// 타이머 이벤트를 감지하고 시그널을 보내는 메인 함수
void detect_timer_events(TimerConfig *config) 
{
    time_t start_time = time(NULL);
    while (1) 
    {
        if (config->use_target_time) 
        {
            if (check_target_time(config->target_time)) 
            {
                send_timer_signal(config->target_pid, config->signal_number);
                break;
            }
        } 
        else 
        {
            if (check_duration(start_time, config->duration)) 
            {
                send_timer_signal(config->target_pid, config->signal_number);
                break;
            }
        }
        sleep(1);
    }
}

// 시계 시간을 Unix timestamp로 변환
time_t convert_clock_time_to_timestamp(int hour, int minute) 
{
    time_t now = time(NULL);
    struct tm *target_tm = localtime(&now);
    
    target_tm->tm_hour = hour;
    target_tm->tm_min = minute;
    target_tm->tm_sec = 0;
    
    time_t target_time = mktime(target_tm);
    
    // 만약 지정된 시각이 현재 시각보다 이전이라면, 다음 날로 설정
    if (target_time <= now) 
    {
        target_tm->tm_mday += 1;  // 다음 날로 설정
        target_time = mktime(target_tm);
    }
    
    return target_time;
}

// 타이머 설정 함수 - 시계 시각용
TimerConfig* set_clock_time_timer(int hour, int minute, pid_t pid, int signum) 
{
    TimerConfig* config = malloc(sizeof(TimerConfig));
    config->target_time = convert_clock_time_to_timestamp(hour, minute);
    config->target_pid = pid;
    config->signal_number = signum;
    config->use_target_time = true;
    return config;
}

// 타이머 설정 함수 - 지속 시간용
TimerConfig* set_duration_timer(int duration, pid_t pid, int signum) 
{
    TimerConfig* config = malloc(sizeof(TimerConfig));
    config->duration = duration;
    config->target_pid = pid;
    config->signal_number = signum;
    config->use_target_time = false;
    return config;
}

/*int main() 
{
    // 다른 프로세스(PID: 1234)에 5초 후 SIGUSR1 시그널 보내기
    TimerConfig* duration_timer = set_duration_timer(5, getppid(), SIGUSR1);
    detect_timer_events(duration_timer);
    free(duration_timer);
    //첫번째에서 10으로 종류랑 실제 시간이랑 기간 

    // 22시 30분에 SIGUSR2 시그널 보내기 ㅜ멀 실행하라라는 코드를 남길 때는 22:30으로 전달이 될 거라 
    // strcat 써서 hour에 22넣고 min에 30 넣어서 원래 코드에 이식하는 그런 기능이 추가되어야 할 것
    TimerConfig* clock_time_timer = set_clock_time_timer(22, 30, getppid(), SIGUSR2);
    detect_timer_events(clock_time_timer);
    free(clock_time_timer);
    
    return 0;
}*/
int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        return 1;
    }

    if (strcmp(argv[1], "TIMER") == 0) 
    {
        if (argc != 3) 
        {
            return 1;
        }
        int seconds = atoi(argv[2]);
        TimerConfig* duration_timer = set_duration_timer(seconds, getppid(), SIGUSR1);
        detect_timer_events(duration_timer);
        free(duration_timer);
    }
    else if (strcmp(argv[1], "ALARM") == 0) 
    {
        if (argc != 3) 
        {
            return 1;
        }
        
        char hour_str[3] = "";
        char min_str[3] = "";
        char *time_str = argv[2];
        int i;
        
        // 시간 부분 추출
        for(i = 0; time_str[i] != ':' && time_str[i] != '\0' && i < 2; i++) 
        {
            char temp[2] = {time_str[i], '\0'};
            strcat(hour_str, temp);
        }
        
        if(time_str[i] != ':') 
        {
            return 1;
        }
        
        // 분 부분 추출
        i++;
        int j = 0;
        while(time_str[i] != '\0' && j < 2) 
        {
            char temp[2] = {time_str[i], '\0'};
            strcat(min_str, temp);
            i++;
            j++;
        }
        
        int hour = atoi(hour_str);
        int minute = atoi(min_str);
        
        if(hour < 0 || hour > 23 || minute < 0 || minute > 59) 
        {
            return 1;
        }

        TimerConfig* clock_time_timer = set_clock_time_timer(hour, minute, getppid(), SIGUSR1);
        detect_timer_events(clock_time_timer);
        free(clock_time_timer);
    }
    else 
    {
        return 1;
    }

    return 0;
}
