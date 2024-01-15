#ifndef CAP_NODE_PRIMITIVE_TYPE_HH
#define CAP_NODE_PRIMITIVE_TYPE_HH

#include <cap/node/TypeDefinition.hh>

namespace cap
{

class PrimitiveType : public TypeDefinition
{
public:
	PrimitiveType(Token name, size_t size);

	/// Retrieves a primitive type definition based on a token.
	///
	/// \param token The token to determine a primitive type from.
	/// \return The primitive type or nullptr.
	static std::shared_ptr <TypeDefinition> fromToken(const Token& token);

	/// Retrives the void type.
	///
	/// \return The void type.
	static std::shared_ptr <TypeDefinition> getVoid();

	/// Ensures that primitive types are adopted by the shared scope.
	static void ensurePrimitivesRegistered();

private:
	size_t size;
};

}

#endif
