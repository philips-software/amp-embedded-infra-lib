#ifndef INFRA_REALLY_ASSERT_HPP
#define INFRA_REALLY_ASSERT_HPP

//  really_assert is a macro similar to assert, however it is not compiled away in release mode

#include <cassert>
#include <cstdlib>

#ifdef NDEBUG
#define really_assert(condition) if (!(condition)) { std::abort(); } else for (; false; )
#else
#define really_assert(condition) assert(condition)
#endif

#endif
