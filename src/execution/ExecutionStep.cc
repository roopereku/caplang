#include <cap/execution/ExecutionStep.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Statement.hh>

#include <cassert>

namespace cap
{

ExecutionStep::ExecutionStep(std::shared_ptr <BinaryOperator> node, size_t resultIndex)
	: action(node), resultIndex(resultIndex), type(Type::BinaryOperator)
{
}

ExecutionStep::ExecutionStep(std::shared_ptr <UnaryOperator> node, size_t resultIndex)
	: action(node), resultIndex(resultIndex), type(Type::UnaryOperator)
{
}

ExecutionStep::ExecutionStep(std::shared_ptr <BracketOperator> node, size_t resultIndex)
	: action(node), resultIndex(resultIndex), type(Type::BracketOperator)
{
}

ExecutionStep::ExecutionStep(std::shared_ptr <Statement> node, size_t resultIndex)
	: action(node), resultIndex(resultIndex), type(Type::Statement)
{
}

void ExecutionStep::addOperand(Operand&& operand)
{
	if(operand.getType() != Operand::Type::Immediate)
	{
		trivial = false;
		resultIndex = std::min(resultIndex, operand.asResultIndex());

		printf("Add result operand %lu to %s. Minimum is %lu\n", operand.asResultIndex(), action->getTypeString(), resultIndex);
	}

	operands.emplace_back(std::move(operand));
}

size_t ExecutionStep::getResultIndex() const
{
	return resultIndex;
}

bool ExecutionStep::isTrivial() const
{
	return trivial;
}

ExecutionStep::Operand::Operand(std::shared_ptr <Value> immediate)
	: data(immediate), type(Type::Immediate)
{
}

ExecutionStep::Operand::Operand(size_t resultIndex)
	: data(resultIndex), type(Type::ResultIndex)
{
}

ExecutionStep::Operand::Type ExecutionStep::Operand::getType() const
{
	return type;
}

size_t ExecutionStep::Operand::asResultIndex() const
{
	assert(type == Type::ResultIndex);
	return std::get <size_t> (data);
}

std::shared_ptr <Value> ExecutionStep::Operand::asImmediate() const
{
	assert(type == Type::Immediate);
	return std::get <std::shared_ptr <Value>> (data);
}

}
