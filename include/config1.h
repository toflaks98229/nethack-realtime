/* NetHack 5.0	config1.h	$NHDT-Date: 1781973077 2026/06/20 16:31:17 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.32 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Kenneth Lorber, Kensington, Maryland, 2015. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file config1.h
 * @brief Working out what platform this is by asking the compiler.
 *
 * Compilers announce themselves, each with its own symbol. This file translates those announcements into the names the game uses, so that a builder does not have to declare
 * their platform and cannot declare it wrongly.
 *
 * It does two things beyond translating. It resolves contradictions -- one platform's symbol being defined implies another's should not be, and the file undefines what cannot
 * be true. And it disables features the detected platform cannot support, so an option the builder enabled is quietly withdrawn rather than failing to compile.
 *
 * @note That withdrawal is deliberate and is why the file undefines rather than complains. An option that a platform cannot support is not a mistake by the builder, and
 *       failing the build over it would be worse than proceeding without it.
 * @note One of the compiler-specific blocks here does not detect anything: it redefines the preprocessor's own facility for testing whether a name is defined, for a compiler
 *       that lacked it. Its accompanying comment works through the arithmetic that makes that possible and admits the one case it cannot handle.
 * @warning Read before config.h's platform section takes effect. So anything decided here is what that section then sees, and a platform symbol defined by hand in config.h
 *          may be undone here.
 */

/**
 * @file config1.h
 * @brief 컴파일러에게 물어 이것이 어느 플랫폼인지 알아내기.
 *
 * 컴파일러들은 자신을 알리며, 각각 자기 심볼을 가진다. 이 파일은 그 알림을 게임이 쓰는 이름으로 번역한다. 그래서 빌드하는 사람이 자기 플랫폼을 선언할 필요가 없고, 잘못 선언할 수도 없다.
 *
 * 번역 말고도 두 가지를 한다. 모순을 해결한다. 한 플랫폼의 심볼이 정의되어 있다는 것은 다른 플랫폼의 것이 정의되어서는 안 된다는 뜻이며, 이 파일이 참일 수 없는 것을 정의 해제한다. 그리고 감지된 플랫폼이 지원할 수 없는 기능을 끈다. 그래서 빌드하는 사람이
 * 켠 선택지가 컴파일에 실패하는 대신 조용히 철회된다.
 *
 * @note 그 철회는 의도적이며, 이 파일이 불평하는 대신 정의 해제하는 이유다. 플랫폼이 지원할 수 없는 선택지는 빌드하는 사람의 잘못이 아니고, 그것 때문에 빌드를 실패시키는 것은 그것 없이 진행하는 것보다 나쁘다.
 * @note 여기의 컴파일러별 블록 중 하나는 아무것도 감지하지 않는다. 어떤 이름이 정의되었는지 검사하는 전처리기 자체의 기능을, 그것이 없던 컴파일러를 위해 재정의한다. 딸린 주석이 그것을 가능하게 하는 산술을 풀어 보이고 다룰 수 없는 한 경우를 인정한다.
 * @warning config.h 의 플랫폼 절이 효력을 갖기 전에 읽힌다. 그래서 여기서 정해진 것이 그 절이 보게 되는 것이고, config.h 에서 손으로 정의한 플랫폼 심볼이 여기서 되돌려질 수 있다.
 */

#ifndef CONFIG1_H
#define CONFIG1_H

/*
 * MS DOS - compilers
 *
 * Microsoft C auto-defines MSDOS,
 * Borland C   auto-defines __MSDOS__,
 * DJGPP       auto-defines MSDOS.
 */

/* #define MSDOS */ /* use if not defined by compiler or cases below */

#ifdef __MSDOS__ /* for Borland C */
#ifndef MSDOS
#define MSDOS
#endif
#endif

#ifdef __TURBOC__
#define __MSC /* increase Borland C compatibility in libraries */
#endif

#ifdef MSDOS
#undef UNIX
#ifndef CROSSCOMPILE
#define SHORT_FILENAMES
#endif
/* this is not fully-implemented yet for msdos */
#ifdef ENHANCED_SYMBOLS
#undef ENHANCED_SYMBOLS
#endif
#endif

