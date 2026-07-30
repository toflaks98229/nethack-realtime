/* NetHack 5.0	tradstdc.h	$NHDT-Date: 1781973090 2026/06/20 16:31:30 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.71 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file tradstdc.h
 * @brief Making one body of code compile under compilers of several eras.
 *
 * NetHack is old enough to have been written before the C standard, and it still builds on compilers from before it, alongside compilers from well after. This file is where
 * that is made possible: it works out what the compiler supports and supplies whatever it lacks, so the rest of the game can be written in one dialect.
 *
 * Three problems are solved here, and they are worth distinguishing.
 *
 * The largest is variable argument lists, which the old and new dialects handle in completely different ways. So the game does not use either directly. It declares and reads
 * such functions through macros, and this file defines those macros three different ways -- for the modern facility, for the older one, and for compilers with neither. The
 * third is the interesting one: with no facility at all, the arguments are declared individually and shifted along by hand. Its own comment calls that inherently risky.
 *
 * The second is argument widening. Older compilers promoted small types to whole integers when passing them, and the game's prototypes therefore have to name a different type
 * for such an argument depending on the compiler. That is what the @c _P names are for -- each is a small type or an integer, decided here.
 *
 * The third is compiler attributes: telling the compiler that a function never returns, that a fall-through is intended, that an argument must not be null. These are pure
 * assistance -- every one of them compiles to nothing where it is unavailable -- and the file prefers a standard spelling where there is one and a compiler-specific one
 * otherwise.
 *
 * @note The single language-level value defined near the end is what the rest of the game tests to ask "how modern is this compiler". Deriving it once here means nothing else
 *       has to reason about version symbols.
 * @warning The widening names are not decorative. Declaring a function with the wrong one produces a prototype that disagrees with the definition, and on a compiler that
 *          widens, the argument read will be the wrong size.
 */

/**
 * @file tradstdc.h
 * @brief 하나의 코드 본체를 여러 시대의 컴파일러에서 컴파일되게 만들기.
 *
 * NetHack 은 C 표준보다 앞서 쓰였을 만큼 오래되었고, 지금도 그 표준 이전의 컴파일러에서 빌드되며 훨씬 이후의 컴파일러에서도 함께 빌드된다. 이 파일이 그것을 가능하게 하는 곳이다. 컴파일러가 무엇을 지원하는지 알아내고 없는 것을 제공하므로, 게임의 나머지는
 * 하나의 방언으로 쓰일 수 있다.
 *
 * 여기서 세 문제가 해결되며, 그것들을 구별할 가치가 있다.
 *
 * 가장 큰 것은 가변 인자 목록이며, 옛 방언과 새 방언이 그것을 완전히 다르게 다룬다. 그래서 게임은 어느 쪽도 직접 쓰지 않는다. 그런 함수를 매크로를 통해 선언하고 읽으며, 이 파일이 그 매크로를 세 가지 방식으로 정의한다. 현대적 기능을 위해, 더 예전 것을 위해,
 * 그리고 둘 다 없는 컴파일러를 위해. 세 번째가 흥미로운 것이다. 아무 기능도 없으면 인자들이 개별적으로 선언되고 손으로 밀려 나간다. 그 자신의 주석이 그것을 본질적으로 위험하다고 부른다.
 *
 * 두 번째는 인자 확장이다. 예전 컴파일러는 작은 타입을 전달할 때 온전한 정수로 승격시켰고, 그래서 게임의 프로토타입은 그런 인자에 대해 컴파일러에 따라 다른 타입을 지칭해야 한다. @c _P 이름들이 그것을 위한 것이다. 각각은 작은 타입이거나 정수이며, 여기서
 * 정해진다.
 *
 * 세 번째는 컴파일러 속성이다. 어떤 함수가 결코 반환하지 않는다는 것, 관통이 의도적이라는 것, 어떤 인자가 널이어서는 안 된다는 것을 컴파일러에게 알리는 것. 이들은 순전한 도움이며 -- 그 하나하나가 쓸 수 없는 곳에서는 아무것도로 컴파일된다 -- 이 파일은 표준
 * 표기가 있는 곳에서는 그것을, 아니면 컴파일러별 표기를 택한다.
 *
 * @note 끝 부근에 정의되는 하나의 언어 수준 값이 게임의 나머지가 "이 컴파일러가 얼마나 현대적인가"를 묻기 위해 검사하는 것이다. 여기서 한 번 유도하면 다른 어느 것도 버전 심볼에 대해 따져 볼 필요가 없다.
 * @warning 확장 이름들은 장식이 아니다. 함수를 잘못된 이름으로 선언하면 정의와 어긋나는 프로토타입이 되고, 확장하는 컴파일러에서는 읽히는 인자가 잘못된 크기가 된다.
 */

#ifndef TRADSTDC_H
#define TRADSTDC_H

/**
 * @def void
 * @brief Substitute for the absence of the void type.
 * @warning Redefines a language keyword. Where this takes effect, a function declared as returning nothing returns an integer instead -- which compiles, and means a caller
 *          that assigns the result of such a function is not caught.
 * @note Only for compilers that genuinely lack the type. Nothing in the game is written expecting this.
 */
