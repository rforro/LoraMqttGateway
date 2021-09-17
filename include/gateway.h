#ifndef HASSLORAGATEWAY_GATEWAY_H
#define HASSLORAGATEWAY_GATEWAY_H

#include <StringRingBuffer.h>

#define DEVICE_ID "esp32-%08X"
#define DEVICE_NAME "lora2mqtt"

#define HASS_TOPIC_BASE "homeassistant/%s/%s"
#define HASS_TOPIC_SUFFIX_STATE "/state"
#define HASS_TOPIC_SUFFIX_AVAILIBILITY "/available"

#define UID_SUFFIX_CNTR "C"
#define UID_SUFFIX_UPTM "U"

#define HASS_PAYLOAD_CONF_DEVICE "{\"cns\":[[\"mac\",\"%s\"]],\"ids\":\"%08X\",\"mf\":\"espressif\",\"mdl\":\"ESP32\",\"name\":\"Lora2mqtt\"}"
#define HASS_PAYLOAD_CONF_CNTR "{\"~\":\"%s\",\"avty_t\":\"~/available\",\"dev\":%s,\"ic\":\"hass:counter\",\"name\":\"Lora2mqtt Message Counter\",\
\"stat_t\":\"~/state\",\"uniq_id\":\"%s\",\"val_tpl\":\"{{value_json.msg_cntr}}\"}"
#define HASS_PAYLOAD_CONF_UPTM "{\"~\":\"%s\",\"avty_t\":\"~/available\",\"dev\":%s,\"dev_cla\":\"timestamp\",\"name\":\"Lora2mqtt Uptime\",\
\"stat_t\":\"~/state\",\"unit_of_meas\":\"sec\",\"uniq_id\":\"%s\",\"val_tpl\":\"{{value_json.uptime}}\"}"

#define HASS_PAYLOAD_STATE "{\"msg_cntr\":\"%u\",\"uptime\":\"%s\"}"

#define DELAYED_RESTART 30000
#define OLED_REFRESH_INTERVAL_MS (1000 * 30)
#define HASS_REFRESH_INTERVAL_MS (1000 * 60 * 10)

extern StringRingBuffer Strring;

#endif //HASSLORAGATEWAY_GATEWAY_H