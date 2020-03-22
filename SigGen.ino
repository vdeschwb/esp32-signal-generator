/*
MIT License

Copyright (c) 2020 Benjamin von Deschwanden

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Settings_Module.h"
#include "WiFi_Module.h"

#include <WebServer.h>

#include "DAC_Module.h"
#include "PWM_Module.h"

#include "driver/dac.h"
#include "driver/ledc.h"

#define FILESYSTEM SPIFFS
#define DBG_OUTPUT_PORT Serial

#if FILESYSTEM == FFat
    #include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
    #include <SPIFFS.h>
#endif

// #define FORMAT_FILESYSTEM  // Uncomment to format the file system (only necessary once before first data upload)
// #define SHOW_PASSWORDS  // Uncomment to show passwords in debug serial output
// #define FORCE_AP  // Uncomment to force device into access-point mode at startup (don't try to connect to WiFi)

WiFi_Module* wifi;

WebServer *server;

DAC_Module *dac = new DAC_Module();
PWM_Module *pwm = new PWM_Module();

Settings_Module* settings;

// EEPROM-backed settings
char ssid[STRBUF_LEN] = "";
char password[STRBUF_LEN] = "";
IPAddress local_ip;
IPAddress gateway;
IPAddress subnet;

// Specifies handles for the EEPROM-backed settings
#define NUM_SETTINGS 5
#define SSID_SETTING 0
#define PASSWORD_SETTING 1
#define LOCAL_IP_SETTING 2
#define GATEWAY_SETTING 3
#define SUBNET_SETTING 4

// Get MIME content type from file extension
String getContentType(String filename) {
    if (server->hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".svg")) {
        return "image/svg+xml";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    }
    return "text/plain";
}

// Check if a given file exists
bool exists(String path){
    bool yes = false;
    File file = FILESYSTEM.open(path, "r");
    if (!file.isDirectory()) {
        yes = true;
    }
    file.close();
    return yes;
}

// This method is invoked to handle GET requests on static files
bool handleFileRead(String path) {
    if (path.endsWith("/")) {
        path += "index.html";
    }
    String contentType = getContentType(path);
    if (exists(path)) {
        File file = FILESYSTEM.open(path, "r");
        server->streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

// This method is invoked to handle a reboot command
void handleReboot() {
    server->send(200, "text/html", "Restarting...");
    ESP.restart();
}

// This method is invoked to handle signal generator setup
void handleSetup() {
    // Shared parameters
    uint32_t phase = 0;
    uint32_t frequency = 1000;

    // PWM-specific parameters
    ledc_timer_t timer_num = LEDC_TIMER_0;
    ledc_channel_t pwm_channel = LEDC_CHANNEL_0;
    ledc_timer_bit_t resolution = LEDC_TIMER_10_BIT;
    bool highspeed = true;
    uint32_t duty = 512;
    uint32_t out_pin = 25;

    // DAC-specific parameters
    dac_channel_t dac_channel = DAC_CHANNEL_1;
    uint32_t clk_div = 0;
    uint32_t scale = 0;
    uint32_t invert = 2;

    String type = "square";
    for (uint8_t i = 0; i < server->args(); i++) {
        // String
        if (server->argName(i) == "type") { type = server->arg(i); continue; }

        // int
        if (server->argName(i) == "resolution") { resolution = (ledc_timer_bit_t)atoi(server->arg(i).c_str()); continue; }
        if (server->argName(i) == "timer_num") { timer_num = (ledc_timer_t)atoi(server->arg(i).c_str()); continue; }
        if (server->argName(i) == "pwm_channel") { pwm_channel = (ledc_channel_t)atoi(server->arg(i).c_str()); continue; }
        if (server->argName(i) == "dac_channel") { dac_channel = (dac_channel_t)atoi(server->arg(i).c_str()); continue; }

        // uint32_t
        if (server->argName(i) == "clk_div") { clk_div = strtoul(server->arg(i).c_str(), NULL, 10); continue; }
        if (server->argName(i) == "frequency") { frequency = strtoul(server->arg(i).c_str(), NULL, 10); continue; }
        if (server->argName(i) == "duty") { duty = strtoul(server->arg(i).c_str(), NULL, 10); continue; }
        if (server->argName(i) == "phase") { phase = strtoul(server->arg(i).c_str(), NULL, 10); continue; }
        if (server->argName(i) == "out_pin") { out_pin = strtoul(server->arg(i).c_str(), NULL, 10); continue; }
        if (server->argName(i) == "scale") { scale = strtoul(server->arg(i).c_str(), NULL, 10); continue; }
        if (server->argName(i) == "invert") { invert = strtoul(server->arg(i).c_str(), NULL, 10); continue; }

        // bool
        if (server->argName(i) == "highspeed") { highspeed = (server->arg(i) == "true"); continue; }

        server->send(400, "text/html", "Invalid parameter name: " + server->argName(i)); return;
    }

    if (type == "sine")
    {
        dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
        server->send(200, "text/html", "Sine wave setup successful.");
    }
    if (type == "square")
    {
        pwm->Setup(timer_num, pwm_channel, highspeed, resolution, frequency, duty, phase, out_pin);
        server->send(200, "text/html", "Square wave setup successful.");
    }

   server->send(400, "text/html", "Invalid type.");
}

// This method is invoked to stop a signal generator
void handleStop() {
    String type = "square";
    ledc_channel_t pwm_channel = LEDC_CHANNEL_0;
    dac_channel_t dac_channel = DAC_CHANNEL_1;
    bool highspeed = true;

    for (uint8_t i = 0; i < server->args(); i++) {
        if (server->argName(i) == "type") { type = server->arg(i); continue; }
        if (server->argName(i) == "pwm_channel") { pwm_channel = (ledc_channel_t)atoi(server->arg(i).c_str()); continue; }
        if (server->argName(i) == "dac_channel") { dac_channel = (dac_channel_t)atoi(server->arg(i).c_str()); continue; }
        if (server->argName(i) == "highspeed") { highspeed = (server->arg(i) == "true"); continue; }
        server->send(400, "text/html", "Invalid parameter name: " + server->argName(i)); return;
    }

    if (type == "sine")
    {
        dac->Stop(dac_channel);
        server->send(200, "text/html", "Sine wave stopped successfully.");
    }
    if (type == "square")
    {
        pwm->Stop(pwm_channel, highspeed);
        server->send(200, "text/html", "Square wave stopped successfully.");
    }
            
   server->send(400, "text/html", "Invalid type.");
}

// This method is invoked to get or set the configuration
void handleConfig() {
    switch (server->method())
    {
    case HTTP_GET:
        Serial.println("Getting configuration...");

        server->send(200, "text/html", "{\"ssid\":\"" + String(ssid) + "\"," + "\"local_ip\":\"" + local_ip.toString() + "\"," + "\"gateway\":\"" + gateway.toString() + "\"," + "\"subnet\":\"" + subnet.toString() + "\"}");
        break;
    case HTTP_POST:
        Serial.println("Setting configuration...");

        for (uint8_t i = 0; i < server->args(); i++) {
            if (server->argName(i) == "ssid"){
                String ssid = server->arg(i);
                Serial.print("Setting ssid to '");
                Serial.print(ssid);
                Serial.println("'");
                settings->StoreString(SSID_SETTING, ssid);
            } else if (server->argName(i) == "password"){
                String password = server->arg(i);
                Serial.print("Setting password to '");
                #ifdef SHOW_PASSWORDS
                    Serial.print(password);
                #else
                    Serial.print("****");
                #endif
                Serial.println("'");
                settings->StoreString(PASSWORD_SETTING, password);
            } else if (server->argName(i) == "local_ip"){
                IPAddress local_ip;
                local_ip.fromString(server->arg(i));
                Serial.print("Setting local_ip to '");
                Serial.print(local_ip);
                Serial.println("'");
                settings->StoreIp(LOCAL_IP_SETTING, local_ip);
            } else if (server->argName(i) == "gateway"){
                IPAddress gateway;
                gateway.fromString(server->arg(i));
                Serial.print("Setting gateway to '");
                Serial.print(gateway);
                Serial.println("'");
                settings->StoreIp(GATEWAY_SETTING, gateway);
            } else if (server->argName(i) == "subnet"){
                IPAddress subnet;
                subnet.fromString(server->arg(i));
                Serial.print("Setting subnet to '");
                Serial.print(subnet);
                Serial.println("'");
                settings->StoreIp(SUBNET_SETTING, subnet);
            } else {
                server->send(400, "text/html", "Invalid parameter: " + server->argName(i));
            }
        }

        server->send(200, "text/html", "Configuration updated.");
        break;
    default:
        server->send(405, "text/html", "Invalid method.");
        break;
    }
}

// This method is invoked if the requested resource is not found
void HandleNotFound() {
    if (!handleFileRead(server->uri())) {
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += server->uri();
        message += "\nMethod: ";
        message += (server->method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server->args();
        message += "\n";

        for (uint8_t i = 0; i < server->args(); i++) {
            message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
        }

        server->send(404, "text/plain", message);
    }
}

// Initialize the web server and set up the API endpoints
void SetupWebserver() {
    server = new WebServer(80);
    server->on("/config", handleConfig);
    server->on("/setup", handleSetup);
    server->on("/stop", handleStop);
    server->on("/reboot", handleReboot);
    server->onNotFound(HandleNotFound);
    server->begin();
}

// Load configuration from EEPROM
void loadSettings() {
  Serial.println("Loading Settings from ROM...");

  settings->LoadString(SSID_SETTING, ssid);
  Serial.print("ssid: '");
  Serial.print(ssid);
  Serial.println("'");

  settings->LoadString(PASSWORD_SETTING, password);
  Serial.print("password: '");
  #ifdef SHOW_PASSWORDS
    Serial.print(password);
  #else
    Serial.print("****");
  #endif
  Serial.println("'");

  settings->LoadIp(LOCAL_IP_SETTING, &local_ip);
  Serial.print("local_ip: '");
  Serial.print(local_ip.toString());
  Serial.println("'");

  settings->LoadIp(GATEWAY_SETTING, &gateway);
  Serial.print("gateway: '");
  Serial.print(gateway.toString());
  Serial.println("'");

  settings->LoadIp(SUBNET_SETTING, &subnet);
  Serial.print("subnet: '");
  Serial.print(subnet.toString());
  Serial.println("'");
}

// Reset configuration defaults and store them in EEPROM
void resetDefaults() {
    String ssid = "";
    String password = "";
    IPAddress local_ip;
    local_ip.fromString("");
    IPAddress gateway;
    gateway.fromString("192.168.1.1");
    IPAddress subnet;
    subnet.fromString("255.255.255.0");
    
    settings->StoreString(SSID_SETTING, ssid);
    settings->StoreString(PASSWORD_SETTING, password);
    settings->StoreIp(LOCAL_IP_SETTING, local_ip);
    settings->StoreIp(GATEWAY_SETTING, gateway);
    settings->StoreIp(GATEWAY_SETTING, gateway);
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial.println();

    // Initialize file system
    #ifdef FORMAT_FILESYSTEM
        FILESYSTEM.format();
    #endif
    FILESYSTEM.begin();

    // Initialize and load settings from EEPROM
    settings = new Settings_Module(NUM_SETTINGS);
    loadSettings();

    // Initialize WiFi module
    wifi = new WiFi_Module();
    bool wifi_connected = false;
    #ifndef FORCE_AP
        wifi_connected = wifi->TryConnect(ssid, password, local_ip, gateway, subnet);
    #endif

    if (!wifi_connected) {
        // Wifi connection did not succeed, switch to AP mode
        Serial.println("Could not connect to WiFi");
        wifi->SetupAccessPoint();
    } else {
        Serial.println("Successfully connected to WiFi");
    }

    // Initialize web server
    SetupWebserver();
}

void loop() {
    // Process HTTP requests
    server->handleClient();
}