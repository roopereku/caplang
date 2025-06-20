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

	ParserContext& getParserContext() const;

protected:
	void onNodeExited(std::shared_ptr <Node> node, Result result) override;
	Result onFunction(std::shared_ptr <Function> node) override;
	Result onClassType(std::shared_ptr <ClassType> node) override;
	Result onExpressionRoot(std::shared_ptr <Expression::Root> node) override;
	Result onVariable(std::shared_ptr <Variable> node) override;
	Result onBinaryOperator(std::shared_ptr <BinaryOperator> node) override;
	Result onUnaryOperator(std::shared_ptr <UnaryOperator> node) override;
	Result onBracketOperator(std::shared_ptr <BracketOperator> node) override;
	Result onIdentifier(std::shared_ptr <Identifier> node) override;
	Result onInteger(std::shared_ptr <Integer> node) override;
	Result onString(std::shared_ptr <String> node) override;
	Result onReturn(std::shared_ptr <Return> node) override;

private:
	struct ResolverContext
	{
		ResolverContext() = default;
		ResolverContext(ResolverContext&& rhs);
		ResolverContext& operator=(ResolverContext&& rhs);

		void reset();

		std::shared_ptr <Expression::Root> parameters;
		std::optional <TypeContext> accessedFrom;
	};

	/// Makes sure that the given declaration respects the name shadowing rules.
	bool checkUniqueDeclaration(std::shared_ptr <Declaration> decl);

	Result validateIdentifier(std::shared_ptr <Identifier> node, ResolverContext& resolve);

	Result connectDeclaration(std::shared_ptr <Identifier> node,
			std::shared_ptr <Declaration> decl, ResolverContext& resolve);

	ResolverContext resolverCtx;
	ParserContext& ctx;
};

}

#endif
