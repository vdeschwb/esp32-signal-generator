#include "PWM_Module.h"

void PWM_Module::Stop(ledc_channel_t channel_num, bool highspeed) {
    ledc_mode_t speed_mode = highspeed ? LEDC_HIGH_SPEED_MODE : LEDC_LOW_SPEED_MODE;
    ledc_stop(speed_mode, channel_num, 0);
}

void PWM_Module::Setup(ledc_timer_t timer_num, ledc_channel_t channel_num, bool highspeed, ledc_timer_bit_t resolution, uint32_t frequency, uint32_t duty, int phase, int out_pin) {
    ledc_mode_t speed_mode = highspeed ? LEDC_HIGH_SPEED_MODE : LEDC_LOW_SPEED_MODE;

    ledc_timer_config_t timer_config; 
    timer_config.speed_mode = speed_mode;       // timer mode
    timer_config.duty_resolution = resolution;  // resolution of PWM duty, e.g. LEDC_TIMER_10_BIT
    timer_config.timer_num = timer_num;         // timer index
    timer_config.freq_hz = frequency;           // frequency of PWM signal

    ledc_channel_config_t channel_config = {
        .gpio_num   = out_pin,
        .speed_mode = speed_mode,
        .channel    = channel_num,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = timer_num,
        .duty       = duty,
        .hpoint     = phase
    };

    ledc_timer_config(&timer_config);
    ledc_channel_config(&channel_config);
}
