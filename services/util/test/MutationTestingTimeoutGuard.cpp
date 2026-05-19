#ifdef EMIL_MUTATION_TESTING

#include <csignal>
#include <ctime>
#include <unistd.h>

static void MutationTimeoutHandler(int)
{
    _exit(1);
}

__attribute__((constructor(101))) static void SetupMutationTimeoutGuard()
{
    struct sigaction sa = {};
    sa.sa_handler = MutationTimeoutHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGRTMIN + 1, &sa, nullptr);

    struct sigevent sev = {};
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN + 1;

    timer_t timer;
    timer_create(CLOCK_MONOTONIC, &sev, &timer);

    struct itimerspec its = {};
    its.it_value.tv_sec = 8;
    timer_settime(timer, 0, &its, nullptr);
}

#endif
