#include <Arduino.h>
#include <ArduinoJson.h>
#include <EspNtpTime.h>
#include <EspChipId.h>
#include <PubSubClient.h>
#include <SerPrint.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "config.h"
#include "gateway.h"
#include "loracomm.h"
#include "oled.h"

EspNtpTime NtpTime;
StringRingBuffer Strring(1000);
WiFiClient espClient;
PubSubClient mqtt(espClient);

int connectMqtt();

char client_id[17];
char topic_on_off_line[50];

void setup()
{
  int len_sn;

  SerBegin(115200);
  SerPrintln("Homeassistant Lora Gateway");

  initScreen();

  if (loraInit() != 0)
  {
    SerPrintln("Lora cannot be started, rebooting...");
    delay(DELAYED_RESTART);
    ESP.restart();
  }
  loraReceive();
  SerPrintln("LoRa running");


  SerPrint("Starting wifi connection: ");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.setHostname(WIFI_HOSTNAME);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    SerPrintln("unable to establish wifi connection, rebooting...");
    delay(DELAYED_RESTART);
    ESP.restart();
  };
  SerPrint("success, IP: ");
  SerPrint(WiFi.localIP());
  SerPrint(" and rssi: ");
  SerPrintln(WiFi.RSSI());

  NtpTime.init();
  SerPrint("Waiting for NTP time: ");
  if (NtpTime.waitForTime() == false) {
    SerPrintln("ERROR, cannot obtain NTP time, rebooting...");
    ESP.restart();
  }
  SerPrintln("success");

  len_sn = snprintf(client_id, sizeof(client_id), "ESP32-%08X", EspChipId.get());
  if (len_sn < 0 || (unsigned) len_sn >= (int) sizeof(client_id)) {
        SerPrintln("Client id cannot be constructed");
        ESP.restart();
  };

  len_sn = snprintf(topic_on_off_line, sizeof(topic_on_off_line), MQTT_TOPIC_ONOFFLINE, client_id);
  if (len_sn < 0 || (unsigned) len_sn >= sizeof(topic_on_off_line)) {
    SerPrintln("ERROR, online n offline topic truncated");
    return;
  }

  SerPrint("Starting MQTT connection: ");
  mqtt.setServer(MQTT_BROKER_IP, 1883);
  if (connectMqtt() != 0) {
    SerPrint("ERROR, MQTT failed, rc=");
    SerPrintln(mqtt.state());
    delay(DELAYED_RESTART);
    ESP.restart();
  }

  SerPrintln("initialization done");  
}

void loop() {
  unsigned long curr_ms = millis();
  static unsigned long prev_ms = -1, mqtt_connect_ms;
  static unsigned int msg_counter = 0;
  
  if (curr_ms - prev_ms >= OLED_REFRESH_INTERVAL_MS) {
    prev_ms = curr_ms;
    bool mqtt_state = mqtt.state() == 0 ? true : false;
    refreshInfoScreen(mqtt_state, WiFi.localIP(), ESP.getFreeHeap(), msg_counter);
  }

  if (!mqtt.loop() && (WiFi.status() == WL_CONNECTED)) {
    SerPrintln("Mqtt connection is down, reconnecting...");
    static uint8_t retry_nr = 0;
    if (curr_ms - mqtt_connect_ms >= pow(2, retry_nr) * 1000) {
      mqtt_connect_ms = curr_ms;
      if (connectMqtt() == 0) {
          retry_nr = 0;
      } else {
          retry_nr = (retry_nr + 1) % 17;
      }
    }
  }

  if (!Strring.is_empty()) {
    char encr[300];
    StaticJsonDocument<128> doc;

    Strring.popstr(encr);
    SerPrint("Read from strring buffer: ");
    SerPrintln(encr);

    DeserializationError error = deserializeJson(doc, encr, sizeof(encr));
    if (error) {
      SerPrint("ERROR, deserializeJson() failed: ");
      SerPrintln(error.f_str());
      return;
    } else {
      char payload[100];
      const char* topic = doc["t"];

      if (doc["p"].is<JsonObject>()) {
        JsonObject payloadJson = doc["p"].as<JsonObject>();
        if (serializeJson(payloadJson, payload) >= sizeof(payload)) {
          SerPrintln("ERROR, payload array too short");
          return;
        }
      } else if(doc["p"].is<char*>()) {
        if (strlcpy(payload, doc["p"], sizeof(payload)) >= sizeof(payload)) {
          SerPrintln("ERROR, payload array too short");
          return;
        }
      }

      mqtt.publish(topic, payload, false);
      msg_counter++;
    }
  }
}

/**
 * Connects to MQTT server and sets LWT
 * @returns value from PubSubClient.state()
 */
int connectMqtt() {
  char payload[50];
  char utc[21];

  if (mqtt.connect(client_id, MQTT_USER, MQTT_PASS, topic_on_off_line, 1, true, MQTT_PAYLOAD_OFFLINE)) {
    if (!NtpTime.getUtcIsoString(utc, sizeof(utc))) {
      SerPrintln("ERROR, cannot obtain UTC time");
      return -6;
    }
    int len_sn = snprintf(payload, sizeof(payload), MQTT_PAYLOAD_ONLINE, utc);
    if (len_sn < 0 || (unsigned)len_sn >= sizeof(payload)) {
      SerPrintln("ERROR, online payload truncated");
      return -5;
    }
    mqtt.publish(topic_on_off_line, payload, true);
  }

  return mqtt.state();
}