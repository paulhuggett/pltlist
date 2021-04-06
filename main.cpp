#include <cassert>
#include <future>
#include <iostream>

#include "compilation.hpp"

namespace {

    compilation create_c0 (unsigned const input_ordinal, symbol_table * const symbols) {
        compilation c0;
        auto index = 0U;
        // Define "a" and a fragment which references "b" and "d".
        c0.add_definition (symbols, "a", input_ordinal, index++, {"b", "d"});
        // Define "b" and a fragment which references "d".
        c0.add_definition (symbols, "b", input_ordinal, index++, {"d"});
        // Define "c" and a fragment which references nothing.
        c0.add_definition (symbols, "c", input_ordinal, index++, {});
        return c0;
    }

    compilation create_c1 (unsigned const input_ordinal, symbol_table * const symbols) {
        compilation c1;
        auto index = 0U;
        c1.add_definition (symbols, "d", input_ordinal, index++, {"a", "e"});
        c1.add_definition (symbols, "e", input_ordinal, index++, {});
        return c1;
    }

    void check_result (symbol_list const & global_plt) {
        (void) global_plt;
#ifndef NDEBUG
        std::vector<std::pair<std::string, coordinal>> const expected{
            {"a", std::make_pair (0U, 0U)},
            {"b", std::make_pair (0U, 1U)},
            {"d", std::make_pair (1U, 0U)},
            {"e", std::make_pair (1U, 1U)},
        };
        assert (global_plt.size () == expected.size ());
        auto expected_it = std::begin (expected);
        for (auto const & p : global_plt) {
            assert (std::make_pair (p->name (), p->coord ()) == *expected_it);
            ++expected_it;
        }
#endif
    }

} // end anonymous namespace


int main () {
    symbol_table symbols;

    compilation const c0 = create_c0 (0U, &symbols);
    compilation const c1 = create_c1 (1U, &symbols);

    std::atomic<unsigned> plt_entries{0};

    // Each asynchronous job returns the collection of symbols that it added to the PLT.
    std::future<symbol_list> c0plt =
        std::async ([&] () { return c0.scan (&symbols, &plt_entries); });
    std::future<symbol_list> c1plt =
        std::async ([&] () { return c1.scan (&symbols, &plt_entries); });

    symbol_list global_plt;
    c0plt.wait ();
    global_plt.splice (std::end (global_plt), c0plt.get ());

    c1plt.wait ();
    global_plt.splice (std::end (global_plt), c1plt.get ());

    assert (plt_entries.load () == global_plt.size ());

    // This sort is what provides the guarantee that every run will produce the same results,
    // regardless of the execution order of the threads.
    global_plt.sort ([] (symbol const * const a, symbol const * const b) { return *a < *b; });

    std::ostream & os = std::cout;
    for (symbol * p : global_plt) {
        os << *p << '\n';
    }
    os << '\n';
    check_result (global_plt);
}
