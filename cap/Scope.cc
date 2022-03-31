#include "Scope.hh"
#include "Debug.hh"

Cap::Scope::Scope(Scope* parent, ScopeContext ctx, size_t begin, size_t end)
		:	parent(parent), ctx(ctx), begin(begin), end(end),
			root(nullptr), node(&root)
{
	//	If this scope belongs to a function, the first node always contains parameters
	if(ctx == ScopeContext::Function)
	{
		root.type = SyntaxTreeNode::Type::Parameters;

		//	Create a Variable node so that the expression parser can add variables without "var"
		root.left = std::make_shared <SyntaxTreeNode> (&root, nullptr, SyntaxTreeNode::Type::Variable);

		//	This node is used for the first expression inside the function
		root.right = std::make_shared <SyntaxTreeNode> (&root, nullptr, SyntaxTreeNode::Type::Expression);
	}

	else root.type = SyntaxTreeNode::Type::Expression;
}

Cap::Function& Cap::Scope::addFunction(Token* name)
{
	functions.emplace_back(name);
	DBG_LOG("Added function '%s'", name ? name->getString().c_str() : "anonymous");

	return functions.back();
}

Cap::Type& Cap::Scope::addType(Token* name, size_t begin, size_t end)
{
	types.emplace_back(name);
	types.back().scope = std::make_shared <Scope> (this, ScopeContext::Type, begin, end);
	DBG_LOG("Added type '%s'", name->getString().c_str());

	return types.back();
}

Cap::Variable& Cap::Scope::addVariable(Token* name)
{
	variables.emplace_back(name);
	DBG_LOG("Added variable '%s'", name->getString().c_str());
	return variables.back();
}

Cap::Function* Cap::Scope::findFunction(Token* name)
{
	for(auto& it : functions)
	{
		if(it.name && it.name->tokenEquals(name))
			return &it;
	}

	return parent == nullptr ? nullptr : parent->findFunction(name);
}

Cap::Variable* Cap::Scope::findVariable(Token* name)
{
	for(auto& it : variables)
	{
		//DBG_LOG("var '%s'", it.name->getString().c_str());
		if(it.name->tokenEquals(name))
			return &it;
	}

	return parent == nullptr ? nullptr : parent->findVariable(name);
}

Cap::Type* Cap::Scope::findType(Token* name)
{
	for(auto& it : types)
	{
		if(it.name->tokenEquals(name))
			return &it;
	}

	return parent == nullptr ? Type::findPrimitiveType(name) : parent->findType(name);
}

Cap::SyntaxTreeNode* Cap::Scope::validate(Cap::ValidationResult& result)
{
	/*	TODO
	 *	Once there is a way to look into other global scopes,
	 *	look for duplicate declarations */
	SyntaxTreeNode* errorAt = validateNode(&root, result);

	if(result != ValidationResult::Success)
		return errorAt;

	for(auto& t : types)
	{
		DBG_LOG("Validating type %s", t.name->getString().c_str());

		errorAt = t.scope->validate(result);
		if(result != ValidationResult::Success)
			return errorAt;

	}

	for(auto& f : functions)
	{
		DBG_LOG("Validating function %s", f.name->getString().c_str());

		errorAt = f.scope->validate(result);
		if(result != ValidationResult::Success)
			return errorAt;
	}

	return nullptr;
}

