#include <cap/execution/ExecutionSequence.hh>
#include <cap/Traverser.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Statement.hh>
#include <cap/Identifier.hh>
#include <cap/Integer.hh>
#include <cap/String.hh>

#include <cassert>
#include <limits>

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

	Result onBracketOperator(std::shared_ptr <BracketOperator> node) override
	{
		// Since the execution step is constructed before traversing to operands,
		// use the max value possible for the index to make addOperand prioritize
		// the result indices of the operands.
		size_t initialResultindex = node->getInnerRoot()->getFirst() ?
			std::numeric_limits <size_t>::max() : resultIndex;

		ExecutionStep step(node, initialResultindex);
		ArgumentAccessor parameters(node->getInnerRoot());

		while(auto param = parameters.getNext())
		{
			auto operand = getOperand(param);

			// Immediate operands become steps that store an immediate.
			// This is for consistency as all parameters should be behind a result index.
			if(operand.getType() == ExecutionStep::Operand::Type::Immediate)
			{
				if(sequence.begin() != sequence.end())
				{
					resultIndex++;
				}

				ExecutionStep immediateStore(std::move(operand), resultIndex);
				operand = ExecutionStep::Operand(immediateStore.getResultIndex(), param);
				sequence.addStep(std::move(immediateStore));
			}

			step.addOperand(std::move(operand));
		}

		resultIndex = step.getResultIndex();
		currentOperand.emplace(resultIndex, node);
		sequence.addStep(std::move(step));

		return Result::Exit;

	}

	Result onIdentifier(std::shared_ptr <Identifier> node) override
	{
		currentOperand.emplace(node); 
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
