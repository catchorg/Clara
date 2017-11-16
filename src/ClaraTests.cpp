#include <clara.hpp>

#include "catch.hpp"

#include <iostream>

using namespace clara;

template<>
struct Catch::StringMaker<clara::detail::InternalParseResult> {
    static std::string convert( clara::detail::InternalParseResult const& result ) {
        switch( result.type() ) {
            case clara::detail::ResultBase::Ok:
                return "Ok";
            case clara::detail::ResultBase::LogicError:
                return "LogicError '" + result.errorMessage() + "'";
            case clara::detail::ResultBase::RuntimeError:
                return "RuntimeError: '" + result.errorMessage() + "'";
            default:
                return "Unknow type: " + std::to_string( static_cast<int>( result.type() ) );
        }
    }
};

// !TBD
// for Catch:
// error on unrecognised?

// Beyond Catch:
// exceptions or not
// error on unmet requireds
// enum mapping
// sets of values (in addition to vectors)
// arg literals
// --help for option names/ args/ arg literals
// other dependencies/ hierarchical parsers
// Exclusive() parser for choices

TEST_CASE( "single parsers" ) {

    std::string name;
    auto p = Opt(name, "name")
        ["-n"]["--name"]
        ("the name to use");

    REQUIRE( name == "" );

    SECTION( "-n" ) {
        p.parse( Args{ "TestApp", "-n", "Vader" } );
        REQUIRE( name == "Vader");
    }
    SECTION( "--name" ) {
        p.parse( Args{ "TestApp", "--name", "Vader" } );
        REQUIRE( name == "Vader");
    }
    SECTION( "-n:" ) {
        p.parse( Args{ "TestApp", "-n:Vader" } );
        REQUIRE( name == "Vader");
    }
    SECTION( "-n=" ) {
        p.parse( Args{ "TestApp", "-n=Vader" } );
        REQUIRE( name == "Vader");
    }
    SECTION( "no args" ) {
        p.parse( Args{ "TestApp" } );
        REQUIRE( name == "");
    }
    SECTION( "different args" ) {
        p.parse( Args{ "TestApp", "-f" } );
        REQUIRE( name == "");
    }
}

struct Config {
    int m_rngSeed;
    std::string m_name;
    std::vector<std::string> m_tests;
    bool m_flag = false;
    double m_value = 0;
};

TEST_CASE( "Combined parser" ) {
    Config config;

    bool showHelp = false;
    auto parser
            = Help( showHelp )
            | Opt( config.m_rngSeed, "time|value" )
                ["--rng-seed"]["-r"]
                ("set a specific seed for random numbers" )
                .required()
            | Opt( config.m_name, "name" )
                ["-n"]["--name"]
                ( "the name to use" )
            | Opt( config.m_flag )
                ["-f"]["--flag"]
                ( "a flag to set" )
            | Opt( [&]( double value ){ config.m_value = value; }, "number" )
                ["-d"]["--double"]
                ( "just some number" )
            | Arg( config.m_tests, "test name|tags|pattern" )
                ( "which test or tests to use" );

    SECTION( "usage" ) {
        std::ostringstream oss;
        oss << parser;
        auto usage = oss.str();
        REQUIRE(usage ==
                    "Usage:\n"
                    "  <executable> [<test name|tags|pattern> ... ] <options>\n"
                    "\n"
                    "Where arguments are:\n"
                    "  <test name|tags|pattern>    which test or tests to use\n"
                    "\n"
                    "Where options are:\n"
                    "  -?, -h, --help                 display usage information\n"
                    "  --rng-seed, -r <time|value>    set a specific seed for random numbers\n"
                    "  -n, --name <name>              the name to use\n"
                    "  -f, --flag                     a flag to set\n"
                    "  -d, --double <number>          just some number\n"
        );
    }
    SECTION( "some args" ) {
        auto result = parser.parse( Args{ "TestApp", "-r", "42", "-n", "Bill", "-d:123.45", "-f", "test1", "test2" } );
        CHECK( result );
        CHECK( result.value().type() == ParseResultType::Matched );

        REQUIRE( config.m_name == "Bill" );
        REQUIRE( config.m_value == 123.45 );
        REQUIRE( config.m_tests == std::vector<std::string> { "test1", "test2" } );
        CHECK( showHelp == false );
    }
    SECTION( "missing required" ) {
        using namespace Catch::Matchers;

        auto result = parser.parse( Args{ "TestApp", "-n", "Bill", "-d:123.45", "-f", "test1", "test2" } );
        CHECK( !result );
        CHECK_THAT( result.errorMessage(), Contains( "Missing token" ) && Contains( "time|value" ) );
    }
    SECTION( "help" ) {
        auto result = parser.parse( Args{ "TestApp", "-?", "-n:NotSet" } );
        CHECK( result );
        CHECK( result.value().type() == ParseResultType::ShortCircuitAll );
        CHECK( config.m_name == "" ); // We should never have processed -n:NotSet
        CHECK( showHelp == true );
    }
}

