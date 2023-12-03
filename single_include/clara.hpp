// Copyright 2017 Two Blue Cubes Ltd. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See https://github.com/philsquared/Clara for more details

// Clara v1.1.5

#ifndef CLARA_HPP_INCLUDED
#define CLARA_HPP_INCLUDED

#ifndef CLARA_CONFIG_CONSOLE_WIDTH
#define CLARA_CONFIG_CONSOLE_WIDTH 80
#endif

#ifndef CLARA_TEXTFLOW_CONFIG_CONSOLE_WIDTH
#define CLARA_TEXTFLOW_CONFIG_CONSOLE_WIDTH CLARA_CONFIG_CONSOLE_WIDTH
#endif

#ifndef CLARA_CONFIG_OPTIONAL_TYPE
#ifdef __has_include
#if __has_include(<optional>) && __cplusplus >= 201703L
#include <optional>
#define CLARA_CONFIG_OPTIONAL_TYPE std::optional
#endif
#endif
#endif


// ----------- #included from clara_textflow.hpp -----------

// TextFlowCpp
//
// A single-header library for wrapping and laying out basic text, by Phil Nash
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// This project is hosted at https://github.com/philsquared/textflowcpp

#ifndef CLARA_TEXTFLOW_HPP_INCLUDED
#define CLARA_TEXTFLOW_HPP_INCLUDED

#include <cassert>
#include <ostream>
#include <sstream>
#include <vector>

#ifndef CLARA_TEXTFLOW_CONFIG_CONSOLE_WIDTH
#define CLARA_TEXTFLOW_CONFIG_CONSOLE_WIDTH 80
#endif


namespace clara { namespace TextFlow {

    inline auto isWhitespace( char c ) -> bool {
        static std::string chars = " \t\n\r";
        return chars.find( c ) != std::string::npos;
    }
    inline auto isBreakableBefore( char c ) -> bool {
        static std::string chars = "[({<|";
        return chars.find( c ) != std::string::npos;
    }
    inline auto isBreakableAfter( char c ) -> bool {
        static std::string chars = "])}>.,:;*+-=&/\\";
        return chars.find( c ) != std::string::npos;
    }

    class Columns;

    class Column {
        std::vector<std::string> m_strings;
        size_t m_width = CLARA_TEXTFLOW_CONFIG_CONSOLE_WIDTH;
        size_t m_indent = 0;
        size_t m_initialIndent = std::string::npos;

    public:
        class iterator {
            friend Column;

            Column const& m_column;
            size_t m_stringIndex = 0;
            size_t m_pos = 0;

            size_t m_len = 0;
            size_t m_end = 0;
            bool m_suffix = false;

            iterator( Column const& column, size_t stringIndex )
            :   m_column( column ),
                m_stringIndex( stringIndex )
            {}

            auto line() const -> std::string const& { return m_column.m_strings[m_stringIndex]; }

            auto isBoundary( size_t at ) const -> bool {
                assert( at > 0 );
                assert( at <= line().size() );

                return at == line().size() ||
                       ( isWhitespace( line()[at] ) && !isWhitespace( line()[at-1] ) ) ||
                       isBreakableBefore( line()[at] ) ||
                       isBreakableAfter( line()[at-1] );
            }

            void calcLength() {
                assert( m_stringIndex < m_column.m_strings.size() );

                m_suffix = false;
                auto width = m_column.m_width-indent();
                m_end = m_pos;
                while( m_end < line().size() && line()[m_end] != '\n' )
                    ++m_end;

                if( m_end < m_pos + width ) {
                    m_len = m_end - m_pos;
                }
                else {
                    size_t len = width;
                    while (len > 0 && !isBoundary(m_pos + len))
                        --len;
                    while (len > 0 && isWhitespace( line()[m_pos + len - 1] ))
                        --len;

                    if (len > 0) {
                        m_len = len;
                    } else {
                        m_suffix = true;
                        m_len = width - 1;
                    }
                }
            }

            auto indent() const -> size_t {
                auto initial = m_pos == 0 && m_stringIndex == 0 ? m_column.m_initialIndent : std::string::npos;
                return initial == std::string::npos ? m_column.m_indent : initial;
            }

            auto addIndentAndSuffix(std::string const &plain) const -> std::string {
                return std::string( indent(), ' ' ) + (m_suffix ? plain + "-" : plain);
            }

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::string;
            using pointer = value_type*;
            using reference = value_type&;
            using iterator_category = std::forward_iterator_tag;

            explicit iterator( Column const& column ) : m_column( column ) {
                assert( m_column.m_width > m_column.m_indent );
                assert( m_column.m_initialIndent == std::string::npos || m_column.m_width > m_column.m_initialIndent );
                calcLength();
                if( m_len == 0 )
                    m_stringIndex++; // Empty string
            }

            auto operator *() const -> std::string {
                assert( m_stringIndex < m_column.m_strings.size() );
                assert( m_pos <= m_end );
                return addIndentAndSuffix(line().substr(m_pos, m_len));
            }

            auto operator ++() -> iterator& {
                m_pos += m_len;
                if( m_pos < line().size() && line()[m_pos] == '\n' )
                    m_pos += 1;
                else
                    while( m_pos < line().size() && isWhitespace( line()[m_pos] ) )
                        ++m_pos;

                if( m_pos == line().size() ) {
                    m_pos = 0;
                    ++m_stringIndex;
                }
                if( m_stringIndex < m_column.m_strings.size() )
                    calcLength();
                return *this;
            }
            auto operator ++(int) -> iterator {
                iterator prev( *this );
                operator++();
                return prev;
            }

