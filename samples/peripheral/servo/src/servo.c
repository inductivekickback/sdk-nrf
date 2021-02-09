#include "servo.h"

// If this flag is not set in the sequence values that are passed to
// nrf_drv_pwm then the polarity of the PWM waveform will be inverted.
// Inverted values are not useful to a servo so it is assumed that if
// a value in a sequence doesn't have this flag set then that channel
// is not enabled.
#define CH_ENABLED_MASK (0x8000UL)

// The RadioShack micro servo's usable range.
#define RSMS_MIN_VALUE     (600UL)
#define RSMS_MAX_VALUE     (2500UL)
#define RSMS_NEUTRAL_VALUE (((RSMS_MAX_VALUE-RSMS_MIN_VALUE)/2)+RSMS_MIN_VALUE)

// Simple functions for mapping a value betwen [0, 100] to the
// range [RSMS_MIN_VALUE, RSMS_MAX_VALUE] and vice versa.
#define MAP(x) ((x) * (RSMS_MAX_VALUE - RSMS_MIN_VALUE)/100 + RSMS_MIN_VALUE)
#define PAM(x) (((x) - RSMS_MIN_VALUE) * 100 / (RSMS_MAX_VALUE-RSMS_MIN_VALUE))


uint32_t servo_group_init(servo_group_t * p_group,
                              uint8_t ch0_pin,
                              uint8_t ch1_pin,
                              uint8_t ch2_pin,
                              uint8_t ch3_pin)
{
    uint32_t err_code;

    if (NULL == p_group)
    {
        return NRFX_ERROR_INVALID_PARAM;
    }

    p_group->pwm_instance.p_registers  = NRFX_CONCAT_2(NRF_PWM, CONFIG_PWM_PERIPH_IDX);
    p_group->pwm_instance.drv_inst_idx = NRFX_CONCAT_3(NRFX_PWM, CONFIG_PWM_PERIPH_IDX, _INST_IDX);

    nrfx_pwm_config_t const pwm_config =
    {
        .output_pins =
        {
            ch0_pin,
            ch1_pin,
            ch2_pin,
            ch3_pin
        },
        .irq_priority = 0,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 20000, // 20ms
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    err_code = nrfx_pwm_init(&p_group->pwm_instance, &pwm_config, NULL, NULL);
    if (NRFX_SUCCESS != err_code)
    {
        return err_code;
    }

    if (SERVO_PIN_NOT_USED != ch0_pin)
    {
        p_group->pwm_values.channel_0 = (CH_ENABLED_MASK|RSMS_NEUTRAL_VALUE);
    }
    else
    {
        p_group->pwm_values.channel_0 = 0;
    }

    if (SERVO_PIN_NOT_USED != ch1_pin)
    {
        p_group->pwm_values.channel_1 = (CH_ENABLED_MASK|RSMS_NEUTRAL_VALUE);
    }
    else
    {
        p_group->pwm_values.channel_1 = 0;
    }

    if (SERVO_PIN_NOT_USED != ch2_pin)
    {
        p_group->pwm_values.channel_2 = (CH_ENABLED_MASK|RSMS_NEUTRAL_VALUE);
    }
    else
    {
        p_group->pwm_values.channel_2 = 0;
    }

    if (SERVO_PIN_NOT_USED != ch3_pin)
    {
        p_group->pwm_values.channel_3 = (CH_ENABLED_MASK|RSMS_NEUTRAL_VALUE);
    }
    else
    {
        p_group->pwm_values.channel_3 = 0;
    }

    nrf_pwm_sequence_t const seq =
    {
        .values.p_individual = &p_group->pwm_values,
        .length              = 4,
        .repeats             = 0,
        .end_delay           = 0
    };
    err_code = nrfx_pwm_simple_playback(&p_group->pwm_instance,
                                            &seq,
                                            1,
                                            NRFX_PWM_FLAG_LOOP);
    /* NOTE: nrfx_pwm_simple_playback returns NULL instead of NRFX_SUCCESS. */
    if (err_code) {
    	return NRFX_ERROR_INTERNAL;
    }
    return NRFX_SUCCESS;
}


uint32_t servo_value_get(servo_group_t * p_group,
                         uint8_t ch_index,
                         uint8_t * p_value)
{
    uint16_t * p_channel_value;

    switch (ch_index)
    {
    case 0:
        p_channel_value = &p_group->pwm_values.channel_0;
        break;
    case 1:
        p_channel_value = &p_group->pwm_values.channel_1;
        break;
    case 2:
        p_channel_value = &p_group->pwm_values.channel_2;
        break;
    case 3:
        p_channel_value = &p_group->pwm_values.channel_3;
        break;
    default:
        return NRFX_ERROR_INVALID_PARAM;
    }

    if (0 == (*p_channel_value & CH_ENABLED_MASK))
    {
        return NRFX_ERROR_INVALID_PARAM;
    }

    *p_value = (uint8_t)PAM((*p_channel_value & ~CH_ENABLED_MASK));

    return NRFX_SUCCESS;
}


uint32_t servo_value_set(servo_group_t * p_group,
                         uint8_t ch_index,
                         uint8_t value)
{
    uint16_t * p_channel_value;

    if (SERVO_MAX_VALUE < value)
    {
        return NRFX_ERROR_INVALID_PARAM;
    }

    switch (ch_index)
    {
    case 0:
        p_channel_value = &p_group->pwm_values.channel_0;
        break;
    case 1:
        p_channel_value = &p_group->pwm_values.channel_1;
        break;
    case 2:
        p_channel_value = &p_group->pwm_values.channel_2;
        break;
    case 3:
        p_channel_value = &p_group->pwm_values.channel_3;
        break;
    default:
        return NRFX_ERROR_INVALID_PARAM;
    }

    if (0 == (*p_channel_value & CH_ENABLED_MASK))
    {
        return NRFX_ERROR_INVALID_PARAM;
    }

    *p_channel_value = (MAP(value) | CH_ENABLED_MASK);

    return NRFX_SUCCESS;
}
