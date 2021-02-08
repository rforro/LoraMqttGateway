#include <Arduino.h>
#include <ArduinoJson.h>
#include <EspNtpTime.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "gateway.h"
#include "loracomm.h"
#include "oled.h"

EspNtpTime NtpTime;
StringRingBuffer Strring(1000);
WiFiClient espClient;
PubSubClient mqtt(espClient);

uint32_t espGetChipId();
int connectMqtt();

char client_id[15];
char topic_on_off_line[50];

void setup()
{
  int len_sn;

  SprintBegin(115200);
  Sprintln("Homeassistant Lora Gateway");

  initScreen();

  if (loraInit() != 0)
  {
    Sprintln("Lora cannot be started, rebooting...");
    delay(DELAYED_RESTART);
    ESP.restart();
  }
  loraReceive();
  Sprintln("LoRa running");


  Sprint("Starting wifi connection: ");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.setHostname(WIFI_HOSTNAME);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Sprintln("unable to establish wifi connection, rebooting...");
    delay(DELAYED_RESTART);
    ESP.restart();
  };

  Sprint("success, IP: ");
  Sprint(WiFi.localIP());
  Sprint(" and rssi: ");
  Sprintln(WiFi.RSSI());

  NtpTime.init();
  Sprint("Waiting for NTP time: ");
  if (NtpTime.waitForTime() == false) {
    Sprintln("ERROR, cannot obtain NTP time, rebooting...");
    ESP.restart();
  }
  Sprintln("success");

  len_sn = snprintf(client_id, sizeof(client_id), "ESP-%08X", espGetChipId());
  if (len_sn < 0 || (unsigned) len_sn >= (int) sizeof(client_id)) {
        Sprintln("Client id cannot be constructed");
        ESP.restart();
  };

  len_sn = snprintf(topic_on_off_line, sizeof(topic_on_off_line), MQTT_TOPIC_ONOFFLINE, client_id);
  if (len_sn < 0 || (unsigned) len_sn >= sizeof(topic_on_off_line)) {
    Sprintln("ERROR, online n offline topic truncated");
    return;
  }

  Sprint("Starting MQTT connection: ");
  mqtt.setServer(MQTT_BROKER_IP, 1883);
  if (connectMqtt() != 0) {
    Sprint("ERROR, MQTT failed, rc=");
    Sprintln(mqtt.state());
    delay(DELAYED_RESTART);
    ESP.restart();
  }
  Sprintln("success");

  char payload[50];
  char utc[21];
  if (!NtpTime.getUtcIsoString(utc, sizeof(utc))) {
    Sprintln("ERROR, cannot obtain UTC time");
    delay(DELAYED_RESTART);
    ESP.restart();
  }
  len_sn = snprintf(payload, sizeof(payload), MQTT_PAYLOAD_ONLINE, utc);
  if (len_sn < 0 || (unsigned) len_sn >= sizeof(payload)) {
    Sprintln("ERROR, online payload truncated");
    return;
  }
  mqtt.publish(topic_on_off_line, payload, true);
}

void loop()
{
  unsigned long currMs = millis();
  static unsigned long prevMs = -1;
  static unsigned int msg_counter = 0;

  mqtt.loop();
  
  if (currMs - prevMs >= OLED_REFRESH_INTERVAL_MS) {
    prevMs = currMs;
    bool mqtt_state = mqtt.state() == 0 ? true : false;
    refreshInfoScreen(mqtt_state, WiFi.localIP(), ESP.getFreeHeap(), msg_counter);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Sprintln("Wifi connection is down, reconnecting...");
    WiFi.reconnect();
  }

  if(!mqtt.connected()) {
    Sprintln("Mqtt connection is down, reconnecting...");
    connectMqtt();
  }

  if (!Strring.is_empty()) {
    char encr[300];
    StaticJsonDocument<128> doc;

    Strring.popstr(encr);
    Sprint("Read from strring buffer: ");
    Sprintln(encr);

    DeserializationError error = deserializeJson(doc, encr, sizeof(encr));
    if (error) {
      Sprint("ERROR, deserializeJson() failed: ");
      Sprintln(error.f_str());
      return;
    } else {
      char payload[100];
      const char* topic = doc["t"];

      if (doc["p"].is<JsonObject>()) {
        JsonObject payloadJson = doc["p"].as<JsonObject>();
        if (serializeJson(payloadJson, payload) >= sizeof(payload)) {
          Sprintln("ERROR, payload array too short");
          return;
        }
      } else if(doc["p"].is<char*>()) {
        if (strlcpy(payload, doc["p"], sizeof(payload)) >= sizeof(payload)) {
          Sprintln("ERROR, payload array too short");
          return;
        }
      }

      mqtt.publish(topic, payload, false);
      msg_counter++;
    }
  }
}

/**
 * Returns chip id for ESP32 in correct order
 * @return last 6 numbers from serial number/mac address
 */
uint32_t espGetChipId() {
    uint32_t chip_id = 0;
    for (uint8_t i = 0; i < 17; i = i + 8) {
        chip_id |= ((ESP.getEfuseMac() >> (40u - i)) & 0xffu) << i;
    }
    return chip_id;
}

/**
 * Connects to MQTT server and sets LWT
 * @returns value from PubSubClient.state()
 */
int connectMqtt()
{
    mqtt.connect(client_id, MQTT_USER, MQTT_PASS, topic_on_off_line, 1, true, MQTT_PAYLOAD_OFFLINE);
    return mqtt.state();
}