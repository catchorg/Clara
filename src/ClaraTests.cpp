#include "clara.hpp"

#include "catch.hpp"

#include <iostream>

using namespace clara;

namespace Catch {
template<>
struct StringMaker<clara::detail::InternalParseResult> {
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
}

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
                    "usage:\n"
                    "  <executable> [<test name|tags|pattern> ... ] options\n"
                    "\n"
                    "where options are:\n"
                    "  -?, -h, --help                 display usage information\n"
                    "  --rng-seed, -r <time|value>    set a specific seed for random numbers\n"
                    "  -n, --name <name>              the name to use\n"
                    "  -f, --flag                     a flag to set\n"
                    "  -d, --double <number>          just some number\n"
        );
    }
    SECTION( "some args" ) {
        auto result = parser.parse( Args{ "TestApp", "-n", "Bill", "-d:123.45", "-f", "test1", "test2" } );
        CHECK( result );
        CHECK( result.value().type() == ParseResultType::Matched );

        REQUIRE( config.m_name == "Bill" );
        REQUIRE( config.m_value == 123.45 );
        REQUIRE( config.m_tests == std::vector<std::string> { "test1", "test2" } );
        CHECK( showHelp == false );
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
          | Arg( secondPos, "second arg" )
              ( "Second position" );
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
}

TEST_CASE( "usage", "[.]" ) {

    TestOpt config;
    auto cli = config.makeCli();
    std::cout << cli << std::endl;
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

std::string toString( Opt const& opt ) {
    std::ostringstream oss;
    oss << (Parser() | opt);
    return oss.str();
}

TEST_CASE( "Opt value can start with a dash" ) {
    std::string name;
    bool showHelp = false;
    auto parser
            = Help( showHelp )
            | Opt( name, "name" )
                ["-n"]["--name"]
                ( "the name to use" );

    SECTION( "args" ) {
      auto result = parser.parse( Args{ "TestApp", "-n", "-foo" } );
      CHECK( result );
      REQUIRE( name == "-foo" );
    }
    SECTION( "arg separated by =" ) {
      auto result = parser.parse( Args{ "TestApp", "-n=-bar" } );
      CHECK( result );
      REQUIRE( name == "-bar" );
    }
}

TEST_CASE( "different widths" ) {

    std::string s;

    auto shortOpt
        = Opt( s, "short" )
           ["-s"]["--short"]
           ( "not much" );
    auto longHint
        = Opt( s, "A very very long hint that should force the whole line to wrap awkwardly. I hope no-one ever writes anything like thus - but there's always *someone*" )
           ["-x"]
           ("short description");

    auto longDesc
        = Opt( s, "hint")
            ["-y"]
            ( "In this one it's the description field that is really really long. We should be split over several lines, but complete the description before starting to show the next option" );

    auto longOptName
            = Opt( s, "hint")
            ["--this-one-just-has-an-overly-long-option-name-that-should-push-the-left-hand-column-out"]
            ( "short desc" );

    auto longEverything
        = Opt( s, "this is really over the top, but it has to be tested. In this case we have a very long hint (far longer than anyone should ever even think of using), which should be enough to wrap just on its own...")
            ["--and-a-ridiculously-long-long-option-name-that-would-be-silly-to-write-but-hey-if-it-can-handle-this-it-can-handle-anything-right"]
            ( "*and* a stupid long description, which seems a bit redundant give all the other verbosity. But some people just love to write. And read. You have to be prepared to do a lot of both for this to be useful.");

    SECTION( "long hint" )
        REQUIRE_NOTHROW( toString( longHint ) == "?" );

    SECTION( "long desc" )
        REQUIRE_NOTHROW( toString( longDesc ) );

    SECTION( "long opt name" )
        REQUIRE_NOTHROW( toString( longOptName ) == "?" );

    SECTION( "long everything" )
        REQUIRE_NOTHROW( toString( longEverything ) == "?" );
}
