/*
 *  Created by Phil Nash on 3/3/2014.
 *  Copyright 2014 Two Blue Cubes Ltd
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "catch.hpp"
#include "clara.h"


// Helper to deduce size from array literals and pass on to parser
template<size_t size, typename ConfigT>
std::vector<Clara::Parser::Token> parseInto( Clara::CommandLine<ConfigT>& cli, char const * (&argv)[size], ConfigT& config ) {
    return cli.parseInto( Clara::argsToVector( size, argv ), config );
}


struct TestOpt {
    TestOpt() : number( 0 ), index( 0 ), flag( false ) {}

    std::string processName;
    std::string fileName;
    int number;
    int index;
    bool flag;
    std::string firstPos;
    std::string secondPos;
    std::string unpositional;
    
    void setValidIndex( int i ) {
        if( i < 0 || i > 10 )
            throw std::domain_error( "index must be between 0 and 10" );
        index = i;
    }
};

struct TestOpt2 {
    std::string description;
};

#ifdef CATCH_CONFIG_VARIADIC_MACROS

TEST_CASE( "cmdline" ) {

    using namespace Clara;

    TestOpt config;
    CommandLine<TestOpt> cli;

    cli.bindProcessName( &TestOpt::processName );

    cli["-o"]["--output"]
        .describe( "specifies output file" )
        .bind( &TestOpt::fileName, "filename" );
    cli["-n"]
        .bind( &TestOpt::number, "an integral value" );
    cli["-i"]
        .describe( "An index, which is an integer between 0 and 10, inclusive" )    
        .bind( &TestOpt::setValidIndex, "index" );
    cli["-f"]
        .describe( "A flag" )
        .bind( &TestOpt::flag );
    cli[2]
        .describe( "Second position" )
        .bind( &TestOpt::secondPos, "second arg" );
    cli[_]
        .bind( &TestOpt::unpositional, "Unpositional" );
    cli[1]
        .describe( "First position" )
        .bind( &TestOpt::firstPos, "first arg" );



    SECTION( "process name" ) {
        char const * argv[] = { "test", "-o filename.ext" };
        parseInto( cli, argv, config );
        
        CHECK( config.processName == "test" );
    }
    SECTION( "args" ) {
        char const * argv[] = { "test", "-o", "filename.ext" };
        parseInto( cli, argv, config );
        
        CHECK( config.fileName == "filename.ext" );
    }
    SECTION( "arg separated by spaces" ) {
        char const * argv[] = { "test", "-o filename.ext" };
        parseInto( cli, argv, config );
        
        CHECK( config.fileName == "filename.ext" );
    }
    SECTION( "arg separated by colon" ) {
        const char* argv[] = { "test", "-o:filename.ext" };
        parseInto( cli, argv, config );

        CHECK( config.fileName == "filename.ext" );
    }
    SECTION( "arg separated by =" ) {
        const char* argv[] = { "test", "-o=filename.ext" };
        parseInto( cli, argv, config );

        CHECK( config.fileName == "filename.ext" );
    }
    SECTION( "long opt" ) {
        const char* argv[] = { "test", "--output %stdout" };
        parseInto( cli, argv, config );

        CHECK( config.fileName == "%stdout" );
    }

    SECTION( "a number" ) {
        const char* argv[] = { "test", "-n 42" };
        parseInto( cli, argv, config );

        CHECK( config.number == 42 );
    }
    SECTION( "not a number" ) {
        const char* argv[] = { "test", "-n forty-two" };
        CHECK_THROWS( parseInto( cli, argv, config ) );

        CHECK( config.number == 0 );
    }

    SECTION( "parse" ) {
        const char* argv[] = { "test" };
        const TestOpt other = cli.parse( Clara::argsToVector( 1, argv ) );
    }

    SECTION( "two parsers" ) {

        TestOpt config1;
        TestOpt2 config2;
        Clara::CommandLine<TestOpt2> cli2;

        cli2["-d"]["--description"]
            .describe( "description" )
            .bind( &TestOpt2::description, "some text" );

        const char* argv[] = { "test", "-n 42", "-d some-text" };
        std::vector<Clara::Parser::Token> unusedTokens = parseInto( cli2, argv, config2 );

        CHECK( config2.description == "some-text" );

        REQUIRE_FALSE( unusedTokens.empty() );
        cli.populate( unusedTokens, config1 );
        CHECK( config1.number == 42 );
    }

    SECTION( "methods" ) {

        SECTION( "in range" ) {
            const char* argv[] = { "test", "-i 3" };
            parseInto( cli, argv, config );

            REQUIRE( config.index == 3 );
        }
        SECTION( "out of range" ) {
            const char* argv[] = { "test", "-i 42" };

            REQUIRE_THROWS( parseInto( cli, argv, config ) );
        }
    }
    
    SECTION( "flags" ) {

        SECTION( "set" ) {
            const char* argv[] = { "test", "-f" };
            parseInto( cli, argv, config );

            REQUIRE( config.flag );
        }
        SECTION( "not set" ) {
            const char* argv[] = { "test" };
            parseInto( cli, argv, config );

            REQUIRE( config.flag == false );
        }

#ifdef CLARA_PLATFORM_WINDOWS
        SECTION( "forward slash" ) {
            const char* argv[] = { "test", "/f" };
            parseInto( cli, argv, config );
            
            REQUIRE( config.flag );
        }
#endif
    }
    SECTION( "positional" ) {

        const char* argv[] = { "test", "-f", "1st", "-o", "filename", "2nd", "3rd" };
        parseInto( cli, argv, config );

        REQUIRE( config.firstPos == "1st" );
        REQUIRE( config.secondPos == "2nd" );
        REQUIRE( config.unpositional == "3rd" );
    }
    SECTION( "usage" ) {
        std::cout << cli.usage( "testApp" ) << std::endl;
    }
}

TEST_CASE( "Invalid parsers" )
{
    TestOpt config;
    Clara::CommandLine<TestOpt> cli;
//    cli.bindProcessName( &TestOpt::processName );

    SECTION( "no bind" )
    {
        cli["-o"]
            .describe( "specifies output file" );

        const char* argv[] = { "test", "-o", "filename" };
        REQUIRE_THROWS( parseInto( cli, argv, config ) );
    }
    SECTION( "no options" )
    {
        const char* argv[] = { "test", "-o", "filename" };
        REQUIRE_THROWS( parseInto( cli, argv, config ) );
    }
}

TEST_CASE( "Combinations" )
{
    TestOpt config;
    Clara::CommandLine<TestOpt> cli;

    SECTION( "no proc name" )
    {
        cli["-o"].bind( &TestOpt::fileName, "filename" );

        const char* argv[] = { "test", "-o", "filename" };
        parseInto( cli, argv, config );
    }
    SECTION( "positional only" )
    {
        cli[1].bind( &TestOpt::firstPos, "1st" );
        cli[2].bind( &TestOpt::secondPos, "2nd" );
        const char* argv[] = { "test", "one", "two" };
        parseInto( cli, argv, config );

        CHECK( config.firstPos == "one" );
        CHECK( config.secondPos == "two" );
    }
    SECTION( "floating only" )
    {
        using namespace Clara; // for _
        cli[_].bind( &TestOpt::unpositional, "floating" );
        const char* argv[] = { "test", "one" };
        parseInto( cli, argv, config );

        CHECK( config.unpositional == "one" );
    }
}

#endif
