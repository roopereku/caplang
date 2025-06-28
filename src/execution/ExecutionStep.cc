#include <cap/execution/ExecutionStep.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Statement.hh>

#include <cassert>

namespace cap
{

ExecutionStep::ExecutionStep(std::shared_ptr <BinaryOperator> node, size_t resultIndex)
	: type(Type::BinaryOperator), action(node), resultIndex(resultIndex)
{
}

ExecutionStep::ExecutionStep(std::shared_ptr <UnaryOperator> node, size_t resultIndex)
	: type(Type::UnaryOperator), action(node), resultIndex(resultIndex)
{
}

ExecutionStep::ExecutionStep(std::shared_ptr <BracketOperator> node, size_t resultIndex)
	: type(Type::BracketOperator), action(node), resultIndex(resultIndex)
{
}

ExecutionStep::ExecutionStep(std::shared_ptr <Statement> node, size_t resultIndex)
	: type(Type::Statement), action(node), resultIndex(resultIndex)
{
}

ExecutionStep::ExecutionStep(Operand&& immediate, size_t resultIndex)
	: type(Type::StoreImmediate), resultIndex(resultIndex)
{
	assert(immediate.getType() == Operand::Type::Immediate);
	addOperand(std::move(immediate));
}

void ExecutionStep::addOperand(Operand&& operand)
{
	if(operand.getType() != Operand::Type::Immediate)
	{
		trivial = false;
		resultIndex = std::min(resultIndex, operand.asResultIndex());
	}

	operands.emplace_back(std::move(operand));
}

const std::vector <ExecutionStep::Operand>& ExecutionStep::getOperands() const
{
	return operands;
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
	: node(immediate), resultIndex(-1), type(Type::Immediate)
{
}

ExecutionStep::Operand::Operand(size_t resultIndex, std::shared_ptr <Node> resultFrom)
	: node(resultFrom), resultIndex(resultIndex), type(Type::Result)
{
}

ExecutionStep::Operand::Type ExecutionStep::Operand::getType() const
{
	return type;
}

size_t ExecutionStep::Operand::asResultIndex() const
{
	assert(type == Type::Result);
	return resultIndex;
}

std::shared_ptr <Value> ExecutionStep::Operand::asImmediate() const
{
	assert(type == Type::Immediate);
	return std::static_pointer_cast <Value> (node);
}

std::shared_ptr <Node> ExecutionStep::Operand::getNode() const
{
	return node;
}

}
