#include <stdio.h>

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define MQTT_HOST   "stofradar.nl"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "revspace/button/doorbell"

static char esp_id[16];

static WiFiManager wifiManager;
static WiFiClient wifi;
static PubSubClient mqtt(wifi);
static volatile bool doorbell_pressed = false;

static void mqtt_callback(char *topic, uint8_t * payload, unsigned int length)
{
    doorbell_pressed = true;
}

void setup(void)
{
    // initialize serial port
    Serial.begin(115200);
    Serial.println("\nDOORBELL\n");

    // get ESP id
    sprintf(esp_id, "%08X", ESP.getChipId());
    Serial.print("ESP ID: ");
    Serial.println(esp_id);

    // connect to wifi
    Serial.println("Starting WIFI manager ...");
    wifiManager.setConfigPortalTimeout(120);
    wifiManager.autoConnect("ESP-DOORBELL");

    // MQTT init
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(mqtt_callback);
}

static bool mqtt_connect(const char *topic)
{
    bool result;

    result = mqtt.connect(esp_id);
    if (result) {
        result = mqtt.subscribe(topic);
    }
    return result;
}


void loop(void)
{
    if (doorbell_pressed) {
        doorbell_pressed = false;
        Serial.println("DOORBELL!");
    }

    if (!mqtt.connected()) {
        if (!mqtt_connect(MQTT_TOPIC)) {
            Serial.println("MQTT failed, restarting ...");
            ESP.restart();
        }
    }

    mqtt.loop();
}
