#include "Logger.hh"
#include "Scope.hh"
#include "Debug.hh"

Cap::Scope::Scope(Scope* parent, ScopeContext ctx)
		:	parent(parent), ctx(ctx),
			root(nullptr), node(&root)
{
	//	If this scope belongs to a function, the first node always contains parameters
	if(ctx == ScopeContext::Function)
	{
		root.type = SyntaxTreeNode::Type::Parameters;

		//	Create a Variable node so that the expression parser can add variables without "var"
		root.left = std::make_shared <SyntaxTreeNode> (&root, nullptr, SyntaxTreeNode::Type::Variable);

		//	This node is used for the contents of the function
		root.right = std::make_shared <SyntaxTreeNode> (&root, nullptr, SyntaxTreeNode::Type::None);

		//	Start with the parameters
		node = root.left.get();
	}

	else root.type = SyntaxTreeNode::Type::Expression;
}

Cap::Scope::Scope(Scope&& rhs)
	:	parent(rhs.parent), ctx(rhs.ctx),
		root(std::move(rhs.root)), node(rhs.node),
		types(std::move(rhs.types)), blocks(std::move(rhs.blocks)),
		variables(std::move(rhs.variables)), functions(std::move(rhs.functions))
{
	rhs.parent = nullptr;
	rhs.node = nullptr;
}

Cap::Function& Cap::Scope::addFunction(Token* name)
{
	functions.emplace_back(name);
	functions.back().scope = std::make_shared <Scope> (this, ScopeContext::Function);
	DBG_LOG("Added function '%s'", name ? name->getString().c_str() : "anonymous");

	return functions.back();
}

Cap::Type& Cap::Scope::addType(Token* name)
{
	types.emplace_back(name);
	types.back().scope = std::make_shared <Scope> (this, ScopeContext::Type);
	DBG_LOG("Added type '%s'", name->getString().c_str());

	return types.back();
}

Cap::Variable& Cap::Scope::addVariable(Token* name)
{
	unsigned depth = variables.empty() ? 0 : variables.back().depth + 1;
	variables.emplace_back(name, depth);

	return variables.back();
}

Cap::Scope& Cap::Scope::addBlock(ScopeContext ctx)
{
	blocks.emplace_back(this, ctx);
	return blocks.back();
}

Cap::Function* Cap::Scope::findFunction(size_t index)
{
	return &functions[index];
}

