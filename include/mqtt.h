#ifndef HASSLORAGATEWAY_MQTT_H
#define HASSLORAGATEWAY_MQTT_H

void initMqtt();
int connectMqtt(const char * client_id);
int isMqttConnected();
void runMqtt();
void publishMqtt(const char *topic, const char *payload);

#endif //HASSLORAGATEWAY_MQTT_H