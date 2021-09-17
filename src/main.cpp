#include <Arduino.h>
#include <ArduinoJson.h>
#include <EspNtpTime.h>
#include <EspChipId.h>
#include <PubSubClient.h>
#include <SerPrint.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "config.h"
#include "gateway.h"
#include "loracomm.h"
#include "oled.h"

EspNtpTime NtpTime;
StringRingBuffer Strring(1000);
WiFiClientSecure espClient;
PubSubClient mqtt(espClient);

int connectMqtt();

char client_id[20];
char uptime[21];

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
  WiFi.setHostname(DEVICE_NAME);

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
  NtpTime.getUtcIsoString(uptime);

  len_sn = snprintf(client_id, sizeof(client_id), DEVICE_ID, EspChipId.get());
  if (len_sn < 0 || (unsigned) len_sn >= (int) sizeof(client_id)) {
        SerPrintln("Client id cannot be constructed");
        ESP.restart();
  };

  SerPrint("Starting MQTT connection: ");
  espClient.setCACert((const char *) ca_crt_start);
  espClient.setCertificate((const char *) certificate_crt_start);
  espClient.setPrivateKey((const char *) private_key_start);
  mqtt.setServer(MQTT_BROKER_IP, 8883);
  if (connectMqtt() != 0) {
    char err_buf[256];

    SerPrint("ERROR, MQTT failed, rc=");
    SerPrintln(mqtt.state());
    espClient.lastError(err_buf, sizeof(err_buf));
    SerPrint("SSL error: ");
    SerPrintln(err_buf);

    delay(DELAYED_RESTART);
    ESP.restart();
  }
  SerPrintln("success");

  //publish HASS mqtt autodiscovery config
  {
    char dev_conf[150], mac_addr[18];
    uint8_t mac[6];
    const char *configs[] = {HASS_PAYLOAD_CONF_CNTR, HASS_PAYLOAD_CONF_UPTM};
    const char *suffixes[] = {UID_SUFFIX_CNTR, UID_SUFFIX_UPTM};
    const char *components[] = {"sensor", "sensor"};

    mqtt.setBufferSize(512);

    WiFi.macAddress(mac);
    sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    if (snprintf(dev_conf, sizeof(dev_conf), HASS_PAYLOAD_CONF_DEVICE, mac_addr, EspChipId.get()) >= (int) sizeof(dev_conf)) {
      SerPrintln("ERROR, device config cannot be constructed");
    }

    for (int i=0; i<(sizeof(configs) / sizeof(*configs)); i++) {
      char config_doc[400], topic_config[75], uid[20], topic_base[75];
      int len_tc;

      len_tc = snprintf(topic_config, sizeof(topic_config), HASS_TOPIC_BASE, components[i], client_id);
      if (len_tc < 0 || (unsigned) len_tc >= sizeof(topic_config)) {
        SerPrintln("ERROR, config topic cannot be constructed");
        return;
      }
      if (strlcat(topic_config, suffixes[i], sizeof(topic_config)) >= sizeof(topic_config)) {
        SerPrintln("ERROR, config topic suffix cannot be constructed");
      }
      if (strlcat(topic_config, "/config", sizeof(topic_config)) >= sizeof(topic_config)) {
        SerPrintln("ERROR, config topic cannot be constructed");
      }
      len_sn = snprintf(uid, sizeof(uid), "%08X", EspChipId.get());
      if (len_sn < 0 || (unsigned) len_sn >= (int) sizeof(uid)) {
        SerPrintln("Client id cannot be constructed");
      };
      if (strlcat(uid, suffixes[i], sizeof(uid)) >= sizeof(uid)) {
        SerPrintln("ERROR, uid cannot be constructed");
      }

      len_tc = snprintf(topic_base, sizeof(topic_base), HASS_TOPIC_BASE, DEVICE_NAME, client_id);
      if (len_tc < 0 || (unsigned) len_tc >= sizeof(topic_base)) {
        SerPrintln("ERROR, base topic cannot be constructed");
        return;
      }

      len_sn = snprintf(config_doc, sizeof(config_doc), configs[i], topic_base, dev_conf, uid);
      if (len_sn < 0 || (unsigned) len_sn >= (int) sizeof(config_doc)) {
        SerPrintln("Client id cannot be constructed");
      };

      mqtt.publish(topic_config, config_doc, true);
      delay(1000);
    }
    mqtt.setBufferSize(256);
  }

  SerPrintln("initialization done");  
}

void loop() {
  unsigned long curr_ms = millis();
  static unsigned long prev_ms = OLED_REFRESH_INTERVAL_MS, prev_ms_hass = HASS_REFRESH_INTERVAL_MS, mqtt_connect_ms;
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

  if (!mqtt.connected()) {
    return;  // no point to execute the rest of the code if mqtt is disconnected
  }

  if (curr_ms - prev_ms_hass >= HASS_REFRESH_INTERVAL_MS) {
    char state[100];
    char top[100];
    prev_ms_hass = curr_ms;
    int len_sn;

    snprintf(top, sizeof(top), HASS_TOPIC_BASE, DEVICE_NAME, client_id);
    if (strlcat(top, HASS_TOPIC_SUFFIX_STATE, sizeof(top)) >= sizeof(top)) {
      SerPrintln("ERROR, state topic truncated");
    }

    len_sn = snprintf(state, sizeof(state), HASS_PAYLOAD_STATE, msg_counter, uptime);
    if (len_sn < 0 || (unsigned) len_sn >= sizeof(state)) SerPrintln("ERROR, payload state truncated");

    mqtt.publish(top, state, false);
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
  int len_tp;
  char will_topic[75];

  len_tp = snprintf(will_topic, sizeof(will_topic), HASS_TOPIC_BASE, DEVICE_NAME, client_id);
  if (len_tp < 0 || (unsigned) len_tp >= sizeof(will_topic)) {
    SerPrintln("ERROR, will topic truncated");
    return -5;
  }
  if (strlcat(will_topic, HASS_TOPIC_SUFFIX_AVAILIBILITY, sizeof(will_topic)) >= sizeof(will_topic)) {
    SerPrintln("ERROR, will topic cannot be constructed");
  }

  if (mqtt.connect(client_id, will_topic, 1, true, "offline")) {
    mqtt.publish(will_topic, "online", true);
  }

  return mqtt.state();
}