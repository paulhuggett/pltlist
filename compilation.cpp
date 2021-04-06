#include "compilation.hpp"

#include <cassert>

// add definition
// ~~~~~~~~~~~~~~
void compilation::add_definition (symbol_table * symbols, std::string const & name,
                                  unsigned input_ordinal, unsigned member_ordinal,
                                  std::initializer_list<std::string> xrefs) {
    symbols->emplace (name, symbol{name, std::make_pair (input_ordinal, member_ordinal)});
    definitions_.emplace_back (xrefs);
}

// scan
// ~~~~
symbol_list compilation::scan (symbol_table * const symbols,
                               std::atomic<unsigned> * const plt_entries) const {
    symbol_list result;
    for (fragment const & definition : definitions_) {
        for (std::string const & xref : definition.xrefs ()) {
            auto pos = symbols->find (xref);
            assert (pos != symbols->end ());
            symbol & s = pos->second;

            // For the purposes of this demo, _all_ references are assumed to require a PLT
            // entry.
            if (s.should_create_plt_entry ()) {
                plt_entries->fetch_add (1U, std::memory_order_relaxed);
                result.emplace_back (&s);
            }
        }
    }
    return result;
}
