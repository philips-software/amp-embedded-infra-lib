#ifndef INFRA_FUNCTION_HPP
#define INFRA_FUNCTION_HPP

//  Function is an object which can contain any callable function object, e.g. free function pointers
//  and lambda expressions. A Function is declared by providing the function signature and optionally
//  extra storage space for the function. For example, Function<void()> f; declares a function object
//  f that has void return and no parameters. Function<int(char x)> g; declares a function object g
//  that has an int return and one parameter of type char.
//  Examples:
//
//  Function<void()> f = [this]() { Trigger(); };
//  if (f)
//      f();        // call the lambda function which calls this->Trigger
//  f = nullptr;    // Clear the function
//
//  Since no heap is used, all data used by the function object must be stored inside Function itself.
//  The amount of data necessary is not known upfront, so the Function is parameterized with an
//  ExtraSize parameter to reserve storage. For example, this lambda expression requires no storage:
//  []() {} while this expression requires 8 bytes of extra storage: [this, &x]() { DoSomething(x); }.
//
//  Function<void()> f = [this, &x]() { DoSomething(x); }; // Compile error, too much storage is needed
//  Function<void(), 8> g = [this, &x]() { DoSomething(x); }; // Ok.

#include "infra/util/ByteRange.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/StaticStorage.hpp"
#include <cstddef>
#include <cstring>
#include <functional>
#include <ostream>
#include <tuple>
#include <type_traits>

#ifndef INFRA_DEFAULT_FUNCTION_EXTRA_SIZE                                                                   //TICS !POR#021
#define INFRA_DEFAULT_FUNCTION_EXTRA_SIZE (2 * sizeof(void*))
#endif

#ifndef UTIL_FUNCTION_ALIGNMENT                                                                             //TICS !POR#021
#define UTIL_FUNCTION_ALIGNMENT uint32_t
#endif

namespace infra
{
    template<class F, std::size_t ExtraSize = INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>
    class Function;

    namespace detail
    {
        template<class Signature, std::size_t ExtraSize>
        struct InvokerFunctions;

        template<std::size_t ExtraSize, class Result, class... Args>
        struct InvokerFunctions<Result(Args...), ExtraSize>
        {
            typedef InvokerFunctions<Result(Args...), ExtraSize> InvokerFunctionsType;

            // A hand-crafted Virtual Method Table is used so that the destruct and copyStruct functions
            // do not need to be implemented for trivial types (such as most lambdas), which saves space
            struct VirtualMethodTable
            {
                typedef Result(*Invoker)(const InvokerFunctionsType& invokerFunctions, Args...);
                typedef void(*Destructor)(InvokerFunctionsType& invokerFunctions);
                typedef void(*CopyConstructor)(const InvokerFunctionsType& from, InvokerFunctionsType& to);

                Invoker invoke;
                Destructor destruct;
                CopyConstructor copyConstruct;
            };

            const VirtualMethodTable* virtualMethodTable = nullptr;

            typedef typename std::aligned_storage<ExtraSize, std::alignment_of<UTIL_FUNCTION_ALIGNMENT>::value>::type StorageType;
            StorageType data;

            template<class F>
                static Result StaticInvoke(const InvokerFunctionsType& invokerFunctions, Args... args);
            template<class F>
                static void StaticDestruct(InvokerFunctionsType& invokerFunctions);
            template<class F>
                static void StaticCopyConstruct(const InvokerFunctionsType& from, InvokerFunctionsType& to);
            template<class F>
                static const VirtualMethodTable* StaticVirtualMethodTable(typename std::enable_if<std::is_trivial<F>::value>::type* = 0);
            template<class F>
                static const VirtualMethodTable* StaticVirtualMethodTable(typename std::enable_if<!std::is_trivial<F>::value>::type* = 0);
            template<class F>
                static void Construct(InvokerFunctionsType& invokerFunctions, F&& f);
        };
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    class Function<Result(Args...), ExtraSize>
    {
    public:
        typedef Result ResultType;

    public:
        Function() = default;
        Function(std::nullptr_t);                                                       //TICS !INT#001
        Function(const Function& other);

        template<class F>                                                               //TICS !INT#001
            Function(F f);

        ~Function();

        Function& operator=(const Function& other);
        Function& operator=(std::nullptr_t);

        explicit operator bool() const;

        ResultType operator()(Args... args) const;
        ResultType Invoke(const std::tuple<Args...>& args) const;

        void Swap(Function& other);

    private:
        template<class F>
            void Assign(F f);

        void Clear();
        bool Initialized() const;

    private:
        typedef detail::InvokerFunctions<Result(Args...), ExtraSize> StorageType;

        static void CopyConstruct(const StorageType& from, StorageType& to);
        static void Destruct(StorageType& storage);

        StorageType invokerFunctions;

        template<class F, std::size_t ExtraSize2>
        friend class Function;
    };

