#include "string_expression.h"

#include "atoms.h"
#include "formula.h"
#include "arithmetic.h"
#include "expressions.h"
#include "list_access.h"
#include "field_access.h"
#include "reference.h"

#include <model/fluent.h>
#include <model/procedural.h>
#include <model/arithmetic.h>
#include <model/string.h>

#include <boost/spirit/include/qi_alternative.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_expect.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/phoenix/object/new.hpp>
#include <boost/phoenix/object/dynamic_cast.hpp>
#include <boost/phoenix/operator/self.hpp>


namespace gologpp {
namespace parser {


StringExpressionParser::StringExpressionParser()
: StringExpressionParser::base_type(expression, "string_expression")
{
	expression = concatenation(_r1) | unary_expr(_r1);
	expression.name("string_expression");

	unary_expr =
		conversion(_r1)
		| string_literal() | var_ref(_r1)
		| typed_reference<Fluent>()(_r1, StringType::name())
		| typed_reference<Function>()(_r1, StringType::name())
		| mixed_field_access()(_r1, StringType::name())
		| mixed_list_access()(_r1, StringType::name())
	;
	unary_expr.name("unary_string_expression");

	var_ref = var_usage()(_r1, val(StringType::name())) [
		_val = new_<Reference<Variable>>(_1)
	];
	var_ref.name("reference_to_string_variable");

	conversion = ( lit("to_string") > '(' > convertible_expr(_r1) > ')' ) [
		_val = new_<ToString>(_1)
	];

	convertible_expr =
		numeric_expression(_r1)
		| boolean_expression(_r1)
		| symbolic_expression(_r1);
	convertible_expr.name("implicitly_convertible_expression");

	concatenation = (unary_expr(_r1) >> "+" >> expression(_r1)) [
		_val = new_<StringConcatenation>(_1, _2)
	];
	concatenation.name("string_concatenation");

	GOLOGPP_DEBUG_NODES(
		(expression)(unary_expr)(var_ref)(conversion)
		(convertible_expr)(concatenation)(implicit_conversion)
	)
}


} // namespace parser
} // namespace gologpp
