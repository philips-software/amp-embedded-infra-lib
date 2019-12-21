#ifndef INFRA_SHARED_OWNED_OBSERVER_HPP
#define INFRA_SHARED_OWNED_OBSERVER_HPP

#include "infra/util/SharedPtr.hpp"
#include <cassert>

namespace infra
{
    template<class ObserverType>
    class SharedOwningSubject;

    template<class Descendant, class SubjectType_>
    class SharedOwnedObserver
    {
    protected:
        SharedOwnedObserver() = default;
        SharedOwnedObserver(const SharedOwnedObserver& other) = delete;
        SharedOwnedObserver& operator=(const SharedOwnedObserver& other) = delete;
        ~SharedOwnedObserver();

    public:
        using SubjectType = SubjectType_;

        bool IsAttached() const;
        SubjectType& Subject() const;

        void Detach();
        virtual void Attached(SubjectType& subject) {}
        virtual void Detaching() {}

    private:
        friend SharedOwningSubject<Descendant>;

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

        bool IsAttached() const;
        ObserverType& Observer() const;
        infra::SharedPtr<ObserverType> ObserverPtr() const;

    private:
        friend ObserverType;

        infra::SharedPtr<ObserverType> observer;
    };

    //// Implementation ////

    template<class Descendant, class SubjectType_>
    SharedOwnedObserver<Descendant, SubjectType_>::~SharedOwnedObserver()
    {
        if (subject != nullptr)
            Detach();
    }

    template<class Descendant, class SubjectType_>
    bool SharedOwnedObserver<Descendant, SubjectType_>::IsAttached() const
    {
        return subject != nullptr;
    }

    template<class Descendant, class SubjectType_>
    typename SharedOwnedObserver<Descendant, SubjectType_>::SubjectType& SharedOwnedObserver<Descendant, SubjectType_>::Subject() const
    {
        return *subject;
    }

    template<class Descendant, class SubjectType_>
    void SharedOwnedObserver<Descendant, SubjectType_>::Detach()
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
    bool SharedOwningSubject<ObserverType>::IsAttached() const
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
        this->observer->subject = static_cast<typename ObserverType::SubjectType*>(this);
        this->observer->Attached(*static_cast<typename ObserverType::SubjectType*>(this));
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