/**
 * @def void
 * @brief void 타입이 없는 경우를 위한 대체.
 * @warning 언어 키워드를 재정의한다. 이것이 효력을 갖는 곳에서는 아무것도 반환하지 않도록 선언된 함수가 대신 정수를 반환한다. 그것은 컴파일되며, 그런 함수의 결과를 대입하는 호출자가 걸리지 않는다는 뜻이다.
 * @note 그 타입이 정말로 없는 컴파일러를 위한 것만이다. 게임의 어떤 것도 이것을 기대하고 쓰이지 않았다.
 */
#if defined(DUMB) && !defined(NOVOID)
#define NOVOID
#endif

#ifdef NOVOID
#define void int
#endif

/*
 * Borland C provides enough ANSI C compatibility in its Borland C++
 * mode to warrant this.  But it does not set __STDC__ unless it compiles
 * in its ANSI keywords only mode, which prevents use of <dos.h> and
 * far pointer use.
 */
#if (defined(__STDC__) || defined(__TURBOC__)) && !defined(NOTSTDC)
#define NHSTDC
#endif

#if defined(ultrix) && defined(__STDC__) && !defined(__LANGUAGE_C)
/* Ultrix seems to be in a constant state of flux.  This check attempts to
 * set up ansi compatibility if it wasn't set up correctly by the compiler.
 */
#ifdef mips
#define __mips mips
#endif
#ifdef LANGUAGE_C
#define __LANGUAGE_C LANGUAGE_C
#endif
#endif

/*
 * ANSI X3J11 detection.
 * Makes substitutes for compatibility with the old C standard.
 */

/**
 * @name Variable argument lists
 * @brief One way of writing a function with a variable number of arguments, however the compiler supports it.
 *
 * Three dialects, and the game uses none of them directly. It declares such a function with the declaration macros and reads its arguments with the access macros, and this
 * file defines them for whichever facility exists.
 *
 * @note The declaration macros open a brace that the ending macro closes. So a function written with them looks unbalanced when read casually, and the pairing must be kept.
 * @warning The fallback for compilers with no facility at all is genuinely unsafe, and its own comment says so: the arguments are fixed slots shifted along by hand. Its
 *          accompanying caveat lists what breaks it -- any floating point value, and any integer wider than a pointer.
 * @{
 */
/**
 * @name 가변 인자 목록
 * @brief 인자 개수가 정해지지 않은 함수를 쓰는 하나의 방식. 컴파일러가 그것을 어떻게 지원하든.
 *
 * 세 방언이 있고, 게임은 그 중 어느 것도 직접 쓰지 않는다. 그런 함수를 선언 매크로로 선언하고 그 인자를 접근 매크로로 읽으며, 이 파일이 존재하는 기능에 맞춰 그것들을 정의한다.
 *
 * @note 선언 매크로가 중괄호를 열고 끝 매크로가 그것을 닫는다. 그래서 그것들로 쓰인 함수는 무심히 읽으면 짝이 맞지 않아 보이며, 그 짝을 지켜야 한다.
 * @warning 아무 기능도 없는 컴파일러를 위한 대비책은 정말로 안전하지 않으며, 그 자신의 주석이 그렇게 말한다. 인자들이 손으로 밀려 나가는 고정된 칸이다. 딸린 경고가 그것을 깨뜨리는 것을 나열한다. 어떤 부동소수점 값이든, 그리고 포인터보다 넓은 어떤 정수든.
 * @{
 */
/* Decide how to handle variable parameter lists:
 * USE_STDARG means use the ANSI <stdarg.h> facilities (only ANSI compilers
 * should do this, and only if the library supports it).
 * USE_VARARGS means use the <varargs.h> facilities.  Again, this should only
 * be done if the library supports it.  ANSI is *not* required for this.
 * Otherwise, the kludgy old methods are used.
 */

/* #define USE_VARARGS */ /* use <varargs.h> instead of <stdarg.h> */
/* #define USE_OLDARGS */ /* don't use any variable argument facilities */

#if defined(apollo) /* Apollos have stdarg(3) but not stdarg.h */
#define USE_VARARGS
#endif

#if !defined(USE_STDARG) && !defined(USE_VARARGS) && !defined(USE_OLDARGS)
/* the old VARARGS and OLDARGS stuff is still here, but since we're
   requiring C99 these days it's unlikely to be useful */
#define USE_STDARG
#endif

