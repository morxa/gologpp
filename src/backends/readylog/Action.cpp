#include "Action.h"
#include "Execution.h"
#include "EffectAxiom.h"
#include "Scope.h"
#include <gologpp/Action.h>

#include <eclipseclass.h>
#include <utility>

namespace gologpp {
namespace readylog {

/*Action::Action(generic::Action &&action)
: Translatable<generic::Action, tuple<EC_word, EC_word, EC_word>>(std::move(action))
{}


void Action::init(tuple<EC_word, EC_word, EC_word> &cache)
{
	EC_word *args = new EC_word[arity()];
	EC_word prim_action = ::term(EC_functor("prim_action", 1), ::term(
	                  EC_functor(name().c_str(), arity()), args));

	EC_word poss = dynamic_cast<const Translatable<generic::BooleanExpression, EC_word> &>(precondition()).translate();
	EC_word causes_val = dynamic_cast<const gologpp::readylog::EffectAxiom &>(effect()).translate();

	cache = std::tie(prim_action, poss, causes_val);
}*/

} // namespace readylog


/*void generic::Action::init_impl(unique_ptr<Implementation<Action>> &impl)
{
	if (!impl) {
		impl.reset(new Implementation<generic::Action>(*this));
		precondition_->init();
	}
}*/


Implementation<generic::Action>::Implementation(const generic::Action &a)
: action_(a)
{}


EC_word Implementation<generic::Action>::prim_action()
{
	action_.scope().impl_cast<generic::Scope>().init_vars();
	return ::term(EC_functor("prim_action", 1), term());
}


EC_word Implementation<generic::Action>::poss()
{
	action_.scope().impl_cast<generic::Scope>().init_vars();
	return ::term(
		EC_functor("poss", 2), term(),
		dynamic_cast<ReadylogExpression &>(action_.precondition().implementation()).term()
	);
}


EC_word Implementation<generic::Action>::term()
{
	return ::term(
		EC_functor(action_.name().c_str(), action_.arity()),
		action_.scope().impl_cast<generic::Scope>().variables(action_.args())
	);
}


EC_word Implementation<generic::Action>::causes_val()
{
	action_.scope().impl_cast<generic::Scope>().init_vars();
	return action_.effect().impl_cast<generic::EffectAxiom>().term();
}



} // namespace gologpp