/*
 * Mac Stuff.
 */
#if defined(__APPLE__) && defined(__MACH__)
#define MACOS
#endif

#ifdef macintosh /* Auto-defined symbol for MPW compilers (sc and mrc) */
#define MAC68K
#endif

#ifdef THINK_C /* Think C auto-defined symbol */
#define MAC68K
#define NEED_VARARGS
#endif

#ifdef __MWERKS__ /* defined by Metrowerks' Codewarrior compiler */
#ifndef __BEOS__  /* BeOS */
#define MAC68K
#endif
#define NEED_VARARGS
#define USE_STDARG
#endif

#if defined(MAC68K) || defined(__BEOS__)
#define DLB
#undef UNIX
#endif

#ifdef __BEOS__
#define NEED_VARARGS
#endif

/*
 * Amiga setup.
 */
#ifdef AZTEC_C   /* Manx auto-defines this */
#ifdef MCH_AMIGA /* Manx auto-defines this for AMIGA */
#ifndef AMIGA
#define AMIGA    /* define for Commodore-Amiga */
#endif           /* (SAS/C auto-defines AMIGA) */
#define AZTEC_50 /* define for version 5.0 of manx */
#endif
#endif
#ifdef __SASC_60
#define NEARDATA __near /* put some data close */
#else
#ifdef _DCC
#define NEARDATA __near /* put some data close */
#else
#define NEARDATA
#endif
#endif
#ifdef AMIGA
#define NEED_VARARGS
#undef UNIX
#define DLB
#define HACKDIR "NetHack:"

#endif

/*
 * Atari auto-detection
 */

#ifdef atarist
#undef UNIX
#ifndef TOS
#define TOS
#endif
#else
#ifdef __MINT__
#undef UNIX
#ifndef TOS
#define TOS
#endif
#endif
#endif

/*
 * Windows NT Autodetection
 */
#ifdef _WIN32_WCE
#define WIN_CE
#ifndef WIN32
#define WIN32
#endif
#endif

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
#undef UNIX
#undef MSDOS
#define NHSTDC
#define USE_STDARG
#define NEED_VARARGS

#ifndef WIN_CE
#define STRNCMPI
#define STRCMPI
#endif

#endif

#if defined(__linux__) && defined(__GNUC__) && !defined(_GNU_SOURCE)
/* ensure _GNU_SOURCE is defined before including any system headers */
#define _GNU_SOURCE
#endif

#ifdef __vms
#ifndef VMS
#define VMS
#endif
#endif

#ifdef VMS /* really old compilers need special handling, detected here */
#undef UNIX
#ifdef __DECC
#ifndef __DECC_VER /* buggy early versions want widened prototypes */
#define NOTSTDC    /* except when typedefs are involved            */
        /* [25 or so years later...  That was probably uchar widening to */
        /* 'unsigned int' rather than anything to do with typedefs.  pr] */
#define USE_VARARGS
#else              /* __DECC_VER not defined */
#if __DECC_VER >= 70000000
#define VMSVSI
#endif /* _DECC_VER >= 70000000 */
#ifndef VMSVSI
#define NHSTDC
#define USE_STDARG
#define POSIX_TYPES
#ifndef _DECC_V4_SOURCE /* only def here if not already def'd on comd line */
#define _DECC_V4_SOURCE /* avoid some incompatible V5.x (and later) changes */
#endif
#endif /* !VMSVSI */
#endif /*__DECC_VER*/
#undef __HIDE_FORBIDDEN_NAMES /* need non-ANSI library support functions */
#ifndef VMSVSI
#ifdef VAXC    /* DEC C in VAX C compatibility mode; 'signed' works   */
#define signed /* but causes diagnostic about VAX C not supporting it */
#endif
#else /*!__DECC*/
#ifdef VAXC /* must use CC/DEFINE=ANCIENT_VAXC for vaxc v2.2 or older */
#define signed
#ifdef ANCIENT_VAXC /* vaxc v2.2 and earlier [lots of warnings to come] */
#define KR1ED       /* simulate defined() */
#define USE_VARARGS
#else                       /* vaxc v2.3,2.4,or 3.x, or decc in vaxc mode */
#if defined(USE_PROTOTYPES) /* this breaks 2.2 (*forces* use of ANCIENT)*/
#define __STDC__ 0 /* vaxc is not yet ANSI compliant, but close enough */
#include <stddef.h>
#define UNWIDENED_PROTOTYPES
#endif
#define USE_STDARG
#endif
#endif              /*VAXC*/
#endif              /*__DECC*/
#ifdef VERYOLD_VMS  /* v4.5 or earlier; no longer available for testing */
#define USE_OLDARGS /* <varargs.h> is there, vprintf & vsprintf aren't */
#ifdef USE_VARARGS
#undef USE_VARARGS
#endif
#ifdef USE_STDARG
#undef USE_STDARG
#endif
#endif
#endif /* !VMSVSI */
#endif /*VMS*/

