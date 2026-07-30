/* NetHack 5.0	hacklib.h	$NHDT-Date: 1781973081 2026/06/20 16:31:21 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.5 $ */
/* Copyright (c) Steve Creps, 1988.                               */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file hacklib.h
 * @brief Small utilities that know nothing about the game.
 *
 * String tidying, character tests, a little arithmetic and some geometry. None
 * of it touches the dungeon, the hero or the map, which is deliberate: the build
 * tools link against this too, so a dependency on game state would make that
 * impossible.
 *
 * Two contracts matter more than anything else here, and neither is visible in a
 * signature. Some routines modify the string they are given and hand it back, so
 * the result is the argument rather than a copy. Others build their answer in a
 * buffer they own, so the result survives only until the next call.
 *
 * @warning Which routines return a private buffer is documented per function
 *          below, because the consequences differ: one shared buffer means a
 *          second call destroys the first answer, so two results cannot be used
 *          in the same expression.
 */

/**
 * @file hacklib.h
 * @brief 게임에 대해 아무것도 알지 못하는 작은 유틸리티들.
 *
 * 문자열 정돈, 문자 검사, 약간의 산술과 기하다. 어느 것도 던전이나 영웅, 지도를 건드리지
 * 않으며 이는 의도된 것이다. 빌드 도구도 이것을 링크하므로, 게임 상태에 의존한다면
 * 그것이 불가능해진다.
 *
 * 여기서 무엇보다 중요한 두 가지 계약이 있고, 둘 다 시그니처에 드러나지 않는다. 어떤
 * 루틴은 받은 문자열을 직접 고쳐 그것을 되돌려 준다. 결과가 사본이 아니라 인자 자신이다.
 * 다른 루틴은 자기가 소유한 버퍼에 답을 만든다. 그 결과는 다음 호출까지만 살아 있다.
 *
 * @warning 어느 루틴이 비공개 버퍼를 반환하는지는 아래 함수별로 기록했다. 결과가
 *          다르기 때문이다. 공유 버퍼가 하나뿐이면 두 번째 호출이 첫 답을 파괴하므로,
 *          두 결과를 같은 식 안에서 쓸 수 없다.
 */

#ifndef HACKLIB_H
#define HACKLIB_H

/*
 * hacklib true library functions
 */

/** @brief Whether a character is a decimal digit or an ASCII letter. */
/** @brief 문자가 10진 숫자인지, ASCII 알파벳인지. */
extern boolean digit(char);
extern boolean letter(char);

/** @brief Force a character to upper or lower case, leaving others unchanged. */
/** @brief 문자를 대문자 또는 소문자로 만든다. 그 밖의 문자는 그대로 둔다. */
extern char highc(char);
extern char lowc(char);

/**
 * @brief Recase a string in place.
 * @param[in,out] s String to modify.
 * @return @p s itself, so the call can be used as an expression.
 * @note @c upstart capitalizes only the first character and @c upwords every
 *       word; both alter the caller's buffer.
 * @warning @c upstart accepts NULL, unlike its neighbours -- the existing comment
 *          notes that this is tolerated rather than intended.
 */
/**
 * @brief 문자열의 대소문자를 제자리에서 바꾼다.
 * @param[in,out] s 고칠 문자열.
 * @return @p s 자신. 그래서 호출을 식으로 쓸 수 있다.
 * @note @c upstart 는 첫 글자만, @c upwords 는 모든 단어를 대문자로 만든다. 둘 다
 *       호출자의 버퍼를 고친다.
 * @warning @c upstart 는 이웃들과 달리 NULL 을 받아들인다. 기존 주석이 밝히듯 의도된
 *          것이 아니라 용인되고 있는 것이다.
 */
extern char *lcase(char *) NONNULL NONNULLARG1;
extern char *ucase(char *) NONNULL NONNULLARG1;
extern char *upstart(char *); /* ought to be changed to NONNULL NONNULLARG1
                               * and the code changed to not allow NULL arg */
extern char *upwords(char *) NONNULL NONNULLARG1;

/**
 * @brief Tidy whitespace in place.
 * @param[in,out] s String to modify.
 * @return @p s itself.
 * @note @c mungspaces collapses runs of spaces, @c trimspaces removes leading and
 *       trailing ones, @c strip_newline removes a trailing newline.
 * @warning These shorten the string, so another pointer already taken into it may
 *          end up past the terminator.
 */
/**
 * @brief 공백을 제자리에서 정돈한다.
 * @param[in,out] s 고칠 문자열.
 * @return @p s 자신.
 * @note @c mungspaces 는 연속된 공백을 하나로 줄이고, @c trimspaces 는 앞뒤 공백을
 *       없애며, @c strip_newline 은 끝의 줄바꿈을 없앤다.
 * @warning 문자열이 짧아지므로, 이미 그 안을 가리키던 다른 포인터가 종료 문자 뒤로
 *          밀려날 수 있다.
 */
extern char *mungspaces(char *) NONNULL NONNULLARG1;
extern char *trimspaces(char *) NONNULL NONNULLARG1;
extern char *strip_newline(char *) NONNULL NONNULLARG1;

