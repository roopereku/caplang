#ifndef CAP_EXECTUION_STEP_HH
#define CAP_EXECTUION_STEP_HH

#include <cap/Node.hh>
#include <cap/Value.hh>

#include <vector>

namespace cap
{

class BinaryOperator;
class UnaryOperator;
class BracketOperator;
class Statement;

class ExecutionStep
{
public:
	enum class Type
	{
		BinaryOperator,
		UnaryOperator,
		BracketOperator,
		Statement,
	};

	class Operand;

	ExecutionStep(std::shared_ptr <BinaryOperator> node, size_t resultIndex = -1);
	ExecutionStep(std::shared_ptr <UnaryOperator> node, size_t resultIndex = -1);
	ExecutionStep(std::shared_ptr <BracketOperator> node, size_t resultIndex = -1);
	ExecutionStep(std::shared_ptr <Statement> node, size_t resultIndex = -1);

	void addOperand(Operand&& operand);

	size_t getResultIndex() const;
	bool isTrivial() const;

//private:
	std::shared_ptr <Node> action;
	std::vector <Operand> operands;
	size_t resultIndex = 0;
	bool trivial = true;
	Type type;
};

class ExecutionStep::Operand
{
public:
	enum class Type
	{
		Result,
		Immediate
	};

	Operand(std::shared_ptr <Value> immediate);
	Operand(size_t resultIndex, std::shared_ptr <Node> resultFrom);

	std::shared_ptr <Node> getNode() const;

	Type getType() const;
	size_t asResultIndex() const;
	std::shared_ptr <Value> asImmediate() const;

private:
	std::shared_ptr <Node> node;
	size_t resultIndex;
	Type type;
};

}

#endif