Cap::Scope* Cap::Scope::findBlock(size_t index)
{
	return &blocks[index];
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

bool Cap::Scope::validate(CodeGenerator& codeGen)
{
	root.list();

	/*	TODO
	 *	Once there is a way to look into other global scopes,
	 *	look for duplicate declarations */

	SyntaxTreeNode* n = &root;
	codeGen.setScope(*this);

	/*	Validate each "line" separately. The left node of the root
	 *	likely contains an expression and the right node contains the
	 *	next thing. This means that we can just skip to the right node */
	while(n->right)
	{
		//	The expression isn't unused if the current node is one of these
		unusedExpression =	n->type != SyntaxTreeNode::Type::Parameters &&
							n->type != SyntaxTreeNode::Type::Return &&
							n->type != SyntaxTreeNode::Type::ElseIf &&
							n->type != SyntaxTreeNode::Type::While &&
							n->type != SyntaxTreeNode::Type::If;

		lineStart = n;
		initializedVariable = nullptr;
		//DBG_LOG("CTX '%s'", n->getTypeString());

		//	If the node is not a block, it contains an expression that needs validating
		if(n->type != SyntaxTreeNode::Type::Block)
		{
			//	Validate the inner contents of the node
			if(!validateNode(n->left.get()))
				return false;

			//	Unused expressions are forbidden
			if(unusedExpression)
			{
				Logger::error(*n->left->value, "Unused expression");
				return false;
			}

			if(initializedVariable)
				initializedVariable->initialized = true;

			//	The line is valid so let's generate code for it
			if(!codeGen.generateLine(*n))
				return false;
		}

		//	If the node contains an index to the block, validate said block
		else
		{
			Logger::warning("Validating scope of misc block %u", n->value->length);
			if(!findBlock(n->value->length)->validate(codeGen))
				return false;

			codeGen.setScope(*this, true);
		}
		
		n = n->right.get();
	}

	for(auto& t : types)
	{
		DBG_LOG("Validating type %s", t.name->getString().c_str());

		CodeGenerator cg;
		if(!t.scope->validate(cg))
			return false;

		Logger::warning("Final output of type\n%s", cg.getOutput().c_str());
	}

	for(auto& f : functions)
	{
		DBG_LOG("Validating function %s", f.name->getString().c_str());

		CodeGenerator cg;
		if(!f.scope->validate(cg))
			return false;

		DBG_LOG("Final output of function\n%s", cg.getOutput().c_str());
	}

	return true;
}

bool Cap::Scope::validateNode(SyntaxTreeNode* n)
{
	//	Is the node an operator?
	if(n->type <= SyntaxTreeNode::Type::UnaryNegative)
	{
		DBG_LOG("Operator '%s'", n->getTypeString());
		bool checkConversion = true;

		SyntaxTreeNode* leftmost = findAppropriateNode(n->left.get());
		NodeInfo left = getNodeInfoRecursive(leftmost);
		if(!left.at) return false;

		//DBG_LOG("Leftmost at '%s' is '%s' '%s'", n->getTypeString(), leftmost->getTypeString(), leftmost->value->getString().c_str());

		if(isNodeTypeName(left))
		{
			Logger::error(*left.at->value, "Can't use a typename in an expression");
			return false;
		}

		//	Is the operator unary?
		if(n->type >= SyntaxTreeNode::Type::Not)
		{
		}

		//	The operator requires something on both sides
		else
		{
			SyntaxTreeNode* rightmost = findAppropriateNode(n->right.get());
			NodeInfo right = getNodeInfoRecursive(rightmost);
			if(!right.at) return false;

			//DBG_LOG("Rightmost at '%s' is '%s' '%s'", n->getTypeString(), rightmost->getTypeString(), rightmost->value->getString().c_str());

			//	Is the operator assignment?
			if(n->type == SyntaxTreeNode::Type::Assign)
			{
				/*	TODO
				 *	Implement assignment to subscripts and function calls. When something
				 *	is assigned to a function call, the function should return a reference */
				if(	leftmost->type == SyntaxTreeNode::Type::Call ||
					leftmost->type == SyntaxTreeNode::Type::Subscript)
				{
					Logger::error("TODO: Implement assigning to '%s'", leftmost->getTypeString());
					return false;
				}

				//	The expression isn't unused if it assigns something
				unusedExpression = false;

				//	NOTE This should never happen :-)
				if(!left.v)
				{
					Logger::error(*left.at->value, "???: Left isn't a variable in assignment");
					return false;
				}

				//	Is the rightmost node just a typename
				if(isNodeTypeName(right))
				{
					//	Type names can only be used in a variable declaration
					if(lineStart->type != SyntaxTreeNode::Type::Variable)
					{
						Logger::error(*right.at->value, "Type can only be set in a variable declaration");
						return false;
					}

					//DBG_LOG("Variable '%s' now has type '%s'",
							//left.v->name->getString().c_str(), right.t->name->getString().c_str());

					left.v->type = right.t;
					checkConversion = false;
				}

				//	The rightmost node isn't a typename which means that it's a value
				else if(!left.v->initialized)
				{
					//DBG_LOG("Variable '%s' is now initialized with type '%s'",
					//		left.v->name->getString().c_str(), right.t->name->getString().c_str());

					checkConversion = left.v->type != nullptr;

					left.v->type = right.t;
					initializedVariable = left.v;
				}
			}

			//	The rightmost node can't be a typename outside assignments
			else if(isNodeTypeName(right))
			{
				Logger::error(*right.at->value, "Can't use a typename in an expression");
				return false;
			}

			//	If the rightmost node is a variable which hasn't been initialized, it can't be used
			if(right.v && !right.v->initialized)
			{
				Logger::error(*right.at->value, "Can't use variable '%s' before it is initialized",
						right.v->name->getString().c_str());

				return false;
			}

			//DBG_LOG("l %p r %p", left.t, right.t);
			//DBG_LOG("Check conversion %d", checkConversion);

			//	Prevent the usage of incompatible types
			if(checkConversion && !left.t->hasConversion(right.t))
			{
				Logger::error(*right.at->value, "Type '%s' has no conversion to '%s'",
						right.t->name->getString().c_str(), left.t->name->getString().c_str());

				return false;
			}

		}

		//	FIXME It would be nice to start code generation here
	}

	//	Validate the left node
	if(n->left && !validateNode(n->left.get()))
		return false;

	//	Validate the right node
	if(n->right && !validateNode(n->right.get()))
		return false;

	return true;
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
		if(	current->type == SyntaxTreeNode::Type::Access ||
			current->type == SyntaxTreeNode::Type::Subscript ||
			current->type == SyntaxTreeNode::Type::Call)
		{
			return current;
		}

		current = current->left.get();
	}

	return current;
}

