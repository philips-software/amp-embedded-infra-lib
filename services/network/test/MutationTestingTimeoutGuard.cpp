#ifdef EMIL_MUTATION_TESTING

#include <csignal>
#include <unistd.h>

static void MutationTimeoutHandler(int)
{
    _exit(1);
}

__attribute__((constructor(65535))) static void SetupMutationTimeoutGuard()
{
    struct sigaction sa = {};
    sa.sa_handler = MutationTimeoutHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, nullptr);
    alarm(8);
}

#endif
