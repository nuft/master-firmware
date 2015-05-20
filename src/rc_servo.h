#ifndef RC_SERVO_H
#define RC_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

void rc_servo_init(void);

// pos range: 0.0 <= pos <= 1.0
void rc_servo_set_pos(unsigned int servo, float pos);

#ifdef __cplusplus
}
#endif

#endif /* RC_SERVO_H */
