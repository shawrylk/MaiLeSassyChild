#ifndef _STUB_H__
#define _STUB_H__

#include <string>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <csignal>

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

extern volatile int mode;
extern std::condition_variable cv;
extern std::mutex m;
extern bool ready;

class SerialStub
{

public:
    SerialStub() {}

    void print(const char *str) { std::cout << "Arduino: " << str; }
    void print(int num) { std::cout << "Arduino: " << num; }
    void print(std::string str) { std::cout << "Arduino: " << str; }

    void println(const char *str) { std::cout << "Arduino: " << str << std::endl; }
    void println(std::string str) { std::cout << "Arduino: " << str << std::endl; }
    void println(int num) { std::cout << "Arduino: " << num << std::endl; }

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
    char read(...) { return 0xFF; }
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

    std::string getString(...) { return std::string("OK"); }

    int POST(const char *str)
    {
        std::cout << "Arduino: POST: " << str << std::endl;
        return 1;
    }
};

class WifiStub
{
public:
    void begin(...) {}

    int status(...) { return 0; }

    std::string macAddress() { return std::string("5E-51-4F-9B-BB-A3"); }
};

class ESPStub
{
public:
    void deepSleep(...)
    {
        std::cout << "Sleep forever\n\n";
        while (1)
        {
        };
    }
};
class JsonObject
{
public:
    bool success() { return true; }

    int operator[](String index)
    {
        if (index == "Mode")
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [] { return ready; });
            ready = false;
            return mode;
        }
        return 0;
    }
};
template <int i>
class StaticJsonBuffer
{
public:
    static JsonObject &parseObject(String str)
    {
        static JsonObject js;
        return js;
    }
};

void delay(int secs);

void enrollHandler();

void updateDBHandler();

int sendInfoToServer(String *strTemplate);

void convertTemplateToString(char *bytesTemplate, String *strTemplate);

void registerFingerprintHandler();

void setup();

void loop();

int millis(...);

extern SerialStub Serial;

extern WifiStub WiFi;

extern ESPStub ESP;

#endif