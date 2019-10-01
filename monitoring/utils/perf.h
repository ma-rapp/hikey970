#pragma once

#include <asm/unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <string.h>
#include <iostream>

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags);

class PerfCounter
{
public:
    PerfCounter(int tid, int cpu, std::string name) : tid(tid), cpu(cpu), name(name), type(PerfCounter::getType(name)), event(PerfCounter::getEvent(name)), fd(0) { }
    void setUp();
    void tearDown();

    int getThreadId()
    {
        return tid;
    }

    int getCPUId()
    {
        return cpu;
    }

    const std::string getName()
    {
        return name;
    }

    long long getCounterValue()
    {
        if (fd != 0)
        {
            updateCounterValue();
        }
        return value;
    }

    bool isActive()
    {
        return fd != 0;
    }

    static int getEvent(std::string name)
    {
        if      (name == "L1I_CACHE_REFILL")    return 0x01;
        else if (name == "L1I_TLB_REFILL")      return 0x02;
        else if (name == "L1D_CACHE_REFILL")    return 0x03;
        else if (name == "L1D_CACHE")           return 0x04;
        else if (name == "L1D_TLB_REFILL")      return 0x05;
        else if (name == "LD_RETIRED")          return 0x06; // only LITTLE
        else if (name == "ST_RETIRED")          return 0x07; // only LITTLE
        else if (name == "INST_RETIRED")        return 0x08;
        else if (name == "EXC_TAKEN")           return 0x09;
        else if (name == "EXC_RETURN")          return 0x0A;
        else if (name == "CID_WRITE_RETIRED")   return 0x0B;
        else if (name == "BR_MIS_PRED")         return 0x10;
        else if (name == "CPU_CYCLES")          return 0x11;
        else if (name == "BR_PRED")             return 0x12;
        else if (name == "MEM_ACCESS")          return 0x13;
        else if (name == "L1I_CACHE")           return 0x14;
        else if (name == "L2D_CACHE")           return 0x16;
        else if (name == "L2D_CACHE_REFILL")    return 0x17;
        else if (name == "MEMORY_ERROR")        return 0x1A; // only LITTLE
        else if (name == "L1D_CACHE_RD")        return 0x40; // only big
        else if (name == "L1D_CACHE_WR")        return 0x41; // only big
        else if (name == "L2D_CACHE_RD")        return 0x50; // only big
        else if (name == "L2D_CACHE_WR")        return 0x51; // only big
        else if (name == "LD_SPEC")             return 0x70; // only big
        else if (name == "ST_SPEC")             return 0x71; // only big
        else if (name == "LDST_SPEC")           return 0x72; // only big
        else if (name == "EXC_IRQ")             return 0x86;
        else if (name == "EXC_FIQ")             return 0x87;
        else if (name == "STALL_LOAD")          return 0xE7; // only LITTLE
        else if (name == "STALL_WRITE")         return 0xE8; // only LITTLE
        else if (name == "SW_CONTEXT_SWITCHES") return PERF_COUNT_SW_CONTEXT_SWITCHES;
        else if (name == "SW_PAGE_FAULTS_MIN")  return PERF_COUNT_SW_PAGE_FAULTS_MIN;
        else if (name == "SW_PAGE_FAULTS_MAJ")  return PERF_COUNT_SW_PAGE_FAULTS_MAJ;
        else
        {
            std::cout << "unknown performance counter: " << name << std::endl;
            exit(1);
        }
    }

