#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiManager.h>
#include <memory>

#ifndef PRODUCT_NAME
#define PRODUCT_NAME "OBA-Relays-V1"
#endif

#ifndef PRODUCT_MODEL
#define PRODUCT_MODEL "OBA-RELAYS-ESP01-V1"
#endif

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

#ifndef UPDATE_MANIFEST_URL
#define UPDATE_MANIFEST_URL "https://obacanak-iot.github.io/OBA-Relays-V1/oba-relays-v1/version.json"
#endif

#ifndef CONFIG_PORTAL_TIMEOUT_SECONDS
#define CONFIG_PORTAL_TIMEOUT_SECONDS 180
#endif

static constexpr uint32_t MQTT_RECONNECT_MS = 5000;
static constexpr uint32_t DISCOVERY_PUBLISH_MS = 60000;
static constexpr uint32_t DIAGNOSTIC_PUBLISH_MS = 60000;
static constexpr uint32_t UPDATE_CHECK_MS = 21600000;
static constexpr uint16_t EEPROM_SIZE = 256;
static constexpr uint8_t EEPROM_MAGIC = 0x52;
static constexpr uint8_t EEPROM_VERSION = 1;

static const uint8_t ROLE1_ON[] = {0xA0, 0x01, 0x01, 0xA2};
static const uint8_t ROLE1_OFF[] = {0xA0, 0x01, 0x00, 0xA1};
static const uint8_t ROLE2_ON[] = {0xA0, 0x02, 0x01, 0xA3};
static const uint8_t ROLE2_OFF[] = {0xA0, 0x02, 0x00, 0xA2};

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
WiFiManager wifiManager;

WiFiManagerParameter mqttHostParam("mqtt_host", "MQTT sunucu", "", 40);
WiFiManagerParameter mqttPortParam("mqtt_port", "MQTT port", "1883", 6);
WiFiManagerParameter mqttUserParam("mqtt_user", "MQTT kullanici", "", 32);
WiFiManagerParameter mqttPassParam("mqtt_pass", "MQTT sifre", "", 64, "type='password'");
WiFiManagerParameter deviceNameParam("device_name", "Cihaz adi", "", 32);

struct Settings {
  char mqttHost[40] = "";
  uint16_t mqttPort = 1883;
  char mqttUser[32] = "";
  char mqttPass[64] = "";
  char deviceName[32] = "";
};

struct PersistedConfig {
  uint8_t magic = EEPROM_MAGIC;
  uint8_t version = EEPROM_VERSION;
  Settings settings;
};

Settings settings;
bool relay1State = false;
bool relay2State = false;
bool discoverySent = false;
uint32_t lastMqttAttemptAt = 0;
uint32_t lastDiscoveryAt = 0;
uint32_t lastDiagnosticPublishAt = 0;
uint32_t lastUpdateCheckAt = 0;
String latestFirmwareVersion = FIRMWARE_VERSION;
String latestReleaseUrl = "https://github.com/obacanak-iot/OBA-Relays-V1";
String latestReleaseSummary = "Manual update available.";

void copyParam(char *target, size_t targetSize, const char *value) {
  if (!value) {
    target[0] = '\0';
    return;
  }
  strlcpy(target, value, targetSize);
}

String deviceId() {
  uint8_t mac[6];
  WiFi.macAddress(mac);

  char id[13];
  snprintf(
    id,
    sizeof(id),
    "%02X%02X%02X%02X%02X%02X",
    mac[0],
    mac[1],
    mac[2],
    mac[3],
    mac[4],
    mac[5]
  );
  return String(id);
}

String defaultDeviceName() {
  return String("oba-relays-") + deviceId();
}

String deviceName() {
  return strlen(settings.deviceName) > 0 ? String(settings.deviceName) : defaultDeviceName();
}

String baseTopic() {
  return String("oba/relays/") + deviceId();
}

String availabilityTopic() {
  return baseTopic() + "/status";
}

String diagnosticsTopic() {
  return baseTopic() + "/diagnostics";
}

String relayStateTopic(uint8_t relay) {
  return baseTopic() + "/role" + relay + "/state";
}

String relaySetTopic(uint8_t relay) {
  return baseTopic() + "/role" + relay + "/set";
}

