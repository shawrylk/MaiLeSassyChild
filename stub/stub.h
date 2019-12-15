#ifndef _STUB_H__
#define _STUB_H__

#include <string>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define boolean bool
#define String std::string
#define INPUT_PULLUP 0
#define WL_CONNECTED 0
#define FALLING 0
#define ESP8266 1
#define BYTE 0
#define __STUB__
// #define while   printf("line: %d\nfile: %s\n", __LINE__, __FILE__); \
                while
class SerialStub
{

public:
    SerialStub() {}

    void print(const char * str) { std::cout << str; }
    void print(int num) { std::cout << num; }
    void print(std::string str) { std::cout << str; }

    void println(const char * str) { std::cout << str << std::endl; }
    void println(std::string str) { std::cout << str << std::endl; }
    void println(int num) { std::cout << num << std::endl; }

    void begin(int a) {}

    bool operator!(void)
    {
        return false;
    }
};

class Stream
{
public:
    void write(...) {}
    char read(...) { return 0; }
    bool available(...) { return false; }
};

class HardwareSerial : public Stream
{
public:
    HardwareSerial(...) {}

    void begin(...) {}
};

class SoftwareSerial : public Stream
{
public:
    SoftwareSerial(...) {}

    void begin(...) {}
};

class HTTPClient
{
public:
    void begin(...) {}

    void addHeader(...) {}

    std::string &&getString(...) { return std::move(std::string("")); }

    int POST(...) { return 0; }
};

class WifiStub
{
public:
    void begin(...) {}

    int status(...) { return 0; }

    std::string &&macAddress(...) { return std::move(std::string("")); }
};

class ESPStub
{
public:
    void deepSleep(...) {}
};

void pinMode(...);

void attachInterrupt(...);

void registerFingerprint();

void updateDB();

void delay(...);

void enrollHandler();

void updateDBHandler();

int sendTemplateToServer(char *strTemplate);

void convertTemplateToString(char *bytesTemplate, char *strTemplate);

void registerFingerprintHandler();

void setup();

void loop();

int millis(...);

extern SerialStub Serial;

extern WifiStub WiFi;

extern ESPStub ESP;

extern volatile int mode;
#endif