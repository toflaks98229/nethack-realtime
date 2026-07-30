/* NetHack 5.0	integer.h	$NHDT-Date: 1781973081 2026/06/20 16:31:21 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.20 $ */
/*      Copyright (c) 2016 by Michael Allison          */
/* NetHack may be freely redistributed.  See license for details. */

/* integer.h -- provide sized integer types
 *
 * We try to sort out a way to provide sized integer types
 * in here. The strong preference is to try to let a
 * compiler-supplied header file set up the types.
 *
 * If your compiler is C99 conforming and sets a value of
 * __STDC_VERSION__ >= 199901L, then <stdint.h> is supposed
 * to be available for inclusion.
 *
 * If your compiler doesn't set __STDC_VERSION__ to indicate
 * full conformance to C99, but does actually supply a suitable
 * <stdint.h>, you can pass a compiler flag -DHAS_STDINT_H
 * during build to cause the inclusion of <stdint.h> anyway.
 *
 * If <stdint.h> doesn't get included, then the code in the
 * STDINT_WORKAROUND section of code is not skipped and will
 * be used to set up the types.
 *
 * We acknowledge that some ongoing maintenance may be needed
 * over time if people send us code updates for making the
 * determination of whether <stdint.h> is available, or
 * require adjustments to the base type used for some
 * compiler/platform combinations.
 *
 */

/**
 * @file integer.h
 * @brief Integer types of a known width, however the compiler provides them.
 *
 * Saved games and the random number generator need integers whose width does not
 * change from one platform to another, but the standard header that declares them
 * cannot simply be included: some compilers claim conformance without shipping it,
 * and others ship it without claiming conformance. So the file works out which case
 * it is in and falls back to declaring the types itself.
 *
 * Once that is settled the short names -- @c int16, @c uint32 and their siblings --
 * are what the rest of the game uses, so no other file has to repeat the reasoning.
 *
 * @note The only place a 64-bit integer is genuinely required is the Isaac64 random
 *       number generator; a platform without one can turn that generator off in
 *       config.h and the definitions here stop mattering.
 * @warning A wrong base type here is not a compile error. It silently changes the
 *          width of everything written to a save file.
 */

/**
 * @file integer.h
 * @brief 폭이 정해진 정수 타입. 컴파일러가 무엇을 제공하든.
 *
 * 저장 게임과 난수 생성기는 플랫폼이 달라져도 폭이 바뀌지 않는 정수를 필요로 한다. 그런데
 * 그것을 선언하는 표준 헤더를 그냥 포함할 수는 없다. 어떤 컴파일러는 그 헤더를 제공하지 않으면서
 * 표준을 따른다고 주장하고, 어떤 컴파일러는 주장하지 않으면서 제공한다. 그래서 이 파일이 어느
 * 경우인지 판별하고, 아니면 직접 타입을 선언한다.
 *
 * 그것이 정리되면 게임의 나머지 부분은 @c int16, @c uint32 같은 짧은 이름만 쓰므로, 다른 파일이
 * 같은 판별을 되풀이할 필요가 없다.
 *
 * @note 64비트 정수가 실제로 필요한 곳은 Isaac64 난수 생성기 하나뿐이다. 그것이 없는 플랫폼은
 *       config.h 에서 그 생성기를 끌 수 있고, 그러면 여기의 정의는 무의미해진다.
 * @warning 여기서 기반 타입이 틀리는 것은 컴파일 오류가 아니다. 저장 파일에 기록되는 모든 것의
 *          폭이 조용히 바뀐다.
 */

#ifndef INTEGER_H
#define INTEGER_H

/* DEC C (aka Compaq C for a while, HP C these days) for VMS is
   classified as a freestanding implementation rather than a hosted one
   and even though it claims to be C99, it does not provide <stdint.h>. */
#if defined(__DECC) && defined(VMS) && !defined(HAS_STDINT_H)
#define HAS_INTTYPES_H
#else /*!__DECC*/

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#if !defined(HAS_STDINT_H)
/* The compiler claims to conform to C99. Use stdint.h */
#define HAS_STDINT_H
#endif  /* !HAS_STDINT_H */
#if !defined(HAS_INTTYPES_H)
/* The compiler claims to conform to C99. Use inttypes.h */
#define HAS_INTTYPES_H
#endif  /* !HAS_INTTYPES_H */
#if defined(__GNUC__) && defined(__INT64_MAX__) && !defined(HAS_STDINT_H)
#define HAS_STDINT_H
#endif
#endif  /* claims to be C99 */

#endif /*?__DECC*/

#ifdef HAS_STDINT_H
#include <stdint.h>
#define SKIP_STDINT_WORKAROUND
#endif
#ifdef HAS_INTTYPES_H
#include <inttypes.h>
#endif  /* HAS_INTTYPES_H */

#ifndef SKIP_STDINT_WORKAROUND /* !C99 */
/*
 * STDINT_WORKAROUND section begins here
 */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;

