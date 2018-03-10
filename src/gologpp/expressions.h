#ifndef GOLOGPP_EXPRESSIONS_H_
#define GOLOGPP_EXPRESSIONS_H_

#include <memory>
#include "Language.h"

namespace gologpp {


enum ExpressionTypeTag {
	BOOLEAN_EXPRESSION,
	VALUE_EXPRESSION,
	STATEMENT
};


class Expression : public std::enable_shared_from_this<Expression>, public virtual AbstractLanguageElement {
protected:
	Expression(Scope &parent_scope);
	Expression(const Expression &) = delete;
	Expression(Expression &&) = default;
	Expression &operator = (const Expression &) = delete;

public:
	virtual ~Expression() = default;

	Scope &parent_scope();
	const Scope &parent_scope() const;

	static ExpressionTypeTag expression_type_tag();

private:
	Scope &parent_scope_;
};


class BooleanExpression : public Expression {
public:
	typedef BooleanExpression expression_t;

	BooleanExpression(BooleanExpression &&) = default;
	using Expression::Expression;

	static ExpressionTypeTag expression_type_tag();
};


class ValueExpression : public Expression {
public:
	typedef ValueExpression expression_t;

	ValueExpression(ValueExpression &&) = default;
	using Expression::Expression;

	static ExpressionTypeTag expression_type_tag();
};


class Statement : public Expression {
public:
	typedef Statement expression_t;

	Statement(Statement &&) = default;
	using Expression::Expression;

	static ExpressionTypeTag expression_type_tag();
};


}

#endif // GOLOGPP_EXPRESSIONS_H_