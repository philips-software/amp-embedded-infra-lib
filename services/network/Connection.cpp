#include "services/network/Connection.hpp"

namespace services
{
    void ConnectionObserver::Close()
    {
        Subject().CloseAndDestroy();
    }

    void ConnectionObserver::Abort()
    {
        Subject().AbortAndDestroy();
    }

    void Connection::ResetOwnership()
    {
        if (Attached())
            Detach();
    }
}
