#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 1024
// Settings: Stores persistant settings, loads and saves to EEPROM

class Settings
{
  // change eeprom config version ONLY when new parameter is added and need reset the parameter
  unsigned int configVersion = 1;

public:
  String deviceNameStr;
  struct Data
  {                              // do not re-sort this struct
    unsigned int coVers;         // config version, if changed, previus config will erased
    char deviceName[20];         // device name
    char ssid[20];               // auth SSID
    char password[20];           // auth password
    char botToken[50];           // Bot Token (Get from Botfather)
    char chatID[20];             // user ID
  } data;

  void load()
  {
    data = {}; // clear bevor load data
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(0, data);
    EEPROM.end();
    coVersCheck();
    sanitycheck();
    deviceNameStr = data.deviceName;
  }

  void save()
  {
    sanitycheck();
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, data);
    EEPROM.commit();
    EEPROM.end();
  }

  void reset()
  {
    data = {};
    save();
  }

private:
  // check the variables from eeprom

  void sanitycheck()
  {
    if (strlen(data.deviceName) == 0 || strlen(data.deviceName) >= 20)
    {
      strcpy(data.deviceName, "GRD MAX 4.1");
    }
    if (strlen(data.ssid) == 0 || strlen(data.ssid) >= 20)
    {
      strcpy(data.ssid, "YOUR SSID");
    }
    if (strlen(data.password) == 0 || strlen(data.password) >= 20)
    {
      strcpy(data.password, "YOUR PASSWORD");
    }
    if (strlen(data.botToken) == 0 || strlen(data.botToken) >= 50)
    {
      strcpy(data.botToken, "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
    if (strlen(data.chatID) == 0 || strlen(data.chatID) >= 20)
    {
      strcpy(data.chatID, "XXXXXXXXXX");
    }
  }
  void coVersCheck()
  {
    if (data.coVers != configVersion)
    {
      data.coVers = configVersion;
      strcpy(data.deviceName, "GRD MAX 4.1");
      strcpy(data.ssid, "YOUR SSID");
      strcpy(data.password, "YOUR PASSWORD");
      strcpy(data.botToken, "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
      strcpy(data.chatID, "XXXXXXXXXX");
      save();
      load();
    }
  }
};
