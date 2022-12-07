#include "X86Intel.hh"
#include "../Scope.hh"
#include "../Logger.hh"
#include "../Debug.hh"
#include "../Variable.hh"

#define EXPLAIN_CODE

#ifdef EXPLAIN_CODE
#define EXPLANATION(what) what 

#else
#define EXPLANATION(what) ""

#endif

void Cap::Arch::X86Intel::prepareForLine()
{
	currentRegister = 0;
	previousRegister = 0;
	didArithmetic = false;
	registerHasValue.fill(false);
}

bool Cap::Arch::X86Intel::generateInstruction(SyntaxTreeNode& node, std::string& code)
{
	using T = SyntaxTreeNode::Type;

	if(node.type == T::Call)
	{
		code += "call " + node.value->getString() + "\n";
		registerHasValue[currentRegister] = true;
		didArithmetic = false;
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

			DBG_LOG("Stackpointer -> %lu because '%s' was added", stackPointer, v->name->getString().c_str());
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

		didArithmetic = false;
		return true;
	}

	//	What's the name of the operation
	switch(node.type)
	{
		case T::Addition: op = "add"; break;
		case T::Division: op = "div"; break;
		case T::Subtraction: op = "sub"; break;
		case T::Multiplication: op = "imul"; break;

		case T::UnaryNegative: op = "neg"; break;

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
					EXPLANATION("\t; Loading value of variable " + v->name->getString()) + "\n";
		}

		registerHasValue[currentRegister] = true;
		didArithmetic = false;
	}

	if(t == InstructionType::Arithmetic)
	{
		//	Which side of the node has a literal or an identifier
		bool rightValue = node.right->type == T::Value;
		
		//	If neither side has a value, use the current and the previous register as operands
		if(!leftValue && !rightValue)
		{
			if(previousRegister == 0)
			{
				code += EXPLANATION(std::string("\n; Swap ") + registers[currentRegister] + " and " + registers[previousRegister] + "\n");
				std::swap(currentRegister, previousRegister);
			}

			code += EXPLANATION("\n; Operators on both sides\n");

			code += std::string(op) + " " + registers[currentRegister] + ", " + registers[previousRegister] + "\n";
			registerHasValue[currentRegister] = true;
			registerHasValue[previousRegister] = false;
		}

		else
		{
			//	If there's a value on the right side, use it as an operand
			if(rightValue)
			{
				Variable* v = scope->findVariable(node.right->value);

				//	If there's no variable on the right, it should be a literal value
				if(v == nullptr)
					code += std::string(op) + " " + registers[currentRegister] + ", " + getValue(*node.right) + "\n";

				else
				{
					//	If there is a variable on the right, use a value from the stack as the operand
					auto it = stackLocations.find(v);
					code += std::string(op) + " " + registers[currentRegister] + ", [rbp-" + std::to_string(it->second) + "]\n";
				}

				registerHasValue[currentRegister] = true;
			}

			//	If there's an operator on the right side, use the previous register as an operand
			else
			{
				code += std::string(op) + " " + registers[currentRegister] + ", " + registers[previousRegister] +
						EXPLANATION("\t; Register " + registers[previousRegister] + " is no longer used") + "\n";

				registerHasValue[currentRegister] = true;
				registerHasValue[previousRegister] = false;

				previousRegister = currentRegister;
				code += EXPLANATION(std::string("\n; Register ") + registers[previousRegister] + " is now the previous register\n");

				currentRegister--;

				code += EXPLANATION(std::string("; Register ") + registers[currentRegister] + " is now the current register\n");
			}
		}

		didArithmetic = true;
	}

	else if(t == InstructionType::Unary)
	{
		DBG_LOG("Unary");

		/*	If the last instruction used a as it's right operand, we
		 *	still need t
		 */
		if(didArithmetic)// && currentRegister > 0)
		{
			code += EXPLANATION(std::string("\n; Use ") + registers[previousRegister] + " instead of " + registers[currentRegister] + " because the last instruction was arithmetic\n");
			code += std::string(op) + " " + registers[previousRegister] + "\n";
		}

		else code += std::string(op) + " " + registers[currentRegister] + "\n";
		didArithmetic = false;
	}

	return true;
}
