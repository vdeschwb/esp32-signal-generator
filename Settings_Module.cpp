#include "Settings_Module.h"

Settings_Module::Settings_Module(int num_handles) {
    if (!EEPROM.begin(1000)) {
        Serial.println("Failed to initialise EEPROM");
    }

    for (int i = 0; i < num_handles; i++)
    {
        addresses[i] = EEPROM.readInt(current_address);
        current_address += sizeof(int32_t);
    }
}

void Settings_Module::registerAddress(int handle, int length) {
    addresses[handle] = current_address;
    current_address += length;
    EEPROM.writeInt(handle * sizeof(uint32_t), addresses[handle]);
}

void Settings_Module::StoreString(int handle, String value) {
    registerAddress(handle, STRBUF_LEN);
    EEPROM.writeString(addresses[handle], value);
    EEPROM.commit();
}

void Settings_Module::StoreULong(int handle, unsigned long value) {
    registerAddress(handle, sizeof(unsigned long));
    EEPROM.writeULong(addresses[handle], value);
    EEPROM.commit();
}

void Settings_Module::StoreIp(int handle, IPAddress value) {
    registerAddress(handle, IPADDR_LEN);
    EEPROM.writeString(addresses[handle], value.toString());
    EEPROM.commit();
}

void Settings_Module::LoadString(int handle, String* dest) {
    *dest = EEPROM.readString(addresses[handle]);
}

void Settings_Module::LoadULong(int handle, unsigned long* dest) {
    *dest = EEPROM.readULong(addresses[handle]);
}

void Settings_Module::LoadIp(int handle, IPAddress* value) {
    String buf = EEPROM.readString(addresses[handle]);
    value->fromString(buf);
}
