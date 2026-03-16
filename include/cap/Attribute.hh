#ifndef CAP_ATTRIBUTE_HH
#define CAP_ATTRIBUTE_HH

#include <cap/Expression.hh>
#include <cap/Declaration.hh>

namespace cap
{

class Attribute : public Expression::Root
{
public:
	Attribute();

	unsigned getPrecedence() override;

	const char* getTypeString() const override;

	bool validate(Validator& validator);

	std::shared_ptr <Declaration> getReferred() const
	{
		return m_referredDeclaration.lock();
	}

private:
	std::shared_ptr <Declaration> findReferred(std::shared_ptr <Expression> node, Validator& validator) const;

	std::weak_ptr <Declaration> m_referredDeclaration;
};

}

#endif
