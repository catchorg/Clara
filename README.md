Clara
=====

A simple to use command line parser for C++

This is still at quite an early stage and should not be considered mature.
However it is being used quite successully in many projects already.

There is virtually no documentation at the moment, except the following:

Clara is a single-header library. It is implemented in several header files which are then stictched together with a Python script. The stitched header can be found in the `include` directory. The original, unstitched, files are in `srcs`.

To use, just `#include "Clara.h"` - but, in exactly one source file, preceed that with `#define CLARA_CONFIG_MAIN`.

For usage please see the unit tests or look at how it is used in the Catch code-base (catch-lib.net).