#ifdef vax
/* just in case someone thinks a DECstation is a vax. It's not, it's a mips */
#ifdef ULTRIX_PROTO
#undef ULTRIX_PROTO
#endif
#ifdef ULTRIX_CC20
#undef ULTRIX_CC20
#endif
#endif

/**
 * @def defined
 * @brief The preprocessor's defined-test, reimplemented arithmetically for a compiler that lacks it.
 *
 * A name that is not defined expands to nothing, and a name defined without a value also expands to nothing -- so the two cannot be told apart by expansion alone. The trick
 * here is to build an expression whose value differs between the two cases, and the comment below works through each one.
 *
 * @warning Redefining a preprocessor keyword. It works only for the cases the comment enumerates, and the comment ends by naming the one it cannot handle: a name defined as a
 *          string literal produces an expression that is not merely wrong but will not compile.
 * @warning Also inexact for a value supplied on the command line, since compilers disagree on what such a name defaults to. The comment says so and accepts it.
 * @note Only defined for one long-obsolete compiler. Nothing else in the game is written with this in mind.
 */
/**
 * @def defined
 * @brief 전처리기의 정의 여부 검사. 그것이 없는 컴파일러를 위해 산술로 다시 구현한 것.
 *
 * 정의되지 않은 이름은 아무것도로 펼쳐지고, 값 없이 정의된 이름도 아무것도로 펼쳐진다. 그래서 그 둘을 펼침만으로는 구별할 수 없다. 여기의 요령은 두 경우에서 값이 달라지는 수식을 만드는 것이며, 아래의 주석이 각 경우를 풀어 보인다.
 *
 * @warning 전처리기 키워드를 재정의한다. 그 주석이 열거하는 경우들에서만 작동하며, 주석은 다룰 수 없는 한 경우를 밝히며 끝난다. 문자열 리터럴로 정의된 이름은 단지 틀린 것이 아니라 컴파일되지 않는 수식을 만든다.
 * @warning 명령줄에서 주어진 값에 대해서도 정확하지 않다. 그런 이름의 기본값이 무엇인지에 대해 컴파일러들이 서로 다르기 때문이다. 주석이 그것을 말하고 받아들인다.
 * @note 오래전에 폐기된 어느 컴파일러 하나에 대해서만 정의된다. 게임의 다른 어떤 것도 이것을 염두에 두고 쓰이지 않았다.
 */
#ifdef KR1ED /* For compilers which cannot handle defined() */
#define defined(x) (-x - 1 != -1)
/* Because:
 * #define FOO => FOO={} => defined( ) => (-1 != - - 1) => 1
 * #define FOO 1 or on command-line -DFOO
 *      => defined(1) => (-1 != - 1 - 1) => 1
 * if FOO isn't defined, FOO=0. But some compilers default to 0 instead of 1
 * for -DFOO, oh well.
 *      => defined(0) => (-1 != - 0 - 1) => 0
 *
 * But:
 * defined("") => (-1 != - "" - 1)
 *   [which is an unavoidable catastrophe.]
 */
#endif

#endif /* CONFIG1_H */
