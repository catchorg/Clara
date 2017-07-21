# Clara 1.0

A simple to use, composable, command line parser for C++ 11 and beyond.

Clara is a single-header library.

To use, just `#include "clara.hpp"`

For usage please see the unit tests or look at how it is used in the Catch code-base (catch-lib.net).
Documentation will be coming soon.

Some of the key features:

- A single header file with no external dependencies (except the std library).
- Define your interface once to get parsing, type conversions and usage strings with no redundancy.
- Composable. Each `Opt` or `Arg` is an independent parser. Combine these to produce a composite parser - this can be done in stages across multiple function calls - or even projects.
- Bind parsers directly to variables that will receive the results of the parse - no intermediate dictionaries to worry about.
- Or can also bind parsers to lambdas for more custom handling.
- Deduces types from bound variables or lambdas and performs type conversions (via `ostream <<`), with error handling, behind the scenes.
- Bind parsers to vectors for args that can have multiple values.
- Uses Result types for error propagation, rather than exceptions (doesn't yet build with exceptions disabled, but that will be coming later)
- Models POSIX standards for short and long opt behaviour.


## Old version

If you used the earlier, v0.x, version of Clara please note that this is a complete rewrite which assumes C++11 and has
a different interface (composability was a big step forward). Conversion between v0.x and v1.x is a fairly simple and mechanical task, but is a bit of manual 
work - so don't take this version until you're ready (and, of course, able to use C++11).

I hope you'll find the new interface an improvement - and this will be built on to offer new features moving forwards.
I don't expect to maintain v0.x any further, but it remains on a branch.