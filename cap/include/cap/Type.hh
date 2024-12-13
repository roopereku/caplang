#ifndef CAP_TYPE_HH
#define CAP_TYPE_HH

#include <cap/Scope.hh>

namespace cap
{

class Type : public Scope
{
public:
	/// Parses members of a type
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This type or the parent node.
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

private:
	std::shared_ptr <Node> firstType;
};

}

#endif