    class Execute
    {
    public:
        template<std::size_t ExtraSize>
        struct WithExtraSize
        {
            explicit WithExtraSize(Function<void(), ExtraSize> f);
        };

        explicit Execute(Function<void()> f);
    };

    class ExecuteOnDestruction
    {
    public:
        template<std::size_t ExtraSize>
        struct WithExtraSize
        {
            explicit WithExtraSize(Function<void(), ExtraSize> f);
            ~WithExtraSize();

        private:
            Function<void(), ExtraSize> f;
        };

        explicit ExecuteOnDestruction(Function<void()> f);
        ~ExecuteOnDestruction();

    private:
        Function<void()> f;
    };

    extern const infra::Function<void()> emptyFunction;

    template<std::size_t ExtraSize, class Result, class... Args>
        void swap(Function<Result(Args...), ExtraSize>& x, Function<Result(Args...), ExtraSize>& y);

    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator==(const Function<Result(Args...), ExtraSize>& f, std::nullptr_t);
    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator==(std::nullptr_t, const Function<Result(Args...), ExtraSize>& f);
    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator!=(const Function<Result(Args...), ExtraSize>& f, std::nullptr_t);
    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator!=(std::nullptr_t, const Function<Result(Args...), ExtraSize>& f);

#ifdef CCOLA_HOST_BUILD //TICS !POR#021
    // gtest uses PrintTo to display the contents of Function
    template<class... Args>
    struct PrintParameterNames;

    template<>
    struct PrintParameterNames<>
    {
        PrintParameterNames(std::ostream* os)
        {}
    };

    template<class Arg>
    struct PrintParameterNames<Arg>
    {
        PrintParameterNames(std::ostream* os)
        {
            *os << typeid(Arg).name();
        }
    };

    template<class Arg, class Arg2, class... Args>
    struct PrintParameterNames<Arg, Arg2, Args...>
    {
        PrintParameterNames(std::ostream* os)
        {
            *os << typeid(Arg).name() << ", ";

            PrintParameterNames<Arg2, Args...> print(os);
        }
    };

    template<class R, class... Args>
    void PrintTo(Function<R(Args...)>, std::ostream* os)
    {
        *os << "Function<" << typeid(R).name() << "(";

        PrintParameterNames<Args...> print(os);

        *os << ")>";
    }
#endif

    //// Implementation ////

    namespace detail
    {

        template<std::size_t ExtraSize, class Result, class... Args>
        template<class F>
        Result InvokerFunctions<Result(Args...), ExtraSize>::StaticInvoke(const InvokerFunctionsType& invokerFunctions, Args... args)
        {
            return (reinterpret_cast<const F&>(invokerFunctions.data))(args...);
        }

        template<std::size_t ExtraSize, class Result, class... Args>
        template<class F>
        void InvokerFunctions<Result(Args...), ExtraSize>::StaticDestruct(InvokerFunctionsType& invokerFunctions)
        {
            reinterpret_cast<F&>(invokerFunctions.data).~F();                                                                                       //TICS !CFL#024
            invokerFunctions.virtualMethodTable = nullptr;
        }

        template<std::size_t ExtraSize, class Result, class... Args>
        template<class F>
        void InvokerFunctions<Result(Args...), ExtraSize>::StaticCopyConstruct(const InvokerFunctionsType& from, InvokerFunctionsType& to)
        {
            new (&to.data) F(reinterpret_cast<const F&>(from.data));                                                                                //TICS !OAL#011
            std::memset(reinterpret_cast<char*>(&to.data) + sizeof(F), 0, ExtraSize - sizeof(F));                                                   //TICS !COV_CPP_NO_EFFECT_13
            to.virtualMethodTable = StaticVirtualMethodTable<F>();
        }