struct TestOpt {
    std::string processName;
    std::string fileName;
    int number = 0;
    int index = 0;
    bool flag = false;
    std::string firstPos;
    std::string secondPos;
    std::vector<std::string> unpositional;

    auto makeCli() -> Parser {
        return ExeName( processName )
          | Opt( fileName, "filename" )
              ["-o"]["--output"]
              ( "specifies output file" )
          | Opt( number, "an integral value" )
              ["-n"]
          | Opt( [&]( int i ) {
                    if (i < 0 || i > 10)
                        return ParserResult::runtimeError("index must be between 0 and 10");
                    else {
                        index = i;
                        return ParserResult::ok( ParseResultType::Matched );
                    }
                }, "index" )
              ["-i"]
              ( "An index, which is an integer between 0 and 10, inclusive" )
          | Opt( flag )
              ["-f"]
              ( "A flag" )
          | Arg( firstPos, "first arg" )
              ( "First position" )
          | Arg( secondPos, "second arg" );
    }
};

struct TestOpt2 {
    std::string description;
};

TEST_CASE( "cmdline" ) {

    TestOpt config;
    auto cli = config.makeCli();

    SECTION( "exe name" ) {
        auto result = cli.parse( { "TestApp", "-o", "filename.ext" } );
        CHECK( result );
        CHECK( config.processName == "TestApp" );
    }
    SECTION( "args" ) {
        auto result = cli.parse( { "TestApp", "-o", "filename.ext" } );
        CHECK( result );
        CHECK( config.fileName == "filename.ext" );
    }
    SECTION( "arg separated by colon" ) {
        auto result = cli.parse( { "TestApp", "-o:filename.ext" } );
        CHECK( result );
        CHECK( config.fileName == "filename.ext" );
    }
    SECTION( "arg separated by =" ) {
        auto result = cli.parse( { "TestApp", "-o=filename.ext" } );
        CHECK( result );
        CHECK( config.fileName == "filename.ext" );
    }
    SECTION( "long opt" ) {
        auto result = cli.parse( { "TestApp", "--output", "%stdout" } );
        CHECK( result );
        CHECK( config.fileName == "%stdout" );
    }
    SECTION( "a number" ) {
        auto result = cli.parse( { "TestApp", "-n", "42" } );
        CHECK( result );
        CHECK( config.number == 42 );
    }
    SECTION( "not a number" ) {
        auto result = cli.parse( { "TestApp", "-n", "forty-two" } );
        CHECK( !result );
        CHECK( result.errorMessage() == "Unable to convert 'forty-two' to destination type" );

        CHECK( config.number == 0 );
    }

    SECTION( "methods" ) {

        SECTION( "in range" ) {
            auto result = cli.parse( { "TestApp", "-i", "3" } );
            CHECK( result );

            REQUIRE( config.index == 3 );
        }
        SECTION( "out of range" ) {
            auto result = cli.parse( { "TestApp", "-i", "42" } );
            CHECK( !result );
            CHECK( result.errorMessage() == "index must be between 0 and 10" );
        }
    }

    SECTION( "flags" ) {

        SECTION("set") {
            auto result = cli.parse({ "TestApp", "-f" });
            CHECK( result );

            REQUIRE(config.flag);
        }
        SECTION("not set") {
            auto result = cli.parse({ "TestApp" });
            CHECK( result );
            CHECK( result.value().type() == ParseResultType::NoMatch);

            REQUIRE(config.flag == false);
        }
    }

#ifdef CLARA_PLATFORM_WINDOWS
    SECTION( "forward slash" ) {
        auto result = cli.parse( { "TestApp", "/f" } );
        CHECK(result);

        REQUIRE( config.flag );
    }
#endif

    SECTION( "args" ) {

        auto result = cli.parse( { "TestApp", "-f", "1st", "-o", "filename", "2nd" } );
        CHECK( result );

        REQUIRE( config.firstPos == "1st" );
        REQUIRE( config.secondPos == "2nd" );
    }
    SECTION( "usage" ) {
        std::ostringstream oss;
        oss << cli;
        auto usage = oss.str();
        REQUIRE(usage ==
                    "Usage:\n"
                    "  <executable> [<first arg> <second arg>] <options>\n"
                    "\n"
                    "Where arguments are:\n"
                    "  <first arg>    First position\n"
                    "\n"
                    "Where options are:\n"
                    "  -o, --output <filename>    specifies output file\n"
                    "  -n <an integral value>\n"
                    "  -i <index>                 An index, which is an integer between 0 and 10,\n"
                    "                             inclusive\n"
                    "  -f                         A flag\n"
        );
    }
}

