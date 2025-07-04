#ifndef CAP_MODIFIER_ROOT_HH
#define CAP_MODIFIER_ROOT_HH

#include <cap/Expression.hh>

namespace cap
{

class ModifierRoot : public Expression::Root
{
public:
	enum class Type
	{
		TypeReference
	};

	ModifierRoot(Type type);

	/// Parser a modified token from a token.
	///
	/// \param ctx The parser context to get the source from.
	/// \param token The token representing a source location.
	/// \return New modifier root if any.
	static std::shared_ptr <ModifierRoot> create(ParserContext& ctx, Token token);

	/// Gets the type of this declaration root.
	///
	/// \return The type of this declaration root.
	Type getType() const;

	/// Gets the precedence value for modifiers roots.
	///
	/// \return modifierRootPrecedence.
	unsigned getPrecedence() override;

	static const char* getTypeString(Type type);
	const char* getTypeString() const override;

private:
	Type type;
};

}

#endif
