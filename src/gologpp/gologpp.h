#ifndef GOLOGPP_H_
#define GOLOGPP_H_

#include <memory>
#include <vector>

namespace gologpp {

typedef unsigned char arity_t;

template<class T>      using shp         = std::shared_ptr<T>;
template<class T>      using shared_ptr  = std::shared_ptr<T>;
template<class T>      using uqp         = std::unique_ptr<T>;
template<class T>      using unique_ptr  = std::unique_ptr<T>;
template<class... Ts>  using tuple       = std::tuple<Ts...>;
template<class T>      using vector      = std::vector<T>;

using string = std::string;


namespace generic {

class AbstractExecutionContext;

class Action;
class EffectAxiom;
class Fluent;
class Scope;

class AnyValue;
class Variable;

class Negation;
class Comparison;
class Conjunction;
class Disjunction;
class Implication;
class ExistentialQuantification;
class UniversalQuantification;

class Block;
class Choose;
class Conditional;
class Assignment;
class Pick;
class Call;
class Search;
class Test;
class While;
class Procedure;

template<class>
class Reference;


template<class action_impl, class fluent_impl, class proc_impl>
struct ImplConfig {
	typedef action_impl action_impl_t;
	typedef fluent_impl fluent_impl_t;
	typedef proc_impl proc_impl_t;
};

} // namespace generic


} // namespace gologpp

#endif // GOLOGPP_H_
