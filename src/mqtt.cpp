#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "gateway.h"
#include "mqtt.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

/**
 * Loads and sets mqtt certificates and broker address
 * @return: void
 */
void initMqtt()
{
    /** Until ESP32 doesn't include native mDNS resolving this has to be commented out **/
    //    espClient.setCACert((const char *) ca_cert);
    //    espClient.setCertificate((const char *) client_cert);
    //    espClient.setPrivateKey((const char *) client_private_key);
    mqtt.setServer(MQTT_BROKER_IP, 1883);
}

/**
 * Connects to mqtt broker, sets LWT message and publishes device information
 * In order to connect using TLS, time has to be set.
 * @return: 0 if successfully connected, otherwise -1
 */
int connectMqtt(const char * client_id)
{
    if (!mqtt.connect(client_id, MQTT_USER, MQTT_PASS))
    {
        char err_buf[256];

        Sprint("MQTT failed, rc=");
        Sprintln(mqtt.state());
        // espClient.lastError(err_buf, sizeof(err_buf));
        Sprint("SSL error: ");
        Sprintln(err_buf);

        return -1;
    }

    mqtt.publish("/topic", "message is online");
    return 0;
}

/**
 * Checks if mqtt connection is still running
 * @return 0 if not connected, otherwise -1
 */
int isMqttConnected() {
    return mqtt.connected() ? -1 : 0;
}

/**
 * Holds connection open
 */
void runMqtt() {
    mqtt.loop();
}

void publishMqtt(const char *topic, const char *payload) {
    mqtt.publish(topic, payload, false);
}