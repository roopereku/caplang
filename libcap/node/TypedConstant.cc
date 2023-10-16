#include <cap/node/TypedConstant.hh>
#include <cap/PrimitiveType.hh>

namespace cap
{

Type& TypedConstant::getResultType()
{
	return type;
}

}
