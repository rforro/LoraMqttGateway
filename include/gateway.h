#ifndef HASSLORAGATEWAY_GATEWAY_H
#define HASSLORAGATEWAY_GATEWAY_H

#include <StringRingBuffer.h>

#define MQTT_TOPIC_ONOFFLINE "smartburg/device/%s/state"
#define MQTT_PAYLOAD_ONLINE "{\"date\":\"%s\",\"state\":\"online\"}"
#define MQTT_PAYLOAD_OFFLINE "{\"state\":\"offline\"}"

#define DELAYED_RESTART 30000
#define OLED_REFRESH_INTERVAL_MS (1000 * 30)

extern StringRingBuffer Strring;

#endif //HASSLORAGATEWAY_GATEWAY_H