        template<std::size_t ExtraSize, class Result, class... Args>
        template<class F>
        const typename InvokerFunctions<Result(Args...), ExtraSize>::VirtualMethodTable*
            InvokerFunctions<Result(Args...), ExtraSize>::StaticVirtualMethodTable(typename std::enable_if<std::is_trivial<F>::value>::type*)
        {
            static const VirtualMethodTable table = { &StaticInvoke<F>, nullptr, nullptr };
            return &table;
        }

        template<std::size_t ExtraSize, class Result, class... Args>
        template<class F>
        const typename InvokerFunctions<Result(Args...), ExtraSize>::VirtualMethodTable*
            InvokerFunctions<Result(Args...), ExtraSize>::StaticVirtualMethodTable(typename std::enable_if<!std::is_trivial<F>::value>::type*)
        {
            static const VirtualMethodTable table = { &StaticInvoke<F>, &StaticDestruct<F>, &StaticCopyConstruct<F> };
            return &table;
        }

        template<std::size_t ExtraSize, class Result, class... Args>
        template<class F>
        void InvokerFunctions<Result(Args...), ExtraSize>::Construct(InvokerFunctionsType& invokerFunctions, F&& f)
        {
            static_assert(sizeof(F) <= ExtraSize, "Not enough static storage availabe for construction of derived type");
            static_assert(std::alignment_of<F>::value <= sizeof(UTIL_FUNCTION_ALIGNMENT), "Alignment of U is larger than alignment of this function");

            new (&invokerFunctions.data) F(std::forward<F>(f));                                                                                     //TICS !OAL#011
            std::memset(reinterpret_cast<char*>(&invokerFunctions.data) + sizeof(F), 0, ExtraSize - sizeof(F));                                     //TICS !COV_CPP_NO_EFFECT_13
            invokerFunctions.virtualMethodTable = StaticVirtualMethodTable<F>();
        }
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    Function<Result(Args...), ExtraSize>::Function(std::nullptr_t)
    {}

