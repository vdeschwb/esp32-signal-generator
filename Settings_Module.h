#ifndef Settings_Module_h
#define Settings_Module_h

#include "EEPROM.h"

#define STRBUF_LEN 256
#define IPADDR_LEN 16

#define MAX_NUM_ENTRIES 256

class Settings_Module {
    public:
        Settings_Module(int num_handles);
        ~Settings_Module();
        void StoreString(int handle, String value);
        void StoreULong(int handle, unsigned long value);
        void StoreIp(int handle, IPAddress value);
        void LoadString(int handle, String* dest);
        void LoadULong(int handle, unsigned long* dest);
        void LoadIp(int handle, IPAddress* value);
    private:
        int addresses[MAX_NUM_ENTRIES];
        int current_address = 0;
        void registerAddress(int handle, int length);
};

#endif
