#pragma once
// openciv4: boost::bind over std::bind; Boost's placeholders _1.._9 live at global scope.
#include <functional>
namespace boost
{
	using std::bind;
}
using namespace std::placeholders;
