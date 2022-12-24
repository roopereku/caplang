#include "X86Intel.hh"
#include "../Scope.hh"
#include "../Logger.hh"
#include "../Debug.hh"
#include "../Variable.hh"

//#define EXPLAIN_CODE

#ifdef EXPLAIN_CODE
#define EXPLANATION(what) what 

#else
#define EXPLANATION(what) ""

#endif

void Cap::Arch::X86Intel::prepareForLine()
{
	currentRegister = 0;
	previousRegister = 0;
	previousNode = nullptr;
	registerUsedAt.clear();
	registerHasValue.fill(false);
}

bool Cap::Arch::X86Intel::generateInstruction(SyntaxTreeNode& node, std::string& code)
{
	using T = SyntaxTreeNode::Type;

	if(node.type == T::Call)
	{
		code += "call " + node.value->getString() + "\n";
		registerHasValue[currentRegister] = true;
		return true;
	}

	InstructionType t = getType(node.type);
	std::string op = "nop";

	if(t == InstructionType::Assignment)
	{
		Variable* v = scope->findVariable(node.left->value);
		auto it = stackLocations.find(v);

		if(it == stackLocations.end())
		{
			//	FIXME Use insert instead
			stackLocations[v] = stackPointer;
			it = stackLocations.find(v);

			stackPointer += v->type->baseSize;
		}

		//	Is there a value on the right side?
		if(node.right->type == T::Value)
		{
			//	TODO Ignore typenames though this should probably be done before code generation
			code += "mov [rbp-" + std::to_string(it->second) + "], " + getValue(*node.right) + "\n";
		}

		//	The right side is an operator
		else
		{
			code += "mov [rbp-" + std::to_string(it->second) + "], " + registers[currentRegister] + "\n";
			registerHasValue[currentRegister] = false;
		}

		return true;
	}

	//	What's the name of the operation
	switch(node.type)
	{
		case T::Addition: op = "add"; break;
		case T::Division: op = "div"; break;
		case T::Subtraction: op = "sub"; break;
		case T::Multiplication: op = "imul"; break;

		case T::BitwiseShiftLeft: op = "shl"; break;
		case T::BitwiseShiftRight: op = "shr"; break;
		case T::BitwiseAND: op = "and"; break;
		case T::BitwiseOR: op = "or"; break;
		case T::BitwiseXOR: op = "xor"; break;
		case T::BitwiseNOT: op = "not"; break;

		case T::UnaryNegative: op = "neg"; break;

		case T::UnaryPositive:
			Logger::error("TODO: Ignore unary positive. This probably should be done in parseLine()");
			return false;
		   break;

		default:
			Logger::error("???: Passed '%s' to X86Intel::generateInstruction", node.getTypeString());
			return false;
	}

	bool leftValue = node.left->type == T::Value;

	//	Is there a value on the left side?
	if(leftValue)
	{
		//	If there's already a value in the register, move on to the next before loading in a value
		if(registerHasValue[currentRegister])
		{
			code += EXPLANATION(std::string("\n; Register ") + registers[currentRegister] + " has a value so use the next one\n");

			previousRegister = currentRegister;
			currentRegister++;

			code += EXPLANATION(std::string("\n; The current register is now ") + registers[currentRegister] + "\n");
		}

		Variable* v = scope->findVariable(node.left->value);

		//	Load the left immediate value to the current register if there's no variable
		if(v == nullptr)
			code += std::string("mov ") + registers[currentRegister] + ", " + getValue(*node.left) + "\n";

		else
		{
			//	If there is a variable, load a value from the stack to some register
			auto it = stackLocations.find(v);
			code += std::string("mov ") + registers[currentRegister] + ", [rbp-" + std::to_string(it->second) + "]" +
					EXPLANATION("\t; Load value of variable " + v->name->getString()) + "\n";
		}

		registerHasValue[currentRegister] = true;
	}

	if(t == InstructionType::Arithmetic)
	{
		/*	If there is an operator on the left which isn't the previous operator,
		 *	look up what register needs to be used */
		if(!leftValue && previousNode != node.left.get())
		{
			size_t newCurrent = registerUsedAt[node.left.get()];
			previousRegister = currentRegister;

			currentRegister = newCurrent;
			registerHasValue[currentRegister] = true;
		}

		//	Is the right node an operator?
		if(node.right->type != T::Value)
		{
			size_t rightOperand;

			/*	If the right node is the previous node, use the previous register as
			 *	the right operand because the current register has changed. The previous
			 *	register should contain whatever was previously calculated */
			if(previousNode == node.right.get())
				rightOperand = previousRegister;

			else
			{
				Logger::error(*node.value, "???: Right node isn't the previous one");
				return false;
			}

			code += std::string(op) + " " + registers[currentRegister] + ", " + registers[rightOperand] + "\n";
			registerHasValue[rightOperand] = false;
		}

		else
		{
			code += std::string(op) + " " + registers[currentRegister] + ", " + getValue(*node.right) + "\n";
		}
	}

	else if(t == InstructionType::Unary)
	{
		code += std::string(op) + " " + registers[currentRegister] + "\n";
	}

	previousNode = &node;
	registerUsedAt[&node] = currentRegister;

	return true;
}
