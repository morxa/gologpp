#ifndef GOLOGPP_UTILITIES_H_
#define GOLOGPP_UTILITIES_H_

#include "gologpp.h"
#include <string>


namespace gologpp {
namespace generic {

class Name {
public:
    Name(const string &name);
    Name(Name &&other);
    Name(const Name &other) = default;
    virtual ~Name() = default;
    
    const string &name() const;
    virtual bool operator == (const Name &other) const;
    virtual size_t hash() const;
    
private:
    const string name_;
};


class Identifier : public Name {
public:
    Identifier(const string &name, arity_t arity);
    Identifier(Identifier &&other);
    Identifier(const Identifier &other) = default;
    
    virtual ~Identifier() override = default;
    
    arity_t arity() const;
    virtual bool operator == (const Identifier &other) const;
    virtual size_t hash() const override;

private:
    const arity_t arity_;
};


} // namespace generic
} // namespace gologpp


#endif // GOLOGPP_UTILITIES_H_
