/*
 *  Created by Phil on 10/02/2016.
 *  Copyright 2016 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef TWOBLUECUBES_CLARA_COMPILERS_H_INCLUDED
#define TWOBLUECUBES_CLARA_COMPILERS_H_INCLUDED

// Detect a number of compiler features - mostly C++11/14 conformance - by compiler
// The following features are defined:
//
// CLARA_CONFIG_CPP11_NULLPTR : is nullptr supported?
// CLARA_CONFIG_CPP11_NOEXCEPT : is noexcept supported?
// CLARA_CONFIG_CPP11_GENERATED_METHODS : The delete and default keywords for compiler generated methods
// CLARA_CONFIG_CPP11_OVERRIDE : is override supported?
// CLARA_CONFIG_CPP11_UNIQUE_PTR : is unique_ptr supported (otherwise use auto_ptr)

// CLARA_CONFIG_CPP11_OR_GREATER : Is C++11 supported?

// CLARA_CONFIG_VARIADIC_MACROS : are variadic macros supported?

// In general each macro has a _NO_<feature name> form
// (e.g. CLARA_CONFIG_CPP11_NO_NULLPTR) which disables the feature.
// Many features, at point of detection, define an _INTERNAL_ macro, so they
// can be combined, en-mass, with the _NO_ forms later.

// All the C++11 features can be disabled with CLARA_CONFIG_NO_CPP11

#ifdef __clang__

#  if __has_feature(cxx_nullptr)
#    define CLARA_INTERNAL_CONFIG_CPP11_NULLPTR
#  endif

#  if __has_feature(cxx_noexcept)
#    define CLARA_INTERNAL_CONFIG_CPP11_NOEXCEPT
#  endif

#endif // __clang__

////////////////////////////////////////////////////////////////////////////////
// GCC
#ifdef __GNUC__

#if __GNUC__ == 4 && __GNUC_MINOR__ >= 6 && defined(__GXX_EXPERIMENTAL_CXX0X__)
#   define CLARA_INTERNAL_CONFIG_CPP11_NULLPTR
#endif

// - otherwise more recent versions define __cplusplus >= 201103L
// and will get picked up below

#endif // __GNUC__

////////////////////////////////////////////////////////////////////////////////
// Visual C++
#ifdef _MSC_VER

#if (_MSC_VER >= 1600)
#   define CLARA_INTERNAL_CONFIG_CPP11_NULLPTR
#   define CLARA_INTERNAL_CONFIG_CPP11_UNIQUE_PTR
#endif

#if (_MSC_VER >= 1900 ) // (VC++ 13 (VS2015))
#define CLARA_INTERNAL_CONFIG_CPP11_NOEXCEPT
#define CLARA_INTERNAL_CONFIG_CPP11_GENERATED_METHODS
#endif

#endif // _MSC_VER


////////////////////////////////////////////////////////////////////////////////
// C++ language feature support

// catch all support for C++11
#if defined(__cplusplus) && __cplusplus >= 201103L

#  define CLARA_CPP11_OR_GREATER

#  if !defined(CLARA_INTERNAL_CONFIG_CPP11_NULLPTR)
#    define CLARA_INTERNAL_CONFIG_CPP11_NULLPTR
#  endif

#  ifndef CLARA_INTERNAL_CONFIG_CPP11_NOEXCEPT
#    define CLARA_INTERNAL_CONFIG_CPP11_NOEXCEPT
#  endif

#  ifndef CLARA_INTERNAL_CONFIG_CPP11_GENERATED_METHODS
#    define CLARA_INTERNAL_CONFIG_CPP11_GENERATED_METHODS
#  endif

#  if !defined(CLARA_INTERNAL_CONFIG_CPP11_OVERRIDE)
#    define CLARA_INTERNAL_CONFIG_CPP11_OVERRIDE
#  endif
#  if !defined(CLARA_INTERNAL_CONFIG_CPP11_UNIQUE_PTR)
#    define CLARA_INTERNAL_CONFIG_CPP11_UNIQUE_PTR
#  endif


#endif // __cplusplus >= 201103L

// Now set the actual defines based on the above + anything the user has configured
#if defined(CLARA_INTERNAL_CONFIG_CPP11_NULLPTR) && !defined(CLARA_CONFIG_CPP11_NO_NULLPTR) && !defined(CLARA_CONFIG_CPP11_NULLPTR) && !defined(CLARA_CONFIG_NO_CPP11)
#   define CLARA_CONFIG_CPP11_NULLPTR
#endif
#if defined(CLARA_INTERNAL_CONFIG_CPP11_NOEXCEPT) && !defined(CLARA_CONFIG_CPP11_NO_NOEXCEPT) && !defined(CLARA_CONFIG_CPP11_NOEXCEPT) && !defined(CLARA_CONFIG_NO_CPP11)
#   define CLARA_CONFIG_CPP11_NOEXCEPT
#endif
#if defined(CLARA_INTERNAL_CONFIG_CPP11_GENERATED_METHODS) && !defined(CLARA_CONFIG_CPP11_NO_GENERATED_METHODS) && !defined(CLARA_CONFIG_CPP11_GENERATED_METHODS) && !defined(CLARA_CONFIG_NO_CPP11)
#   define CLARA_CONFIG_CPP11_GENERATED_METHODS
#endif
#if defined(CLARA_INTERNAL_CONFIG_CPP11_OVERRIDE) && !defined(CLARA_CONFIG_NO_OVERRIDE) && !defined(CLARA_CONFIG_CPP11_OVERRIDE) && !defined(CLARA_CONFIG_NO_CPP11)
#   define CLARA_CONFIG_CPP11_OVERRIDE
#endif
#if defined(CLARA_INTERNAL_CONFIG_CPP11_UNIQUE_PTR) && !defined(CLARA_CONFIG_NO_UNIQUE_PTR) && !defined(CLARA_CONFIG_CPP11_UNIQUE_PTR) && !defined(CLARA_CONFIG_NO_CPP11)
#   define CLARA_CONFIG_CPP11_UNIQUE_PTR
#endif


// noexcept support:
#if defined(CLARA_CONFIG_CPP11_NOEXCEPT) && !defined(CLARA_NOEXCEPT)
#  define CLARA_NOEXCEPT noexcept
#  define CLARA_NOEXCEPT_IS(x) noexcept(x)
#else
#  define CLARA_NOEXCEPT throw()
#  define CLARA_NOEXCEPT_IS(x)
#endif

// nullptr support
#ifdef CLARA_CONFIG_CPP11_NULLPTR
#   define CLARA_NULL nullptr
#else
#   define CLARA_NULL NULL
#endif

// override support
#ifdef CLARA_CONFIG_CPP11_OVERRIDE
#   define CLARA_OVERRIDE override
#else
#   define CLARA_OVERRIDE
#endif

// unique_ptr support
#ifdef CLARA_CONFIG_CPP11_UNIQUE_PTR
#   define CLARA_AUTO_PTR( T ) std::unique_ptr<T>
#else
#   define CLARA_AUTO_PTR( T ) std::auto_ptr<T>
#endif

#endif // TWOBLUECUBES_CLARA_COMPILERS_H_INCLUDED

