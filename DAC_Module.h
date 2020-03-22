#ifndef DAC_Module_h
#define DAC_Module_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "soc/rtc_io_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc.h"

#include "driver/dac.h"

class DAC_Module {
    public:
        void Stop(dac_channel_t channel);
        void Setup(dac_channel_t channel, int clk_div, int frequency, int scale, int phase, int invert);
    private:
        void dac_cosine_enable(dac_channel_t channel, int invert);
        void dac_cosine_disable(dac_channel_t channel);
        void dac_frequency_set(int clk_8m_div, int frequency_step);
        void dac_scale_set(dac_channel_t channel, int scale);
        void dac_offset_set(dac_channel_t channel, int offset);
        void dac_invert_set(dac_channel_t channel, int invert);
};

#endif

