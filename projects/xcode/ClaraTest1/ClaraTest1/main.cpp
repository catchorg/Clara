#include "clara.h"

#include <iostream>

struct Config
{
    std::string colour;
};

// This is just a really basic test harness for now - really just to verify
// that the header stitching process worked out.
// It will be reworked to more thoroughly demonstrated what Clara can do.

// In the meantime see Catch for a better example of Clara in action

int main(int argc, const char * argv[])
{
    Clara::CommandLine<Config> cli;
    cli.bind( &Config::colour )
        .describe( "specify a colour" )
        .shortOpt( "c")
        .longOpt( "colour" )
        .hint( "widget colour" );

    Config cfg = cli.parseInto( argc, argv );

    std::cout << "Hello, " << cfg.colour << " World!\n";
}

