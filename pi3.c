#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define NSEC_PER_SEC 1000000000ULL   // Nanoseconds per second
#define RT_PRIORITY 99               // Real-time priority (highest)
#define TEST_DURATION_SEC 10         // Duration to test in seconds

void set_realtime_priority() {
    struct sched_param param;

    // Set real-time priority
    param.sched_priority = RT_PRIORITY;

    // Set the scheduling policy to SCHED_FIFO
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("Failed to set real-time priority");
        exit(EXIT_FAILURE);
    }

    printf("Real-time priority set to %d with SCHED_FIFO policy.\n", RT_PRIORITY);
}

unsigned long long timespec_to_ns(struct timespec *ts) {
    return (unsigned long long)ts->tv_sec * NSEC_PER_SEC + ts->tv_nsec;
}

void rt_task() {
    struct timespec start, end, interval;
    unsigned long long min_latency = NSEC_PER_SEC;
    unsigned long long max_latency = 0;
    unsigned long long jitter, prev_time, curr_time;

    clock_gettime(CLOCK_MONOTONIC, &start);
    prev_time = timespec_to_ns(&start);

    printf("Running real-time task for %d seconds...\n", TEST_DURATION_SEC);

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        curr_time = timespec_to_ns(&end);

        jitter = curr_time - prev_time;  // Time difference in nanoseconds

        if (jitter < min_latency) min_latency = jitter;
        if (jitter > max_latency) max_latency = jitter;

        prev_time = curr_time;

        // Sleep for 1 millisecond to simulate task workload
        interval.tv_sec = 0;
        interval.tv_nsec = 1000000;  // 1 millisecond
        nanosleep(&interval, NULL);

        if (curr_time - timespec_to_ns(&start) > TEST_DURATION_SEC * NSEC_PER_SEC) {
            break;
        }
    }

    printf("Real-time task completed.\n");
    printf("Minimum Latency: %llu ns\n", min_latency);
    printf("Maximum Latency: %llu ns\n", max_latency);
    printf("Jitter: %llu ns\n", max_latency - min_latency);
}

int main() {
    printf("Real-Time Behavior Verification Program\n");

    // Step 1: Set real-time scheduling
    set_realtime_priority();

    // Step 2: Bind the task to a specific CPU core for isolation (optional)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset); // Bind to CPU core 1

    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
        perror("Failed to set CPU affinity");
        exit(EXIT_FAILURE);
    }
    printf("Task bound to CPU core 1.\n");

    // Step 3: Run real-time task
    rt_task();

    return 0;
}
