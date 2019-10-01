
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <iostream>
#include <fcntl.h>

#include "console.h"

class TempManager
{
public:
    TempManager(unsigned int epochMs) : epoch(epochMs) {}
    void run()
    {
        std::cout << currentDateTime() << "start temperature monitoring" << std::endl;

        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;

        while (true)
        {
            periodic();
        }
    }

private:
    void periodic()
    {
        std::string nowText = currentDateTime();

        // wait for 1 second
        std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::chrono::milliseconds untilNextUpdate = nextUpdate - now;
        nextUpdate += epoch;
        if (untilNextUpdate.count() > 0)
        {
            std::cout << nowText << "sleep for " << untilNextUpdate.count() << " ms" << std::endl;
            usleep(untilNextUpdate.count() * 1000);
        }
        else
        {
            std::cout << nowText << "late" << std::endl;
        }
        
        int t = getCurrentTemperature();
        std::cout << nowText << "temp " << t << std::endl;
    }

    int getCurrentTemperature()
    {
        int fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);

        char t[128];
        size_t r = pread(fd, t, 128, 0);
        if (r <= 0)
        {
            std::cout << "read weird number of bytes: " << r << std::endl;
            exit(1);
        }
        return atoi(t);
    }

    std::chrono::milliseconds epoch;
    std::chrono::milliseconds nextUpdate;
};

int main(int argc, char **argv)
{
    TempManager m(1000);
    m.run();

    std::cout << currentDateTime() << "exit program" << std::endl;

    return 0;
}