#if defined(__WATCOMC__) && !defined(__386__)
/* Open Watcom providing a 16 bit build for MS-DOS or OS/2 */
/* int is 16 bits; use long for 32 bits */
typedef long int int32_t;
typedef unsigned long int uint32_t;
#else
/* Otherwise, assume either a 32- or 64-bit compiler */
/* long may be 64 bits; use int for 32 bits */
typedef int int32_t;
typedef unsigned int uint32_t;
#endif

/* The only place where nethack cares about 64-bit integers is in the
   Isaac64 random number generator.  If your environment can't support
   64-bit integers, you should comment out USE_ISAAC64 in config.h so
   that the previous RNG gets used instead.  Then this file will be
   inhibited and it won't matter what the int64_t and uint64_t lines are. */

#if defined(__cplusplus)
#include <stdint.h>
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif

#endif /* !C99 */

/* Provide int8, uint8, int16, uint16, int32, uint32, int64 and uint64 */
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

/* Also provide ushort, uint, ulong */
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

/**
 * @def AppendLongDigit
 * @brief Append one decimal digit to a number, reporting overflow instead of wrapping.
 *
 * Used while reading a number a character at a time, where the input may be longer
 * than a @c long can hold. Signed overflow has no defined behaviour, so the check
 * happens before the multiply rather than after it.
 *
 * @param L the number so far; must not be negative
 * @param D the digit to append
 * @return the new number, or @c -1L if appending would not fit
 * @warning @c -1L is the overflow report, so a caller that treats the result as a
 *          number without checking will read a huge input as negative one.
 */
/**
 * @def AppendLongDigit
 * @brief 수에 십진 숫자 하나를 덧붙인다. 넘칠 때는 순환하지 않고 넘쳤음을 알린다.
 *
 * 숫자를 한 문자씩 읽어 들일 때 쓰이며, 입력이 @c long 이 담을 수 있는 것보다 길 수 있다. 부호
 * 있는 정수의 넘침은 정의된 동작이 없으므로, 검사는 곱한 뒤가 아니라 곱하기 전에 이뤄진다.
 *
 * @param L 지금까지의 수. 음수여서는 안 된다
 * @param D 덧붙일 숫자
 * @return 새 수. 덧붙이면 담기지 않을 경우 @c -1L
 * @warning @c -1L 이 넘침을 알리는 값이다. 그래서 검사하지 않고 결과를 수로 취급하는 호출자는
 *          아주 큰 입력을 음의 일로 읽는다.
 */
/* for non-negative L, calculate L * 10 + D, avoiding signed overflow;
   yields -1 if overflow would have happened;
   assumes compiler will optimize the constants */
#define AppendLongDigit(L,D) \
    (((L) < LONG_MAX / 10L                                      \
      || ((L) == LONG_MAX / 10L && (D) <= LONG_MAX % 10L))      \
     ? (L) * 10L + (D)                                          \
     : -1L)

/**
 * @def nowrap_add
 * @brief Add two non-negative numbers, saturating at the largest @c long rather than
 *        wrapping.
 *
 * For running totals -- score, turns, weight -- where a value pinned at the maximum
 * is a believable answer and a value that has wrapped to negative is not.
 *
 * @param a first addend; must not be negative
 * @param b second addend; must not be negative
 * @return the sum, or @c LONG_MAX if the true sum would not fit
 * @note Both arguments are evaluated more than once, so an argument with a side
 *       effect will have it applied twice.
 * @warning Neither argument is cast, so mixing narrower types is the caller's
 *          responsibility -- the promotion happens before the check does.
 */
/**
 * @def nowrap_add
 * @brief 음이 아닌 두 수를 더한다. 넘칠 때는 순환하지 않고 가장 큰 @c long 에서 멈춘다.
 *
 * 점수, 턴 수, 무게처럼 누적되는 값에 쓴다. 최대값에 붙어 있는 값은 그럴듯한 답이지만, 순환해서
 * 음수가 된 값은 그렇지 않다.
 *
 * @param a 첫 번째 피가수. 음수여서는 안 된다
 * @param b 두 번째 피가수. 음수여서는 안 된다
 * @return 합. 참된 합이 담기지 않을 경우 @c LONG_MAX
 * @note 두 인자 모두 한 번 넘게 평가된다. 그래서 부작용이 있는 인자는 그 부작용이 두 번 적용된다.
 * @warning 어느 인자도 형변환되지 않으므로, 더 좁은 타입을 섞는 것은 호출자의 책임이다. 승격은
 *          검사보다 먼저 일어난다.
 */
/* add a and b, return max long value if overflow would have occurred;
   assumes that both a and b are non-negative; caller should apply
   cast(s) to (long) in the arguments if any are needed */
#define nowrap_add(a,b) ((a) <= (LONG_MAX - (b)) ? ((a) + (b)) : LONG_MAX)

#endif /* INTEGER_H */
