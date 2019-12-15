#include "stub.h"

SerialStub Serial;
ESPStub ESP;
WifiStub WiFi;

int main()
{
    int mymode;
    setup();
    while (1)
    {
        printf("Enter mode:\n\t0. Enroll\n\t1. Register new fingerprint\n\t2. Update database\n\tPress Ctrl + C to exit\n");
        std::cin >> mymode;
        if (std::cin.fail())
        {
            std::cout << "Not regconize mode, try input again ...\n";
            std::cin.clear();
            std::cin.ignore(256, '\n');
            continue;
        }
        switch (mymode)
        {
        case 0:
        case 1:
        case 2:
            mode = mymode;
            break;
        default:
            std::cout << "Not regconize mode, try input again ...\n";
            continue;
        }
        loop();
    }
}

void pinMode(...) {}

void attachInterrupt(...) {}

void delay(...) {}

int millis(...) { return 0; }
