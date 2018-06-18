#include "fluent.h"

namespace gologpp {


AbstractFluent::AbstractFluent(Scope *own_scope, const string &name, const vector<shared_ptr<AbstractVariable>> &args)
: Global(name, args)
, scope_(own_scope)
{}


Scope &AbstractFluent::scope()
{ return *scope_; }


const Scope &AbstractFluent::scope() const
{ return *scope_; }



} // namespace gologpp