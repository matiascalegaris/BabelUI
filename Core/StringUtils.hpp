#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_
// GNU is supposed to have vsnprintf and vsnwprintf.  But only the newer
  // distributions do.
#include <string>
#include <stdint.h>
#include <vector>

// MACRO: SS_WIN32
// ---------------
//      When this flag is set, we are building code for the Win32 platform and
//      may use Win32 specific functions (such as LoadString).  This gives us
//      a couple of nice extras for the code.
//
//      Obviously, Microsoft's is not the only compiler available for Win32 out
//      there.  So I can't just check to see if _MSC_VER is defined to detect
//      if I'm building on Win32.  So for now, if you use MS Visual C++ or
//      Borland's compiler, I turn this on.  Otherwise you may turn it on
//      yourself, if you prefer

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(_WIN32)
 #define SS_WIN32
#endif

// MACRO: SS_ANSI
// --------------
//      When this macro is defined, the code attempts only to use ANSI/ISO
//      standard library functions to do it's work.  It will NOT attempt to use
//      any Win32 of Visual C++ specific functions -- even if they are
//      available.  You may define this flag yourself to prevent any Win32
//      of VC++ specific functions from being called.

// If we're not on Win32, we MUST use an ANSI build

#ifndef SS_WIN32
    #if !defined(SS_NO_ANSI)
        #define SS_ANSI
    #endif
#endif

// MACRO: SS_ALLOCA
// ----------------
//      Some implementations of the Standard C Library have a non-standard
//      function known as alloca().  This functions allows one to allocate a
//      variable amount of memory on the stack.  It is needed to implement
//      the ASCII/MBCS conversion macros.
//
//      I wanted to find some way to determine automatically if alloca() is
//    available on this platform via compiler flags but that is asking for
//    trouble.  The crude test presented here will likely need fixing on
//    other platforms.  Therefore I'll leave it up to you to fiddle with
//    this test to determine if it exists.  Just make sure SS_ALLOCA is or
//    is not defined as appropriate and you control this feature.

#if defined(_MSC_VER) && !defined(SS_ANSI)
  #define SS_ALLOCA
#endif

// If they have not #included W32Base.h (part of my W32 utility library) then
// we need to define some stuff.  Otherwise, this is all defined there.

#if !defined(W32BASE_H)

  // If they want us to use only standard C++ stuff (no Win32 stuff)

#ifdef SS_ANSI

  // On Win32 we have TCHAR.H so just include it.  This is NOT violating
	  // the spirit of SS_ANSI as we are not calling any Win32 functions here.

#ifdef SS_WIN32

#include <TCHAR.H>
#include <WTYPES.H>
#ifndef STRICT
#define STRICT
#endif

  // ... but on non-Win32 platforms, we must #define the types we need.

#else

typedef const char*    PCSTR;
typedef char*      PSTR;
typedef const wchar_t*  PCWSTR;
typedef wchar_t*    PWSTR;
#ifdef UNICODE
typedef wchar_t    TCHAR;
#else
typedef char    TCHAR;
#endif
typedef wchar_t      OLECHAR;

#endif  // #ifndef _WIN32


// Make sure ASSERT and verify are defined using only ANSI stuff

#ifndef ASSERT
#include <assert.h>
#define ASSERT(f) assert((f))
#endif
#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(x) ASSERT((x))
#else
#define VERIFY(x) x
#endif
#endif

#else // ...else SS_ANSI is NOT defined

#include <TCHAR.H>
#include <WTYPES.H>
#ifndef STRICT
#define STRICT
#endif

// Make sure ASSERT and verify are defined

#ifndef ASSERT
#include <crtdbg.h>
#define ASSERT(f) _ASSERTE((f))
#endif
#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(x) ASSERT((x))
#else
#define VERIFY(x) x
#endif
#endif

#endif // #ifdef SS_ANSI

#ifndef UNUSED
#define UNUSED(x) x
#endif

#endif // #ifndef W32BASE_H

#if defined(__GNUC__)

inline int ssvsprintf(PSTR pA, size_t nCount, PCSTR pFmtA, va_list vl)
{
	return vsnprintf(pA, nCount, pFmtA, vl);
}
inline int ssvsprintf(PWSTR pW, size_t nCount, PCWSTR pFmtW, va_list vl)
{
	return vswprintf(pW, nCount, pFmtW, vl);
}

// Microsofties can use
#elif defined(_MSC_VER) && !defined(SS_ANSI)

inline int  ssvsprintf(PSTR pA, size_t nCount, PCSTR pFmtA, va_list vl)
{
	return _vsnprintf_s(pA, nCount, nCount, pFmtA, vl);
}
inline int  ssvsprintf(PWSTR pW, size_t nCount, PCWSTR pFmtW, va_list vl)
{
	return _vsnwprintf_s(pW, nCount, nCount, pFmtW, vl);
}

#elif defined (SS_DANGEROUS_FORMAT)  // ignore buffer size parameter if needed?

inline int ssvsprintf(PSTR pA, size_t /*nCount*/, PCSTR pFmtA, va_list vl)
{
	return vsprintf(pA, pFmtA, vl);
}

inline int ssvsprintf(PWSTR pW, size_t nCount, PCWSTR pFmtW, va_list vl)
{
	// JMO: Some distributions of the "C" have a version of vswprintf that
		// takes 3 arguments (e.g. Microsoft, Borland, GNU).  Others have a
		// version which takes 4 arguments (an extra "count" argument in the
		// second position.  The best stab I can take at this so far is that if
		// you are NOT running with MS, Borland, or GNU, then I'll assume you
		// have the version that takes 4 arguments.
		//
		// I'm sure that these checks don't catch every platform correctly so if
		// you get compiler errors on one of the lines immediately below, it's
		// probably because your implemntation takes a different number of
		// arguments.  You can comment out the offending line (and use the
		// alternate version) or you can figure out what compiler flag to check
		// and add that preprocessor check in.  Regardless, if you get an error
		// on these lines, I'd sure like to hear from you about it.
		//
		// Thanks to Ronny Schulz for the SGI-specific checks here.

//  #if !defined(__MWERKS__) && !defined(__SUNPRO_CC_COMPAT) && !defined(__SUNPRO_CC)
#if    !defined(_MSC_VER) \
        && !defined (__BORLANDC__) \
        && !defined(__GNUC__) \
        && !defined(__sgi)

	return vswprintf(pW, nCount, pFmtW, vl);

	// suddenly with the current SGI 7.3 compiler there is no such function as
	// vswprintf and the substitute needs explicit casts to compile

#elif defined(__sgi)

	nCount;
	return vsprintf((char *)pW, (char *)pFmtW, vl);

#else

	nCount;
	return vswprintf(pW, pFmtW, vl);

#endif

}

#endif

#endif