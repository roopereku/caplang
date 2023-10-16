#ifndef CAP_VARIABLE_HH
#define CAP_VARIABLE_HH

#include <cap/Token.hh>
#include <cap/node/Value.hh>

#include <memory>

namespace cap
{

class Type;

class Variable
{
public:
	Variable(std::shared_ptr <Value> name, Type& type)
		: name(std::move(name)), type(type)
	{
	}

	const Token& getName()
	{
		return name->getToken();
	}

	Type& getType() const
	{
		return type;
	}

private:
	std::shared_ptr <Value> name;
	Type& type;
};

}

#endif
