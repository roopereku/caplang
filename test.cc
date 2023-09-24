#include <cap/SourceFile.hh>
#include <cap/node/FunctionDeclaration.hh>
#include <cap/node/TypeDeclaration.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/FunctionCall.hh>
#include <cap/node/Subscript.hh>
#include <cap/node/Operator.hh>
#include <cap/node/Statement.hh>
#include <cap/node/Return.hh>

#include <fstream>

class GraphGenerator
{
public:
	GraphGenerator(std::string_view path) : file(path.data())
	{
		file << "@startmindmap\n";
	}

	~GraphGenerator()
	{
		file << "@endmindmap\n";
	}

	void generate(std::shared_ptr <cap::Node> root)
	{
		generateNode(root, 1);
	}

private:
	void generateNode(std::shared_ptr <cap::Node> node, unsigned depth)
	{
		printf("%u '%s' %u\n", node->id, node->getToken().c_str(), depth);

		if(node->isExpression())
		{
			//printf("Expression node\n");
			auto expr = std::static_pointer_cast <cap::Expression> (node);

			if(expr->isOperator())
			{
				//printf("Operator\n");
				auto op = std::static_pointer_cast <cap::Operator> (node);

				if(op->isTwoSided())
				{
					//printf("Two sided operator\n");
					auto twoSided = std::static_pointer_cast <cap::TwoSidedOperator> (op);
					file << indent(depth) << twoSided->getTypeString() << "\n";

					generateNode(twoSided->getLeft(), depth + 1);
					generateNode(twoSided->getRight(), depth + 1);
				}

				else if(op->isOneSided())
				{
					auto oneSided = std::static_pointer_cast <cap::OneSidedOperator> (op);
					file << indent(depth) << oneSided->getTypeString() << "\n";

					if(oneSided->type == cap::OneSidedOperator::Type::FunctionCall)
					{
						//	printf("Function call %p\n", oneSided->getExpression().get());
						auto call = std::static_pointer_cast <cap::FunctionCall> (oneSided);
						generateNode(call->getParameters(), depth + 1);

						if(call->getExpression())
							generateNode(call->getExpression(), depth + 1);
					}

					else if(oneSided->type == cap::OneSidedOperator::Type::Subscript)
					{
						auto subscript = std::static_pointer_cast <cap::Subscript> (oneSided);
						generateNode(subscript->getContents(), depth + 1);

						if(subscript->getExpression())
							generateNode(subscript->getExpression(), depth + 1);
					}

					else generateNode(oneSided->getExpression(), depth + 1);
				}
			}

			else if(expr->isValue())
			{
				//printf("Value\n");
				file << indent(depth) << "Value: " << expr->getToken() << "\n";
			}

			else if(expr->isExpressionRoot())
			{
				printf("Expression root\n");
				file << indent(depth) << "Expression root\n";

				if(!std::static_pointer_cast <cap::ExpressionRoot> (node)->getRoot())
				{
					printf("No root\n");
				}

				else
				{
					generateNode(std::static_pointer_cast <cap::ExpressionRoot> (node)->getRoot(), depth + 1);
				}
			}
		}

		else if(node->isFunctionDeclaration())
		{
			auto decl = std::static_pointer_cast <cap::FunctionDeclaration> (node);

			//printf("Function declaration\n");
			file << indent(depth) << "Function: " << decl->function->getName().getString() << "\n";
			generateNode(decl->function->getRoot(), depth + 1);
		}

		else if(node->isTypeDeclaration())
		{
			auto decl = std::static_pointer_cast <cap::TypeDeclaration> (node);

			file << indent(depth) << "Type: " << decl->type->getName().getString() << "\n";
			generateNode(decl->type->getRoot(), depth + 1);
		}

		else if(node->isStatement())
		{
			auto statement = std::static_pointer_cast <cap::Statement> (node);
			file << indent(depth) << "Statement: " << statement->getToken() << "\n";

			if(statement->isReturn())
			{
				generateNode(std::static_pointer_cast <cap::Return> (statement)->expression, depth + 1);
			}
		}

		if(node->hasNext())
		{
			generateNode(node->getNext(), depth);
		}
	}

	std::string indent(unsigned depth)
	{
		return std::string(depth, '*') + ' ';
	}

	std::ofstream file;
};

int main()
{
	cap::SourceFile entry("../test.cap");

	GraphGenerator gen("output");
	gen.generate(entry.getGlobal().getRoot());
}
