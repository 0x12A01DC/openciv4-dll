#pragma once
// openciv4: Boost's compile-time assertion on the standard one. The SDK's game cores get it
// through Boost.Python's headers (AdvCiv's ScaledNum.h in AND2-based cores uses it so).
#ifndef BOOST_STATIC_ASSERT
#define BOOST_STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)
#endif