/**
 * @brief Address the terminating NUL of a string.
 * @param[in] s String to examine.
 * @return A pointer into @p s, at its terminator.
 * @note Used to append without rescanning; @c c_eos is the same for a string
 *       that must not be written through.
 * @warning Points into the caller's buffer, so it is invalidated by anything that
 *          shortens or reallocates that buffer.
 */
/**
 * @brief 문자열의 종료 NUL 위치를 가리킨다.
 * @param[in] s 살펴볼 문자열.
 * @return @p s 안의 포인터. 종료 문자 위치다.
 * @note 다시 훑지 않고 덧붙이기 위해 쓴다. @c c_eos 는 쓰기를 허용하지 않는 문자열에
 *       대한 같은 함수다.
 * @warning 호출자의 버퍼 안을 가리키므로, 그 버퍼를 짧게 만들거나 재할당하는 어떤
 *          일에도 무효가 된다.
 */
extern char *eos(char *) NONNULL NONNULLARG1;
extern const char *c_eos(const char *) NONNULL NONNULLARG1;
extern boolean str_start_is(const char *, const char *, boolean) NONNULLPTRS;
extern boolean str_end_is(const char *, const char *) NONNULLPTRS;
extern int str_lines_maxlen(const char *);
extern char *strkitten(char *, char) NONNULL NONNULLARG1;
extern void copynchars(char *, const char *, int) NONNULLARG12;
extern char chrcasecpy(int, int);
extern char *strcasecpy(char *, const char *) NONNULL NONNULLPTRS;
/**
 * @brief Form the possessive or the present participle of a word.
 * @param[in] s Word to inflect.
 * @return The inflected word.
 * @note Handles the pronouns specially, so "it" becomes "its" and "you" becomes
 *       "your" rather than gaining an apostrophe.
 * @warning Built in one buffer owned by the function. A second call destroys the
 *          previous answer, so two of these cannot be used in one expression.
 */
/**
 * @brief 단어의 소유격 또는 현재분사를 만든다.
 * @param[in] s 활용할 단어.
 * @return 활용된 단어.
 * @note 대명사를 특별히 처리한다. 그래서 "it"은 아포스트로피가 붙는 대신 "its"가 되고
 *       "you"는 "your"가 된다.
 * @warning 함수가 소유한 하나의 버퍼에 만든다. 두 번째 호출이 앞의 답을 파괴하므로,
 *          이 함수 두 개를 한 식 안에서 쓸 수 없다.
 */
extern char *s_suffix(const char *) NONNULL NONNULLARG1;
extern char *ing_suffix(const char *) NONNULL NONNULLARG1;
extern char *xcrypt(const char *, char *) NONNULL NONNULLPTRS;
extern boolean onlyspace(const char *) NONNULLARG1;
extern char *tabexpand(char *) NONNULL NONNULLARG1;
/**
 * @brief Render a character so a control or meta key can be printed.
 * @param[in] c Character to describe.
 * @return Its printable form, such as "^C" or "M-x".
 * @note Unlike the other buffer-returning helpers here, this rotates through
 *       several buffers, so a small number of results can be live at once --
 *       which is what lets two keys appear in the same message.
 * @warning The pool is finite. Beyond that many live results the oldest is
 *          overwritten, so it is a convenience for one message, not a guarantee.
 */
/**
 * @brief 제어 키나 메타 키를 인쇄할 수 있도록 문자를 표현한다.
 * @param[in] c 표현할 문자.
 * @return "^C" 나 "M-x" 같은 인쇄 가능한 형태.
 * @note 여기의 다른 버퍼 반환 함수들과 달리 여러 버퍼를 돌려 쓴다. 그래서 소수의 결과가
 *       동시에 살아 있을 수 있으며, 덕분에 한 메시지에 두 개의 키가 나타날 수 있다.
 * @warning 버퍼 수는 유한하다. 그보다 많은 결과를 동시에 쓰면 가장 오래된 것이 덮이므로,
 *          보장이 아니라 한 메시지를 위한 편의다.
 */
extern char *visctrl(char) NONNULL;
extern char *stripchars(char *, const char *,
                                            const char *) NONNULL NONNULLPTRS;
extern char *stripdigits(char *) NONNULL NONNULLARG1;
extern char *strsubst(char *, const char *, const char *) NONNULL NONNULLPTRS;
extern int strNsubst(char *, const char *, const char *, int) NONNULLPTRS;
extern const char *findword(const char *, const char *, int,
                                                         boolean) NONNULLARG2;
/**
 * @brief The ordinal suffix for a number -- "st", "nd", "rd" or "th".
 * @param[in] n The number; expected to be non-negative.
 * @return A string literal, so it may be held indefinitely.
 * @note Gets the teens right, where the last digit alone would be wrong.
 */
/**
 * @brief 숫자의 서수 접미사. "st", "nd", "rd", "th" 중 하나.
 * @param[in] n 그 숫자. 음수가 아닐 것으로 기대한다.
 * @return 문자열 리터럴이므로 얼마든지 오래 보관해도 된다.
 * @note 마지막 자리만으로는 틀리는 10대 숫자들을 올바르게 처리한다.
 */
