#include <algorithm>
#include <dirent.h>
#include <iostream>
#include <unistd.h>

#include "console.h"
#include "processes.h"

#define PROC_DIRECTORY "/proc/"

bool isNumeric(const char* string)
{
    for ( ; *string; string++)
        if (*string < '0' || *string > '9')
            return false;
    return true;
}

std::vector<int> getRunningProcesses()
{
    std::vector<int> pids;

    DIR *dirp = opendir("/proc");
    dirent *dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        if (isNumeric(dp->d_name))
        {   
            pids.push_back(atoi(dp->d_name));
        } 
    } 
    (void)closedir(dirp);

    return pids;
}

std::string getProcessName(int pid)
{
    char* name = (char*)calloc(1024, sizeof(char));
    if (name)
    {
        sprintf(name, "/proc/%d/cmdline", pid);
        FILE* f = fopen(name, "r");
        if (f) {
            size_t size;
            size = fread(name, sizeof(char), 1024, f);
            if (size > 0) {
                if ('\n' == name[size-1])
                    name[size-1] = '\0';
            }
            fclose(f);
        }
        else
        {
            std::cerr << currentDateTime() << "could not get process name of pid " << pid << " (fopen error)" << std::endl;
            return "";
        }
    }
    else
    {
        std::cerr << currentDateTime() << "could not get process name of pid " << pid << " (calloc error)" << std::endl;
        exit(1);
    }
    std::string s(name);
    free(name);

    // strip
    size_t space = s.find(' ');
    if (space != std::string::npos)
    {
        s.erase(space);
    }
    size_t slash = s.rfind('/');
    if (slash != std::string::npos)
    {
        s.erase(0, slash+1);
    }

    return s;
}

std::vector<int> getThreads(int pid)
{
    std::vector<int> tids;

    char* name = (char*)calloc(1024, sizeof(char));
    if (!name)
    {
        std::cerr << currentDateTime() << "could not get threads of pid " << pid << " (calloc error)" << std::endl;
        exit(1);
    }
    sprintf(name, "/proc/%d/task", pid);

    DIR *dirp = opendir(name);
    if (dirp != NULL)
    {
        dirent *dp;
        while ((dp = readdir(dirp)) != NULL)
        {
            if (isNumeric(dp->d_name))
            {   
                tids.push_back(atoi(dp->d_name));
            } 
        } 
        (void)closedir(dirp);
    }

    return tids;
}

int findBenchmark(std::string name)
{
    std::cout << currentDateTime() << "waiting for next occurence of benchmark " << name << std::endl;

    std::vector<int> alreadyRunning;
    bool firstIteration = true;
    while (true)
    {
        for (const int &pid : getRunningProcesses())
        {
            std::string processName = getProcessName(pid);
            if (processName == name)
            {
                if (firstIteration) {
                    alreadyRunning.push_back(pid);
                    std::cout << currentDateTime() << "skipped process " << pid << " (already running)" << std::endl;
                } else {
                    if (std::find(alreadyRunning.begin(), alreadyRunning.end(), pid) == alreadyRunning.end()) {
                        std::cout << currentDateTime() << "locked to pid " << pid << " (" << getProcessName(pid) << ")" << std::endl;
                        return pid;
                    }
                }
            }
        }
        usleep(10 * 1000);
        firstIteration = false;
    }
}

bool processRunning(int pid)
{
    char* name = (char*)calloc(1024, sizeof(char));
    if (!name)
    {
        std::cerr << currentDateTime() << "could not get threads of pid " << pid << " (calloc error)" << std::endl;
        exit(1);
    }
    sprintf(name, "/proc/%d/task", pid);

    DIR *dirp = opendir(name);
    if (dirp != NULL)
    {
        (void)closedir(dirp);
        return true;
    }
    else
    {
        return false;
    }
}

void observe(int pid, std::function<void(int tid)> threadStarted, std::function<void(int tid)> threadEnded)
{
    observe(pid, threadStarted, threadEnded, NULL);
}

void observe(int pid, std::function<void(int tid)> threadStarted, std::function<void(int tid)> threadEnded, std::function<void(void)> periodicCallback, long waitPeriod)
{
    std::vector<int> threadIds;
    while (true)
    {
        std::vector<int> tids = getThreads(pid);

        for (unsigned int i = 0; i < threadIds.size(); i++)
        {
            if (std::find(tids.begin(), tids.end(), threadIds.at(i)) == tids.end())
            {
                // remove finished threads
                std::cout << currentDateTime() << "thread " << threadIds.at(i) << " finished" << std::endl;
                threadEnded(threadIds.at(i));
                threadIds.erase(threadIds.begin() + i);
            }
        }
        for (const int& tid : tids)
        {
            if (std::find(threadIds.begin(), threadIds.end(), tid) == threadIds.end())
            {
                // migrate new task
                std::cout << currentDateTime() << "thread " << tid << " spawned" << std::endl;
                threadStarted(tid);
                threadIds.push_back(tid);
            }
        }

        if (threadIds.size() == 0)
        {
            std::cout << currentDateTime() << "last thread finished" << std::endl;
            return;
        }

        if (periodicCallback != NULL)
        {
            periodicCallback();
        }
        if (waitPeriod > 0)
        {
            usleep(waitPeriod);
        }
    }
}
void observe(int pid, bool instruction_stopper, std::function<void(int tid)> threadStarted, std::function<void(int tid)> threadEnded, std::function<bool(void)> periodicCallback, long waitPeriod)
{
    std::vector<int> threadIds;
    bool to_be_continued;
    while (true)
    {
        std::vector<int> tids = getThreads(pid);

        for (unsigned int i = 0; i < threadIds.size(); i++)
        {
            if (std::find(tids.begin(), tids.end(), threadIds.at(i)) == tids.end())
            {
                // remove finished threads
                std::cout << currentDateTime() << "thread " << threadIds.at(i) << " finished" << std::endl;
                threadEnded(threadIds.at(i));
                threadIds.erase(threadIds.begin() + i);
            }
        }
        for (const int& tid : tids)
        {
            if (std::find(threadIds.begin(), threadIds.end(), tid) == threadIds.end())
            {
                // migrate new task
                std::cout << currentDateTime() << "thread " << tid << " spawned" << std::endl;
                threadStarted(tid);
                threadIds.push_back(tid);
            }
        }

        if (threadIds.size() == 0)
        {
            std::cout << currentDateTime() << "last thread finished" << std::endl;
            return;
        }

        if (periodicCallback != NULL)
        {
            to_be_continued = periodicCallback();
            if(!to_be_continued)
            {
                std::cout << currentDateTime() << "reached 5,000,000,000 instructions - finishing..." << std::endl;
                return;
            }
        }
        if (waitPeriod > 0)
        {
            usleep(waitPeriod);
        }
    }
}