TEST_CASE( "Invalid parsers" )
{
    TestOpt config;

    SECTION( "no options" )
    {
        auto cli = Opt( config.number, "number" );
        auto result = cli.parse( { "TestApp", "-o", "filename" } );
        CHECK( !result );
        CHECK( result.errorMessage() == "No options supplied to Opt" );
    }
}

TEST_CASE( "Multiple flags" ) {
    bool a = false, b = false, c = false;
    auto cli = Opt( a )["-a"] | Opt( b )["-b"] | Opt( c )["-c"];

    SECTION( "separately" ) {
        auto result = cli.parse({ "TestApp", "-a", "-b", "-c" });
        CHECK(result);
        CHECK(a);
        CHECK(b);
        CHECK(c);
    }
    SECTION( "combined" ) {
        auto result = cli.parse({ "TestApp", "-abc" });
        CHECK(result);
        CHECK(a);
        CHECK(b);
        CHECK(c);
    }
}

TEST_CASE( "Unrecognised opts" ) {
    using namespace Catch::Matchers;

    bool a = false;
    Parser cli = Parser() | Opt( a )["-a"];

    auto result = cli.parse( { "TestApp", "-b" } );
    CHECK( !result );
    CHECK_THAT( result.errorMessage(), Contains( "Unrecognised token") && Contains( "-b" ) );
}

TEST_CASE( "Subcommands" ) {
    using namespace Catch::Matchers;

    std::string subcommand, subArg;
    bool showHelp = false, subOpt = false;

    auto cli = (
          // create a full parser
          Parser{} | Help{ showHelp }
        , Cmd{ subcommand, "subcommand" }( "Execute subcommand" )
          | Arg{ subArg, "arg1" }( "Arg1" ).required()
          | Opt{ subOpt }["--opt"]( "Opt" )
        , Cmd{ subcommand, "important" }( "Execute important subcommand" ).alludeInUsage()
          | Arg{ subArg, "arg1" }( "Arg1" ).required()
          | Opt{ subOpt }["--opt"]( "Opt" )
        , Cmd{ subcommand, "internal" }( "Execute another subcommand" ).hidden()
    );
    
    REQUIRE( subcommand == "" );

    SECTION( "subcommand.1" ) {
        auto result = cli.parse( { "TestApp", "subcommand", "a1" } );
        CHECK( result );
        CHECK( result.value().type() == ParseResultType::Matched );
        CHECK( !showHelp );
        CHECK_THAT( subcommand, Equals( "subcommand" ) );
        CHECK_THAT( subArg, Equals( "a1" ) );
        CHECK( !subOpt);
    }
    SECTION( "subcommand.2" ) {
        auto result = cli.parse( { "TestApp", "subcommand", "a1", "--opt" } );
        CHECK( result );
        CHECK( result.value().type() == ParseResultType::Matched );
        CHECK( !showHelp );
        CHECK_THAT( subcommand, Equals( "subcommand" ) );
        CHECK_THAT( subArg, Equals( "a1" ) );
        CHECK( subOpt );
    }
    SECTION( "hidden subcommand" ) {
        auto result = cli.parse( { "TestApp", "internal" } );
        CHECK( result );
        CHECK( result.value().type() == ParseResultType::Matched );
        CHECK( !showHelp );
        CHECK_THAT( subcommand, Equals( "internal" ) );
        CHECK_THAT( subArg, Equals( "" ) );
        CHECK( !subOpt );
    }
    SECTION( "unmatched subcommand" ) {
        auto result = cli.parse( { "TestApp", "xyz" } );
        CHECK( !result );
        CHECK_THAT( result.errorMessage(), Contains( "Unrecognised token" ) && Contains( "xyz" ) );
        CHECK( !showHelp );
        CHECK_THAT( subcommand, Equals( "" ) );
        CHECK_THAT( subArg, Equals( "" ) );
        CHECK( !subOpt );
    }
    SECTION( "app version" ) {
        auto result = cli.parse( { "TestApp", "-h" } );
        CHECK( result );
        CHECK( showHelp );
        CHECK_THAT( subcommand, Equals( "" ) );
        CHECK_THAT( subArg, Equals( "" ) );
        CHECK( !subOpt);
    }
    SECTION( "app usage" ) {
        std::ostringstream oss;
        oss << cli;
        auto usage = oss.str();
        REQUIRE(usage ==
            R"(Usage:
  <executable> <options>
  <executable> <subcommand>
  <executable> important <arg1> <options>

Where options are:
  -?, -h, --help    display usage information

Where subcommands are:
  subcommand    Execute subcommand
  important     Execute important subcommand
)"
        );
    }
    SECTION( "subcommand usage" ) {
        std::ostringstream oss;
        oss << *cli.findCmd( "subcommand" );
        auto usage = oss.str();
        REQUIRE(usage ==
            R"(Execute subcommand

Usage:
  subcommand <arg1> <options>

Where arguments are:
  <arg1>    Arg1

Where options are:
  --opt    Opt
)"
);
    }
}
