#include "gtest/gtest.h"
#include "infra/util/BoundedString.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/Variant.hpp"

namespace
{
    // Added to test if the limitation of visualizing anonymous namespaces in the Visual Studio debugger
    // is still there.
    // https://docs.microsoft.com/en-us/visualstudio/debugger/expressions-in-the-debugger?view=vs-2017

    struct AnonymousContent
    {
        int value = 10;
    };
}

struct Content
{
    infra::BoundedString::WithStorage<32> string;
    infra::BoundedConstString constString = "ABCD";
    int value = 5;
};

TEST(NatvisTest, visualize_BoundedString_variants)
{
    std::string reference = "ABCDE";

    infra::BoundedString empty;
    infra::BoundedConstString emptyConst;
    infra::BoundedString::WithStorage<32> withStorage;

    std::array<const char, 5> data{ 'A', 'B', 'C', 'D', 'E' };
    infra::BoundedString withData(const_cast<char*>(data.data()), 3);
    infra::BoundedConstString withDataConst(data.data(), 3);
}

TEST(NatvisTest, visualize_BoundedVector)
{
    std::vector<int> reference(3);

    infra::BoundedVector<int>::WithMaxSize<5> empty;
    infra::BoundedVector<int>::WithMaxSize<5> nonEmpty{ { 1, 2, 3 } };
    infra::BoundedVector<Content>::WithMaxSize<5> vectorWithOwnType{ { Content() } };
}

TEST(NatvisTest, visualize_Function)
{
    infra::Function<void()> empty;
    infra::Function<int()> lambda = [] {
        return 10;
    };
    infra::Function<void(void*)> function = free;
}

TEST(NatvisTest, visualize_Optional)
{
    infra::Optional<int> emptyOptional;
    infra::Optional<int> optional{ infra::inPlace, 1 };
    infra::Optional<Content> optionalWithOwnType{ infra::inPlace, Content() };
}

TEST(NatvisTest, visualize_Variant)
{
    infra::Variant<int, char, bool> emptyVariant;
    infra::Variant<int, char, bool> variantWithBool{ true };
    infra::Variant<int, char, bool> variantWithInt{ 1 };
    infra::Variant<int, char, bool> variantWithChar{ 'A' };

    infra::Variant<Content, bool> variantWithOwnType{ Content() };
}

/*
TEST(NatvisTest, visualize_PolymorphicVariant)
{
    struct Base {};
    struct DerivedInt { int data = 10; };
    struct DerivedChar { char data = 'A'; };
    struct DerivedComposite { DerivedInt i; DerivedChar c; };
}
*/
TEST(NatvisTest, visualize_types_from_anonymous_namespace)
{
    infra::Optional<AnonymousContent> anonymousOptional{ infra::inPlace, AnonymousContent() };
}
