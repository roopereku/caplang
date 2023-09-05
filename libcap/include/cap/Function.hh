#ifndef CAP_FUNCTION_HH
#define CAP_FUNCTION_HH

#include <cap/NamedScope.hh>
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

	bool parse(Tokenizer& tokens, BraceMatcher& braces) override;

private:
	Parameters parameters;
};

}

#endif
