#ifndef CAP_CLASS_TYPE_HH
#define CAP_CLASS_TYPE_HH

#include <cap/Scope.hh>
#include <cap/Declaration.hh>
#include <cap/TypeDefinition.hh>

namespace cap
{

class ClassType : public TypeDefinition
{
public:
	ClassType();

	/// Parses members of a class type
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This type or the parent node.
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

	/// Gets the root of the expression representing the base types if any.
	///
	/// \return The root of the base type expression or null.
	std::shared_ptr <Expression::Root> getBaseTypeRoot();

	/// Gets the root of the expression representing the generic if any.
	///
	/// \return The root of the generic expression or null.
	std::shared_ptr <Expression::Root> getGenericRoot();

	/// Gets the body of this class type.
	///
	/// \return The body of this class type.
	std::shared_ptr <Scope> getBody();

	/// Validates this class type.
	///
	/// \param validator The validator used for traversal.
	/// \return True if validation succeeded.
	bool validate(Validator& validator) override;

	const char* getTypeString() const override;

private:
	std::shared_ptr <Expression::Root> baseTypes;
	std::shared_ptr <Expression::Root> generic;
	std::shared_ptr <Scope> body;
};

}

#endif
