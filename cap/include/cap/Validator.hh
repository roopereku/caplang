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
	struct ResolverContext
	{
		std::shared_ptr <Expression::Root> parameters;
		TypeContext accessedFrom;
	};

	Result validateIdentifier(std::shared_ptr <Value> node, ResolverContext& resolve);

	Result connectDeclaration(std::shared_ptr <Value> node,
			std::shared_ptr <Declaration> decl, ResolverContext& resolve);

	ResolverContext resolverCtx;
	ParserContext& ctx;
};

}

#endif
