#include <LittleFS.h>
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <WebSerial.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "secrets.h"

const uint16_t kIrPin = 4;
IRsend irsend(kIrPin);

WiFiMulti wifiMulti;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

const char* hotspot_ssid = HOTSPOT_SSID;
const char* hotspot_password = HOTSPOT_PASS;

AsyncWebServer server(80);

void log_msg(String msg) {
    Serial.println(msg);
    WebSerial.print(msg);
}

bool hasInternet() {
    WiFiClient client;
    client.setTimeout(2000);
    if (client.connect("google.com", 80)) {
        client.stop();
        return true;
    }
    return false;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Initialisation LittleFS...");
    if(!LittleFS.begin(true)){
        Serial.println("Erreur LittleFS...");
        return;
    }

    irsend.begin();

    Serial.println("Initialisation WiFi...");

    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid, password);
    wifiMulti.addAP(hotspot_ssid, hotspot_password);
    Serial.println("Connexion...");
    while (wifiMulti.run() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.println("IP: " + WiFi.localIP().toString());

    ArduinoOTA.begin();

    WebSerial.begin(&server);
    WebSerial.onMessage([](uint8_t *data, size_t len) {
        Serial.printf("Received %lu bytes from WebSerial: ", len);
        Serial.write(data, len);
        Serial.println();
        WebSerial.println("Received Data...");
        String d = "";
        for(size_t i = 0; i < len; i++){
          d += char(data[i]);
        }
        WebSerial.println(d);
      });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    server.serveStatic("/", LittleFS, "/");

    server.on("/api/ir", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("code")) {
            String codeStr = request->getParam("code")->value();
            uint32_t code = strtoul(codeStr.c_str(), NULL, 16);

            irsend.sendSAMSUNG(code);

            log_msg("------IR------");
            log_msg("IR: 0x" + codeStr);
            if (request->hasParam("name")) {
                String name = request->getParam("name")->value();
                log_msg("Name: " + name);
            }
            log_msg("--------------");

            request->send(200);
        } else {
            request->send(400, "text/plain", "No ir code provided ?code=''");
        }
    });

    server.begin();

    log_msg("Ready");
}

void loop() {
    ArduinoOTA.handle();
    static unsigned long last_print_time = millis();

    if ((unsigned long)(millis() - last_print_time) > 30000) {
        log_msg(F("-----Status-----"));
        log_msg(F("IP address: \n"));
        log_msg(WiFi.localIP().toString().c_str());

        log_msg(F("SSID: "));
        log_msg(WiFi.SSID().c_str());

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Uptime: %lums", millis());
        log_msg(buffer);

        snprintf(buffer, sizeof(buffer), "Free heap: %u", ESP.getFreeHeap());
        log_msg(buffer);

        log_msg(F("-----Wifi-----"));

        if (WiFi.status() == WL_CONNECTED) {
            if (String(WiFi.SSID()) == ssid && !hasInternet()) {
                log_msg("No internet, searching Hotspot.");
                WiFi.disconnect();
            }
            else if (String(WiFi.SSID()) == hotspot_ssid) {
                log_msg("On Hotspot.");
                wifiMulti.run();
            }
            else {
                log_msg("On Wifi.");
           }
        } else {
            wifiMulti.run();
        }
        log_msg(F("------------"));
        last_print_time = millis();
    }
    WebSerial.loop();
}
