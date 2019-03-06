#include "field_access.h"
#include "reference.h"
#include "utilities.h"

#include <model/action.h>
#include <model/procedural.h>
#include <model/fluent.h>

#include <boost/spirit/home/qi/nonterminal/error_handler.hpp>
#include <boost/spirit/include/qi_alternative.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/qi_list.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_char.hpp>

#include <boost/phoenix/object/dynamic_cast.hpp>
#include <boost/phoenix/object/delete.hpp>
#include <boost/phoenix/object/new.hpp>
#include <boost/phoenix/operator/self.hpp>
#include <boost/phoenix/operator/logical.hpp>
#include <boost/phoenix/statement/if.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>
#include <boost/phoenix/bind/bind_function.hpp>

#include <model/reference.h>

namespace gologpp {
namespace parser {


template<class GologT>
ReferenceParser<GologT>::ReferenceParser()
: ReferenceParser::base_type(pred_ref, string("reference_to_") + typeid(GologT).name())
{
	pred_ref = (((r_name() >> "(") > -(
		any_expr(_r1) %  ","
	) ) > ")") [
		_val = new_<Reference<GologT>>(_1, _2),
		if_(!phoenix::bind(&Reference<GologT>::consistent, *_val)) [
			_pass = false,
			delete_(_val)
		]
	];
	pred_ref.name(string("reference_to_") + typeid(GologT).name());

	any_expr = boolean_expr(_r1)
		| numeric_expr(_r1)
		| string_expr(_r1)
		| symbolic_expr(_r1)
		| compound_expr(_r1)
	;
	any_expr.name("any_value_expression");


	BOOST_SPIRIT_DEBUG_NODES((pred_ref)(any_expr));
}


template struct ReferenceParser<Action>;
template struct ReferenceParser<ExogAction>;
template struct ReferenceParser<Procedure>;

#define GOLOGPP_INSTANTIATE_WITH_EXPR_T(_, seq) \
	template \
	struct ReferenceParser < \
		BOOST_PP_SEQ_ELEM(0, seq) < \
			BOOST_PP_SEQ_ELEM(1, seq) \
		> \
	>;

BOOST_PP_SEQ_FOR_EACH_PRODUCT(
	GOLOGPP_INSTANTIATE_WITH_EXPR_T,
	((Function)(Fluent)) (GOLOGPP_VALUE_TYPES)
)






} // namespace parser
} // namespace gologpp
