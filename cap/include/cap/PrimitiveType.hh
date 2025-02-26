#ifndef CAP_PRIMITIVE_TYPE
#define CAP_PRIMITIVE_TYPE

#include <cap/TypeDefinition.hh>
#include <cap/Source.hh>

namespace cap
{

class PrimitiveType : public TypeDefinition
{
public:
	PrimitiveType(std::wstring_view name, size_t bytes);

	/// Gets a primitive type based on a name in a token.
	///
	/// \param source The source to match against.
	/// \param token The token to get a name from.
	/// \return Primitive type or null.
	static std::shared_ptr <PrimitiveType> matchName(Source& source, Token token);

	/// Gets a primitive type based on the type of a token.
	///
	/// \param token The token to match against a primitive type.
	/// \return Primitive type or null.
	static std::shared_ptr <PrimitiveType> matchToken(Token token);

	/// Sets the referred type for this primitive type.
	///
	/// \param validator The validator used for traversal.
	/// \return True if validation succeeded.
	bool validate(Validator& validator) override;

	const char* getTypeString() const override;

private:
	size_t size;
};

}

#endif
