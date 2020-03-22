#ifndef PWM_Module_h
#define PWM_Module_h

#include "driver/ledc.h"

class PWM_Module {
    public:
        void Stop(ledc_channel_t channel_num, bool highspeed);
        void Setup(ledc_timer_t timer_num, ledc_channel_t channel_num, bool highspeed, ledc_timer_bit_t resolution, uint32_t frequency, uint32_t duty, int phase, int out_pin);
    private:
};

#endif

