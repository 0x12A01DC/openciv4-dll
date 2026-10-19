#pragma once
// openciv4: boost::shared_ptr over std::shared_ptr.
#include <memory>
namespace boost
{
	using std::shared_ptr;
	using std::make_shared;
	using std::dynamic_pointer_cast;
	using std::static_pointer_cast;
}
