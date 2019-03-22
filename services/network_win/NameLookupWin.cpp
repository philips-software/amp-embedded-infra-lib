#include "infra/event/EventDispatcher.hpp"
#include "services/network_win/NameLookupWin.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>

namespace services
{
    NameLookupWin::NameLookupWin()
        : lookupThread([this]() { Run(); })
    {}

    NameLookupWin::~NameLookupWin()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);

            run = false;
            condition.notify_one();
        }

        lookupThread.join();
    }

    void NameLookupWin::Lookup(NameResolverResult& result)
    {
        assert(result.Versions() != IPVersions::ipv6);
        std::lock_guard<std::mutex> lock(mutex);
        nameLookup.push_back(result);
        condition.notify_one();
    }

    void NameLookupWin::CancelLookup(NameResolverResult& result)
    {
        std::lock_guard<std::mutex> lock(mutex);
        nameLookup.erase(result);
    }

    void NameLookupWin::Run()
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

    void NameLookupWin::LookupIPv4()
    {
        std::string terminatedHostname{ currentLookup->Hostname().begin(), currentLookup->Hostname().end() };
        terminatedHostname.push_back('\0');

        addrinfo* entry;
        int res = getaddrinfo(terminatedHostname.c_str(), nullptr, nullptr, &entry);
        if (res == 0)
            for (; entry != nullptr; entry = entry->ai_next)
                if (entry->ai_family == AF_INET)
                {
                    sockaddr_in* address = reinterpret_cast<sockaddr_in*>(entry->ai_addr);
                    auto ipv4Address = services::IPv4Address{ address->sin_addr.s_net, address->sin_addr.s_host, address->sin_addr.s_lh, address->sin_addr.s_impno };
                    infra::EventDispatcher::Instance().Schedule([this, ipv4Address]()
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        if (&nameLookup.front() == currentLookup)
                        {
                            nameLookup.pop_front();
                            currentLookup->NameLookupDone(ipv4Address);
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
