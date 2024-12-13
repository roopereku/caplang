#ifndef CAP_TYPE_REFERENCE_HH
#define CAP_TYPE_REFERENCE_HH

#include <cap/Node.hh>

namespace cap
{

class TypeReference : public Node
{
public:
	/// Parses a type reference.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This node or the parent node.
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;
};

}

#endif
