#ifndef CAP_VALIDATOR_HH
#define CAP_VALIDATOR_HH

#include <cap/Traverser.hh>
#include <cap/ParserContext.hh>

namespace cap
{

class Validator : public Traverser
{
public:
	Validator(ParserContext& ctx);

protected:
	void onNodeExited(std::shared_ptr <Node> node, Result result) override;
	Result onFunction(std::shared_ptr <Function> node) override;
	Result onClassType(std::shared_ptr <ClassType> node) override;
	Result onExpressionRoot(std::shared_ptr <Expression::Root> node) override;
	Result onDeclarationRoot(std::shared_ptr <Declaration::Root> node) override;
	Result onBinaryOperator(std::shared_ptr <BinaryOperator> node) override;
	Result onBracketOperator(std::shared_ptr <BracketOperator> node) override;
	Result onValue(std::shared_ptr <Value> node) override;

private:
	bool checkAssignment(std::shared_ptr <Expression> node, std::shared_ptr <Scope> target);
	bool checkDeclarationTarget(std::shared_ptr <Expression> node, bool onlyValue);

	bool checkDeclaration(std::shared_ptr <Scope> scope, std::shared_ptr <Node> name);

	bool isValueAndIdentifier(std::shared_ptr <Expression> node);

	ParserContext& ctx;

	std::shared_ptr <Expression::Root> associatedParameters;
};

}

#endif
