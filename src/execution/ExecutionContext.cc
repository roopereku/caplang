#include <cap/execution/ExecutionContext.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Statement.hh>

#include <cassert>

namespace cap
{

bool ExecutionContext::isFinished() const
{
	return frames.empty();
}

void ExecutionContext::start(std::shared_ptr <Function> node)
{
	auto it = sequenceLookup.find(node);
	if(it == sequenceLookup.end())
	{
		it = sequenceLookup.insert(std::make_pair(node, ExecutionSequence(node))).first;
	}

	frames.emplace(it->second);
}

void ExecutionContext::step()
{
	assert(!frames.empty());
	if(!frames.top().step())
	{
		frames.pop();
	}
}

ExecutionContext::Frame::Frame(ExecutionSequence& sequence)
	: sequence(sequence), pointer(sequence.begin())
{
	// TODO: Initialize parameters using the represented function.
	// TODO: Initialize all local declarations.	
}

bool ExecutionContext::Frame::step()
{
	switch(pointer->type)
	{
		case ExecutionStep::Type::BinaryOperator:
		{
			executeBinaryOperator();
			break;
		}

		case ExecutionStep::Type::UnaryOperator:
		{
			executeUnaryOperator();
			break;
		}

		case ExecutionStep::Type::BracketOperator:
		{
			executeBracketOperator();
			break;
		}

		case ExecutionStep::Type::Statement:
		{
			executeStatement();
			break;
		}
	}

	pointer++;
	return pointer != sequence.end();
}

void ExecutionContext::Frame::executeBinaryOperator()
{
	auto op = std::static_pointer_cast <BinaryOperator> (pointer->action);
	switch(op->getType())
	{
		case BinaryOperator::Type::Assign:
		{
			break;
		}

		case BinaryOperator::Type::Access:
		{
			break;
		}

		case BinaryOperator::Type::Comma:
		{
			break;
		}

		case BinaryOperator::Type::Add:
		{
			break;
		}

		case BinaryOperator::Type::Subtract:
		{
			break;
		}

		case BinaryOperator::Type::Multiply:
		{
			break;
		}

		case BinaryOperator::Type::Divide:
		{
			break;
		}

		case BinaryOperator::Type::Modulus:
		{
			break;
		}

		case BinaryOperator::Type::Exponent:
		{
			break;
		}

		case BinaryOperator::Type::Or:
		{
			break;
		}

		case BinaryOperator::Type::And:
		{
			break;
		}

		case BinaryOperator::Type::Less:
		{
			break;
		}

		case BinaryOperator::Type::Greater:
		{
			break;
		}

		case BinaryOperator::Type::LessEqual:
		{
			break;
		}

		case BinaryOperator::Type::GreaterEqual:
		{
			break;
		}

		case BinaryOperator::Type::Equal:
		{
			break;
		}

		case BinaryOperator::Type::Inequal:
		{
			break;
		}

		case BinaryOperator::Type::BitwiseShiftLeft:
		{
			break;
		}

		case BinaryOperator::Type::BitwiseShiftRight:
		{
			break;
		}

		case BinaryOperator::Type::BitwiseAnd:
		{
			break;
		}

		case BinaryOperator::Type::BitwiseOr:
		{
			break;
		}

		case BinaryOperator::Type::BitwiseXor:
		{
			break;
		}
	}
}

void ExecutionContext::Frame::executeUnaryOperator()
{
}

void ExecutionContext::Frame::executeBracketOperator()
{
}

void ExecutionContext::Frame::executeStatement()
{
	auto statement = std::static_pointer_cast <Statement> (pointer->action);
	switch(statement->getType())
	{
		case Statement::Type::VariableRoot:
		{
			assert(false && "TODO: Unnecessary case");
			break;
		}

		case Statement::Type::Return:
		{
			// TODO: Somehow emit a result as a return value.

			// Go to the last step so that Frame::step will return false after incrementing the step.
			pointer = sequence.end() - 1;
			break;
		}
	}
}

}
