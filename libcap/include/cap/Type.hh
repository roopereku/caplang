#ifndef CAP_TYPE_HH
#define CAP_TYPE_HH

#include <cap/NamedScope.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>

namespace cap
{

class Type : public NamedScope
{
public:
	Type(Scope& parent, Token&& name)
		: NamedScope(parent, std::move(name))
	{
	}

	bool parse(ParserState& state) override;

	virtual bool isPrimitive()
	{
		return false;
	}

	virtual bool hasOperator(TwoSidedOperator::Type type);
	virtual bool hasOperator(OneSidedOperator::Type type);

	static Type& getPrimitive(Token::Type tokenType);
	static Type& getInvalid();

private:
};

}

#endif
