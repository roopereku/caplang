#include <cap/Value.hh>

#include <cassert>

namespace cap
{

bool Value::isComplete() const
{
	assert(false && "cap::Value::isComplete should never be called");
	return false;
}

}
