#ifndef CAP_NODE_GENERIC_INSTANTIATION_HH
#define CAP_NODE_GENERIC_INSTANTIATION_HH

#include <cap/node/Value.hh>

namespace cap
{

class GenericInstantiation : public Value
{
public:
	// TODO: Pass the rightmost node of target to Value as the token.
	GenericInstantiation(std::shared_ptr <Expression> target, std::shared_ptr <Expression> argument)
		: Value(target->token), target(target), argument(argument)
	{
	}

	const char* getTypeString() override
	{
		return "Generic instantiation";
	}

	bool isGeneric() override
	{
		return true;
	}

	const std::shared_ptr <Expression> target;
	const std::shared_ptr <Expression> argument;
};

}

#endif
