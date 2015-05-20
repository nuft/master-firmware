#include <ch.h>
#include <hal.h>
#include "rc_servo.h"

#define RC_SERVO_FREQ       1000000 // 1MHz
#define RC_SERVO_PWM_PERIOD 20000   // 20 ms period

static bool rc_servo_initialized = false;

struct servo_list {
    PWMDriver *driver;
    pwmchannel_t channel;
};

static const struct servo_list servo_list[] =
{
    {.driver = &PWMD9, .channel = 0},   // PE5,  TIM9_CH1
    {.driver = &PWMD9, .channel = 1},   // PE6,  TIM9_CH2

    {.driver = &PWMD1, .channel = 0},   // PE9,  TIM1_CH1
    {.driver = &PWMD1, .channel = 1},   // PE11, TIM1_CH2
    {.driver = &PWMD1, .channel = 2},   // PE13, TIM1_CH3
    {.driver = &PWMD1, .channel = 3},   // PE14, TIM1_CH4

    {.driver = &PWMD4, .channel = 0},   // PD12, TIM4_CH1
    {.driver = &PWMD4, .channel = 1},   // PD13, TIM4_CH2
    {.driver = &PWMD4, .channel = 2},   // PD14, TIM4_CH3
    {.driver = &PWMD4, .channel = 3}    // PD15, TIM4_CH4
};

#define SERVO_LIST_LEN (sizeof(servo_list)/sizeof(struct servo_list))

static const PWMConfig pwmcfg = {
    RC_SERVO_FREQ,
    RC_SERVO_PWM_PERIOD,
    NULL,   // no callback
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL}
    },
    0,      // TIMx_CR2 value
    0       // TIMx_DIER value
};

void rc_servo_init(void)
{
    rc_servo_initialized = true;
    pwmStart(&PWMD9, &pwmcfg);
    pwmStart(&PWMD1, &pwmcfg);
    pwmStart(&PWMD4, &pwmcfg);
}

// returns counter corresponding to a period of 1 to 2 ms.
static pwmcnt_t pos_to_pwmcnt(float pos)
{
    if (pos > 1) {
        pos = 1;
    } else if (pos < 0) {
        pos = 0;
    }
    return (pwmcnt_t)(pos * RC_SERVO_FREQ / 1000) + 1000;
}

void rc_servo_set_pos(unsigned int servo, float pos)
{
    if (!rc_servo_initialized || servo >= SERVO_LIST_LEN) {
        return;
    }
    pwmcnt_t cnt = pos_to_pwmcnt(pos);
    pwmEnableChannel(servo_list[servo].driver, servo_list[servo].channel, cnt);
}