    template<std::size_t ExtraSize, class Result, class... Args>
    Function<Result(Args...), ExtraSize>::Function(const Function& other)
    {
        if (other.Initialized())
            CopyConstruct(other.invokerFunctions, invokerFunctions);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    template<class F>
    Function<Result(Args...), ExtraSize>::Function(F f)
    {
        Assign(std::forward<F>(f));
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    Function<Result(Args...), ExtraSize>::~Function()
    {
        Clear();
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    Function<Result(Args...), ExtraSize>& Function<Result(Args...), ExtraSize>::operator=(const Function& other)
    {
        if (this != &other)
        {
            Clear();

            if (other)
                CopyConstruct(other.invokerFunctions, invokerFunctions);
        }

        return *this;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    Function<Result(Args...), ExtraSize>& Function<Result(Args...), ExtraSize>::operator=(std::nullptr_t)
    {
        Clear();
        return *this;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    void Function<Result(Args...), ExtraSize>::CopyConstruct(const StorageType& from, StorageType& to)
    {
        if (from.virtualMethodTable->copyConstruct != nullptr)                                                      //TICS !CON#007
            from.virtualMethodTable->copyConstruct(from, to);
        else
            Copy(MakeByteRange(from), MakeByteRange(to));
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    void Function<Result(Args...), ExtraSize>::Destruct(StorageType& storage)
    {
        if (storage.virtualMethodTable->destruct != nullptr)                                                        //TICS !CON#007
            storage.virtualMethodTable->destruct(storage);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    void Function<Result(Args...), ExtraSize>::Swap(Function& other)
    {
        using std::swap;

        if (Initialized() && other.Initialized())
        {
            Function temp(std::move(*this));
            *this = std::move(other);
            other = std::move(temp);
        }
        else if (Initialized())
        {
            CopyConstruct(invokerFunctions, other.invokerFunctions);
            Destruct(invokerFunctions);
            invokerFunctions.virtualMethodTable = nullptr;
        }
        else if (other.Initialized())
        {
            CopyConstruct(other.invokerFunctions, invokerFunctions);
            Destruct(other.invokerFunctions);
            other.invokerFunctions.virtualMethodTable = nullptr;
        }
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    template<class F>
    void Function<Result(Args...), ExtraSize>::Assign(F f)
    {
        Clear();
        StorageType::template Construct<F>(invokerFunctions, std::forward<F>(f));
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    Function<Result(Args...), ExtraSize>::operator bool() const
    {
        return Initialized();
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    typename Function<Result(Args...), ExtraSize>::ResultType Function<Result(Args...), ExtraSize>::operator()(Args... args) const
    {
        return invokerFunctions.virtualMethodTable->invoke(invokerFunctions, args...);
    }

    template<std::size_t N, class ResultType, class... Args>
    struct TupleInvokeHelper;

    template<class ResultType, class... Args>
    struct TupleInvokeHelper<0, ResultType, Args...>
    {
        template<class InvokerFunctions, class FunctionPointer>
        static ResultType Invoke(const InvokerFunctions& invokerFunctions, FunctionPointer invoke, const std::tuple<Args...>& args)
        {
            return invoke(invokerFunctions);
        }
    };

    template<class ResultType, class... Args>
    struct TupleInvokeHelper<1, ResultType, Args...>
    {
        template<class InvokerFunctions, class FunctionPointer>
        static ResultType Invoke(const InvokerFunctions& invokerFunctions, FunctionPointer invoke, const std::tuple<Args...>& args)
        {
            return invoke(invokerFunctions, std::get<0>(args));
        }
    };

    template<class ResultType, class... Args>
    struct TupleInvokeHelper<2, ResultType, Args...>
    {
        template<class InvokerFunctions, class FunctionPointer>
        static ResultType Invoke(const InvokerFunctions& invokerFunctions, FunctionPointer invoke, const std::tuple<Args...>& args)
        {
            return invoke(invokerFunctions, std::get<0>(args), std::get<1>(args));
        }
    };

    template<class ResultType, class... Args>
    struct TupleInvokeHelper<3, ResultType, Args...>
    {
        template<class InvokerFunctions, class FunctionPointer>
        static ResultType Invoke(const InvokerFunctions& invokerFunctions, FunctionPointer invoke, const std::tuple<Args...>& args)
        {
            return invoke(invokerFunctions, std::get<0>(args), std::get<1>(args), std::get<2>(args));
        }
    };

    template<std::size_t ExtraSize, class Result, class... Args>
    typename Function<Result(Args...), ExtraSize>::ResultType Function<Result(Args...), ExtraSize>::Invoke(const std::tuple<Args...>& args) const
    {
        return TupleInvokeHelper<sizeof...(Args), ResultType, Args...>::Invoke(invokerFunctions, invokerFunctions.virtualMethodTable->invoke, args);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    void Function<Result(Args...), ExtraSize>::Clear()
    {
        if (Initialized())
        {
            Destruct(invokerFunctions);
            invokerFunctions.virtualMethodTable = nullptr;
        }
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool Function<Result(Args...), ExtraSize>::Initialized() const
    {
        return invokerFunctions.virtualMethodTable != nullptr;                                                  //TICS !CON#007
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    void swap(Function<Result(Args...), ExtraSize>& x, Function<Result(Args...), ExtraSize>& y)
    {
        x.Swap(y);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator==(const Function<Result(Args...), ExtraSize>& f, std::nullptr_t)
    {
        return !f;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator==(std::nullptr_t, const Function<Result(Args...), ExtraSize>& f)
    {
        return !f;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator!=(const Function<Result(Args...), ExtraSize>& f, std::nullptr_t)
    {
        return !(f == nullptr);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator!=(std::nullptr_t, const Function<Result(Args...), ExtraSize>& f)
    {
        return !(f == nullptr);
    }

    template<std::size_t ExtraSize>
    Execute::WithExtraSize<ExtraSize>::WithExtraSize(Function<void(), ExtraSize> f)
    {
        f();
    }

    template<std::size_t ExtraSize>
    ExecuteOnDestruction::WithExtraSize<ExtraSize>::WithExtraSize(Function<void(), ExtraSize> f)
        : f(f)
    {}

    template<std::size_t ExtraSize>
    ExecuteOnDestruction::WithExtraSize<ExtraSize>::~WithExtraSize()
    {
        f();
    }
}

#endif
