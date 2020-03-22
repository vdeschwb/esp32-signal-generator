#include "WiFi_Module.h"

bool WiFi_Module::TryConnect(String ssid, String password, IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
    bool wifi_connected = false;

    bool wifi_configured = ssid[0];
    if (wifi_configured) {
        WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
        WiFi.mode(WIFI_STA); //init wifi mode

        // Set static IP, gateway and subnet if the static IP is specified
        if (local_ip[0] != 0) {
            Serial.println("Configuring static IP...");
            if (!WiFi.config(local_ip, gateway, subnet)) {
                Serial.println("STA Failed to configure");
            }
        }

        // Connect using WPA personal
        WiFi.begin(ssid.c_str(), password.c_str());

        int ctr = 10;
        while (WiFi.status() != WL_CONNECTED && ctr-- > 0) {
            delay(500);
        }

        wifi_connected = ctr >= 0;
    }

    return wifi_connected;
}

IPAddress WiFi_Module::SetupAccessPoint()
{
    // Run device as access point with SSID "SigGen"
    WiFi.disconnect(true);
    WiFi.softAP("SigGen");
    return WiFi.softAPIP();
}
