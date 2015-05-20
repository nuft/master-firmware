#include <string.h>
#include "rpc_callbacks.h"
#include "config.h"
#include <parameter/parameter_msgpack.h>
#include "hal.h"
#include "main.h"
#include "motor_manager.h"
#include "uavcan_node.h"
#include "rc_servo.h"

const char *error_msg_bad_argc = "Error: invalid argument count.";
const char *error_msg_bad_format = "Error: invalid argument format.";
const char *error_msg_invalid_arg = "Error: invalid argument value.";


static void ping_cb(void *p, int argc, cmp_ctx_t *input, cmp_ctx_t *output)
{
    (void) argc;
    (void) input;
    (void) p;
    cmp_write_str(output, "pong", 4);
}

static void msgpack_error_cb(void *arg, const char *id, const char *err)
{
    cmp_ctx_t *output = (cmp_ctx_t *)arg;
    cmp_write_array(output, 2);
    cmp_write_str(output, id, strlen(id));
    cmp_write_str(output, err, strlen(err));
}

static void config_update_cb(void *p, int argc, cmp_ctx_t *input, cmp_ctx_t *output)
{
    (void) argc;
    (void) output;
    (void) p;
    parameter_msgpack_read_cmp(&global_config, input, msgpack_error_cb, (void *)output);
}

static void create_motor_driver(void *p, int argc, cmp_ctx_t *input, cmp_ctx_t *output)
{
    (void) p;
    char actuator_id[25];
    uint32_t actuator_id_len = sizeof(actuator_id);

    if (argc != 1) {
        cmp_write_str(output, error_msg_bad_argc, strlen(error_msg_bad_argc));
        return;
    }

    cmp_read_str(input, actuator_id, &actuator_id_len);

    motor_manager_create_driver(&motor_manager, actuator_id);
}

// takes as argument a MessagePack map with servo nuber and position.
static void rc_servo_set_pos_cb(void *p, int argc, cmp_ctx_t *input, cmp_ctx_t *output)
{
    (void)p;

    if (argc != 1) {
        cmp_write_str(output, error_msg_bad_argc, strlen(error_msg_bad_argc));
        return;
    }

    uint32_t size = 0;
    if (!cmp_read_map(input, &size)) {
        goto bad_arg;
    }

    uint32_t i;
    for (i = 0; i < size; i++) {
        uint32_t servo;
        if (!cmp_read_uint(input, &servo)) {
            goto bad_arg;
        }
        float position;
        if (!cmp_read_float(input, &position)) {
            goto bad_arg;
        }
        rc_servo_set_pos(servo, position);
    }
    return;

bad_arg:
    cmp_write_str(output, error_msg_bad_format, strlen(error_msg_bad_format));
}

static void led_cb(void *p, int argc, cmp_ctx_t *input, cmp_ctx_t *output)
{
    (void) p;

    char led_name[32];
    uint32_t led_name_len;
    bool led_status;
    bool success;

    if (argc != 2) {
        cmp_write_str(output, error_msg_bad_argc, strlen(error_msg_bad_argc));
    }

    led_name_len = sizeof led_name;
    success = true;
    success &= cmp_read_str(input, led_name, &led_name_len);
    success &= cmp_read_bool(input, &led_status);

    if (success) {
        if (!strcmp(led_name, "ready")) {
            palWritePad(GPIOF, GPIOF_LED_READY, led_status);
        } else if (!strcmp(led_name, "debug")) {
            palWritePad(GPIOF, GPIOF_LED_DEBUG, led_status);
        } else if (!strcmp(led_name, "error")) {
            palWritePad(GPIOF, GPIOF_LED_ERROR, led_status);
        } else if (!strcmp(led_name, "power_error")) {
            palWritePad(GPIOF, GPIOF_LED_POWER_ERROR, led_status);
        } else if (!strcmp(led_name, "pc_error")) {
            palWritePad(GPIOF, GPIOF_LED_PC_ERROR, led_status);
        } else if (!strcmp(led_name, "bus_error")) {
            palWritePad(GPIOF, GPIOF_LED_BUS_ERROR, led_status);
        } else if (!strcmp(led_name, "yellow_1")) {
            palWritePad(GPIOF, GPIOF_LED_YELLOW_1, led_status);
        } else if (!strcmp(led_name, "yellow_2")) {
            palWritePad(GPIOF, GPIOF_LED_YELLOW_2, led_status);
        } else if (!strcmp(led_name, "green_1")) {
            palWritePad(GPIOF, GPIOF_LED_GREEN_1, led_status);
        } else if (!strcmp(led_name, "green_2")) {
            palWritePad(GPIOF, GPIOF_LED_GREEN_2, led_status);
        } else {
            cmp_write_str(output, error_msg_invalid_arg,
                                  strlen(error_msg_invalid_arg));

        }
    } else {
        cmp_write_str(output, error_msg_bad_format,
                              strlen(error_msg_bad_format));
    }
}

static void reboot_node(void *p, int argc, cmp_ctx_t *input, cmp_ctx_t *output)
{
    uint8_t id;
    (void) p;

    if (argc != 1) {
        cmp_write_str(output, error_msg_bad_argc, strlen(error_msg_bad_argc));
        return;
    }

    if(cmp_read_u8(input, &id) == false) {
        cmp_write_str(output, error_msg_bad_format,
                strlen(error_msg_bad_format));
        return;
    }

   uavcan_node_send_reboot(id);
}

service_call_method service_call_callbacks[] = {
    {.name="ping", .cb=ping_cb},
    {.name="config_update", .cb=config_update_cb},
    {.name="led_set", .cb=led_cb},
    {.name="actuator_create_driver", .cb=create_motor_driver},
    {.name="reboot_node", .cb=reboot_node},
    {.name="rc_servo_set_pos", .cb=rc_servo_set_pos_cb},
};

const unsigned int service_call_callbacks_len =
    sizeof(service_call_callbacks) / sizeof(service_call_callbacks[0]);