            auto operator ==( iterator const& other ) const -> bool {
                return
                    m_pos == other.m_pos &&
                    m_stringIndex == other.m_stringIndex &&
                    &m_column == &other.m_column;
            }
            auto operator !=( iterator const& other ) const -> bool {
                return !operator==( other );
            }
        };
        using const_iterator = iterator;

        explicit Column( std::string const& text ) { m_strings.push_back( text ); }

        auto width( size_t newWidth ) -> Column& {
            assert( newWidth > 0 );
            m_width = newWidth;
            return *this;
        }
        auto indent( size_t newIndent ) -> Column& {
            m_indent = newIndent;
            return *this;
        }
        auto initialIndent( size_t newIndent ) -> Column& {
            m_initialIndent = newIndent;
            return *this;
        }

        auto width() const -> size_t { return m_width; }
        auto begin() const -> iterator { return iterator( *this ); }
        auto end() const -> iterator { return { *this, m_strings.size() }; }

        inline friend std::ostream& operator << ( std::ostream& os, Column const& col ) {
            bool first = true;
            for( auto line : col ) {
                if( first )
                    first = false;
                else
                    os << "\n";
                os <<  line;
            }
            return os;
        }

        auto operator + ( Column const& other ) -> Columns;

        auto toString() const -> std::string {
            std::ostringstream oss;
            oss << *this;
            return oss.str();
        }
    };

    class Spacer : public Column {

    public:
        explicit Spacer( size_t spaceWidth ) : Column( "" ) {
            width( spaceWidth );
        }
    };

    class Columns {
        std::vector<Column> m_columns;

    public:

        class iterator {
            friend Columns;
            struct EndTag {};

            std::vector<Column> const& m_columns;
            std::vector<Column::iterator> m_iterators;
            size_t m_activeIterators;

            iterator( Columns const& columns, EndTag )
            :   m_columns( columns.m_columns ),
                m_activeIterators( 0 )
            {
                m_iterators.reserve( m_columns.size() );

                for( auto const& col : m_columns )
                    m_iterators.push_back( col.end() );
            }

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::string;
            using pointer = value_type*;
            using reference = value_type&;
            using iterator_category = std::forward_iterator_tag;

            explicit iterator( Columns const& columns )
            :   m_columns( columns.m_columns ),
                m_activeIterators( m_columns.size() )
            {
                m_iterators.reserve( m_columns.size() );

                for( auto const& col : m_columns )
                    m_iterators.push_back( col.begin() );
            }

            auto operator ==( iterator const& other ) const -> bool {
                return m_iterators == other.m_iterators;
            }
            auto operator !=( iterator const& other ) const -> bool {
                return m_iterators != other.m_iterators;
            }
            auto operator *() const -> std::string {
                std::string row, padding;

                for( size_t i = 0; i < m_columns.size(); ++i ) {
                    auto width = m_columns[i].width();
                    if( m_iterators[i] != m_columns[i].end() ) {
                        std::string col = *m_iterators[i];
                        row += padding + col;
                        if( col.size() < width )
                            padding = std::string( width - col.size(), ' ' );
                        else
                            padding = "";
                    }
                    else {
                        padding += std::string( width, ' ' );
                    }
                }
                return row;
            }
            auto operator ++() -> iterator& {
                for( size_t i = 0; i < m_columns.size(); ++i ) {
                    if (m_iterators[i] != m_columns[i].end())
                        ++m_iterators[i];
                }
                return *this;
            }
            auto operator ++(int) -> iterator {
                iterator prev( *this );
                operator++();
                return prev;
            }
        };
        using const_iterator = iterator;

        auto begin() const -> iterator { return iterator( *this ); }
        auto end() const -> iterator { return { *this, iterator::EndTag() }; }

        auto operator += ( Column const& col ) -> Columns& {
            m_columns.push_back( col );
            return *this;
        }
        auto operator + ( Column const& col ) -> Columns {
            Columns combined = *this;
            combined += col;
            return combined;
        }

        inline friend std::ostream& operator << ( std::ostream& os, Columns const& cols ) {

            bool first = true;
            for( auto line : cols ) {
                if( first )
                    first = false;
                else
                    os << "\n";
                os << line;
            }
            return os;
        }

        auto toString() const -> std::string {
            std::ostringstream oss;
            oss << *this;
            return oss.str();
        }
    };

    inline auto Column::operator + ( Column const& other ) -> Columns {
        Columns cols;
        cols += *this;
        cols += other;
        return cols;
    }
}}

#endif // CLARA_TEXTFLOW_HPP_INCLUDED

// ----------- end of #include from clara_textflow.hpp -----------
// ........... back in clara.hpp


#include <cctype>
#include <memory>
#include <set>
#include <utility>
#include <algorithm>

#if !defined(CLARA_PLATFORM_WINDOWS) && ( defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) )
#define CLARA_PLATFORM_WINDOWS
#endif

namespace clara {
namespace detail {

    // Traits for extracting arg and return type of lambdas (for single argument lambdas)
    template<typename L>
    struct UnaryLambdaTraits : UnaryLambdaTraits<decltype( &L::operator() )> {};

    template<typename ClassT, typename ReturnT, typename... Args>
    struct UnaryLambdaTraits<ReturnT( ClassT::* )( Args... ) const> {
        static const bool isValid = false;
    };

    template<typename ClassT, typename ReturnT, typename ArgT>
    struct UnaryLambdaTraits<ReturnT( ClassT::* )( ArgT ) const> {
        static const bool isValid = true;
        using ArgType = typename std::remove_const<typename std::remove_reference<ArgT>::type>::type;
        using ReturnType = ReturnT;
    };

    class TokenStream;

    // Transport for raw args (copied from main args, or supplied via init list for testing)
    class Args {
        friend TokenStream;
        std::string m_exeName;
        std::vector<std::string> m_args;

