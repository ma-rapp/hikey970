#include "perf.h"
#include <fcntl.h>

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    if (ret == -1) {
       fprintf(stderr, "Error opening perf event, got: %d\n", errno);
       exit(EXIT_FAILURE);
    }
    return ret;
}

void PerfCounter::setUp()
{
    value = 0;

    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = type;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = event;
    pe.disabled = 1;
    pe.exclude_kernel = tid >= 0 ? 1 : 0;
    pe.exclude_hv = tid >= 0 ? 1 : 0;
    pe.inherit = 0;

    fd = perf_event_open(&pe, tid, cpu, -1, 0);

    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
}

void PerfCounter::tearDown()
{
    updateCounterValue();
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    close(fd);
    fd = 0;
}

void CCIPerfCounter::setUp()
{
    char name[128];
    sprintf(name, "/dev/cci_pmu_%d_cntr", counterNb);
    fd = open(name, O_RDWR);
    if (fd == -1) {
        std::cerr << "could not setup CCI perfcounter (could not open " << name << ")" << std::endl;
        exit(1);
    }
    value = 0;

    char buf[128];
    int ct = sprintf(buf, "%u", event);
    if (write(fd, buf, ct) <= 0)
    {
        std::cerr << "could not setup CCI perfcounter (could not write event id, got errno " << errno << ")" << std::endl;
        exit(1);
    }

    ioctl(fd, 0xC0, 0);  // reset
    ioctl(fd, 0xC1, 0);  // enable
}

void CCIPerfCounter::tearDown()
{
    updateCounterValue();
    ioctl(fd, 0xC2, 0);  // disable
    close(fd);
    fd = 0;
}