    static __u64 getType(std::string name)
    {
        if      (name == "L1I_CACHE_REFILL")    return PERF_TYPE_RAW;
        else if (name == "L1I_TLB_REFILL")      return PERF_TYPE_RAW;
        else if (name == "L1D_CACHE_REFILL")    return PERF_TYPE_RAW;
        else if (name == "L1D_CACHE")           return PERF_TYPE_RAW;
        else if (name == "L1D_TLB_REFILL")      return PERF_TYPE_RAW;
        else if (name == "LD_RETIRED")          return PERF_TYPE_RAW;
        else if (name == "ST_RETIRED")          return PERF_TYPE_RAW;
        else if (name == "INST_RETIRED")        return PERF_TYPE_RAW;
        else if (name == "EXC_TAKEN")           return PERF_TYPE_RAW;
        else if (name == "EXC_RETURN")          return PERF_TYPE_RAW;
        else if (name == "CID_WRITE_RETIRED")   return PERF_TYPE_RAW;
        else if (name == "BR_MIS_PRED")         return PERF_TYPE_RAW;
        else if (name == "CPU_CYCLES")          return PERF_TYPE_RAW;
        else if (name == "BR_PRED")             return PERF_TYPE_RAW;
        else if (name == "MEM_ACCESS")          return PERF_TYPE_RAW;
        else if (name == "L1I_CACHE")           return PERF_TYPE_RAW;
        else if (name == "L2D_CACHE")           return PERF_TYPE_RAW;
        else if (name == "L2D_CACHE_REFILL")    return PERF_TYPE_RAW;
        else if (name == "MEMORY_ERROR")        return PERF_TYPE_RAW;
        else if (name == "L1D_CACHE_RD")        return PERF_TYPE_RAW;
        else if (name == "L1D_CACHE_WR")        return PERF_TYPE_RAW;
        else if (name == "L2D_CACHE_RD")        return PERF_TYPE_RAW;
        else if (name == "L2D_CACHE_WR")        return PERF_TYPE_RAW;
        else if (name == "LD_SPEC")             return PERF_TYPE_RAW;
        else if (name == "ST_SPEC")             return PERF_TYPE_RAW;
        else if (name == "LDST_SPEC")           return PERF_TYPE_RAW;
        else if (name == "EXC_IRQ")             return PERF_TYPE_RAW;
        else if (name == "EXC_FIQ")             return PERF_TYPE_RAW;
        else if (name == "STALL_LOAD")          return PERF_TYPE_RAW;
        else if (name == "STALL_WRITE")         return PERF_TYPE_RAW;
        else if (name == "SW_CONTEXT_SWITCHES") return PERF_TYPE_SOFTWARE;
        else if (name == "SW_PAGE_FAULTS_MIN")  return PERF_TYPE_SOFTWARE;
        else if (name == "SW_PAGE_FAULTS_MAJ")  return PERF_TYPE_SOFTWARE;
        else
        {
            std::cout << "unknown performance counter: " << name << std::endl;
            exit(1);
        }
    }

private:
    void updateCounterValue()
    {
        size_t r = read(fd, &value, sizeof(long long));
        if (r != sizeof(long long))
        {
            std::cout << "updateCounterValue read weird number of bytes: " << r << std::endl;
            exit(1);
        }
    }

    int tid;
    int cpu;
    std::string name;
    __u64 type;
    uint64_t event;
    int fd;
    long long value;
};

class CCIPerfCounter
{
public:
    CCIPerfCounter(int counterNb, std::string interfaceName, std::string eventName) : counterNb(counterNb), interfaceName(interfaceName), eventName(eventName), event(CCIPerfCounter::getEvent(interfaceName, eventName)) { }
    void setUp();
    void tearDown();

    const std::string getInterfaceName()
    {
        return interfaceName;
    }

    const std::string getEventName()
    {
        return eventName;
    }

    long long getCounterValue()
    {
        if (fd != 0)
        {
            updateCounterValue();
        }
        return value;
    }