Cap::SyntaxTreeNode* Cap::Scope::validateNode(SyntaxTreeNode* n, ValidationResult& result)
{
	//	Does the current node contains an operator
	if(n->type <= SyntaxTreeNode::Type::UnaryNegative)
	{
		//	Does the expression do anything
		if(	n->parent->type == SyntaxTreeNode::Type::Expression &&
			n->type != SyntaxTreeNode::Type::Assign)
	{
			printf("UNUSED EXPRESSION on line at %u:%u\n", n->value->line, n->value->column);
		}

		/*	If the current node contains an access operator, we only need to
		 *	get info starting from said node because of
		 *	the way getNodeInfoRecursive() works */
		SyntaxTreeNode* leftOrigin = n->type == SyntaxTreeNode::Type::Access ?
			n : n->left.get();

		//	Get information about the left node
		NodeInfo left = getNodeInfoRecursive(leftOrigin, result);

		if(result != ValidationResult::Success)
			return left.at;

		//	The left node can only be a plain typename if an access is being made
		if(isNodeTypeName(left) && left.at->parent->type != SyntaxTreeNode::Type::Access)
		{
			result = ValidationResult::InvalidOperand;
			return left.at;
		}

		DBG_LOG("Left of '%s' is '%s'", n->value->getString().c_str(), left.at->value->getString().c_str());

		//	Does said operator have 2 operands?
		if(	n->type <= SyntaxTreeNode::Type::Power &&
			n->type != SyntaxTreeNode::Type::Access)
		{
			NodeInfo right = getNodeInfoRecursive(n->right.get(), result);

			if(result != ValidationResult::Success)
				return right.at;

			bool isRightType = isNodeTypeName(right);

			//	Does the right node have a conversion to the type of the left node
			if(left.t && right.t && !right.t->hasConversion(*left.t))
			{
				//	TODO add a way to return a range betweem 2 nodes
				result = ValidationResult::NoConversion;
				return right.at;
			}

			DBG_LOG("Right of '%s' is '%s'", n->value->getString().c_str(), right.at->value->getString().c_str());

			if(n->type == SyntaxTreeNode::Type::Assign)
			{
				//	If the node on the right doesn't contain a plain type, it's a value
				if(!isRightType)
				{
					if(left.v)
					{
						//	Forbid assigning a variable it's own value before it's initialized
						if(!left.v->initialized && left.at->value->tokenEquals(right.at->value))
						{
							result = ValidationResult::UseBeforeInit;
							return right.at;
						}

						left.v->type = right.t;
						left.v->initialized = true;

						DBG_LOG("Variable '%s' initialized", left.v->name->getString().c_str());
					}
				}

				else
				{
					//	Is the left node a variable that's initialized
					if(left.v)
					{
						//	Types cannot be assigned to variables if they already have types
						if(left.v->type && isRightType)
						{
							result = ValidationResult::TypingOutsideInit;
							return right.at;
						}

						//	If the variable doesn't have a type, give it one
						else if(!left.v->type)
						{
							DBG_LOG("Assigning type '%s' to '%s'", right.t->name->getString().c_str(), left.v->name->getString().c_str());
							left.v->type = right.t;
						}
					}
				}
			}

			else
			{
				//	The right node can't be a plain type if no assignment was done
				if(isRightType)
				{
					result = ValidationResult::InvalidOperand;
					return right.at;
				}

				if(left.v && !left.v->initialized)
				{
					result = ValidationResult::UseBeforeInit;
					return left.at;
				}

				if(right.v && !right.v->initialized)
				{
					result = ValidationResult::UseBeforeInit;
					return right.at;
				}
			}
		}

		else
		{
		}
	}

	SyntaxTreeNode* resultNode;

	//	Validate the left node
	if(n->left && (resultNode = validateNode(n->left.get(), result)))
		return resultNode;

	//	Validate the right node
	if(n->right && (resultNode = validateNode(n->right.get(), result)))
		return resultNode;

	return nullptr;
}

Cap::SyntaxTreeNode* Cap::Scope::findAppropriateNode(SyntaxTreeNode* n)
{
	if(	n->type == SyntaxTreeNode::Type::Value ||
		n->type == SyntaxTreeNode::Type::Call ||
		n->type == SyntaxTreeNode::Type::Subscript)
	{
		return n;
	}

	SyntaxTreeNode* current = n;

	while(current->left)
	{
		if(current->type == SyntaxTreeNode::Type::Access)
			return current;

		current = current->left.get();
	}

	return current;
}

Cap::Scope::NodeInfo Cap::Scope::getNodeInfoRecursive(SyntaxTreeNode* n, ValidationResult& result)
{
	Scope* scope = this;
	NodeInfo info;
	info.at = findAppropriateNode(n);

	while(info.at->type == SyntaxTreeNode::Type::Access)
	{
		NodeInfo left = scope->getNodeInfo(info.at->left.get(), result);

		if(result != ValidationResult::Success)
			return left;

		//	FIXME use a more describing result
		//	Accessing a function scope is forbidden
		if(left.f)
		{
			result = ValidationResult::InvalidOperand;
			return left;
		}

		else if(left.v)
		{
			DBG_LOG("VARIABLE '%s'", left.at->value->getString().c_str());
			//	The left operand has to be initialized
			if(!left.v->initialized)
			{
				result = ValidationResult::UseBeforeInit;
				return left;
			}

			//	TODO use the scope of the type of the variable
		}

		if(left.t)
		{
			//	FIXME use a more describing result
			//	Primitive types don't have visible members
			if(left.t->isPrimitive)
			{
				result = ValidationResult::InvalidOperand;
				return left;
			}

			DBG_LOG("UPDATE SCOPE TO '%s'", left.at->value->getString().c_str());
			scope = left.t->scope.get();
		}

		info.at = info.at->right.get();
	}

	return scope->getNodeInfo(info.at, result);
}

Cap::Scope::NodeInfo Cap::Scope::getNodeInfo(SyntaxTreeNode* n, ValidationResult& result)
{
	NodeInfo info;
	info.at = n;

	info.v = findVariable(info.at->value);
	info.t = findType(info.at->value);
	info.f = findFunction(info.at->value);

	//	Is the given node a known identifier?
	if(!info.v && !info.t && !info.f)
	{
		result = ValidationResult::IdentifierNotFound;
		return info;
	}

	if(info.v)
		info.t = info.v->type;

	return info;
}

bool Cap::Scope::isNodeTypeName(NodeInfo& info)
{
	return	info.at->value->type == TokenType::Identifier &&
			info.at->type == SyntaxTreeNode::Type::Value &&
			!info.v && info.t;
}
