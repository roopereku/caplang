#ifndef CAP_SIGNATURE_HH
#define CAP_SIGNATURE_HH

#include <cap/Node.hh>

namespace cap
{

/// Signature represents a combination of types used
/// for things such as function parameters.
class Signature : public Node
{
public:
	Signature() : Node(Type::Custom) {}

	/// Parses type references within parenthesis.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This signature or the parent node.
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

private:
};

}

#endif
