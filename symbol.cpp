#include "symbol.hpp"

#include <ostream>

bool symbol::operator< (symbol const & other) const noexcept {
    return coordinal_ < other.coordinal_;
}

bool symbol::operator== (symbol const & other) const noexcept {
    return coordinal_ == other.coordinal_;
}

bool symbol::operator!= (symbol const & other) const noexcept {
    return !operator== (other);
}

bool symbol::should_create_plt_entry () {
    bool expected = false;
    return has_plt_.compare_exchange_strong (expected, true);
}

std::ostream & symbol::write (std::ostream & os) const {
    return os << "name:" << name_ << " (" << std::get<0> (coordinal_) << ','
              << std::get<1> (coordinal_) << ')';
}

std::ostream & operator<< (std::ostream & os, symbol const & s) {
    return s.write (os);
}
