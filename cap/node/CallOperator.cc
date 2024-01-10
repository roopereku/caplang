#include <cap/node/CallOperator.hh>

namespace cap
{

void CallOperator::setTarget(std::shared_ptr <Expression>&& node)
{
	target = std::move(node);
}

std::shared_ptr <Expression> CallOperator::getTarget()
{
	return target;
}

}
