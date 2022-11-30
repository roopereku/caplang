#include "../Scope.hh"
#include "../Logger.hh"
#include "../Variable.hh"
#include "Test.hh"

void Cap::Arch::Test::prepareForLine()
{
	accumulator = 0;
}

bool Cap::Arch::Test::generateInstruction(SyntaxTreeNode& node, std::string& code)
{
	using T = SyntaxTreeNode::Type;

	InstructionType t = getType(node.type);
	std::string op = "nop";

	//	On assignment, save to a slow corresponding to the variable's depth
	if(t == InstructionType::Assignment)
	{
		Variable* v = scope.findVariable(node.left->value);
		code += "save " + std::to_string(v->depth) + "\n";

		return true;
	}

	//	What's the name of the operation
	switch(node.type)
	{
		case T::Addition: op = "add"; break;
		case T::Division: op = "div"; break;
		case T::Subtraction: op = "sub"; break;
		case T::Multiplication: op = "mul"; break;
		case T::Power: op = "pow"; break;

		default:
			Logger::error("???: Passed '%s' to generateInstructionTest", node.getTypeString());
			return false;
	}

	int oldAccumulator = accumulator;

	if(t == InstructionType::Arithmetic)
	{
		//	Which side of the node has a literal or an identifier
		bool leftValue = node.left->type == T::Value;
		bool rightValue = node.right->type == T::Value;

		if(leftValue)
		{
			//	Increment and limit the accumulator index to 2
			if(++accumulator > 2)
				accumulator = 2;

			/*	Example: 48 - 12
			 *	Because the left side of the node has a value and the operation
			 *	could be subtraction, we need to store the value to some other
			 *	accumulator so that we can later load 12 to the first accumulator
			 *	and subtract with that accumulator as the value */
			code += "select " + std::to_string(accumulator) + "\n";

			//	If the value is an identifier, it should be a variable
			if(node.left->value->type == TokenType::Identifier)
			{
				//	Tell the interpreter to load a value from the storage
				Variable* v = scope.findVariable(node.left->value);
				code += "load " + std::to_string(v->depth) + "\n";
			}

			//	Store a literal value to the accumulator
			else code += "store " + node.left->value->getString() + "\n";
		}

		//	If neither side has a value, they contain operators
		if(!rightValue && !leftValue)
		{
			/*	Select the first accumulator so that asub can be used with
			 *	the second accumulator as the value */
			accumulator--;
			code += "select " + std::to_string(accumulator) + "\n";
		}

		//	If there's a value on the right side, call some operator with a literal
		if(rightValue)
			code += op + " " + node.right->value->getString() + "\n";

		//	If there's no value on the right side, call some operator with an accumulator
		else
			code += "a" + op + " " + std::to_string(oldAccumulator) + "\n";
	}

	return true;
}
