#ifndef CAP_TYPE_HH
#define CAP_TYPE_HH

#include <cap/NamedScope.hh>

namespace cap
{

class Type : public NamedScope
{
public:
	Type(Scope& parent, Token&& name)
		: NamedScope(parent, std::move(name))
	{
	}

	bool parse(Tokenizer& tokens, BraceMatcher& braces) override;

private:
};

}

#endif