#ifdef NEED_VARARGS /* only define these if necessary */
/*
 * These changed in 3.6.0.  VA_END() provides a hidden
 * closing brace to complement VA_DECL()'s hidden opening brace, so code
 * started with VA_DECL() needs an extra opening brace to complement
 * the explicit final closing brace.  This was done so that the source
 * would look less strange, where VA_DECL() appeared to introduce a
 * function whose opening brace was missing; there are now visible and
 * invisible braces at beginning and end.  Sample usage:
 void foo VA_DECL(int, arg)  --macro expansion has a hidden opening brace
 {  --explicit opening brace (actually introduces a nested block)
 VA_START(bar);
 ...code for foo...
 VA_END();  --expansion provides a closing brace for the nested block
 }  --closing brace, pairs with the hidden one in VA_DECL()
 * Reading the code--or using source browsing tools which match braces--
 * results in seeing a matched set of braces.  Usage of VA_END() is
 * potentially trickier, but nethack uses it in a straightforward manner.
 */

#ifdef USE_STDARG
#include <stdarg.h>
#define VA_DECL(typ1, var1) \
    (typ1 var1, ...)        \
    {                       \
        va_list the_args;
#define VA_DECL2(typ1, var1, typ2, var2) \
    (typ1 var1, typ2 var2, ...)          \
    {                                    \
        va_list the_args;
#define VA_INIT(var1, typ1)
#define VA_NEXT(var1, typ1) (var1 = va_arg(the_args, typ1))
#define VA_ARGS the_args
#define VA_START(x) va_start(the_args, x)
#define VA_END()      \
    va_end(the_args); \
    }
#define VA_PASS1(a1) a1
#if defined(ULTRIX_PROTO) && !defined(_VA_LIST_)
#define _VA_LIST_ /* prevents multiple def in stdio.h */
#endif
#else

#ifdef USE_VARARGS
#include <varargs.h>
#define VA_DECL(typ1, var1) \
    (va_alist) va_dcl       \
    {                       \
        va_list the_args;   \
        typ1 var1;
#define VA_DECL2(typ1, var1, typ2, var2) \
    (va_alist) va_dcl                    \
    {                                    \
        va_list the_args;                \
        typ1 var1;                       \
        typ2 var2;
#define VA_ARGS the_args
#define VA_START(x) va_start(the_args)
#define VA_INIT(var1, typ1) var1 = va_arg(the_args, typ1)
#define VA_NEXT(var1, typ1) (var1 = va_arg(the_args, typ1))
#define VA_END()      \
    va_end(the_args); \
    }
#define VA_PASS1(a1) a1
#else

/*USE_OLDARGS*/
/*
 * CAVEAT:  passing double (including float promoted to double) will
 * almost certainly break this, as would any integer type bigger than
 * sizeof (char *).
 * NetHack avoids floating point, and any configuration able to use
 * 'long long int' or I64P32 or the like should be using USE_STDARG.
 */
#ifndef VA_TYPE
typedef const char *vA;
#define VA_TYPE
#endif
#define VA_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9
#define VA_DECL(typ1, var1)                                             \
    (var1, VA_ARGS) typ1 var1; vA VA_ARGS;                              \
    {
#define VA_DECL2(typ1, var1, typ2, var2)                                \
    (var1, var2, VA_ARGS) typ1 var1; typ2 var2; vA VA_ARGS;             \
    {
#define VA_START(x)
#define VA_INIT(var1, typ1)
/* This is inherently risky, and should only be attempted as a
   very last resort; manipulating arguments which haven't actually
   been passed may or may not cause severe trouble depending on
   the function-calling/argument-passing mechanism being used.

   [nethack's core doesn't use VA_NEXT() so doesn't use VA_SHIFT()
   either, and this definition is just retained for completeness.
   lev_comp does use VA_NEXT(), but it passes all 'argX' arguments.
   Note: as of 5.0.0, lev_comp doesn't exist anymore.]
 */
#define VA_SHIFT()                                                    \
    (arg1 = arg2, arg2 = arg3, arg3 = arg4, arg4 = arg5, arg5 = arg6, \
     arg6 = arg7, arg7 = arg8, arg8 = arg9, arg9 = 0)
#define VA_NEXT(var1, typ1) ((var1 = (typ1) arg1), VA_SHIFT(), var1)
#define VA_END() }
/* needed in pline.c, where full number of arguments is known and expected */
#define VA_PASS1(a1)                                                  \
    (vA) a1, (vA) 0, (vA) 0, (vA) 0, (vA) 0, (vA) 0, (vA) 0, (vA) 0, (vA) 0
#endif
#endif

#endif /* NEED_VARARGS */
/** @} */

/**
 * @name A pointer to anything
 * @brief The game's name for an untyped pointer.
 * @note Two spellings for one idea, and the existing comment explains why: one is always a macro and the other usually a typedef. A platform whose compiler cannot express an
 *       untyped pointer can define the typedef as a character pointer instead, which the guard here allows.
 * @warning So it is not necessarily a void pointer. Code that relies on void-pointer arithmetic rules, or on it being interchangeable with any pointer type without a cast,
 *          may not be portable.
 * @{
 */
/**
 * @name 무엇이든 가리키는 포인터
 * @brief 타입 없는 포인터에 대한 게임의 이름.
 * @note 하나의 발상에 두 표기이며, 기존 주석이 그 이유를 설명한다. 하나는 항상 매크로이고 다른 하나는 보통 typedef 다. 타입 없는 포인터를 표현할 수 없는 컴파일러의 플랫폼은 그 typedef 를 대신 문자 포인터로 정의할 수 있고, 여기의 보호가 그것을 허용한다.
 * @warning 그래서 그것이 반드시 void 포인터는 아니다. void 포인터의 산술 규칙에 의존하거나, 형변환 없이 어떤 포인터 타입과도 바꿔 쓸 수 있다는 데 의존하는 코드는 이식 가능하지 않을 수 있다.
 * @{
 */
/* generic pointer, always a macro; genericptr_t is usually a typedef */
#define genericptr void *
#ifndef genericptr_t
typedef genericptr genericptr_t; /* (void *) or (char *) */
#endif
/** @} */

#ifndef NO_PTR_FMT
/* We actually want to know which systems have an ANSI run-time library
 * to know which support the %p format for printing pointers.
 * Since we require C99 or later, assume the library supports it. */
#define HAS_PTR_FMT
#endif

/**
 * @name Argument widening
 * @brief Whether a prototype should name a small type or the integer it is promoted to.
 *
 * The comment below is the explanation and is worth reading whole. In short: the standard requires a prototype for an old-style definition to name the widened type, but many
 * compilers accept the narrow type and typecheck against it -- which is more useful. So the game names the narrow type where that works and the widened one where it does not,
 * and this section decides which.
 *
 * The existing comment's last clause gives the real motive: writing the prototypes to satisfy the standard everywhere would lose the typechecking, and people kept trying to
 * do it.
 *
 * @note One case is subtler still, and its own comment sets it out: a particular compiler in a particular mode widens unsigned narrow types by the old sign-preserving rules
 *       rather than the standard's value-preserving ones, so one of the names has to be overridden for it specifically.
 * @note With neither widened nor unwidened prototypes selected, all of these vanish -- the argument lists expand to nothing, so there is nothing to name.
 * @{
 */
/**
 * @name 인자 확장
 * @brief 프로토타입이 작은 타입을 지칭해야 하는지, 그것이 승격되는 정수를 지칭해야 하는지.
 *
 * 아래의 주석이 그 설명이며 전체를 읽어 볼 가치가 있다. 요약하면, 표준은 옛 방식 정의에 대한 프로토타입이 확장된 타입을 지칭하기를 요구하지만, 많은 컴파일러가 좁은 타입을 받아들이고 그것에 대해 타입 검사를 한다. 그것이 더 쓸모 있다. 그래서 게임은 그것이 통하는
 * 곳에서는 좁은 타입을 지칭하고 통하지 않는 곳에서는 확장된 것을 지칭하며, 이 절이 어느 쪽인지 정한다.
 *
 * 기존 주석의 마지막 절이 진짜 동기를 밝힌다. 프로토타입을 어디서나 표준을 만족시키도록 쓰면 타입 검사를 잃게 되는데, 사람들이 계속 그렇게 하려고 했다.
 *
 * @note 한 경우는 더 미묘하며 그 자신의 주석이 그것을 밝힌다. 특정 모드의 특정 컴파일러가 부호 없는 좁은 타입을 표준의 값 보존 규칙이 아니라 옛 부호 보존 규칙으로 확장하므로, 이름 중 하나가 그것에 대해서만 따로 덮어써져야 한다.
 * @note 확장도 비확장도 선택되지 않으면 이 이름들 전부가 사라진다. 인자 목록이 아무것도로 펼쳐지므로 지칭할 것이 없다.
 * @{
 */
/*
 * According to ANSI C, prototypes for old-style function definitions like
 *   int func(arg) short arg; { ... }
 * must specify widened arguments (char and short to int, float to double),
 *   int func(int);
 * same as how narrow arguments get passed when there is no prototype info.
 * However, various compilers accept shorter arguments (char, short, etc.)
 * in prototypes and do typechecking with them.  Therefore this mess to
 * allow the better typechecking while also allowing some prototypes for
 * the ANSI compilers so people quit trying to fix the prototypes to match
 * the standard and thus lose the typechecking.
 */
#if defined(MSDOS) && !defined(__GO32__)
#define UNWIDENED_PROTOTYPES
#endif
#if defined(AMIGA) && !defined(AZTEC_50)
#define UNWIDENED_PROTOTYPES
#endif
#if defined(macintosh) && (defined(__SC__) || defined(__MRC__))
#define WIDENED_PROTOTYPES
#endif
#if defined(__MWERKS__) && defined(__BEOS__)
#define UNWIDENED_PROTOTYPES
#endif
#if defined(WIN32)
#define UNWIDENED_PROTOTYPES
#endif

#if defined(ULTRIX_PROTO) && defined(ULTRIX_CC20)
#define UNWIDENED_PROTOTYPES
#endif
#if defined(apollo)
#define UNWIDENED_PROTOTYPES
#endif

#ifndef UNWIDENED_PROTOTYPES
#if defined(NHSTDC) || defined(ULTRIX_PROTO) || defined(THINK_C)
#ifndef WIDENED_PROTOTYPES
#define WIDENED_PROTOTYPES
#endif
#endif
#endif

/* this applies to both VMS and Digital Unix/HP Tru64 */
#ifdef WIDENED_PROTOTYPES
/* ANSI C uses "value preserving rules", where 'unsigned char' and
   'unsigned short' promote to 'int' if signed int is big enough to hold
   all possible values, rather than traditional "sign preserving rules"
   where 'unsigned char' and 'unsigned short' promote to 'unsigned int'.
   However, the ANSI C rules aren't binding on non-ANSI compilers.
   When DEC C (aka Compaq C, then HP C) is in non-standard 'common' mode
   it supports prototypes that expect widened types, but it uses the old
   sign preserving rules for how to widen narrow unsigned types.  (In its
   default 'relaxed' mode, __STDC__ is 1 and uchar widens to 'int'.) */
#if defined(__DECC) && (!defined(__STDC__) || !__STDC__)
#define UCHAR_P unsigned int
#endif
#endif

/* These are used for arguments within VDECL prototype declarations.
 */
#ifdef UNWIDENED_PROTOTYPES
#define CHAR_P char
#define SCHAR_P schar
#define UCHAR_P uchar
#define XCHAR_P coordxy
#define SHORT_P short
#ifndef SKIP_BOOLEAN
#define BOOLEAN_P boolean
#endif
#define ALIGNTYP_P aligntyp
#else
#ifdef WIDENED_PROTOTYPES
#define CHAR_P int
#define SCHAR_P int
#ifndef UCHAR_P
#define UCHAR_P int
#endif
#define XCHAR_P int
#define SHORT_P int
#define BOOLEAN_P int
#define ALIGNTYP_P int
#else
/* Neither widened nor unwidened prototypes.  Argument list expansion
 * by VDECL always empty; all xxx_P vanish so defs aren't needed. */
#endif
#endif
/** @} */

/**
 * @name Pointer types in function-pointer declarations
 * @brief Names for the two commonest structure pointers, for use only in declaring pointers to functions.
 * @note They exist for one compiler that could not handle a structure pointer in a prototype, as the existing comment records, and substituting an untyped pointer kept it
 *       building at the cost of the typechecking.
 * @warning Only for function-pointer declarations, as the comment insists. Using them elsewhere spreads the loss of typechecking to code that does not need it.
 * @{
 */
/**
 * @name 함수 포인터 선언에서의 포인터 타입
 * @brief 가장 흔한 두 구조체 포인터에 붙인 이름. 함수에 대한 포인터를 선언할 때만 쓰기 위한 것.
 * @note 기존 주석이 기록하듯 프로토타입 안의 구조체 포인터를 다룰 수 없던 어느 컴파일러 하나를 위해 존재하며, 타입 없는 포인터로 갈아 넣는 것이 타입 검사를 대가로 빌드를 유지시켰다.
 * @warning 그 주석이 강조하듯 함수 포인터 선언에만 쓴다. 다른 곳에 쓰면 타입 검사의 손실이 그것을 필요로 하지 않는 코드에까지 퍼진다.
 * @{
 */
/* OBJ_P and MONST_P should _only_ be used for declaring function pointers.
 */
#if defined(ULTRIX_PROTO) && !defined(__STDC__)
/* The ultrix 2.0 and 2.1 compilers (on Ultrix 4.0 and 4.2 respectively) can't
 * handle "struct obj *" constructs in prototypes.  Their bugs are different,
 * but both seem to work if we put "void*" in the prototype instead.  This
 * gives us minimal prototype checking but avoids the compiler bugs.
 */
#define OBJ_P void *
#define MONST_P void *
#else
#define OBJ_P struct obj *
#define MONST_P struct monst *
#endif
/** @} */

#if 0
/* The problem below is still the case through 4.0.5F, but the suggested
 * compiler flags in the Makefiles suppress the nasty messages, so we don't
 * need to be quite so drastic.
 */
#if defined(__sgi) && !defined(__GNUC__)
/*
 * As of IRIX 4.0.1, /bin/cc claims to be an ANSI compiler, but it thinks
 * it's impossible for a prototype to match an old-style definition with
 * unwidened argument types.  Thus, we have to turn off all NetHack
 * prototypes, and avoid declaring several system functions, since the system
 * include files have prototypes and the compiler also complains that
 * prototyped and unprototyped declarations don't match.
 */
#undef VDECL
#define VDECL(f, p) f()
#endif
#endif

/* MetaWare High-C defaults to unsigned chars */
/* AIX 3.2 needs this also */
#if defined(__HC__) || defined(_AIX32)
#undef signed
#endif

/*
 * Language
 * Standard
 *
 *          NetHack 5.0 range
 *         /
 *        /
 *   C2y X      NetHack 3.6 and earlier range
 *   C23 X     /
 *   C17 X    X
 *   C11 X    X
 *   C99 X    X
 *   C89      X
 *
 *
 * The NetHack 5.0 source code currently makes use of the following
 * C99 (and above) language features:
 *
 *     commas at the end of enumerator lists
 *     variable declarations in for loop initializers
 *     mixing declarations and code
 *     variadic macros
 *     'long long'
 *
 * The NetHack 5.0 source code adheres to the following greater-than C99
 * language restrictions:
 *
 *     Removal of K&R function definitions
 *     Removal of implicit int
 */

/**
 * @def NH_C
 * @brief Which era of the language this build is being compiled under.
 *
 * One of three values, and it is what the rest of the game tests to ask how modern the compiler is. Working it out once here means no other file has to reason about version
 * symbols or their absence.
 *
 * @note Deliberately coarse. The three levels are the ones the game's code actually distinguishes, and the diagram and lists above set out which language features are relied on
 *       and which are deliberately avoided -- the latter being the more useful list when writing new code.
 * @note An undeterminable standard is treated as the oldest rather than as an error, so an unrecognised compiler gets the most conservative dialect.
 * @warning Not the version symbol's own value. Comparing this against a raw standard version number will not match, since the middle level is named for the standard it means
 *          rather than the value that standard reports.
 */
/**
 * @def NH_C
 * @brief 이 빌드가 언어의 어느 시대에서 컴파일되고 있는지.
 *
 * 세 값 중 하나이며, 게임의 나머지가 컴파일러가 얼마나 현대적인지 묻기 위해 검사하는 것이다. 여기서 한 번 알아내면 다른 어느 파일도 버전 심볼이나 그것의 부재에 대해 따져 볼 필요가 없다.
 *
 * @note 의도적으로 거칠다. 세 단계는 게임의 코드가 실제로 구별하는 것들이며, 위의 도표와 목록이 어떤 언어 기능에 의존하고 어떤 것을 의도적으로 피하는지를 밝힌다. 새 코드를 쓸 때 더 쓸모 있는 것은 뒤의 목록이다.
 * @note 표준을 판별할 수 없는 경우는 오류가 아니라 가장 오래된 것으로 취급된다. 그래서 알 수 없는 컴파일러가 가장 보수적인 방언을 받는다.
 * @warning 버전 심볼 자신의 값이 아니다. 이것을 날 표준 버전 번호와 비교하면 맞지 않는다. 가운데 단계가 그 표준이 보고하는 값이 아니라 그것이 뜻하는 표준의 이름을 따라 붙어 있기 때문이다.
 */
/*
 * Provide a shorthand way of checking for a certain C standard
 * in the NetHack header files by always setting NH_C to one
 * of three possible values (as of January 2025):
 *
 * NH_C >= 202300L     Being compiled under C23 or greater
 * NH_C >= 199900L     Being compiled under C99 or greater
 * NH_C >= 198900L     Being compiled under C89 or greater,
 *                     or C std could not be determined.
 */
#if defined(__STDC_VERSION__)
#if (__STDC_VERSION__ >= 202000L)
#define NH_C 202300L
#else
#define NH_C 199900L
#endif  /* C23 or C99 */
#else   /* __STDC_VERSION not defined */
#define NH_C 198900L
#endif  /* __STDC_VERSION not defined */
#ifndef NH_C
#define NH_C 198900L
#endif

/* NH_C is now defined to 198900L or 199900L or 202300L */

/**
 * @name Compiler attributes
 * @brief Telling the compiler things it cannot work out, so it can check more and warn less.
 *
 * Each of these says something true about a function or a statement: that it never returns, that a fall-through is intended, that a value must not be null, that an unused thing
 * is unused on purpose. The compiler can then diagnose real mistakes and stop reporting deliberate ones.
 *
 * The definitions are tried in order of preference: the standard spelling first, then each compiler's own, then nothing at all. The last is the important part -- every one of
 * these expands to nothing where it is unavailable, so a build on an older compiler loses the checking and nothing else.
 *
 * @note That is why they may be used unconditionally at every declaration. There is no version to test before writing one.
 * @warning They are assertions, not enforcement. Marking a pointer as never null does not check it; it tells the compiler it may assume so, which turns a violation from a
 *          diagnosable mistake into undefined behaviour.
 * @{
 */
/**
 * @name 컴파일러 속성
 * @brief 컴파일러가 알아낼 수 없는 것을 알려 주어, 더 많이 검사하고 덜 경고하게 하기.
 *
 * 이들 각각은 함수나 문장에 대해 참인 무언가를 말한다. 그것이 결코 반환하지 않는다는 것, 관통이 의도적이라는 것, 어떤 값이 널이어서는 안 된다는 것, 쓰이지 않는 것이 일부러 쓰이지 않는다는 것. 그러면 컴파일러가 진짜 잘못을 진단하고 의도적인 것을 보고하기를 그만둘
 * 수 있다.
 *
 * 정의는 선호 순서로 시도된다. 표준 표기가 먼저, 그다음 각 컴파일러 자신의 것, 그다음 아무것도 아닌 것. 마지막이 중요한 부분이다. 이들 하나하나가 쓸 수 없는 곳에서는 아무것도로 펼쳐지므로, 더 예전 컴파일러에서의 빌드는 그 검사를 잃고 다른 것은 잃지 않는다.
 *
 * @note 그래서 이들을 모든 선언에서 조건 없이 쓸 수 있다. 하나를 쓰기 전에 검사할 버전이 없다.
 * @warning 이들은 강제가 아니라 주장이다. 어떤 포인터를 결코 널이 아니라고 표시하는 것이 그것을 검사하지는 않는다. 컴파일러에게 그렇다고 가정해도 된다고 알리는 것이며, 그것은 위반을 진단할 수 있는 잘못에서 정의되지 않은 동작으로 바꾼다.
 * @{
 */
#if NH_C >= 202300L
/* Give first priority to standard */
#ifndef __has_c_attribute
#define __has_c_attribute(x) 0
#endif
/*
 * noreturn
 */
#ifndef ATTRNORETURN
#define ATTRNORETURN [[noreturn]]
/* #warning [[noreturn]] from C23 */
#endif  /* ATTRNORETURN not defined */
/*
 * fallthrough
 */
#if __has_c_attribute(fallthrough)
/* Standard attribute is available, use it. */
#define FALLTHROUGH [[fallthrough]]
/* #warning [[fallthrough]] from C23 */
#endif  /* __has_c_attribute(fallthrough) */
/*
 * maybe_unused
 */
#if __has_c_attribute(maybe_unused)
#ifndef ATTRUNUSED
#define ATTRUNUSED [[maybe_unused]]
#endif
#endif  /* __has_c_attribute(maybe_unused) */
#endif  /* NH_C >= 202300L */

/*
 * Compiler-specific
 */

#ifdef __clang__
/* clang's gcc emulation is sufficient for nethack's usage */
#ifndef __GNUC__
#define __GNUC__ 5 /* high enough for returns_nonnull */
#endif
#endif

/*
 * gcc (and also clang which masquerades as__GNUC__==5 due to #define above)
 *
 * Allow gcc2 and above to check parameters of printf-like calls with
 * -Wformat; append this to a prototype declaration (see pline() in extern.h).
 */
#ifdef __GNUC__
#if (__GNUC__ >= 2) && !defined(USE_OLDARGS)
#define PRINTF_F(f, v) __attribute__((format(printf, f, v)))
#endif
#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define PRINTF_F_PTR(f, v) PRINTF_F(f, v)
#endif
#if __GNUC__ >= 3
#ifndef ATTRUNUSED
#define UNUSED __attribute__((unused))
#endif
#ifndef ATTRNORETURN
#ifndef NORETURN
#define NORETURN __attribute__((noreturn))
/* #warning NORETURN __attribute__((noreturn)) from __GNUC__ >= 3 */
#endif  /* NORETURN */
#endif  /* ATTRNORETURN */
#endif  /* __GNUC__ >= 3 */
#if __GNUC__ >= 5
#ifndef NONNULLS_DEFINED
#define DO_DEFINE_NONNULLS
#endif  /* !NONNULLS_DEFINED */
/* #pragma message is available */
#define NH_PRAGMA_MESSAGE 1
#endif  /* __GNUC__ greater than or equal to 5 */
#if (!defined(__linux__) && !defined(MACOS)) || defined(GCC_URWARN)
/* disable gcc's __attribute__((__warn_unused_result__)) since explicitly
   discarding the result by casting to (void) is not accepted as a 'use' */
#define __warn_unused_result__ /*empty*/
#define warn_unused_result /*empty*/
#endif  /* GCC_URWARN || !__linux || !MACOS */
#endif  /* __GNUC__ || clang masquerading as __GNUC__==5 */

/*
 * clang-specific
 *
 */
#if defined(__clang__)
#ifndef FALLTHROUGH
#if defined(__clang_major__)
#if __clang_major__ >= 9
#define FALLTHROUGH __attribute__((fallthrough))
/* #warning FALLTHROUGH __attribute__((fallthrough)) from clang */
#endif  /* __clang_major__ greater than or equal to 9 */
#endif  /* __clang_major__ is defined */
#endif  /* FALLTHROUGH */
#if !defined(DO_DEFINE_NONNULLS)
#define DO_DEFINE_NONNULLS
#endif
#endif  /* __clang__ */

/*
 * NONNULL args
 */
#if defined(DO_DEFINE_NONNULLS) && !defined(NONNULLS_DEFINED)
#define NONNULL __attribute__((returns_nonnull))
#define NONNULLPTRS __attribute__((nonnull))
#define NONNULLARG1 __attribute__((nonnull (1)))
#define NONNULLARG2 __attribute__((nonnull (2)))
#define NONNULLARG3 __attribute__((nonnull (3)))
#define NONNULLARG4 __attribute__((nonnull (4)))
#define NONNULLARG5 __attribute__((nonnull (5)))
#define NONNULLARG6 __attribute__((nonnull (6)))
#define NONNULLARG7 __attribute__((nonnull (7))) /* for bhit() */
#define NONNULLARG12 __attribute__((nonnull (1, 2)))
#define NONNULLARG23 __attribute__((nonnull (2, 3)))
#define NONNULLARG123 __attribute__((nonnull (1, 2, 3)))
#define NONNULLARG13 __attribute__((nonnull (1, 3)))
#define NONNULLARG14 __attribute__((nonnull (1, 4))) /* for query_category */
#define NONNULLARG134 __attribute__((nonnull (1, 3, 4))) /* for do_stone_mon */
#define NONNULLARG145 __attribute__((nonnull (1, 4, 5))) /* find_roll_to_hit */
#define NONNULLARG17 __attribute__((nonnull (1, 7))) /* for askchain() */
#define NONNULLARG24 __attribute__((nonnull (2, 4))) /* query_objlist() */
#define NONNULLARG45 __attribute__((nonnull (4, 5))) /* do_screen_descri... */
#define NONNULLS_DEFINED
#undef DO_DEFINE_NONNULLS
#endif  /* DO_DEFINE_NONNULLS && !NONNULLS_DEFINED */

/*
 * Microsoft compiler
 */
#ifdef _MSC_VER
#ifndef ATTRNORETURN
#define ATTRNORETURN __declspec(noreturn)
/* #warning ATTRNORETURN __declspec(noreturn) from _MSC_VER */
#endif
/* #pragma message is available */
#define NH_PRAGMA_MESSAGE 1
#endif  /* _MSC_VER */

#if !defined(UNUSED) && defined(ATTRUNUSED)
#define UNUSED ATTRUNUSED
#endif

/* Fallback implementations */
#ifndef PRINTF_F
#define PRINTF_F(f, v)
#endif
#ifndef PRINTF_F_PTR
#define PRINTF_F_PTR(f, v)
#endif
#ifndef UNUSED
#define UNUSED
#endif
#ifndef ATTRUNUSED
#define ATTRUNUSED
#endif
#ifndef FALLTHROUGH
#define FALLTHROUGH
#endif
#ifndef ATTRNORETURN
#define ATTRNORETURN
#endif
#ifndef NORETURN
#define NORETURN
#endif
#ifndef NONNULLS_DEFINED
#define NONNULL
#define NONNULLPTRS
#define NONNULLARG1
#define NONNULLARG2
#define NONNULLARG3
#define NONNULLARG4
#define NONNULLARG5
#define NONNULLARG6
#define NONNULLARG7
#define NONNULLARG12
#define NONNULLARG23
#define NONNULLARG123
#define NONNULLARG13
#define NONNULLARG14
#define NONNULLARG134
#define NONNULLARG145
#define NONNULLARG17
#define NONNULLARG24
#define NONNULLARG45
#define NONNULLS_DEFINED
#endif  /* NONNULLS_DEFINED */
#ifndef NO_NNARGS
#define NO_NNARGS /*empty*/
#endif  /* NO_NNARGS */

/** @} */

/**
 * @name Withdrawn functions
 * @brief Two old routines declared unusable so that using one fails at compile time rather than at link time.
 *
 * Both have standard replacements the game uses instead. The reason for going to the trouble of declaring them at all is in the existing comment: on a non-Unix platform they do
 * not exist, so using one produces a link failure -- which is a far worse place to discover the mistake than the line that made it.
 *
 * @note So these declarations exist in order to fail. They are only made for compilers that support marking a function unavailable, which is why they are guarded by compiler
 *       version.
 * @note Excluded from one display's build, where a system header declares them and the marking would conflict.
 * @{
 */
/**
 * @name 철회된 함수
 * @brief 두 개의 예전 루틴을 쓸 수 없다고 선언해, 그것을 쓰면 링크 시점이 아니라 컴파일 시점에 실패하도록 한 것.
 *
 * 둘 다 게임이 대신 쓰는 표준 대체물이 있다. 그것들을 굳이 선언하는 이유는 기존 주석에 있다. Unix 가 아닌 플랫폼에서는 그것들이 존재하지 않으므로, 하나를 쓰면 링크 실패가 난다. 그것은 그 잘못을 발견할 곳으로서 그것을 만든 줄보다 훨씬 나쁘다.
 *
 * @note 그래서 이 선언들은 실패하기 위해 존재한다. 함수를 쓸 수 없다고 표시할 수 있는 컴파일러에 대해서만 이뤄지며, 그래서 컴파일러 버전으로 보호되어 있다.
 * @note 어느 표시부의 빌드에서는 제외된다. 그곳에서는 시스템 헤더가 그것들을 선언하고 이 표시가 충돌한다.
 * @{
 */
#if !defined(X11_BUILD) && !defined(__cplusplus)
#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ >= 12
extern char *index(const char *s, int c) __attribute__ ((unavailable));
extern char *rindex(const char *s, int c) __attribute__ ((unavailable));
#endif
#endif
#if defined(__clang__)
#if __clang_major__ >= 7
extern char *index(const char *s, int c) __attribute__ ((unavailable));
extern char *rindex(const char *s, int c) __attribute__ ((unavailable));
#endif
#endif
#endif
/** @} */

#endif /* TRADSTDC_H */
