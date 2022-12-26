#include "Base.hh"
#include "../Debug.hh"
#include "../Logger.hh"

#include <sstream>

const char* Cap::Arch::Base::instructionTypeString(InstructionType t)
{
	switch(t)
	{
		case InstructionType::Arithmetic: return "Arithmetic";
		case InstructionType::Assignment: return "Assignment";
		case InstructionType::Comparison: return "Comparison";
		case InstructionType::Unary: return "Unary";
	}

	return "";
}

void Cap::Arch::Base::setScope(Scope& scope)
{
	code += "; Scope set\n";
	this->scope = &scope;
}

std::string Cap::Arch::Base::getValue(SyntaxTreeNode& node)
{
	switch(node.value->type)
	{
		case TokenType::Integer:
		case TokenType::Identifier:
			return node.value->getString();

		case TokenType::Character:
		{
			int ch = static_cast <int> (*node.value->begin);
			return std::to_string(ch);
		}

		case TokenType::Hexadecimal:
		{
			std::stringstream ss;
			int dec;

			ss << std::hex << node.value->getString();
			ss >> dec;

			return std::to_string(dec);
		}

		case TokenType::Binary:
		{
			int dec = 0;
			int cmp = 1;

			//	Create an integer from the string representation of a binary number
			for(int i = node.value->length - 1; i >= 0; i--)
			{
				char c = *(node.value->begin + i);
				if(c == '1') dec |= cmp;
				cmp <<= 1;
			}

			return std::to_string(dec);
		}

		default:
			Logger::error(*node.value, "???: Passed '%s' to Arch::Base::getValue", node.value->getTypeString());
			return "";
	}
}

Cap::Arch::Base::InstructionType Cap::Arch::Base::getType(SyntaxTreeNode::Type t)
{
	switch(t)
	{
		case SyntaxTreeNode::Type::Assign:
			return InstructionType::Assignment;

		case SyntaxTreeNode::Type::Or:
		case SyntaxTreeNode::Type::And:
		case SyntaxTreeNode::Type::BitwiseOR:
		case SyntaxTreeNode::Type::BitwiseAND:
		case SyntaxTreeNode::Type::BitwiseXOR:
		case SyntaxTreeNode::Type::BitwiseShiftLeft:
		case SyntaxTreeNode::Type::BitwiseShiftRight:
		case SyntaxTreeNode::Type::Addition:
		case SyntaxTreeNode::Type::Subtraction:
		case SyntaxTreeNode::Type::Multiplication:
		case SyntaxTreeNode::Type::Division:
		case SyntaxTreeNode::Type::Modulus:
		case SyntaxTreeNode::Type::Power:
			return InstructionType::Arithmetic;

		case SyntaxTreeNode::Type::Equal:
		case SyntaxTreeNode::Type::Inequal:
		case SyntaxTreeNode::Type::Less:
		case SyntaxTreeNode::Type::Greater:
		case SyntaxTreeNode::Type::LessEqual:
		case SyntaxTreeNode::Type::GreaterEqual:
			return InstructionType::Comparison;

		case SyntaxTreeNode::Type::Not:
		case SyntaxTreeNode::Type::Reference:
		case SyntaxTreeNode::Type::BitwiseNOT:
		case SyntaxTreeNode::Type::UnaryPositive:
		case SyntaxTreeNode::Type::UnaryNegative:
			return InstructionType::Unary;

		default:
			Logger::error("???: Requested instruction type for '%s'", SyntaxTreeNode::getTypeString(t));
			return InstructionType::Arithmetic;
	}
}
