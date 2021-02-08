#include <Arduino.h>
#include <ArduinoJson.h>
#include <EspNtpTime.h>
#include <WiFi.h>
#include "gateway.h"
#include "loracomm.h"
#include "mqtt.h"

EspNtpTime NtpTime;
StringRingBuffer Strring(1000);
char client_id[15];

uint32_t espGetChipId();

void setup()
{
  SprintBegin(115200);
  Sprintln("Homeassistant Lora Gateway");

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

  if (snprintf(client_id, sizeof(client_id), "ESP-%08X", espGetChipId()) >= (int) sizeof(client_id)) {
        Sprintln("Client id cannot be constructed");
        ESP.restart();
  };

  Sprint("Starting MQTT connection: ");
  initMqtt();
  if(connectMqtt(client_id) != 0) {
    delay(DELAYED_RESTART);
    ESP.restart();
  }
  Sprintln("success");
}

void loop()
{
  runMqtt();

  if (WiFi.status() != WL_CONNECTED)
  {
    Sprintln("Wifi connection is down, reconnecting...");
    WiFi.reconnect();
  }

  if(!isMqttConnected()) {
    Sprintln("Mqtt connection is down, reconnecting...");
    connectMqtt(client_id);
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

      publishMqtt(topic, payload);
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