    public:
        // exeName is especially useful for Win32 programs which get passed the whole commandline
        // (argv returned from `CommandLineToArgvW` doesn't contain the executable path)
        Args( int argc, char const* const* argv, std::string exeName = std::string{} ) {
            bool exeOffArgv = exeName.empty();
            m_exeName = exeOffArgv ? argv[0] : std::move(exeName);
            m_args.assign(argv + (exeOffArgv ? 1 : 0), argv + argc);
        }

        Args( std::initializer_list<std::string> args )
        :   m_exeName( *args.begin() ),
            m_args( args.begin()+1, args.end() )
        {}

        auto exeName() const -> std::string const& {
            return m_exeName;
        }
    };

    // Wraps a token coming from a token stream. These may not directly correspond to strings as a single string
    // may encode an option + its argument if the : or = form is used
    enum class TokenType {
        Option, Argument
    };
    struct Token {
        TokenType type;
        std::string token;
    };

    inline auto isOptPrefix( char c ) -> bool {
        return c == '-'
#ifdef CLARA_PLATFORM_WINDOWS
            || c == '/'
#endif
        ;
    }

    // Abstracts iterators into args as a stream of tokens, with option arguments uniformly handled
    class TokenStream {
        using Iterator = std::vector<std::string>::const_iterator;
        Iterator it;
        Iterator itEnd;
        std::vector<Token> m_tokenBuffer;

        void loadBuffer() {
            m_tokenBuffer.resize( 0 );

            // Note: not skiping empty strings

            if( it != itEnd ) {
                auto const &next = *it;
                if( isOptPrefix( next[0] ) ) {
                    auto delimiterPos = next.find_first_of( " :=" );
                    if( delimiterPos != std::string::npos ) {
                        m_tokenBuffer.push_back( { TokenType::Option, next.substr( 0, delimiterPos ) } );
                        m_tokenBuffer.push_back( { TokenType::Argument, next.substr( delimiterPos + 1 ) } );
                    } else {
                        if( next[1] != '-' && next.size() > 2 ) {
                            std::string opt = "- ";
                            for( std::size_t i = 1; i < next.size(); ++i ) {
                                opt[1] = next[i];
                                m_tokenBuffer.push_back( { TokenType::Option, opt } );
                            }
                        } else {
                            m_tokenBuffer.push_back( { TokenType::Option, next } );
                        }
                    }
                } else {
                    m_tokenBuffer.push_back( { TokenType::Argument, next } );
                }
            }
        }

    public:
        explicit TokenStream( Args const &args ) : TokenStream( args.m_args.begin(), args.m_args.end() ) {}

        TokenStream( Iterator it, Iterator itEnd ) : it( it ), itEnd( itEnd ) {
            loadBuffer();
        }

        explicit operator bool() const {
            return !m_tokenBuffer.empty() || it != itEnd;
        }

        auto count() const -> std::size_t { return m_tokenBuffer.size() + (itEnd - it); }

        auto operator*() const -> Token {
            assert( !m_tokenBuffer.empty() );
            return m_tokenBuffer.front();
        }

        auto operator->() const -> Token const * {
            assert( !m_tokenBuffer.empty() );
            return &m_tokenBuffer.front();
        }

        auto operator++() -> TokenStream & {
            if( m_tokenBuffer.size() >= 2 ) {
                m_tokenBuffer.erase( m_tokenBuffer.begin() );
            } else {
                if( it != itEnd )
                    ++it;
                loadBuffer();
            }
            return *this;
        }
    };


    class ResultBase {
    public:
        enum Type {
            Ok, LogicError, RuntimeError
        };

    protected:
        ResultBase( Type type ) : m_type( type ) {}
        virtual ~ResultBase() = default;

        virtual void enforceOk() const = 0;

        Type m_type;
    };

    template<typename T>
    class ResultValueBase : public ResultBase {
    public:
        auto value() const -> T const & {
            enforceOk();
            return m_value;
        }

    protected:
        ResultValueBase( Type type ) : ResultBase( type ) {}

        ResultValueBase( ResultValueBase const &other ) : ResultBase( other ) {
            if( m_type == ResultBase::Ok )
                new( &m_value ) T( other.m_value );
        }

        ResultValueBase( Type, T const &value ) : ResultBase( Ok ) {
            new( &m_value ) T( value );
        }

        auto operator=( ResultValueBase const &other ) -> ResultValueBase & {
            if( m_type == ResultBase::Ok )
                m_value.~T();
            ResultBase::operator=(other);
            if( m_type == ResultBase::Ok )
                new( &m_value ) T( other.m_value );
            return *this;
        }

        ~ResultValueBase() override {
            if( m_type == Ok )
                m_value.~T();
        }

        union {
            T m_value;
        };
    };

    template<>
    class ResultValueBase<void> : public ResultBase {
    protected:
        using ResultBase::ResultBase;
    };

    template<typename T = void>
    class BasicResult : public ResultValueBase<T> {
    public:
        template<typename U>
        explicit BasicResult( BasicResult<U> const &other )
        :   ResultValueBase<T>( other.type() ),
            m_errorMessage( other.errorMessage() )
        {
            assert( type() != ResultBase::Ok );
        }

        template<typename U>
        static auto ok( U const &value ) -> BasicResult { return { ResultBase::Ok, value }; }
        static auto ok() -> BasicResult { return { ResultBase::Ok }; }
        static auto logicError( std::string const &message ) -> BasicResult { return { ResultBase::LogicError, message }; }
        static auto runtimeError( std::string const &message ) -> BasicResult { return { ResultBase::RuntimeError, message }; }

