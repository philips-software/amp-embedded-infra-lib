#ifdef EMIL_MUTATION_TESTING

#include <csignal>
#include <unistd.h>

namespace
{
    struct MutationTimeoutGuard
    {
        MutationTimeoutGuard()
        {
            signal(SIGALRM, [](int) { _exit(1); });
            alarm(10);
        }
    };

    static MutationTimeoutGuard guard;
}

#endif