extern const char *ordin(int) NONNULL;

/**
 * @brief Format a number with an explicit sign, as "+3" or "-3".
 * @param[in] n The number.
 * @return Its signed text.
 * @note Used where the sign carries meaning, such as an enchantment.
 * @warning Built in one buffer owned by the function, so a second call destroys
 *          the previous answer.
 */
/**
 * @brief 숫자를 부호를 명시해 "+3" 이나 "-3" 형태로 만든다.
 * @param[in] n 그 숫자.
 * @return 부호가 붙은 문구.
 * @note 마법 강화치처럼 부호 자체가 의미를 갖는 곳에 쓴다.
 * @warning 함수가 소유한 하나의 버퍼에 만든다. 두 번째 호출이 앞의 답을 파괴한다.
 */
extern char *sitoa(int) NONNULL;
/** @brief The sign of a number, as -1, 0 or 1. */
/** @brief 숫자의 부호. -1, 0, 1 중 하나. */
extern int sgn(int);

/**
 * @brief Distances between two map positions.
 * @return @c distmin gives the number of steps, counting a diagonal as one;
 *         @c dist2 gives the squared straight-line distance.
 * @note @c distmin is what "how far to walk" means on this map, since a diagonal
 *       step costs the same as a straight one. @c dist2 is squared so that
 *       comparisons need no square root.
 * @warning The two are not interchangeable, and mixing them silently changes what
 *          a range means.
 */
/**
 * @brief 지도상 두 위치 사이의 거리.
 * @return @c distmin 은 대각선을 한 걸음으로 세는 걸음 수를, @c dist2 는 직선 거리의
 *         제곱을 준다.
 * @note 이 지도에서 "걸어서 얼마나 먼가"는 @c distmin 이다. 대각선 한 걸음이 직선 한
 *       걸음과 같은 값이기 때문이다. @c dist2 가 제곱인 것은 비교에 제곱근이 필요 없게
 *       하기 위함이다.
 * @warning 둘은 서로 바꿔 쓸 수 없으며, 섞어 쓰면 사거리의 의미가 조용히 달라진다.
 */
extern int distmin(coordxy, coordxy, coordxy, coordxy);
extern int dist2(coordxy, coordxy, coordxy, coordxy);

/**
 * @brief Integer square root.
 * @return The largest integer whose square does not exceed the argument.
 * @note Exists so distances can be compared without floating point.
 */
/**
 * @brief 정수 제곱근.
 * @return 제곱이 인자를 넘지 않는 가장 큰 정수.
 * @note 부동소수점 없이 거리를 비교할 수 있도록 존재한다.
 */
extern int isqrt(int);

/**
 * @brief Whether two positions lie on a line a beam could travel.
 * @retval TRUE  They share a row, a column, or a diagonal.
 * @retval FALSE They do not.
 * @note This is the geometry of the eight directions, not true collinearity --
 *       which is why it answers what a zap or a ranged attack can reach.
 */
/**
 * @brief 두 위치가 광선이 지날 수 있는 선 위에 있는지.
 * @retval TRUE  같은 행, 같은 열, 또는 같은 대각선에 있다.
 * @retval FALSE 그렇지 않다.
 * @note 참된 일직선이 아니라 8방향의 기하다. 그래서 쏜 것이나 원거리 공격이 닿을 수
 *       있는 범위에 답한다.
 */
extern boolean online2(coordxy, coordxy, coordxy, coordxy);
#ifndef STRNCMPI
extern int strncmpi(const char *, const char *, int) NONNULLPTRS;
#endif
#ifndef STRSTRI
extern char *strstri(const char *, const char *) NONNULLPTRS;
#endif
#define FITSint(x) FITSint_(x, __func__, __LINE__)
extern int FITSint_(long long, const char *, int);
#define FITSuint(x) FITSuint_(x, __func__, __LINE__)
extern unsigned FITSuint_(unsigned long long, const char *, int);
extern int case_insensitive_comp(const char *, const char *);
extern boolean fuzzymatch(const char *, const char *,
                          const char *, boolean) NONNULLPTRS;
extern int swapbits(int, int, int);
/* note: the snprintf CPP wrapper includes the "fmt" argument in "..."
   (__VA_ARGS__) to allow for zero arguments after fmt */
extern void nh_snprintf(const char *func, int line, char *str,
                        size_t size, const char *fmt, ...) PRINTF_F(5, 6);
extern void nh_snprintf_w_impossible(const char *func, int line, char *str,
                        size_t size, const char *fmt, ...) PRINTF_F(5, 6);

#define Snprintf(str, size, ...) \
    nh_snprintf(__func__, __LINE__, str, size, __VA_ARGS__)

#if 0
/*#define Strlen(s) Strlen_(s, __func__, __LINE__)*/
extern unsigned Strlen_(const char *, const char *, int) NONNULLPTRS;
#endif
extern int unicodeval_to_utf8str(int, uint8 *, size_t);
extern boolean copy_bytes(int, int);
extern const char *datamodel(int);
extern const char *what_datamodel_is_this(int, int, int, int, int, int);

#endif /* HACKLIB_H */