        explicit operator bool() const { return m_type == ResultBase::Ok; }
        auto type() const -> ResultBase::Type { return m_type; }
        auto errorMessage() const -> std::string const& { return m_errorMessage; }

    protected:
        void enforceOk() const override {

            // Errors shouldn't reach this point, but if they do
            // the actual error message will be in m_errorMessage
            assert( m_type != ResultBase::LogicError );
            assert( m_type != ResultBase::RuntimeError );
            if( m_type != ResultBase::Ok )
                std::abort();
        }

        std::string m_errorMessage; // Only populated if resultType is an error

        BasicResult( ResultBase::Type type, std::string const &message )
        :   ResultValueBase<T>(type),
            m_errorMessage(message)
        {
            assert( m_type != ResultBase::Ok );
        }

        using ResultValueBase<T>::ResultValueBase;
        using ResultBase::m_type;
    };

    enum class ParseResultType {
        Matched, NoMatch, ShortCircuitAll, ShortCircuitSame
    };

    class ParseState {
    public:

        ParseState( ParseResultType type, TokenStream const &remainingTokens )
        : m_type(type),
          m_remainingTokens( remainingTokens )
        {}

        auto type() const -> ParseResultType { return m_type; }
        auto remainingTokens() const -> TokenStream { return m_remainingTokens; }

    private:
        ParseResultType m_type;
        TokenStream m_remainingTokens;
    };

    using Result = BasicResult<void>;
    using ParserResult = BasicResult<ParseResultType>;
    using InternalParseResult = BasicResult<ParseState>;

    struct HelpColumns {
        std::string left;
        std::string right;
    };

    template<typename T>
    inline auto convertInto( std::string const &source, T& target ) -> ParserResult {
        std::stringstream ss;
        ss << source;
        ss >> target;
        if( ss.fail() )
            return ParserResult::runtimeError( "Unable to convert '" + source + "' to destination type" );
        else
            return ParserResult::ok( ParseResultType::Matched );
    }
    inline auto convertInto( std::string const &source, std::string& target ) -> ParserResult {
        target = source;
        return ParserResult::ok( ParseResultType::Matched );
    }
    inline auto convertInto( std::string const &source, bool &target ) -> ParserResult {
        std::string srcLC = source;
        std::transform( srcLC.begin(), srcLC.end(), srcLC.begin(), []( char c ) { return static_cast<char>( std::tolower(c) ); } );
        if (srcLC == "y" || srcLC == "1" || srcLC == "true" || srcLC == "yes" || srcLC == "on")
            target = true;
        else if (srcLC == "n" || srcLC == "0" || srcLC == "false" || srcLC == "no" || srcLC == "off")
            target = false;
        else
            return ParserResult::runtimeError( "Expected a boolean value but did not recognise: '" + source + "'" );
        return ParserResult::ok( ParseResultType::Matched );
    }
#ifdef CLARA_CONFIG_OPTIONAL_TYPE
    template<typename T>
    inline auto convertInto( std::string const &source, CLARA_CONFIG_OPTIONAL_TYPE<T>& target ) -> ParserResult {
        T temp;
        auto result = convertInto( source, temp );
        if( result )
            target = std::move(temp);
        return result;
    }
#endif // CLARA_CONFIG_OPTIONAL_TYPE

    struct NonCopyable {
        NonCopyable() = default;
        NonCopyable( NonCopyable const & ) = delete;
        NonCopyable( NonCopyable && ) = delete;
        NonCopyable &operator=( NonCopyable const & ) = delete;
        NonCopyable &operator=( NonCopyable && ) = delete;
    };

    struct BoundRef : NonCopyable {
        virtual ~BoundRef() = default;
        virtual auto isContainer() const -> bool { return false; }
        virtual auto isFlag() const -> bool { return false; }
    };
    struct BoundValueRefBase : BoundRef {
        virtual auto setValue( std::string const &arg ) -> ParserResult = 0;
    };
    struct BoundFlagRefBase : BoundRef {
        virtual auto setFlag( bool flag ) -> ParserResult = 0;
        virtual auto isFlag() const -> bool { return true; }
    };

    template<typename T>
    struct BoundValueRef : BoundValueRefBase {
        T &m_ref;

        explicit BoundValueRef( T &ref ) : m_ref( ref ) {}

        auto setValue( std::string const &arg ) -> ParserResult override {
            return convertInto( arg, m_ref );
        }
    };

    template<typename T>
    struct BoundValueRef<std::vector<T>> : BoundValueRefBase {
        std::vector<T> &m_ref;

        explicit BoundValueRef( std::vector<T> &ref ) : m_ref( ref ) {}

        auto isContainer() const -> bool override { return true; }

        auto setValue( std::string const &arg ) -> ParserResult override {
            T temp;
            auto result = convertInto( arg, temp );
            if( result )
                m_ref.push_back( temp );
            return result;
        }
    };

    struct BoundFlagRef : BoundFlagRefBase {
        bool &m_ref;

        explicit BoundFlagRef( bool &ref ) : m_ref( ref ) {}

        auto setFlag( bool flag ) -> ParserResult override {
            m_ref = flag;
            return ParserResult::ok( ParseResultType::Matched );
        }
    };

    template<typename ReturnType>
    struct LambdaInvoker {
        static_assert( std::is_same<ReturnType, ParserResult>::value, "Lambda must return void or clara::ParserResult" );

        template<typename L, typename ArgType>
        static auto invoke( L const &lambda, ArgType const &arg ) -> ParserResult {
            return lambda( arg );
        }
    };

    template<>
    struct LambdaInvoker<void> {
        template<typename L, typename ArgType>
        static auto invoke( L const &lambda, ArgType const &arg ) -> ParserResult {
            lambda( arg );
            return ParserResult::ok( ParseResultType::Matched );
        }
    };

