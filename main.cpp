#include <atomic>
#include <cassert>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

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
                : name_{std::move (other.name_)}
                , coordinal_{std::move (other.coordinal_)}
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

    bool symbol::operator< (symbol const & other) const noexcept {
        return coordinal_ < other.coordinal_;
    }
    bool symbol::operator== (symbol const & other) const noexcept {
        return coordinal_ == other.coordinal_;
    }
    bool symbol::operator!= (symbol const & other) const noexcept { return !operator== (other); }

    bool symbol::should_create_plt_entry () {
        bool expected = false;
        return has_plt_.compare_exchange_strong (expected, true);
    }

    std::ostream & symbol::write (std::ostream & os) const {
        return os << "name:" << name_ << " (" << std::get<0> (coordinal_) << ','
                  << std::get<1> (coordinal_) << ')';
    }

    std::ostream & operator<< (std::ostream & os, symbol const & s) { return s.write (os); }


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

        void build_xrefs () {}

        template <typename... Args>
        void build_xrefs (std::string x, Args &&... args) {
            xrefs_.emplace_back (std::move (x));
            build_xrefs (args...);
        }
    };

    using symbol_table = std::unordered_map<std::string, symbol>;
    using symbol_list = std::list<symbol *>;

    //*                    _ _      _   _           *
    //*  __ ___ _ __  _ __(_) |__ _| |_(_)___ _ _   *
    //* / _/ _ \ '  \| '_ \ | / _` |  _| / _ \ ' \  *
    //* \__\___/_|_|_| .__/_|_\__,_|\__|_\___/_||_| *
    //*              |_|                            *
    class compilation {
    public:
        void add_definition (symbol_table * symbols, std::string const & name,
                             unsigned input_ordinal, unsigned member_ordinal,
                             std::initializer_list<std::string> xrefs);

        symbol_list scan (symbol_table * const symbols,
                          std::atomic<unsigned> * const plt_entries) const;

    private:
        std::vector<fragment> definitions_;
    };

    // add_definition
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

} // end anonymous namespace


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
    global_plt.sort ();

    std::ostream & os = std::cout;
    for (symbol * p : global_plt) {
        os << *p << '\n';
    }
    os << '\n';
    check_result (global_plt);
}
