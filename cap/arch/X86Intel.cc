#include "X86Intel.hh"
#include "../Scope.hh"
#include "../Logger.hh"
#include "../Debug.hh"
#include "../Variable.hh"

void Cap::Arch::X86Intel::prepareForLine()
{
	currentRegister = 0;
	registerHasValue = false;
}

bool Cap::Arch::X86Intel::generateInstruction(SyntaxTreeNode& node, std::string& code)
{
	using T = SyntaxTreeNode::Type;

	InstructionType t = getType(node.type);
	std::string op = "nop";

	if(t == InstructionType::Assignment)
	{
		Variable* v = scope.findVariable(node.left->value);
		return true;
	}

	//	What's the name of the operation
	switch(node.type)
	{
		case T::Addition: op = "add"; break;
		case T::Division: op = "div"; break;
		case T::Subtraction: op = "sub"; break;
		case T::Multiplication: op = "mul"; break;

		default:
			Logger::error("???: Passed '%s' to X86Intel::generateInstruction", node.getTypeString());
			return false;
	}

	if(t == InstructionType::Arithmetic)
	{
		//	Which side of the node has a literal or an identifier
		bool leftValue = node.left->type == T::Value;
		bool rightValue = node.right->type == T::Value;
		
		//	If neither side has a value, use the current and the previous register as operands
		if(!leftValue && !rightValue)
		{
			/*	Because the operation is applied to the previous register, switch to it
			 *	so that future operations will target that previous too */
			currentRegister--;
			code += std::string(op) + " " + registers[currentRegister] + ", " + registers[currentRegister + 1] + "\n";
			registerHasValue = true;
		}

		else
		{
			//	Is there a value on the left side?
			if(leftValue)
			{
				//	If there's already a value in the register, move on to the next before loading in a value
				if(registerHasValue)
				{
					registerHasValue = false;
					currentRegister++;
				}

				//	Load the left value to the current register
				code += std::string("mov ") + registers[currentRegister] + ", " + node.left->value->getString() + "\n";
			}

			//	If there's a value on the right side, use it as an operand
			if(rightValue)
				code += std::string(op) + " " + registers[currentRegister] + ", " + node.right->value->getString() + "\n";

			//	If there's an operator on the right side, use the previous register as an operand
			else code += std::string(op) + " " + registers[currentRegister] + ", " + registers[currentRegister - 1] + "\n";

			registerHasValue = true;
		}
	}

	return true;
}
