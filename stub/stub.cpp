#include "stub.h"

SerialStub Serial;
ESPStub ESP;
WifiStub WiFi;

std::condition_variable cv;
std::mutex m;
bool ready = false;
std::thread t;

int main()
{
    int mymode = -1;
    t = std::thread([]() {
        setup();
        while (1)
        {
            loop();
        }
    });
    while (mymode == -1)
    {
        printf("\n*****Simulate mode that Server will return to Arduino:*****\n\t \
        0. Enroll\n\t \
        1. Register new fingerprint\n\t \
        2. Update database\n\t \
        Press Ctrl + C to simulate reset event (program is terminated)\n\n");
        std::cin >> mymode;
        if (std::cin.fail())
        {
            std::cerr << "Not regconize mode, try input again ...\n";
            std::cin.clear();
            std::cin.ignore(256, '\n');
            mymode = -1;
            continue;
        }
        switch (mymode)
        {
        case 0:
        case 1:
        case 2:
        {
            {
                std::lock_guard<std::mutex> lk(m);
                ready = true;
                mode = mymode;
            }
            cv.notify_one();
            break;
        }
        default:
            std::cerr << "Not regconize mode, try input again ...\n";
            mymode = -1;
            continue;
        }
    }
    t.join();
}

void pinMode(...) {}

void attachInterrupt(...) {}

void delay(int secs)
{
    if (secs <= 5)
    {
        sleep(secs);
    }
}

int millis(...) { return 0; }
