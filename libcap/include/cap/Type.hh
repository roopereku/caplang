#ifndef CAP_TYPE_HH
#define CAP_TYPE_HH

#include <cap/NamedScope.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>

namespace cap
{

class PrimitiveType;

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

	static PrimitiveType& getPrimitive(Token::Type tokenType);
	static PrimitiveType& getInvalid();

private:
};

}

#endif
