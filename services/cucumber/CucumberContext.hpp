#ifndef SERVICES_CUCUMBER_CONTEXT_HPP
#define SERVICES_CUCUMBER_CONTEXT_HPP

#include <memory>
#include <vector>

namespace services
{
    namespace detail
    {
        class CucumberContext
        {
        public:
            static CucumberContext& Instance();

            template<class T>
            std::weak_ptr<T> Add();

            void Clear();

        protected:
            CucumberContext() = default;
            CucumberContext(const CucumberContext&) = delete;
            CucumberContext& operator=(const CucumberContext&) = delete;

        private:
            std::vector<std::shared_ptr<void>> contexts;
        };

        inline CucumberContext& CucumberContext::Instance()
        {
            static CucumberContext context;
            return context;
        }

        template<class T>
        std::weak_ptr<T> CucumberContext::Add()
        {
            auto shared{ std::make_shared<T>() };
            contexts.push_back(shared);
            return shared;
        }

        inline void CucumberContext::Clear()
        {
            contexts.clear();
        }
    }

    template<class T>
    class CucumberScenarioScope
    {
    public:
        CucumberScenarioScope();

        T& operator*();
        T* operator->();

    private:
        std::shared_ptr<T> context;
        static std::weak_ptr<T> reference;
    };

    template<class T>
    std::weak_ptr<T> CucumberScenarioScope<T>::reference;

    template<class T>
    CucumberScenarioScope<T>::CucumberScenarioScope()
    {
        if (reference.expired())
            reference = detail::CucumberContext::Instance().Add<T>();
        context = reference.lock();
    }

    template<class T>
    T& CucumberScenarioScope<T>::operator*()
    {
        return *(context.get());
    }

    template<class T>
    T* CucumberScenarioScope<T>::operator->()
    {
        return context.get();
    }
}

#endif