String commandTopic() {
  return baseTopic() + "/command";
}

String switchDiscoveryTopic(uint8_t relay) {
  return String("homeassistant/switch/") + deviceId() + "/role" + relay + "/config";
}

String sensorDiscoveryTopic(const char *sensorKey) {
  return String("homeassistant/sensor/") + deviceId() + "/" + sensorKey + "/config";
}

String buttonDiscoveryTopic(const char *buttonKey) {
  return String("homeassistant/button/") + deviceId() + "/" + buttonKey + "/config";
}

String updateDiscoveryTopic() {
  return String("homeassistant/update/") + deviceId() + "/firmware/config";
}

void saveSettings() {
  PersistedConfig config;
  config.magic = EEPROM_MAGIC;
  config.version = EEPROM_VERSION;
  config.settings = settings;
  EEPROM.put(0, config);
  EEPROM.commit();
}

void loadSettings() {
  PersistedConfig config;
  EEPROM.get(0, config);
  if (config.magic == EEPROM_MAGIC && config.version == EEPROM_VERSION) {
    settings = config.settings;
  }
  if (settings.mqttPort == 0) {
    settings.mqttPort = 1883;
  }
}

void publishAvailability() {
  if (mqtt.connected()) {
    mqtt.publish(availabilityTopic().c_str(), "online", true);
  }
}

void sendRelayCommand(uint8_t relay, bool enabled) {
  if (relay == 1) {
    Serial.write(enabled ? ROLE1_ON : ROLE1_OFF, sizeof(ROLE1_ON));
    relay1State = enabled;
  } else if (relay == 2) {
    Serial.write(enabled ? ROLE2_ON : ROLE2_OFF, sizeof(ROLE2_ON));
    relay2State = enabled;
  }
}

void publishRelayState(uint8_t relay) {
  if (!mqtt.connected()) {
    return;
  }
  const bool state = relay == 1 ? relay1State : relay2State;
  mqtt.publish(relayStateTopic(relay).c_str(), state ? "ON" : "OFF", true);
}

void setRelay(uint8_t relay, bool enabled) {
  sendRelayCommand(relay, enabled);
  publishRelayState(relay);
  publishAvailability();
}

void publishJson(const String &topic, const JsonDocument &doc, bool retained) {
  char payload[768];
  const size_t length = serializeJson(doc, payload, sizeof(payload));
  mqtt.publish(topic.c_str(), reinterpret_cast<const uint8_t *>(payload), length, retained);
}

void addDevice(JsonObject device) {
  device["identifiers"][0] = deviceId();
  device["name"] = deviceName();
  device["manufacturer"] = "OBA Canak";
  device["model"] = PRODUCT_MODEL;
  device["sw_version"] = FIRMWARE_VERSION;
}

void publishSwitchDiscovery(uint8_t relay) {
  JsonDocument doc;
  doc["name"] = String("Role") + relay;
  doc["unique_id"] = deviceId() + "_role" + relay;
  doc["availability_topic"] = availabilityTopic();
  doc["state_topic"] = relayStateTopic(relay);
  doc["command_topic"] = relaySetTopic(relay);
  doc["payload_on"] = "ON";
  doc["payload_off"] = "OFF";
  doc["state_on"] = "ON";
  doc["state_off"] = "OFF";

  JsonObject device = doc["device"].to<JsonObject>();
  addDevice(device);
  publishJson(switchDiscoveryTopic(relay), doc, true);
}

void publishSensorDiscovery(const char *key, const char *name, const char *valueTemplate, const char *deviceClass = nullptr, const char *unit = nullptr) {
  JsonDocument doc;
  doc["name"] = name;
  doc["unique_id"] = deviceId() + "_" + key;
  doc["availability_topic"] = availabilityTopic();
  doc["state_topic"] = diagnosticsTopic();
  doc["value_template"] = valueTemplate;
  doc["entity_category"] = "diagnostic";
  if (deviceClass) {
    doc["device_class"] = deviceClass;
  }
  if (unit) {
    doc["unit_of_measurement"] = unit;
  }

  JsonObject device = doc["device"].to<JsonObject>();
  addDevice(device);
  publishJson(sensorDiscoveryTopic(key), doc, true);
}

