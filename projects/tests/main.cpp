#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define CLARA_CONFIG_MAIN
#include "clara.h"

//#include "clara.h"
//
//#include <iostream>
//
//struct Config
//{
//    std::string colour;
//};
//
//// This is just a really basic test harness for now - really just to verify
//// that the header stitching process worked out.
//// It will be reworked to more thoroughly demonstrated what Clara can do.
//
//// In the meantime see Catch for a better example of Clara in action
//
//int main(int argc, const char * argv[])
//{
//    Clara::CommandLine<Config> cli;
////    cli.bind( &Config::colour )
////        .options( "-c", "--colour" )
////        .hint( "widget colour" )
////        .describe( "specify a colour" );
//
////    cli["-c"]["--colour"]
////        .hint( "widget color" )
////        .shortDesc( "specify a colour" )
////        .details( "specifies a colour" )
////        .into( &Config::colour );
////
////    cli.addOpt( "-c", "--colour " )
//    cli["-c"]["--colour"]
//        .hint( "widget colour" )
//        .shortDesc( "specify a colour" )
//        .detail( "specifies a colour" )
//        .into( &Config::colour );
////
////
////    cli += "-c, --colour <widget colour>  specify a colour" >> &Config::colour;
////
////
////    cli += Clara::Opt( "-c", "--colour" )
////            .hint( "widget colour" )
////            .shortDesc( "specify a colour" )
////            .details( "specifies a colour" );
////
////    cli( &Config::colour )["-c"]["--colour"]
////        .hint( "widget colour" )
////        .describe( "specify a colour" );
////
////    cli( &Config::colour )[0]
////        .describe( "specify a colour" );
//
//
//    Config cfg = cli.parseInto( argc, argv );
////    Config cfg = Clara::parse( argc, argv );
//
//    std::cout << "Hello, " << cfg.colour << " World!\n";
//}
//
