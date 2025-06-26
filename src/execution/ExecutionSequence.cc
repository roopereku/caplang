#include <cap/execution/ExecutionSequence.hh>
#include <cap/Traverser.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Statement.hh>
#include <cap/Integer.hh>
#include <cap/String.hh>

#include <cassert>

namespace cap
{

class SequenceCreator : public Traverser
{
public:
	SequenceCreator(ExecutionSequence& sequence)
		: sequence(sequence)
	{
	}

	Result onBinaryOperator(std::shared_ptr <BinaryOperator> node) override
	{
		auto left = getOperand(node->getLeft());
		auto right = getOperand(node->getRight());

		if(sequence.begin() != sequence.end())
		{
			resultIndex++;
		}

		ExecutionStep step(node, resultIndex);
		step.addOperand(std::move(left));
		step.addOperand(std::move(right));

		resultIndex = step.getResultIndex();
		currentOperand.emplace(resultIndex, node);
		sequence.addStep(std::move(step));

		return Result::Exit;
	}

	Result onUnaryOperator(std::shared_ptr <UnaryOperator> node) override
	{
		auto expr = getOperand(node->getExpression());

		if(sequence.begin() != sequence.end())
		{
			resultIndex++;
		}

		ExecutionStep step(node, resultIndex);
		step.addOperand(std::move(expr));

		resultIndex = step.getResultIndex();
		currentOperand.emplace(resultIndex, node);
		sequence.addStep(std::move(step));

		return Result::Exit;
	}

	Result onInteger(std::shared_ptr <Integer> node) override
	{
		currentOperand.emplace(node); 
		return Result::Exit;
	}

	Result onString(std::shared_ptr <String> node) override
	{
		// TODO: String interpolation?
		currentOperand.emplace(node); 
		return Result::Exit;
	}

private:
	ExecutionStep::Operand getOperand(std::shared_ptr <Expression> node)
	{
		currentOperand.reset();
		traverseExpression(node);

		assert(currentOperand.has_value());
		return *currentOperand;
	}

	ExecutionSequence& sequence;
	std::optional <ExecutionStep::Operand> currentOperand;
	size_t resultIndex = 0;
};

ExecutionSequence::ExecutionSequence(std::shared_ptr <Function> root)
	: representedFunction(root)
{
	SequenceCreator creator(*this);
	creator.traverseNode(root->getBody());
}

ExecutionSequence::ExecutionSequence(std::shared_ptr <Expression> root)
{
	SequenceCreator creator(*this);
	creator.traverseNode(root);
}

std::shared_ptr <Function> ExecutionSequence::getRepresentedFunction() const
{
	return representedFunction;
}

ExecutionStep& ExecutionSequence::addStep(ExecutionStep&& step)
{
	steps.emplace_back(std::move(step));
	return steps.back();
}

}
