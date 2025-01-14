// #include <WebSerialLite.h>
#define ARDUINOJSON_USE_DOUBLE 1
#define ARDUINOJSON_USE_LONG_LONG 1
#define JSON_BUFFER 2048
// #define MQTT_BUFFER 512
#define DEBUG_BAUD 115200

#ifdef isUART_HARDWARE
#define INVERTER_TX 1
#define INVERTER_RX 3
#else
#define INVERTER_TX 13
#define INVERTER_RX 12
#endif

#define LED_NET 4
// implement this
// https://github.com/arduino-libraries/Arduino_DebugUtils
// function to split debug out stream into webserial and serial if avaible?
#define DEBUG_WEBLN(...) WebSerial.println(__VA_ARGS__)
#define DEBUG_WEBF(...) WebSerial.printf(__VA_ARGS__)
#define DEBUG_WEB(...) WebSerial.print(__VA_ARGS__)

#ifdef isDEBUG
#define SOFTWARE_VERSION SWVERSION " " HWBOARD " " __DATE__ " " __TIME__
#define DEBUG Serial
#define DEBUG_BEGIN(...) DEBUG.begin(__VA_ARGS__)
#define DEBUG_PRINT(...) DEBUG.print(__VA_ARGS__)
#define DEBUG_PRINTF(...) DEBUG.printf(__VA_ARGS__)
#define DEBUG_WRITE(...) DEBUG.write(__VA_ARGS__)
#define DEBUG_PRINTLN(...) DEBUG.println(__VA_ARGS__)
#else
#define SOFTWARE_VERSION SWVERSION
#define DEBUG_BEGIN(...)
#define DEBUG_PRINT(...)
#define DEBUG_PRINTF(...)
#define DEBUG_WRITE(...)
#define DEBUG_PRINTLN(...)
#endif

/**
 * @brief callback function for wifimanager save config data
 *
 */
void saveConfigCallback();

/**
 * @brief fires up the websocket and send data to the clients
 *
 */
void notifyClients();

/**
 * @brief read the data from bms and put it in the json
 */
void getJsonData();

