#include "constant.h"

#include <boost/functional/hash.hpp>

namespace gologpp {

AbstractConstant::~AbstractConstant()
{}

size_t AbstractConstant::hash() const
{ return boost::hash_value(representation_); }

const AbstractConstant::LiteralVariant &AbstractConstant::variant() const
{ return representation_; }



template<> template<>
Constant<SymbolicExpression>::Constant(const string &repr)
: AbstractConstant(repr)
{}

template<> template<>
Constant<SymbolicExpression>::Constant(string repr)
: AbstractConstant(repr)
{}

template<> template<>
Constant<BooleanExpression>::Constant(bool repr)
: AbstractConstant(repr)
{}

template<> template<>
Constant<NumericExpression>::Constant(int repr)
: AbstractConstant(repr)
{}

template<> template<>
Constant<NumericExpression>::Constant(double repr)
: AbstractConstant(repr)
{}



}