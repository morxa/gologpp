#ifndef GOLOGPP_PARSER_FORMULA_H_
#define GOLOGPP_PARSER_FORMULA_H_

#include <model/formula.h>

#include "utilities.h"


namespace gologpp {
namespace parser {


struct ComparisonParser : grammar<Comparison *(Scope &), locals<Typename>> {
	ComparisonParser();

	rule<Comparison *(Scope &), locals<Typename>> comparison;
	rule<ComparisonOperator()> cmp_op;
	rule<Expression *(Scope &)> comparable_expr;
};



struct BooleanExpressionParser : grammar<Expression *(Scope &)> {
	BooleanExpressionParser();

	rule<Expression *(Scope &)> expression;
	rule<Expression *(Scope &)> unary_expr;
	rule<Expression *(Scope &)> operation_seq;
	rule<Expression *(Scope &)> negation;
	rule<Expression *(Scope &)> brace;
	rule<Expression *(Scope &)> bool_var_ref;
	rule<Expression *(Scope &), locals<Scope *>> quantification;
	rule<QuantificationOperator()> quantification_op;
	rule<BooleanOperator()> bool_op;
	ComparisonParser comparison;
};


} // namespace parser
} // namespace gologpp


#endif // GOLOGPP_PARSER_FORMULA_H_