    template<typename ArgType, typename L>
    inline auto invokeLambda( L const &lambda, std::string const &arg ) -> ParserResult {
        ArgType temp{};
        auto result = convertInto( arg, temp );
        return !result
           ? result
           : LambdaInvoker<typename UnaryLambdaTraits<L>::ReturnType>::invoke( lambda, temp );
    }


    template<typename L>
    struct BoundLambda : BoundValueRefBase {
        L m_lambda;

        static_assert( UnaryLambdaTraits<L>::isValid, "Supplied lambda must take exactly one argument" );
        explicit BoundLambda( L const &lambda ) : m_lambda( lambda ) {}

        auto setValue( std::string const &arg ) -> ParserResult override {
            return invokeLambda<typename UnaryLambdaTraits<L>::ArgType>( m_lambda, arg );
        }
    };

    template<typename L>
    struct BoundFlagLambda : BoundFlagRefBase {
        L m_lambda;

        static_assert( UnaryLambdaTraits<L>::isValid, "Supplied lambda must take exactly one argument" );
        static_assert( std::is_same<typename UnaryLambdaTraits<L>::ArgType, bool>::value, "flags must be boolean" );

        explicit BoundFlagLambda( L const &lambda ) : m_lambda( lambda ) {}

        auto setFlag( bool flag ) -> ParserResult override {
            return LambdaInvoker<typename UnaryLambdaTraits<L>::ReturnType>::invoke( m_lambda, flag );
        }
    };

    enum class Optionality { Optional, Required };

    struct Parser;

    class ParserBase {
    protected:
        bool m_hidden;

    public:
        ParserBase() : m_hidden( false ) {}
        virtual ~ParserBase() = default;
        virtual auto canParse() const -> bool { return false; }
        virtual auto validateSettings() const -> Result { return Result::ok(); }
        virtual auto validateFinal() const -> Result { return Result::ok(); }
        virtual auto internalParse( std::string const& exeName, TokenStream const &tokens ) const->InternalParseResult = 0;

        auto parse( std::string const& exeName, TokenStream const &tokens ) const -> InternalParseResult {
            auto validationResult = validateSettings();
            if( !validationResult )
                return InternalParseResult( validationResult );

            auto result = internalParse( exeName, tokens );

            // Call this even if parsing failed in order to perform cleanup
            validationResult = validateFinal();
            if( result && result.value().type() != ParseResultType::ShortCircuitAll && !validationResult )
                return InternalParseResult( validationResult );

            return result;
        }

        auto parse( Args const &args ) const -> InternalParseResult {
            return parse( args.exeName(), TokenStream( args ) );
        }
    };

    template<typename DerivedT>
    class ComposableParserImpl : public ParserBase {
    public:
        template<typename T>
        auto operator|( T const &other ) const -> Parser;

		template<typename T>
        auto operator+( T const &other ) const -> Parser;
    };

    // Common code and state for Args and Opts
    template<typename DerivedT>
    class ParserRefImpl : public ComposableParserImpl<DerivedT> {
    protected:
        Optionality m_optionality = Optionality::Optional;
        std::shared_ptr<BoundRef> m_ref;
        std::string m_hint;
        std::string m_description;
        mutable std::size_t m_count;

        explicit ParserRefImpl( std::shared_ptr<BoundRef> const &ref )
            : m_ref( ref ),
              m_count( 0 )
        {}

    public:
        template<typename T>
        ParserRefImpl( T &ref, std::string const &hint )
        :   m_ref( std::make_shared<BoundValueRef<T>>( ref ) ),
            m_hint( hint ),
            m_count( 0 )
        {}

        template<typename LambdaT>
        ParserRefImpl( LambdaT const &ref, std::string const &hint )
        :   m_ref( std::make_shared<BoundLambda<LambdaT>>( ref ) ),
            m_hint( hint ),
            m_count( 0 )
        {}

        auto operator()( std::string const &description ) -> DerivedT & {
            m_description = description;
            return static_cast<DerivedT &>( *this );
        }

        auto optional() -> DerivedT & {
            m_optionality = Optionality::Optional;
            return static_cast<DerivedT &>( *this );
        }

        auto required() -> DerivedT & {
            m_optionality = Optionality::Required;
            return static_cast<DerivedT &>( *this );
        }

        auto hidden() -> DerivedT & {
            this->m_hidden = true;
            return static_cast<DerivedT &>(*this);
        }

        auto isOptional() const -> bool {
            return m_optionality == Optionality::Optional;
        }

        virtual auto cardinality() const -> std::size_t {
            if( m_ref->isContainer() )
                return 0;
            else
                return 1;
        }

        auto validateFinal() const -> Result override {
            if( !isOptional() && count() < 1 )
                return Result::runtimeError( "Missing token: " + hint() );
            m_count = 0;
            return ComposableParserImpl<DerivedT>::validateFinal();
        }

        auto canParse() const -> bool override {
            return (cardinality() == 0 || count() < cardinality());
        }

        auto hint() const -> std::string const& { return m_hint; }

        auto count() const -> std::size_t { return m_count; }
    };

    class ExeName : public ComposableParserImpl<ExeName> {
        std::shared_ptr<std::string> m_name;
        std::shared_ptr<std::string> m_description;
        std::shared_ptr<BoundValueRefBase> m_ref;

        template<typename LambdaT>
        static auto makeRef(LambdaT const &lambda) -> std::shared_ptr<BoundValueRefBase> {
            return std::make_shared<BoundLambda<LambdaT>>( lambda) ;
        }

    public:
        ExeName()
          : m_name( std::make_shared<std::string>( "<executable>" ) ),
            m_description( std::make_shared<std::string>() )
        {}