void publishButtonDiscovery(const char *key, const char *name, const char *payload) {
  JsonDocument doc;
  doc["name"] = name;
  doc["unique_id"] = deviceId() + "_" + key;
  doc["availability_topic"] = availabilityTopic();
  doc["command_topic"] = commandTopic();
  doc["payload_press"] = payload;

  JsonObject device = doc["device"].to<JsonObject>();
  addDevice(device);
  publishJson(buttonDiscoveryTopic(key), doc, true);
}

void publishUpdateDiscovery() {
  JsonDocument doc;
  doc["name"] = "Firmware";
  doc["unique_id"] = deviceId() + "_firmware_update";
  doc["availability_topic"] = availabilityTopic();
  doc["state_topic"] = diagnosticsTopic();
  doc["value_template"] = "{{ value_json.latest_version }}";
  doc["installed_version_template"] = "{{ value_json.installed_version }}";
  doc["latest_version_template"] = "{{ value_json.latest_version }}";
  doc["title_template"] = PRODUCT_NAME;
  doc["release_url_template"] = "{{ value_json.release_url }}";
  doc["release_summary_template"] = "{{ value_json.release_summary }}";
  doc["entity_category"] = "diagnostic";

  JsonObject device = doc["device"].to<JsonObject>();
  addDevice(device);
  publishJson(updateDiscoveryTopic(), doc, true);
}

void publishDiscovery() {
  publishSwitchDiscovery(1);
  publishSwitchDiscovery(2);
  publishSensorDiscovery("firmware_version", "Firmware Version", "{{ value_json.firmware_version }}");
  publishSensorDiscovery("wifi_signal", "WiFi Signal", "{{ value_json.wifi_signal }}", "signal_strength", "dBm");
  publishSensorDiscovery("uptime", "Uptime", "{{ value_json.uptime }}", "duration", "s");
  publishSensorDiscovery("ip", "IP", "{{ value_json.ip }}");
  publishSensorDiscovery("ssid", "SSID", "{{ value_json.ssid }}");
  publishButtonDiscovery("restart", "Restart", "restart");
  publishButtonDiscovery("factory_reset", "Factory Reset", "factory_reset");
  publishUpdateDiscovery();
  discoverySent = true;
  lastDiscoveryAt = millis();
}

void publishDiagnostics() {
  if (!mqtt.connected()) {
    return;
  }

  JsonDocument doc;
  doc["firmware_version"] = FIRMWARE_VERSION;
  doc["installed_version"] = FIRMWARE_VERSION;
  doc["latest_version"] = latestFirmwareVersion;
  doc["release_url"] = latestReleaseUrl;
  doc["release_summary"] = latestReleaseSummary;
  doc["wifi_signal"] = WiFi.RSSI();
  doc["uptime"] = millis() / 1000;
  doc["ip"] = WiFi.localIP().toString();
  doc["ssid"] = WiFi.SSID();
  doc["role1"] = relay1State ? "ON" : "OFF";
  doc["role2"] = relay2State ? "ON" : "OFF";

  publishJson(diagnosticsTopic(), doc, true);
  publishRelayState(1);
  publishRelayState(2);
  publishAvailability();
  lastDiagnosticPublishAt = millis();
}

void handleManifest(const String &payload) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    return;
  }

  const char *version = doc["version"] | "";
  if (strlen(version) == 0) {
    return;
  }

  latestFirmwareVersion = version;
  latestReleaseUrl = doc["release_url"] | latestReleaseUrl;
  latestReleaseSummary = doc["release_summary"] | latestReleaseSummary;
}

void checkForUpdates(bool force) {
  const uint32_t now = millis();
  if (!force && now - lastUpdateCheckAt < UPDATE_CHECK_MS) {
    return;
  }
  lastUpdateCheckAt = now;

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();

  HTTPClient http;
  if (!http.begin(*client, UPDATE_MANIFEST_URL)) {
    return;
  }

  const int code = http.GET();
  if (code == HTTP_CODE_OK) {
    handleManifest(http.getString());
  }
  http.end();
}

