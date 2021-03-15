#include <cstdio> // For printf, no cout in multithreaded
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <inttypes.h>
#include <iostream>
#include <functional>
#include <chrono>
#include <atomic>
#include "LinkedListMutex.h"
#include "LinkedListAtomic.h"
#include <string.h>

using std::atomic;
using std::shared_mutex;
using std::thread;

shared_mutex myMutex;
uint64_t val{0};
const int TOTAL_WORK = 100000;
atomic<uint64_t> workUnit;
int workCounter = 0;
int debugEvery = 1000;
LinkedListMutex list = LinkedListMutex();
LinkedListAtomic listAtomic = LinkedListAtomic();

void mutexFunction(uint8_t threadID)
{
    while (workCounter < TOTAL_WORK)
    {

        //Get workpool
        myMutex.lock();
        workCounter++;
        int localWorkCounter = workCounter;
        myMutex.unlock();

        //Get node action and data
        int nodeData = rand() % 100;
        int readWriteDelete = rand() % 10;

        //perform node action
        if (readWriteDelete == 0) //write 10% chance
        {
            if (localWorkCounter % debugEvery == 0)
            {
                printf("%d: Thread %u adding %d\n", localWorkCounter, threadID, nodeData);
            }
            std::unique_lock lock(myMutex);
            list.add(nodeData);
        }
        else if (readWriteDelete == 1) //delete 10% chance
        {
            // // if (localWorkCounter % debugEvery == 0)
            if (true)
            {
                printf("%d: Thread %u removing %d\n", localWorkCounter, threadID, nodeData);
            }
            std::unique_lock lock(myMutex);
            list.remove(nodeData);
        }
        else //read 80% chance
        {
            if (localWorkCounter % debugEvery == 0)
            {
                printf("%d: Thread %u searching %d\n", localWorkCounter, threadID, nodeData);
            }
            std::shared_lock lock(myMutex);
            list.search(nodeData);
        }
    }
}

void atomicFunction(uint8_t threadID)
{
    uint64_t localVal{0};
    while (true)
    {
        localVal = atomic_load(&workUnit);

        if (localVal < TOTAL_WORK)
        {
            if (atomic_compare_exchange_strong(&workUnit, &localVal, localVal + 1)) // DO WORK
            {
                //Get node action and data
                int nodeData = rand() % 100;
                int readWriteDelete = rand() % 10;

                //perform node action
                if (readWriteDelete == 0) //write 10% chance
                {
                    if (localVal % debugEvery == 0)
                    {
                        printf("%lu: Thread %u adding %d\n", localVal, threadID, nodeData);
                    }
                    listAtomic.insert(nodeData);
                }
                else if (readWriteDelete == 1) //delete 10% chance
                {
                    if (localVal % debugEvery == 0)
                    {
                        printf("%lu: Thread %u removing %d\n", localVal, threadID, nodeData);
                    }
                    listAtomic.remove(nodeData);
                }
                else //read 80% chance
                {
                    if (localVal % debugEvery == 0)
                    {
                        printf("%lu: Thread %u searching %d\n", localVal, threadID, nodeData);
                    }
                    listAtomic.find(nodeData);
                }
            }
        }
        else
        {
            return;
        }
    }
}

//________________________________MAIN________________________________//
int main(int argc, char *argv[])
{
    //Command Line Argument to check if atomic or mutex
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " (mutex | atomic)"
                  << " numthreads" << std::endl;
        return 1;
    }
    bool isAtomic = (strcmp(argv[1], "atomic") == 0) ? true : false;
    int numThreads = atoi(argv[2]);

    thread *threads = new thread[numThreads];
    auto t1 = std::chrono::high_resolution_clock::now();
    for (uint8_t i = 0; i < numThreads; i++)
    {
        if (!isAtomic)
        {
            threads[i] = thread(mutexFunction, i);
        }
        else
        {
            threads[i] = thread(atomicFunction, i);
        }
    }
    for (uint8_t i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
    printf("Time elapsed: %g\n", fp_ms.count());
    delete[] threads;

    // for (int i = 0; i < 1000; i++)
    // {
    //     int nodeData = rand() % 100;
    //     printf("adding %d\n", nodeData);
    //     listAtomic.insert(nodeData);
    // }
    // listAtomic.display();
    // for (int i = 0; i < 1000; i++)
    // {
    //     int nodeData = rand() % 100;
    //     printf("removing %d\n", nodeData);
    //     listAtomic.remove(nodeData);
    // }
    // listAtomic.display();

    return 0;
}