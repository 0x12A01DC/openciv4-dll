#pragma once
// openciv4: boost::thread (and the mutex types next to it) over the C++ standard library.
#include <mutex>
#include <thread>
namespace boost
{
	using std::thread;
	using std::mutex;
	using std::recursive_mutex;
	namespace this_thread = std::this_thread;
}