void factoryReset() {
  WiFiManager wm;
  wm.resetSettings();

  PersistedConfig blank;
  EEPROM.put(0, blank);
  EEPROM.commit();

  delay(500);
  ESP.restart();
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String message;
  message.reserve(length);
  for (unsigned int i = 0; i < length; i++) {
    message += static_cast<char>(payload[i]);
  }
  message.trim();

  const String topicString(topic);
  if (topicString == relaySetTopic(1)) {
    setRelay(1, message.equalsIgnoreCase("ON") || message == "1");
  } else if (topicString == relaySetTopic(2)) {
    setRelay(2, message.equalsIgnoreCase("ON") || message == "1");
  } else if (topicString == commandTopic()) {
    if (message == "restart") {
      ESP.restart();
    } else if (message == "factory_reset") {
      factoryReset();
    }
  }
}

void connectMqtt() {
  if (strlen(settings.mqttHost) == 0 || mqtt.connected()) {
    return;
  }

  const uint32_t now = millis();
  if (now - lastMqttAttemptAt < MQTT_RECONNECT_MS) {
    return;
  }
  lastMqttAttemptAt = now;

  mqtt.setServer(settings.mqttHost, settings.mqttPort);
  mqtt.setCallback(mqttCallback);

  const String clientId = deviceName();
  bool connected = false;
  if (strlen(settings.mqttUser) > 0) {
    connected = mqtt.connect(clientId.c_str(), settings.mqttUser, settings.mqttPass, availabilityTopic().c_str(), 0, true, "offline");
  } else {
    connected = mqtt.connect(clientId.c_str(), availabilityTopic().c_str(), 0, true, "offline");
  }

  if (!connected) {
    return;
  }

  mqtt.subscribe(relaySetTopic(1).c_str());
  mqtt.subscribe(relaySetTopic(2).c_str());
  mqtt.subscribe(commandTopic().c_str());
  publishAvailability();
  publishDiscovery();
  publishDiagnostics();
}

void setupWifi() {
  wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT_SECONDS);
  wifiManager.addParameter(&mqttHostParam);
  wifiManager.addParameter(&mqttPortParam);
  wifiManager.addParameter(&mqttUserParam);
  wifiManager.addParameter(&mqttPassParam);
  wifiManager.addParameter(&deviceNameParam);

  const String portalName = String(PRODUCT_MODEL) + "-" + deviceId();
  if (!wifiManager.autoConnect(portalName.c_str())) {
    ESP.restart();
  }

  copyParam(settings.mqttHost, sizeof(settings.mqttHost), mqttHostParam.getValue());
  settings.mqttPort = static_cast<uint16_t>(atoi(mqttPortParam.getValue()));
  if (settings.mqttPort == 0) {
    settings.mqttPort = 1883;
  }
  copyParam(settings.mqttUser, sizeof(settings.mqttUser), mqttUserParam.getValue());
  copyParam(settings.mqttPass, sizeof(settings.mqttPass), mqttPassParam.getValue());
  copyParam(settings.deviceName, sizeof(settings.deviceName), deviceNameParam.getValue());
  saveSettings();
}

void setupOta() {
  ArduinoOTA.setHostname(deviceName().c_str());
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  mqttHostParam.setValue(settings.mqttHost, sizeof(settings.mqttHost));
  char portBuffer[8];
  snprintf(portBuffer, sizeof(portBuffer), "%u", settings.mqttPort);
  mqttPortParam.setValue(portBuffer, sizeof(portBuffer));
  mqttUserParam.setValue(settings.mqttUser, sizeof(settings.mqttUser));
  mqttPassParam.setValue(settings.mqttPass, sizeof(settings.mqttPass));
  deviceNameParam.setValue(settings.deviceName, sizeof(settings.deviceName));

  setupWifi();
  setupOta();
  checkForUpdates(true);
}

void loop() {
  ArduinoOTA.handle();
  connectMqtt();
  mqtt.loop();

  const uint32_t now = millis();
  if (mqtt.connected() && (!discoverySent || now - lastDiscoveryAt > DISCOVERY_PUBLISH_MS)) {
    publishDiscovery();
  }
  if (mqtt.connected() && now - lastDiagnosticPublishAt > DIAGNOSTIC_PUBLISH_MS) {
    publishDiagnostics();
  }
  checkForUpdates(false);
}
