/* NetHack 5.0	cstd.h	$NHDT-Date: 1781973078 2026/06/20 16:31:18 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.10 $ */
/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file cstd.h
 * @brief The standard library headers the game relies on, included once.
 *
 * Gathered here so that every translation unit gets the same set, and so the
 * list of what is depended upon is a single visible thing rather than scattered
 * across a hundred files.
 *
 * The catalogue below records the whole standard set, including the headers the
 * game deliberately does not use -- that is its purpose: to note what exists so
 * that a name from one of them is not accidentally reused, and so adopting one
 * later is a considered step.
 *
 * @note A C++ compiler gets only @c <stdio.h>, since the C++ ports need @c FILE
 *       but supply the rest themselves.
 */

/**
 * @file cstd.h
 * @brief 게임이 의존하는 표준 라이브러리 헤더들을 한 번에 포함한다.
 *
 * 모든 번역 단위가 동일한 집합을 얻도록, 그리고 무엇에 의존하는지가 백 개 파일에
 * 흩어지지 않고 한눈에 보이는 하나가 되도록 여기에 모았다.
 *
 * 아래 목록은 게임이 의도적으로 쓰지 않는 헤더까지 포함해 표준 집합 전체를 기록한다.
 * 그것이 이 목록의 목적이다. 무엇이 존재하는지를 적어 두어 그 안의 이름을 실수로 다시
 * 쓰지 않게 하고, 나중에 그중 하나를 채택하는 일이 숙고된 선택이 되게 한다.
 *
 * @note C++ 컴파일러에는 @c <stdio.h> 만 준다. C++ 포팅은 @c FILE 이 필요하지만
 *       나머지는 스스로 공급하기 때문이다.
 */

#ifndef CSTD_H
#define CSTD_H

/*
 * The list of standard (C99 unless noted otherwise) header files:
 *
 * <assert.h>	         Conditionally compiled macro that calls abort if its
 *                       argument evaluates to zero
 * <complex.h> (C99)     Complex number arithmetic
 * <ctype.h>	         Functions to categorize single characters
 * <errno.h>	         Macros reporting error conditions
 * <fenv.h> (C99)        Floating-point environment
 * <float.h>             Limits of floating-point types
 * <inttypes.h> (C99)    Format conversion of integer types
 * <iso646.h> (C95)      Alternative operator spellings
 * <limits.h>            Ranges of integer types
 * <locale.h>            Localization utilities
 * <math.h>              Common mathematics functions
 * <setjmp.h>            Nonlocal jumps
 * <signal.h>            Signal handling
 * <stdarg.h>            Variable arguments
 * <stdbool.h> (C99)     Macros for boolean type
 * <stddef.h>            Common macro definitions
 * <stdint.h> (C99)      Fixed-width integer types
 * <stdio.h>             Input/output program utilities
 * <stdlib.h>            General utilities: memory management,
 *                       program utilities, string conversions,
 *                       random numbers, algorithms
 * <string.h>            String handling
 * <tgmath.h> (C99)      Type-generic math (macros wrapping math.h and
 *                       complex.h)
 * <time.h>              Time/date utilities
 * <wchar.h> (C95)       Extended multibyte and wide character utilities
 * <wctype.h> (C95)      Functions to categorize single wide character

 * We watch these and try not to conflict with them, or make it tough to adopt
 * these in future:
 *
 * <stdalign.h> (C11)    alignas and alignof convenience macros
 * <stdatomic.h> (C11)   Atomic operations
 * <stdbit.h> (C23)      Macros to work with the byte and bit representations
 *                       of types
 * <stdckdint.h> (C23)   Macros for performing checked integer arithmetic
 * <stdnoreturn.h> (C11) noreturn convenience macro
 * <threads.h> (C11)     Thread library
 * <uchar.h> (C11)       UTF-16 and UTF-32 character utilities
 *
 */
#if !defined(__cplusplus)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

#else /* !__cplusplus */
/* for FILE */
#include <stdio.h>
#endif /* !__cplusplus */
#endif /* CSTD_H */
