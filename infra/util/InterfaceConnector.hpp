#ifndef INFRA_INTERFACE_CONNECTOR_HPP
#define INFRA_INTERFACE_CONNECTOR_HPP

//  The InterfaceConnector class is used to get access to a singleton class. It does not provide
//  singleton functionality by itself, however. It is used by deriving your singleton class
//  from InterfaceConnector, and upon instantiation of your singleton, InterfaceConnector registers
//  where your singleton is. Via the static Instance function, everyone can access the singleton.
//  Used together with an interface, this is a good way to decouple construction of your specific
//  singleton from accessing the singleton via a generic interface. For example:
//
//  class IDisplay
//      : InterfaceConnector<IDisplay>
//  {
//  public:
//      virtual void Write(char s) = 0;
//  };
//
//  --- In the HAL layer:
//
//  class DisplayOnSTM
//      : IDisplay
//  {
//  public:
//      void Write(char s) { ... }
//  };
//
//  --- In an application layer, which is independent of the specific hardware implementation:
//
//  void DoSomethingWithDisplay()
//  {
//      IDisplay::Instance().Write('x');
//  }
//
//  --- In main.cpp:
//
//  int main()
//  {
//      static DisplayOnSTM display;
//
//      ...
//  }

#include <cassert>

namespace infra
{

    template<class DerivedClass>
    class InterfaceConnector
    {
    protected:
        InterfaceConnector();
        explicit InterfaceConnector(DerivedClass* instance);
        InterfaceConnector(const InterfaceConnector&) = delete;
        ~InterfaceConnector();

        InterfaceConnector& operator=(const InterfaceConnector&) = delete;

    public:
        static DerivedClass& Instance();
        static bool InstanceSet();

    private:
        static DerivedClass* sInstance;
    };

    ////    Implementation    ////

    template<class DerivedClass>
    InterfaceConnector<DerivedClass>::InterfaceConnector()
    {
        assert(sInstance == nullptr);
        sInstance = static_cast<DerivedClass*>(this);
    }

    template<class DerivedClass>
    InterfaceConnector<DerivedClass>::InterfaceConnector(DerivedClass* instance)
    {
        assert(sInstance == nullptr);
        sInstance = instance;
    }

    template<class DerivedClass>
    InterfaceConnector<DerivedClass>::~InterfaceConnector()
    {
        assert(sInstance != nullptr);
        sInstance = nullptr;
    }

    template<class DerivedClass>
    DerivedClass& InterfaceConnector<DerivedClass>::Instance()
    {
        assert(sInstance != nullptr);
        return *sInstance;
    }

    template<class DerivedClass>
    bool InterfaceConnector<DerivedClass>::InstanceSet()
    {
        return sInstance != nullptr;
    }

    template<class DerivedClass>
    DerivedClass* InterfaceConnector<DerivedClass>::sInstance = nullptr;
}

#endif
