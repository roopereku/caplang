#ifndef CAP_TYPE_REFERENCE_HH
#define CAP_TYPE_REFERENCE_HH

#include <cap/Expression.hh>
#include <cap/TypeDefinition.hh>

namespace cap
{

class TypeReference : public Expression::Root, public TypeDefinition
{
public:
	TypeReference() :
		Expression::Root(Expression::Type::TypeReference),
		TypeDefinition(TypeDefinition::Type::TypeReference)
	{
	}

	/// Gets the precedence for type reference modifiers.
	///
	/// \return modifierPrecedence.
	unsigned getPrecedence() override
	{
		return modifierPrecedence;
	}

	void setReferred(const TypeContext& ctx)
	{
		TypeContext typeRef(*this);

		m_referred.emplace(ctx);
		typeRef.m_isTypeName = true;
		setResultType(typeRef);
	}

	const std::optional <TypeContext>& getReferred() const
	{
		return m_referred;
	}

	std::wstring toString(bool) const override
	{
		return L"type " + m_referred->toString();
	}

	const char* getTypeString() const override
	{
		return "Type Reference";
	}

private:
	std::optional <TypeContext> m_referred;
};

}

#endif
