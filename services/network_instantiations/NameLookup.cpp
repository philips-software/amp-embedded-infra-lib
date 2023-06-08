#include "services/network_instantiations/NameLookup.hpp"
#include "infra/event/EventDispatcher.hpp"

#ifdef EMIL_NETWORK_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef EMIL_NETWORK_BSD
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

namespace services
{
    NameLookup::NameLookup()
        : lookupThread([this]()
              {
                  Run();
              })
    {}

    NameLookup::~NameLookup()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);

            run = false;
            condition.notify_one();
        }

        lookupThread.join();
    }

    void NameLookup::Lookup(NameResolverResult& result)
    {
        assert(result.Versions() != IPVersions::ipv6);
        std::lock_guard<std::mutex> lock(mutex);
        nameLookup.push_back(result);
        condition.notify_one();
    }

    void NameLookup::CancelLookup(NameResolverResult& result)
    {
        std::lock_guard<std::mutex> lock(mutex);
        nameLookup.erase(result);
    }

    void NameLookup::Run()
    {
        std::unique_lock<std::mutex> lock(mutex);

        while (run)
        {
            if (!nameLookup.empty())
            {
                currentLookup = &nameLookup.front();

                lock.unlock();
                LookupIPv4();
                lock.lock();
            }

            if (run)
                condition.wait(lock);
        }
    }

    void NameLookup::LookupIPv4()
    {
        std::string terminatedHostname{ currentLookup->Hostname().begin(), currentLookup->Hostname().end() };
        terminatedHostname.push_back('\0');

        addrinfo* entry;
        int res = getaddrinfo(terminatedHostname.c_str(), nullptr, nullptr, &entry);
        if (res == 0)
            for (; entry != nullptr; entry = entry->ai_next)
                if (entry->ai_family == AF_INET)
                {
                    const auto address = reinterpret_cast<sockaddr_in*>(entry->ai_addr);
                    const auto ipv4Address = services::ConvertFromUint32(ntohl(address->sin_addr.s_addr));
                    infra::EventDispatcher::Instance().Schedule([this, ipv4Address]()
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            if (&nameLookup.front() == currentLookup)
                            {
                                nameLookup.pop_front();
                                currentLookup->NameLookupDone(ipv4Address, infra::Now() + std::chrono::minutes(5));
                            }
                            condition.notify_one();
                        });
                    return;
                }

        infra::EventDispatcher::Instance().Schedule([this]()
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (&nameLookup.front() == currentLookup)
                {
                    nameLookup.pop_front();
                    auto lookup = currentLookup;
                    lock.unlock();
                    lookup->NameLookupFailed();
                }
                condition.notify_one();
            });
    }
}