        explicit ExeName( std::string &ref ) : ExeName() {
            m_ref = std::make_shared<BoundValueRef<std::string>>( ref );
        }

        template<typename LambdaT>
        explicit ExeName( LambdaT const& lambda ) : ExeName() {
            m_ref = std::make_shared<BoundLambda<LambdaT>>( lambda );
        }

        // The exe name is not parsed out of the normal tokens, but is handled specially
        auto internalParse( std::string const&, TokenStream const &tokens ) const -> InternalParseResult override {
            return InternalParseResult::ok( ParseState( ParseResultType::NoMatch, tokens ) );
        }

        auto name() const -> std::string const& { return *m_name; }
        auto description( std::string d ) -> void { *m_description = move(d); }
        auto description() const -> std::string const& { return *m_description; }
        auto set( std::string newName, bool updateRef = true ) -> ParserResult {

            auto lastSlash = newName.find_last_of( "\\/" );
            auto filename = ( lastSlash == std::string::npos )
                    ? move( newName )
                    : newName.substr( lastSlash+1 );

            *m_name = filename;
            if( m_ref && updateRef )
                return m_ref->setValue( filename );
            else
                return ParserResult::ok( ParseResultType::Matched );
        }
    };

    class Arg : public ParserRefImpl<Arg> {
    public:
        using ParserRefImpl<Arg>::ParserRefImpl;

        auto getHelpColumns() const -> std::vector<HelpColumns> {
            if( m_description.empty() || m_hidden )
                return {};
            else {
                std::ostringstream oss;
                oss << "<" << m_hint << ">";
                return { { oss.str(), m_description} };
            }
        }

        auto internalParse( std::string const &, TokenStream const &tokens ) const -> InternalParseResult override {
            auto remainingTokens = tokens;
            auto const &token = *remainingTokens;
            if( token.type != TokenType::Argument )
                return InternalParseResult::ok( ParseState( ParseResultType::NoMatch, remainingTokens ) );

            assert( !m_ref->isFlag() );
            auto valueRef = static_cast<detail::BoundValueRefBase*>( m_ref.get() );

            auto result = valueRef->setValue( remainingTokens->token );
            if( !result )
                return InternalParseResult( result );
            else {
                ++m_count;
                return InternalParseResult::ok( ParseState( ParseResultType::Matched, ++remainingTokens ) );
            }
        }
    };

    inline auto normaliseOpt( std::string const &optName ) -> std::string {
#ifdef CLARA_PLATFORM_WINDOWS
        if( optName[0] == '/' )
            return "-" + optName.substr( 1 );
        else
#endif
            return optName;
    }

    class Opt : public ParserRefImpl<Opt> {
    protected:
        std::vector<std::string> m_optNames;

    public:
        template<typename LambdaT>
        explicit Opt( LambdaT const &ref ) : ParserRefImpl<Opt>( std::make_shared<BoundFlagLambda<LambdaT>>( ref ) ) {}

        explicit Opt( bool &ref ) : ParserRefImpl<Opt>( std::make_shared<BoundFlagRef>( ref ) ) {}

        template<typename LambdaT>
        Opt( LambdaT const &ref, std::string const &hint ) : ParserRefImpl<Opt>( ref, hint ) {}

        template<typename T>
        Opt( T &ref, std::string const &hint ) : ParserRefImpl<Opt>( ref, hint ) {}

        auto operator[]( std::string const &optName ) -> Opt & {
            m_optNames.push_back( optName );
            return *this;
        }

        auto getHelpColumns() const -> std::vector<HelpColumns> {
            if( m_hidden )
                return {};

            std::ostringstream oss;
            bool first = true;
            for( auto const &opt : m_optNames ) {
                if (first)
                    first = false;
                else
                    oss << ", ";
                oss << opt;
            }
            if( !m_hint.empty() )
                oss << " <" << m_hint << ">";
            return { { oss.str(), m_description } };
        }

        auto isMatch( std::string const &optToken ) const -> bool {
            auto normalisedToken = normaliseOpt( optToken );
            for( auto const &name : m_optNames ) {
                if( normaliseOpt( name ) == normalisedToken )
                    return true;
            }
            return false;
        }

        using ParserBase::parse;

        auto internalParse( std::string const &, TokenStream const &tokens ) const -> InternalParseResult override {
            auto remainingTokens = tokens;
            if( remainingTokens && remainingTokens->type == TokenType::Option ) {
                auto const &token = *remainingTokens;
                if( isMatch( token.token ) ) {
                    if( m_ref->isFlag() ) {
                        auto flagRef = static_cast<detail::BoundFlagRefBase*>( m_ref.get() );
                        auto result = flagRef->setFlag( true );
                        if( !result )
                            return InternalParseResult( result );
                        if( result.value() == ParseResultType::ShortCircuitAll )
                            return InternalParseResult::ok( ParseState( result.value(), remainingTokens ) );
                    } else {
                        auto valueRef = static_cast<detail::BoundValueRefBase*>( m_ref.get() );
                        ++remainingTokens;
                        if( !remainingTokens )
                            return InternalParseResult::runtimeError( "Expected argument following " + token.token );
                        auto const &argToken = *remainingTokens;
                        if( argToken.type != TokenType::Argument )
                            return InternalParseResult::runtimeError( "Expected argument following " + token.token );
                        auto result = valueRef->setValue( argToken.token );
                        if( !result )
                            return InternalParseResult( result );
                        if( result.value() == ParseResultType::ShortCircuitAll )
                            return InternalParseResult::ok( ParseState( result.value(), remainingTokens ) );
                    }
                    ++m_count;
                    return InternalParseResult::ok( ParseState( ParseResultType::Matched, ++remainingTokens ) );
                }
            }
            return InternalParseResult::ok( ParseState( ParseResultType::NoMatch, remainingTokens ) );
        }

