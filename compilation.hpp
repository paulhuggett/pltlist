#ifndef COMPILATION_HPP
#define COMPILATION_HPP

#include <initializer_list>
#include <list>
#include <string>
#include <unordered_map>

#include "fragment.hpp"
#include "symbol.hpp"

using symbol_table = std::unordered_map<std::string, symbol>;
using symbol_list = std::list<symbol *>;

//*                    _ _      _   _           *
//*  __ ___ _ __  _ __(_) |__ _| |_(_)___ _ _   *
//* / _/ _ \ '  \| '_ \ | / _` |  _| / _ \ ' \  *
//* \__\___/_|_|_| .__/_|_\__,_|\__|_\___/_||_| *
//*              |_|                            *
class compilation {
public:
    void add_definition (symbol_table * symbols, std::string const & name, unsigned input_ordinal,
                         unsigned member_ordinal, std::initializer_list<std::string> xrefs);

    symbol_list scan (symbol_table * const symbols,
                      std::atomic<unsigned> * const plt_entries) const;

private:
    std::vector<fragment> definitions_;
};

#endif // COMPILATION_HPP
