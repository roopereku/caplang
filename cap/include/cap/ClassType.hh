#ifndef CAP_CLASS_TYPE_HH
#define CAP_CLASS_TYPE_HH

#include <cap/Scope.hh>

namespace cap
{

class ClassType : public Scope
{
public:
	ClassType();

	/// Parses members of a class type
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This type or the parent node.
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;
};

}

#endif
