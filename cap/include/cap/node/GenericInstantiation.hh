#ifndef CAP_NODE_GENERIC_INSTANTIATION_HH
#define CAP_NODE_GENERIC_INSTANTIATION_HH

#include <cap/node/Value.hh>

namespace cap
{

class GenericInstantiation : public Value
{
public:
	GenericInstantiation(std::shared_ptr <Expression> target);

	static bool parse(Parser& parser);
	std::shared_ptr <Expression> getArguments();
	const char* getTypeString() override;
	bool isGeneric() override;

	const std::shared_ptr <Expression> target;

private:
	std::shared_ptr <Expression> arguments;
};

}

#endif