        auto validateSettings() const -> Result override {
            if( m_optNames.empty() )
                return Result::logicError( "No options supplied to Opt" );
            for( auto const &name : m_optNames ) {
                if( name.empty() )
                    return Result::logicError( "Option name cannot be empty" );
#ifdef CLARA_PLATFORM_WINDOWS
                if( name[0] != '-' && name[0] != '/' )
                    return Result::logicError( "Option name must begin with '-' or '/'" );
#else
                if( name[0] != '-' )
                    return Result::logicError( "Option name must begin with '-'" );
#endif
            }
            return ParserRefImpl<Opt>::validateSettings();
        }
    };

    struct Help : Opt {
        Help( bool &showHelpFlag )
        :   Opt([&]( bool flag ) {
                showHelpFlag = flag;
                return ParserResult::ok( ParseResultType::ShortCircuitAll );
            })
        {
            static_cast<Opt &>( *this )
                    ("display usage information")
                    ["-?"]["-h"]["--help"]
                    .optional();
        }
    };


    struct Parser : ParserBase {

        mutable ExeName m_exeName;
        bool m_isSubcmd = false;
        bool m_alludeInUsage = false;
        std::vector<Parser> m_cmds;
        std::vector<Opt> m_options;
        std::vector<Arg> m_args;

        auto operator|=( ExeName const &exeName ) -> Parser & {
            m_exeName = exeName;
            return *this;
        }

        auto operator|=( Arg const &arg ) -> Parser & {
            m_args.push_back(arg);
            return *this;
        }

        auto operator|=( Opt const &opt ) -> Parser & {
            m_options.push_back(opt);
            return *this;
        }

        auto operator|=( Parser const &other ) -> Parser & {
            m_options.insert(m_options.end(), other.m_options.begin(), other.m_options.end());
            m_args.insert(m_args.end(), other.m_args.begin(), other.m_args.end());
            m_cmds.insert(m_cmds.end(), other.m_cmds.begin(), other.m_cmds.end());
            return *this;
        }

        template<typename T>
        auto operator|( T const &other ) const -> Parser {
            return Parser( *this ) |= other;
        }

        // Forward deprecated interface with '+' instead of '|'
        template<typename T>
        auto operator+=( T const &other ) -> Parser & { return operator|=( other ); }
        template<typename T>
        auto operator+( T const &other ) const -> Parser { return operator|( other ); }

        template<typename Parsers>
        auto getHelpColumns(Parsers const& p) const -> std::vector<HelpColumns> {
            std::vector<HelpColumns> cols;
            for (auto const &o : p) {
                auto childCols = o.getHelpColumns();
                cols.reserve( cols.size() + childCols.size() );
                cols.insert( cols.end(), childCols.begin(), childCols.end() );
            }
            return cols;
        }

        void writeToStream( std::ostream &os ) const {
            // print banner
            if( !m_exeName.description().empty() ) {
                os << m_exeName.description() << std::endl << std::endl;
            }
            if (!m_exeName.name().empty()) {
                auto streamArgsAndOpts = [this, &os]( const Parser& p ) {
                    bool required = true, first = true;
                    for( auto const &arg : p.m_args ) {
                        os << " ";
                        if( arg.isOptional() && required ) {
                            os << "[";
                            required = false;
                        }
                        os << "<" << arg.hint() << ">";
                        if( arg.cardinality() == 0 )
                            os << " ... ";
                    }
                    if( !required )
                        os << "]";
                    if( !p.m_options.empty() )
                        os << " <options>";
                };

                os << "Usage:\n" << "  " << m_exeName.name();
                streamArgsAndOpts( *this );
                if( !m_cmds.empty() ) {
                    os << "\n  " << m_exeName.name();
                    os << " <subcommand>";
                    for( const Parser& sub : m_cmds )
                    {
                        if( sub.m_alludeInUsage )
                        {
                            os << "\n  " << m_exeName.name();
                            os << " " << sub.m_exeName.name();
                            streamArgsAndOpts( sub );
                        }
                    }
                }
                os << "\n";
            }

            auto streamHelpColumns = [&os]( std::vector<HelpColumns> const &rows, const std::string &header ) {
                if( !rows.empty() ) {
                    os << header << std::endl;

                    std::size_t consoleWidth = CLARA_CONFIG_CONSOLE_WIDTH;
                    std::size_t optWidth = 0;
                    for( auto const &cols : rows )
                        optWidth = (std::max)(optWidth, cols.left.size() + 2);

                    optWidth = (std::min)(optWidth, consoleWidth/2);

                    for( auto const &cols : rows ) {
                        auto row =
                            TextFlow::Column( cols.left ).width( optWidth ).indent( 2 ) +
                            TextFlow::Spacer(4) +
                            TextFlow::Column( cols.right ).width( consoleWidth - 7 - optWidth );
                        os << row << std::endl;
                    }
                }
            };

            streamHelpColumns( getHelpColumns( m_args ), "\nWhere arguments are:" );
            streamHelpColumns( getHelpColumns( m_options ), "\nWhere options are:" );

            if( !m_cmds.empty() ) {
                std::vector<HelpColumns> cmdCols;
                {
                    cmdCols.reserve( m_cmds.size() );
                    for( auto const &cmd : m_cmds ) {
                        if( cmd.m_hidden )
                            continue;
                        const std::string &d = cmd.m_exeName.description();
                        // first line
                        cmdCols.push_back({ cmd.m_exeName.name(), d.substr(0, d.find('\n')) });
                    }
                }

                streamHelpColumns(cmdCols, "\nWhere subcommands are:");
            }
        }

