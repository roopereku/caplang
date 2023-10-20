#ifndef CAP_FUNCTION_HH
#define CAP_FUNCTION_HH

#include <cap/NamedScope.hh>
#include <cap/Type.hh>
#include <cap/Parameters.hh>

namespace cap
{

class Function : public NamedScope
{
public:
	Function(Scope& parent, Token&& name)
		: NamedScope(parent, std::move(name)), parameters(*this)
	{
	}

	bool parse(ParserState& state) override;

	Type& getReturnType();
	void initializeReturnType(Type& type);

	bool isFunction() final override
	{
		return true;
	}

private:
	Type* returnType = nullptr;
	Parameters parameters;
};

}

#endif
