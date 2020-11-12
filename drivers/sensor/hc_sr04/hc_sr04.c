#define DT_DRV_COMPAT elecfreaks_hc_sr04

#include <kernel.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <device.h>
#include <sensor/hc_sr04.h>
#include <devicetree.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(hc_sr04, CONFIG_HC_SR04_LOG_LEVEL);

struct hc_sr04_data {
    struct sensor_value   sensor_value;
};

struct hc_sr04_cfg {
;
    const uint8_t trig_pin;
    const uint8_t echo_pin;
};

static int hc_sr04_init(const struct device *dev)
{
    int err;

    struct hc_sr04_data      *p_data = dev->data;
    const struct hc_sr04_cfg *p_cfg  = dev->config;

    p_data->sensor_value.val1 = 0;
    p_data->sensor_value.val2 = 0;

    return 0;
}

static int hc_sr04_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    int      err;
    uint32_t count;

    struct hc_sr04_data      *p_data = dev->data;
    const struct hc_sr04_cfg *p_cfg  = dev->config;

    if (unlikely((SENSOR_CHAN_ALL != chan) && (SENSOR_CHAN_DISTANCE != chan))) {
        return -ENOTSUP;
    }
    return 0;
}

static int hc_sr04_channel_get(const struct device *dev,
                    enum sensor_channel chan,
                    struct sensor_value *val)
{
    const struct hc_sr04_data *p_data = dev->data;

    switch (chan) {
    case SENSOR_CHAN_DISTANCE:
        val->val2 = p_data->sensor_value.val2;
        val->val1 = p_data->sensor_value.val1;
        break;
    default:
        return -ENOTSUP;
    }
    return 0;
}

static const struct sensor_driver_api hc_sr04_driver_api = {
    .sample_fetch = hc_sr04_sample_fetch,
    .channel_get  = hc_sr04_channel_get,
};

#define HC_SR04_DEVICE(n) \
    static const struct hc_sr04_cfg hc_sr04_cfg_##n = { \
        .trig_pin   = DT_GPIO_PIN(DT_NODELABEL(us##n), trig_gpios), \
        .echo_pin   = DT_GPIO_PIN(DT_NODELABEL(us##n), echo_gpios), \
    }; \
    static struct hc_sr04_data hc_sr04_data_##n; \
    DEVICE_AND_API_INIT(hc_sr04_##n, \
                DT_LABEL(DT_NODELABEL(us##n)), \
                hc_sr04_init, \
                &hc_sr04_data_##n, \
                &hc_sr04_cfg_##n, \
                POST_KERNEL, \
                CONFIG_SENSOR_INIT_PRIORITY, \
                &hc_sr04_driver_api);

#if DT_NODE_HAS_STATUS(DT_NODELABEL(us0), okay)
HC_SR04_DEVICE(0)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(us1), okay)
HC_SR04_DEVICE(1)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(us2), okay)
HC_SR04_DEVICE(2)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(us3), okay)
HC_SR04_DEVICE(3)
#endif

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "HC_SR04 driver enabled without any devices"
#endif

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) > 4
#warning "Too many HC_SR04 devices"
#endif
