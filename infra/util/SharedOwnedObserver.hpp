#ifndef INFRA_SHARED_OWNED_OBSERVER_HPP
#define INFRA_SHARED_OWNED_OBSERVER_HPP

#include "infra/util/SharedPtr.hpp"
#include <cassert>

namespace infra
{
    template<class ObserverType>
    class SharedOwningSubject;

    template<class Descendant>
    class SharedOwnedObserver
    {
    protected:
        SharedOwnedObserver() = default;
        SharedOwnedObserver(const SharedOwnedObserver& other) = delete;
        SharedOwnedObserver& operator=(const SharedOwnedObserver& other) = delete;
        ~SharedOwnedObserver();

    public:
        using SubjectType = SharedOwningSubject<SharedOwnedObserver<Descendant>>;

        bool Attached() const;
        SubjectType& Subject() const;

        void Detach();
        virtual void Attached(SubjectType& subject) {}
        virtual void Detaching() {}

    private:
        friend class SubjectType;

        SubjectType* subject = nullptr;
    };

    template<class ObserverType>
    class SharedOwningSubject
    {
    protected:
        SharedOwningSubject() = default;
        SharedOwningSubject(const SharedOwningSubject& other) = delete;
        SharedOwningSubject& operator=(const SharedOwningSubject& other) = delete;
        ~SharedOwningSubject();

    public:
        void Attach(const infra::SharedPtr<ObserverType>& observer);
        void Detach();

        bool Attached() const;
        ObserverType& Observer() const;
        infra::SharedPtr<ObserverType> ObserverPtr() const;

    private:
        friend class ObserverType;

        infra::SharedPtr<ObserverType> observer;
    };

    //// Implementation ////

    template<class Descendant>
    SharedOwnedObserver<Descendant>::~SharedOwnedObserver()
    {
        if (subject != nullptr)
            Detach();
    }

    template<class Descendant>
    bool SharedOwnedObserver<Descendant>::Attached() const
    {
        return subject != nullptr;
    }

    template<class Descendant>
    SubjectType& SharedOwnedObserver<Descendant>::Subject() const
    {
        return *subject;
    }

    template<class Descendant>
    void SharedOwnedObserver<Descendant>::Detach()
    {
        assert(subject != nullptr);
        subject->Detach();
    }

    template<class ObserverType>
    SharedOwningSubject<ObserverType>::~SharedOwningSubject()
    {
        if (observer != nullptr)
            Detach();
    }

    template<class ObserverType>
    bool SharedOwningSubject<ObserverType>::Attached() const
    {
        return observer != nullptr;
    }

    template<class ObserverType>
    ObserverType& SharedOwningSubject<ObserverType>::Observer() const
    {
        return *observer;
    }

    template<class ObserverType>
    infra::SharedPtr<ObserverType> SharedOwningSubject<ObserverType>::ObserverPtr() const
    {
        return observer;
    }

    template<class ObserverType>
    void SharedOwningSubject<ObserverType>::Attach(const infra::SharedPtr<ObserverType>& observer)
    {
        assert(this->observer == nullptr);
        assert(observer != nullptr);

        this->observer = observer;
        this->observer->subject = this;
        this->observer->Attached();
    }

    template<class ObserverType>
    void SharedOwningSubject<ObserverType>::Detach()
    {
        assert(observer != nullptr);

        observer->Detaching();
        this->observer->subject = nullptr;
        observer = nullptr;
    }
}

#endif
