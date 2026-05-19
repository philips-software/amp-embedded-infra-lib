#ifdef EMIL_MUTATION_TESTING

#include <csignal>
#include <sys/time.h>
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
    sigaction(SIGALRM, &sa, nullptr);

    struct itimerval timer = {};
    timer.it_value.tv_sec = 8;
    setitimer(ITIMER_REAL, &timer, nullptr);
}

#endif