        friend auto operator<<( std::ostream &os, Parser const &parser ) -> std::ostream& {
            parser.writeToStream( os );
            return os;
        }

        auto validateSettings() const -> Result override {
            for( auto const &opt : m_options ) {
                auto result = opt.validateSettings();
                if( !result )
                    return result;
            }
            for( auto const &arg : m_args ) {
                auto result = arg.validateSettings();
                if( !result )
                    return result;
            }
            return Result::ok();
        }

        auto validateFinal() const -> Result override {
            for( auto const &opt : m_options ) {
                auto result = opt.validateFinal();
                if( !result )
                    return result;
            }
            for( auto const &arg : m_args ) {
                auto result = arg.validateFinal();
                if( !result )
                    return result;
            }
            return Result::ok();
        }

        auto findCmd( const std::string& cmdName ) const -> Parser const* {
            for( const Parser& cmd : m_cmds ) {
                if( cmd.m_exeName.name() == cmdName )
                    return &cmd;
            }
            return nullptr;
        }

        using ParserBase::parse;

        auto internalParse( std::string const& exeName, TokenStream const &tokens ) const -> InternalParseResult override {
            if( !m_cmds.empty() && tokens ) {
                std::string subCommand = tokens->token;
                if( Parser const *cmd = findCmd( subCommand )) {
                    return cmd->parse( subCommand, tokens );
                }
            }

            struct ParserInfo {
                ParserBase const* parser = nullptr;
            };
            const size_t totalParsers = m_options.size() + m_args.size();
            assert( totalParsers < 512 );
            // ParserInfo parseInfos[totalParsers]; // <-- this is what we really want to do
            ParserBase const* parsers[512];

            {
                size_t i = 0;
                for (auto const &opt : m_options) parsers[i++] = &opt;
                for (auto const &arg : m_args) parsers[i++] = &arg;
            }

            if (m_isSubcmd) {
                if (auto result = m_exeName.set(tokens->token); !result)
                    return InternalParseResult(result);
            }
            else {
                m_exeName.set(exeName);
            }

            auto result = InternalParseResult::ok( m_isSubcmd ?
                ParseState( ParseResultType::Matched, ++TokenStream( tokens ) ) :
                ParseState( ParseResultType::NoMatch, tokens ) );
            while( result.value().remainingTokens() ) {
                bool tokenParsed = false;

                for (auto& parser : parsers) {
                    if( parser->canParse() ) {
                        result = parser->internalParse(exeName, result.value().remainingTokens());
                        if (!result)
                            return result;
                        if (result.value().type() != ParseResultType::NoMatch) {
                            tokenParsed = true;
                            break;
                        }
                    }
                }

                if( result.value().type() == ParseResultType::ShortCircuitAll )
                    return result;
                if( !tokenParsed )
                    return InternalParseResult::runtimeError( "Unrecognised token: " + result.value().remainingTokens()->token );
            }
            return result;
        }
    };

    template<typename DerivedT>
    template<typename T>
    auto ComposableParserImpl<DerivedT>::operator|( T const &other ) const -> Parser {
        return Parser() | static_cast<DerivedT const &>( *this ) | other;
    }

    struct Cmd : public Parser {
        Cmd( std::string& ref, std::string cmdName ) : Parser() {
            ExeName exe{ ref };
            exe.set( move( cmdName ), false );
            m_exeName = exe;
            m_isSubcmd = true;
        }

        template<typename Lambda>
        Cmd( Lambda const& ref, std::string cmdName ) : Parser() {
            ExeName exe{ ref };
            exe.set( move( cmdName ), false );
            m_exeName = exe;
            m_isSubcmd = true;
        }

        auto hidden() -> Cmd& {
            m_hidden = true;
            return *this;
        }

        auto alludeInUsage(bool yes = true) -> Cmd& {
            m_alludeInUsage = yes;
            return *this;
        }

        auto operator()( std::string description ) -> Cmd& {
            m_exeName.description( move( description ) );
            return *this;
        }
    };

    template<typename DerivedT>
    auto operator,( ComposableParserImpl<DerivedT> const &l, Parser const &r ) -> Parser = delete;
    template<typename DerivedT>
    auto operator,( Parser const &l, ComposableParserImpl<DerivedT> const &r ) -> Parser = delete;
    template<typename DerivedT, typename DerivedU>
    auto operator,( ComposableParserImpl<DerivedT> const &l, ComposableParserImpl<DerivedU> const &r ) -> Parser = delete;

    // concatenate parsers as subcommands;
    // precondition: one or both must be subcommand parsers
    inline auto operator,( Parser const &l, Parser const &r ) -> Parser {
        assert( l.m_isSubcmd || r.m_isSubcmd );

        Parser const *p1 = &l, *p2 = &r;
        if ( p1->m_isSubcmd && !p2->m_isSubcmd ) {
            std::swap( p1, p2 );
        }

        Parser p = p1->m_isSubcmd ? Parser{} : Parser{ *p1 };
        if ( p1->m_isSubcmd )
            p.m_cmds.push_back( *p1 );
        p.m_cmds.push_back( *p2 );
        return p;
    }

} // namespace detail


// A Combined parser
using detail::Parser;

using detail::Cmd;

// A parser for options
using detail::Opt;

// A parser for arguments
using detail::Arg;

// Wrapper for argc, argv from main()
using detail::Args;

// Specifies the name of the executable
using detail::ExeName;

// Convenience wrapper for option parser that specifies the help option
using detail::Help;

// enum of result types from a parse
using detail::ParseResultType;

// Result type for parser operation
using detail::ParserResult;


} // namespace clara

#endif // CLARA_HPP_INCLUDED
