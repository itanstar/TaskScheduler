#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

typedef struct {
    time_t target_time;    // 특정 시각까지
    int duration;          // 특정 시간 동안 (초 단위)
    pid_t target_pid;      // 시그널을 보낼 프로세스 ID
    int signal_number;     // 보낼 시그널 번호
    bool use_target_time;  // true면 특정 시각, false면 지속 시간 사용
} TimerConfig;

// 시그널 전송 함수
void send_timer_signal(pid_t pid, int signum) {
    if (kill(pid, signum) == -1) {
        perror("Failed to send signal");
    } else {
        printf("Signal %d sent to process %d\n", signum, pid);
    }
}

// 특정 시각이 되었는지 확인하는 함수
bool check_target_time(time_t target_time) {
    time_t current_time = time(NULL);
    return current_time >= target_time;
}

// 특정 시간이 경과했는지 확인하는 함수
bool check_duration(time_t start_time, int duration) {
    time_t current_time = time(NULL);
    return (current_time - start_time) >= duration;
}

// 타이머 이벤트를 감지하고 시그널을 보내는 메인 함수
void detect_timer_events(TimerConfig *config) {
    time_t start_time = time(NULL);
    
    while (1) {
        if (config->use_target_time) {
            // 특정 시각 도달 확인
            if (check_target_time(config->target_time)) {
                printf("지정된 시각에 도달했습니다!\n");
                send_timer_signal(config->target_pid, config->signal_number);
                break;
            }
        } else {
            // 지정된 시간 경과 확인
            if (check_duration(start_time, config->duration)) {
                printf("지정된 시간이 경과했습니다!\n");
                send_timer_signal(config->target_pid, config->signal_number);
                break;
            }
        }
        
        sleep(1); // CPU 부하를 줄이기 위해 1초 대기
    }
}

// 타이머 설정 함수 - 특정 시각용
TimerConfig* set_target_time_timer(time_t target_time, pid_t pid, int signum) {
    TimerConfig* config = malloc(sizeof(TimerConfig));
    config->target_time = target_time;
    config->target_pid = pid;
    config->signal_number = signum;
    config->use_target_time = true;
    return config;
}

// 타이머 설정 함수 - 지속 시간용
TimerConfig* set_duration_timer(int duration, pid_t pid, int signum) {
    TimerConfig* config = malloc(sizeof(TimerConfig));
    config->duration = duration;
    config->target_pid = pid;
    config->signal_number = signum;
    config->use_target_time = false;
    return config;
}

int main() {
    // 예시: 다른 프로세스(PID: 1234)에 5초 후 SIGUSR1 시그널 보내기
    TimerConfig* duration_timer = set_duration_timer(5, 1234, SIGUSR1);
    detect_timer_events(duration_timer);
    free(duration_timer);
    
    // 예시: 특정 시각(현재 시간 + 10초)에 SIGUSR2 시그널 보내기
    TimerConfig* target_time_timer = set_target_time_timer(time(NULL) + 10, 1234, SIGUSR2);
    detect_timer_events(target_time_timer);
    free(target_time_timer);
    
    return 0;
}