Cap::Scope::NodeInfo Cap::Scope::getNodeInfoRecursive(SyntaxTreeNode* n)
{
	Scope* scope = this;
	NodeInfo info;
	info.at = findAppropriateNode(n);

	while(info.at->type == SyntaxTreeNode::Type::Access)
	{
		NodeInfo left = scope->getNodeInfo(info.at->left.get());

		if(!left.at)
			return left;

		//	Accessing a function scope is forbidden
		if(left.f)
		{
			Logger::error(*left.at->value, "Can't access the scope of function '%s'", left.f->name->getString().c_str());
			left.at = nullptr;
			return left;
		}

		else if(left.v)
		{
			//	The left operand has to be initialized
			if(!left.v->initialized)
			{
				Logger::error(*left.at->value, "Can't use '%s' before it's initialized", left.v->name->getString().c_str());
				left.at = nullptr;
				return left;
			}

			//	TODO use the scope of the type of the variable
		}

		if(left.t)
		{
			//	Primitive types don't have visible members
			if(left.t->isPrimitive)
			{
				Logger::error(*left.at->value, "Can't use '.' after primitive type '%s'", left.t->name->getString().c_str());
				left.at = nullptr;
				return left;
			}

			//DBG_LOG("UPDATE SCOPE TO '%s'", left.at->value->getString().c_str());
			scope = left.t->scope.get();
		}

		info.at = info.at->right.get();
	}

	return scope->getNodeInfo(info.at);
}

Cap::Scope::NodeInfo Cap::Scope::getNodeInfo(SyntaxTreeNode* n)
{
	NodeInfo info;
	info.at = n;

	info.v = findVariable(info.at->value);
	info.f = findFunction(info.at->value);

	//	FIXME use Function::returnType once that is implemented
	if(info.f)
		info.t = Type::findPrimitiveType(TokenType::Integer);

	else if(info.v)
	{
		/*	If the token containing the variable name appears before
		 *	the token that is used when creating the actual variable,
		 *	report an error saying that the variable is not declared */
		if(info.at->value < info.v->name)
		{
			Logger::error(*info.at->value, "Can't use variable '%s' before it's declared", getFullAccessName(info.at).c_str());
			info.at = nullptr;
			return info;
		}

		info.t = info.v->type;
	}

	else info.t = findType(info.at->value);

	//	Is the given node a known identifier?
	if(!info.v && !info.t && !info.f)
	{
		if(n->value->type == TokenType::Identifier)
		{
			Logger::error(*info.at->value, "Unknown identifier '%s'", getFullAccessName(info.at).c_str());
			info.at = nullptr;
			return info;
		}

		//DBG_LOG("Literal '%s'", n->value->getString().c_str());
		//info.t = Type::findPrimitiveType(n->value->type);
	}

	return info;
}

std::string Cap::Scope::getFullAccessName(SyntaxTreeNode* last)
{
	//	First we need to know where the first access node is
	SyntaxTreeNode* current = last;
	while(current->parent->type == SyntaxTreeNode::Type::Access)
		current = current->parent;

	//	If there's no accesses, return a single identifier
	if(current->type != SyntaxTreeNode::Type::Access)
		return current->value->getString();

	std::string name = current->left->value->getString();

	//	Add the rest of the access nodes to the string
	while(current->left)
	{
		name += '.' + current->right->value->getString();
		current = current->right.get();
	}

	return name;
}

bool Cap::Scope::isNodeTypeName(NodeInfo& info)
{
	return	info.at->value->type == TokenType::Identifier &&
			info.at->type == SyntaxTreeNode::Type::Value &&
			!info.v && info.t;
}
