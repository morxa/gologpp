#include "execution.h"

#include <eclipseclass.h>

#include "action.h"
#include "procedural.h"
#include "fluent.h"
#include "utilities.h"
#include "history.h"

#include <model/action.h>


namespace gologpp {


unique_ptr<ReadylogContext> ReadylogContext::instance_;


void ReadylogContext::init(const eclipse_opts &options, unique_ptr<PlatformBackend> &&exec_backend)
{ instance_ = unique_ptr<ReadylogContext>(new ReadylogContext(options, std::move(exec_backend))); }

void ReadylogContext::shutdown()
{ instance_.reset(); }


ReadylogContext::ReadylogContext(const eclipse_opts &options, unique_ptr<PlatformBackend> &&exec_backend)
: ExecutionContext(std::make_unique<ReadylogSemanticsFactory>(), std::move(exec_backend))
, options_(options)
{
	ec_set_option_ptr(EC_OPTION_ECLIPSEDIR, const_cast<void *>(static_cast<const void *>(ECLIPSE_DIR)));
	std::cout << "Using eclipse-clp in " << ECLIPSE_DIR << std::endl;

	int rv;
	if ((rv = ec_init()))
		throw std::runtime_error("Error " + std::to_string(rv) + " initializing ECLiPSe subsystem");

	ec_start_ = new EC_ref();

	std::cout << "Loading readylog from " << READYLOG_PATH " ..." << std::endl;

	if (options.trace)
		post_goal("set_flag(debug_compile, on)");
	else
		post_goal("set_flag(debug_compile, off)");

	post_goal(::term(EC_functor("compile", 1), EC_atom(READYLOG_PATH)));

	if ((last_rv_ = EC_resume(*ec_start_)) == EC_status::EC_succeed)
		std::cout << "... done." << std::endl;
	else
		throw std::runtime_error("Error " + std::to_string(rv) + " loading readylog interpreter");

	if (options.trace) {
		std::cout << "Enabling ECLiPSe debugging." << std::endl;
		if (options.guitrace) {
			post_goal("lib(remote_tools)");
			post_goal("attach_tools");
		}
		else {
			post_goal("lib(tracer_tty)");
			post_goal("lib(toplevel)");
			post_goal("toplevel_init(tty)");
		}
	}

	//ec_query(EC_atom("toggle_dtdebug"));
	
	ec_query(::term(EC_functor("compile", 1),
		EC_atom(SOURCE_DIR "/src/semantics/readylog/boilerplate.pl")
	));
}


ReadylogContext::~ReadylogContext()
{ ec_cleanup(); }


ReadylogContext &ReadylogContext::instance()
{ return *instance_; }


void ReadylogContext::compile(const Block &block)
{
	// Discard result since this is only called for the toplevel program,
	// which only needs to initialize its internal state.
	ec_write(block.semantics().plterm());

	// Boilerplate stuff
	compile_term(::term(EC_functor("events_list", 1), ::nil()));
	compile_term(::term(EC_functor("param_cycletime", 1), EC_word(99999999)));
}


void ReadylogContext::compile(const AbstractAction &aa)
{
	try {
		const Action &action = dynamic_cast<const Action &>(aa);
		Semantics<Action> &action_impl = action.semantics();
		compile_term(action_impl.durative_action());
		compile_term(action_impl.durative_poss());
		for (EC_word causes_val : action_impl.durative_causes_vals())
			compile_term(causes_val);
		if (action.senses())
			compile_term(action_impl.senses());
			// senses/2 declaration not really needed since golog++ appends
			// the sensing result to the history, anyways
	} catch (std::bad_cast &) {
		const ExogAction &action = dynamic_cast<const ExogAction &>(aa);
		Semantics<ExogAction> &action_impl = action.semantics();
		compile_term(action_impl.exog_action());
		compile_term(action_impl.poss());
		for (EC_word &causes_val : action_impl.causes_vals())
			compile_term(causes_val);
	}
}


void ReadylogContext::compile(const Fluent &fluent)
{
	compile_term(fluent.semantics<Fluent>().prim_fluent());
	for (EC_word &initially : fluent.semantics<Fluent>().initially())
		compile_term(initially);
}


void ReadylogContext::compile(const Function &function)
{ compile_term(function.semantics<Function>().definition()); }


void ReadylogContext::compile_term(const EC_word &term)
{
	if (! ec_query (
		::term(EC_functor("compile_term", 1), term)
	) )
		throw EclipseError("Failed to compile_term/1");
	ec_write(term);
}


void ReadylogContext::postcompile()
{
	if (!ec_query(EC_atom("compile_SSAs")))
		throw Bug("Failed to compile SSAs");
}


void ReadylogContext::ec_write(EC_word t)
{ ec_query(::term(EC_functor("printf", 2), "%Qw%n", ::list(t, ::nil()))); }


string ReadylogContext::to_string(EC_word t)
{
	EC_word S = ::newvar();
	EC_word q = ::term(EC_functor("sprintf", 3),
		S, "%Qw", t
	);
	char *rv;
	if(ec_query(q) && S.is_string(&rv) == EC_succeed)
		return rv;
	else
		throw Bug("Error converting eclipse term to string");
}


void ReadylogContext::ec_cut()
{
	EC_resume();
	if (last_rv_ == EC_status::EC_succeed)
		ec_start_->cut_to();
}


bool ReadylogContext::final(Block &program, History &history)
{
	EC_word final = ::term(EC_functor("final", 2),
		program.semantics().current_program(),
		history.semantics().current_history()
	);
	bool rv = ec_query(final);
	return rv;
}


bool ReadylogContext::trans(Block &program, History &history)
{
	if (!options_.guitrace && options_.toplevel) {
		post_goal("toplevel");
		EC_resume();
	}//*/

	EC_ref h1, e1;

	EC_word trans = ::term(EC_functor("trans", 4),
		program.semantics().current_program(),
		history.semantics().current_history(),
		e1, h1
	);

	EC_ref Ball;
	EC_word catch_trans = ::term(EC_functor("catch", 3),
		trans, Ball, EC_atom("true")
	);

	EC_word q;
	if (options_.trace)
		q = ::term(EC_functor("trace", 1), catch_trans);
	else
		q = catch_trans;


	if (ec_query(q)) {
		if (EC_word(Ball).is_var() != EC_succeed)
			// Caught eclipse exception
			throw EclipseError(this->to_string(Ball));
		else {
			// Successful transition
			program.semantics().set_current_program(e1);
			history.semantics().set_current_history(h1);
			return true;
		}
	} else
		return false;
}


bool ReadylogContext::ec_query(EC_word t)
{
	/* Insanity ensues:
	   Since the EC_ref::cut_to() mechanism seems to be broken, as well as post_goal("!"),
	   which simply has no effect, once/1 seems to be the ONLY way to cut choicepoints from
	   posted goals.

	   More info:
	   http://eclipseclp.org/doc/embedding/embroot081.html
	   https://sourceforge.net/p/eclipse-clp/mailman/message/23203000/
	   http://eclipseclp.org/archive/eclipse-users/0704.html
	*/
	//post_goal(::term(EC_functor("writeln", 1), t));
	post_goal(::term(EC_functor("once", 1), t));
	//post_goal(::term(EC_functor("writeln", 1), t));
	last_rv_ = EC_resume(*ec_start_);
	//std::cout << std::endl;

	return last_rv_ == EC_status::EC_succeed;
}



} // namespace gologpp
