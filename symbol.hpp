#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <atomic>
#include <iosfwd>
#include <string>
#include <utility>

// "co-ordinal" is a word I (sort of) made up. Here, it consists of an "input ordinal" (the
// index of the compilation from the user's command-line) and the index of a definition within
// that compilation.
//
// Co-ordinals are used to sort the final list of symbols to ensure that the resulting order is
// consistent regardless of th execution order of the asynchronous tasks.
using coordinal = std::pair<unsigned, unsigned>;

//*                _         _  *
//*  ____  _ _ __ | |__  ___| | *
//* (_-< || | '  \| '_ \/ _ \ | *
//* /__/\_, |_|_|_|_.__/\___/_| *
//*     |__/                    *
class symbol {
public:
    symbol (std::string name, coordinal coordinal) noexcept
            : name_{std::move (name)}
            , coordinal_{std::move (coordinal)}
            , has_plt_{false} {}

    symbol (symbol && other) noexcept
            : name_{other.name_}
            , coordinal_{other.coordinal_}
            , has_plt_{other.has_plt_.load ()} {}

    symbol (symbol const &) = delete;

    ~symbol () noexcept = default;

    symbol & operator= (symbol const &) = delete;
    symbol & operator= (symbol &&) noexcept = delete;

    bool operator< (symbol const & other) const noexcept;
    bool operator== (symbol const & other) const noexcept;
    bool operator!= (symbol const & other) const noexcept;

    std::string const & name () const noexcept { return name_; }
    coordinal const & coord () const noexcept { return coordinal_; }

    // Invoked when a PLT-related fixup targeting this symbol is encountered. The first time the
    // function is called it will return true telling the caller to create a PLT entry;
    // subsequent calls will return false.
    bool should_create_plt_entry ();

    std::ostream & write (std::ostream & os) const;

private:
    std::string const name_;
    coordinal const coordinal_;
    // Has this symbol been assigned a PLT entry?
    std::atomic<bool> has_plt_;
};

std::ostream & operator<< (std::ostream & os, symbol const & s);

#endif // SYMBOL_HPP
