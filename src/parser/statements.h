#ifndef GOLOGPP_PARSER_STATEMENTS_H_
#define GOLOGPP_PARSER_STATEMENTS_H_

#include "utilities.h"
#include "reference.h"
#include "assignment.h"
#include "atoms.h"
#include "formula.h"
#include "arithmetic.h"
#include "string_expression.h"
#include "symbolic_expression.h"
#include "compound_expression.h"
#include "list_expression.h"
#include "list_access.h"


namespace gologpp {
namespace parser {



struct StatementParser : grammar<Expression *(Scope &)> {
	StatementParser();

	rule<Expression *(Scope &)> statement;
	rule<Expression *(Scope &)> simple_statement;
	rule<Expression *(Scope &)> compound_statement;
	rule<Block *(Scope &), locals<Scope *>> block;
	rule<Choose *(Scope &), locals<Scope *>> choose;
	rule<Conditional *(Scope &)> conditional;
	rule<Pick *(Scope &), locals<Scope *, shared_ptr<Variable>>> pick;

	rule<Search *(Scope &)> search;
	rule<Solve *(Scope &)> solve;
	rule<Test *(Scope &)> test;
	rule<While *(Scope &)> r_while;
	rule<Concurrent *(Scope &), locals<Scope *>> concurrent;

	rule<ListPop *(Scope &), locals<ListOpEnd>> list_pop;
	rule<ListPush *(Scope &), locals<ListOpEnd, Typename>> list_push;

	rule<Expression *(Scope &)> empty_statement;

	rule<Return *(Scope &)> return_stmt;

	ReferenceParser<Action> action_call;
	rule<DurativeCall *(Scope &), locals<DurativeCall::Hook>> durative_call;

	AssignmentParser<Reference<Fluent>> fluent_assignment;
	AssignmentParser<FieldAccess> field_assignment;
	AssignmentParser<ListAccess> list_element_assignment;

	BooleanExpressionParser boolean_expression;
	NumericExpressionParser numeric_expression;
};



} // namespace parser
} // namespace gologpp


#endif // GOLOGPP_PARSER_STATEMENTS_H_

