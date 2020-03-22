#ifndef WiFi_Module_h
#define WiFi_Module_h

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

class WiFi_Module {
    public:
        bool TryConnect(char* ssid, char* password, IPAddress local_ip, IPAddress gateway, IPAddress subnet);
        IPAddress SetupAccessPoint();
    private:
};

#endif
