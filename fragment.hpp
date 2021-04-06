#ifndef FRAGMENT_HPP
#define FRAGMENT_HPP

#include <initializer_list>
#include <string>
#include <vector>

//*   __                             _    *
//*  / _|_ _ __ _ __ _ _ __  ___ _ _| |_  *
//* |  _| '_/ _` / _` | '  \/ -_) ' \  _| *
//* |_| |_| \__,_\__, |_|_|_\___|_||_\__| *
//*              |___/                    *
class fragment {
public:
    explicit fragment (std::initializer_list<std::string> xrefs)
            : xrefs_{xrefs} {}

    std::vector<std::string> const & xrefs () const { return xrefs_; }

private:
    // A collection of the names that this "fragment" references via the PLT.
    std::vector<std::string> xrefs_;
};

#endif // FRAGMENT_HPP
