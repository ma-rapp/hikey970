
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <iostream>
#include <fcntl.h>

#include "console.h"

std::ofstream csv_file;

class TempManager
{
public:
    TempManager(unsigned int epochMs) : epoch(epochMs) {}
    void run()
    {
        std::cout << currentDateTimeMilliseconds() << "start temperature monitoring" << std::endl;

        nextUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + epoch;

        while (true)
        {
            periodic();
        }
    }

private:
    void periodic()
    {
        std::string nowText = currentDateTimeMilliseconds();

        // wait for 1 second
        std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::chrono::milliseconds untilNextUpdate = nextUpdate - now;
        nextUpdate += epoch;
        if (untilNextUpdate.count() > 0)
        {
            //std::cout << nowText << "sleep for " << untilNextUpdate.count() << " ms" << std::endl;
            usleep(untilNextUpdate.count() * 1000);
        }
        else
        {
            std::cout << nowText << "late" << std::endl;
        }
        
        int t = getCurrentTemperature();
        csv_file << nowText << ",";
        csv_file << std::to_string(t) << std::endl;
        //std::cout << nowText << "temp " << t << std::endl;
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
    std::string cpufreq = argv[1];
    std::string benchmark = argv[2];
    int cpu = atoi(argv[3]);
    switch(cpu){
        case 1: 
            cpu = 0;
            one_hot_cpu_vector = "1,0,0,0,0,0,0,0";
            break;
        case 2: 
            cpu = 1;
            one_hot_cpu_vector = "0,1,0,0,0,0,0,0";
            break;
        case 4: 
            cpu = 2;
            one_hot_cpu_vector = "0,0,1,0,0,0,0,0";
            break;
        case 8: 
            cpu = 3;
            one_hot_cpu_vector = "0,0,0,1,0,0,0,0";
            break;
        case 10: 
            cpu = 4;
            one_hot_cpu_vector = "0,0,0,0,1,0,0,0";
            break;
        case 20: 
            cpu = 5;
            one_hot_cpu_vector = "0,0,0,0,0,1,0,0";
            break;
        case 40: 
            cpu = 6;
            one_hot_cpu_vector = "0,0,0,0,0,0,1,0";
            break;
        case 80: 
            cpu = 7;
            one_hot_cpu_vector = "0,0,0,0,0,0,0,1";
            break;
        default: std::cout << "error with cpu arg";
    }
    std::string scenario_file = argv[4];
    std::string scenario = std::regex_replace(scenario_file, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));


    std::string filename = "extratemp_scenario" + scenario + "_benchmark-" + benchmark + "_core" + std::to_string(cpu) + "_frequency" + cpufreq + ".csv";
    csv_file.open(filename);
    csv_file << 'time, temperature\n'


    TempManager m(20);
    m.run();

    std::cout << currentDateTimeMilliseconds() << "exit program" << std::endl;

    return 0;
}
