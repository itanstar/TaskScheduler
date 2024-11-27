#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct {
    time_t target_time;    // 특정 시각까지
    int duration;          // 특정 시간 동안 (초 단위)
} TimerConfig;

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

// 타이머 이벤트를 감지하는 메인 함수
void detect_timer_events(TimerConfig *config) 
{
    time_t start_time = time(NULL);
    
    while (1) 
    {
        // 특정 시각 도달 확인
        if (check_target_time(config->target_time)) 
        {
            printf("지정된 시각에 도달했습니다!\n");
            // 여기에 실행할 작업 추가
            break;
        }
        
        // 지정된 시간 경과 확인
        if (check_duration(start_time, config->duration)) 
        {
            printf("지정된 시간이 경과했습니다!\n");
            // 여기에 실행할 작업 추가
            break;
        }
        
        sleep(1); // CPU 부하를 줄이기 위해 1초 대기
    }
}

// 사용 예시
int main() 
{
    TimerConfig config;
    
    // 5초 후의 시각을 목표 시각으로 설정
    config.target_time = time(NULL) + 5;
    // 3초 동안을 지속 시간으로 설정
    config.duration = 3;
    
    detect_timer_events(&config);
    
    return 0;
}