    static uint32_t getInterfaceId(std::string interfaceName)
    {
        if      (interfaceName == "SI0")    return 0x0;
        else if (interfaceName == "LITTLE") return 0x0;
        else if (interfaceName == "SI1")    return 0x1;
        else if (interfaceName == "big")    return 0x1;
        else if (interfaceName == "SI2")    return 0x2;
        else if (interfaceName == "SI3")    return 0x3;
        else if (interfaceName == "SI4")    return 0x4;
        else if (interfaceName == "SI5")    return 0x5;
        else if (interfaceName == "SI6")    return 0x6;
        else if (interfaceName == "MI0")    return 0x8;
        else if (interfaceName == "MI1")    return 0x9;
        else if (interfaceName == "DRAM1")  return 0x9;
        else if (interfaceName == "MI2")    return 0xA;
        else if (interfaceName == "DRAM2")  return 0xA;
        else if (interfaceName == "MI3")    return 0xB;
        else if (interfaceName == "MI4")    return 0xC;
        else if (interfaceName == "MI5")    return 0xD;
        else if (interfaceName == "MI6")    return 0xE;
        else if (interfaceName == "global") return 0xF;
        else
        {
            std::cout << "unknown interface name: " << interfaceName << std::endl;
            exit(1);
        }
    }

    static uint32_t getSlaveEventId(std::string eventName)
    {
        if      (eventName == "RD_REQ_HS") return 0x00; // read request handshake
        else if (eventName == "RD_DAT_HS") return 0x08; // read data handshake
        if      (eventName == "WR_REQ_HS") return 0x0A; // write request handshake
        else if (eventName == "WR_DAT_HS") return 0x12; // write data handshake
        else if (eventName == "RD_REQ_ST") return 0x17; // read request stall
        else if (eventName == "RD_DAT_ST") return 0x18; // read data stall
        else if (eventName == "WR_REQ_ST") return 0x19; // write request stall
        else if (eventName == "WR_DAT_ST") return 0x1A; // write data stall
        else if (eventName == "WR_RES_ST") return 0x1B; // write resp stall
        else if (eventName == "OT_LIM_ST") return 0x1E; // request stall cycle because of OT transaction limit
        else if (eventName == "RD_ARB_ST") return 0x1F; // read stall because of arbitration
        else
        {
            std::cout << "unknown slave event name: " << eventName << std::endl;
            exit(1);
        }
    }

    static uint32_t getMasterEventId(std::string eventName)
    {
        if      (eventName == "RD_DAT_HS") return 0x00; // read data handshake
        else if (eventName == "WR_DAT_HS") return 0x01; // write data handshake
        else if (eventName == "RD_REQ_ST") return 0x02; // read request stall
        else if (eventName == "RD_DAT_ST") return 0x03; // read data stall
        else if (eventName == "WR_REQ_ST") return 0x04; // write request stall
        else if (eventName == "WR_DAT_ST") return 0x05; // write data stall
        else if (eventName == "WR_RES_ST") return 0x06; // write resp stall
        else
        {
            std::cout << "unknown master event name: " << eventName << std::endl;
            exit(1);
        }
    }

    static uint32_t getGlobalEventId(std::string eventName)
    {
        if      (eventName == "ST_TT_FULL")  return 0x0A; // stall because TT full
        else if (eventName == "CCI_GEN_WR")  return 0x0B; // CCI-generated write request
        else if (eventName == "ADDR_HAZ_ST") return 0x0D; // request stall because of address hazard
        else
        {
            std::cout << "unknown global event name: " << eventName << std::endl;
            exit(1);
        }
    }

    static uint32_t getEvent(std::string interfaceName, std::string eventName)
    {
        uint32_t interface = getInterfaceId(interfaceName);
        uint32_t eventId;
        if (interface < 8) {
            eventId = getSlaveEventId(eventName);
        } else if (interface < 15) {
            eventId = getMasterEventId(eventName);
        } else {
            eventId = getGlobalEventId(eventName);
        }
        return (interface << 5) | eventId;
    }

private:
    void updateCounterValue()
    {
        char status[128];
        size_t r = pread(fd, status, 128, 0);
        if (r <= 0)
        {
            std::cout << "updateCounterValue read weird number of bytes: " << r << std::endl;
            exit(1);
        }
        value = atoll(status);
    }

    int counterNb;
    std::string interfaceName;
    std::string eventName;
    uint32_t event;
    int fd;
    long long value;
};
