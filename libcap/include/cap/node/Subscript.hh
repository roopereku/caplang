#ifndef CAP_NODE_SUBSCRIPT_HH
#define CAP_NODE_SUBSCRIPT_HH

#include <cap/node/OneSidedOperator.hh>
#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class Subscript : public OneSidedOperator
{
public:
	Subscript(Token&& token) : OneSidedOperator(std::move(token), Type::Subscript)
	{
	}

	void setContents(std::shared_ptr <ExpressionRoot> contents)
	{
		adopt(contents);
		bracketContents = contents;
	}

	std::shared_ptr <ExpressionRoot> getContents()
	{
		return bracketContents;
	}

private:
	std::shared_ptr <ExpressionRoot> bracketContents;
};

}

#endif
