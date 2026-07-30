/* NetHack 5.0	micro.h	$NHDT-Date: 1781973082 2026/06/20 16:31:22 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.14 $ */
/*      Copyright (c) 2015 by Kenneth Lorber              */
/* NetHack may be freely redistributed.  See license for details. */

/* micro.h - function declarations for various microcomputers */

/**
 * @file micro.h
 * @brief Writing control and meta keystrokes as characters.
 *
 * Key bindings are expressed as characters, but control and meta keys are not
 * printable, so they are built arithmetically from the letter they modify --
 * which is how a table of bindings can hold them as ordinary chars.
 *
 * @note Guarded with @c #ifndef because the same names are defined by some
 *       platforms' own headers; whichever arrives first wins, deliberately.
 * @warning @c M() sets the high bit, so its result depends on whether @c char is
 *          signed on the platform. That is why the cast is written out.
 */

/**
 * @file micro.h
 * @brief 제어 키와 메타 키를 문자로 표현하기.
 *
 * 키 바인딩은 문자로 표현되지만 제어 키와 메타 키는 인쇄 가능한 문자가 아니다. 그래서
 * 수식하는 글자로부터 산술적으로 만들어 낸다. 바인딩 표가 그것들을 평범한 char 로
 * 담을 수 있는 방법이다.
 *
 * @note @c #ifndef 로 감싼 것은 일부 플랫폼의 자체 헤더가 같은 이름을 정의하기
 *       때문이다. 먼저 도착한 쪽이 이기며, 이는 의도된 것이다.
 * @warning @c M() 은 최상위 비트를 세우므로, 결과가 그 플랫폼에서 @c char 가 부호를
 *          갖는지에 달려 있다. 캐스팅을 명시해 둔 이유다.
 */

#ifndef MICRO_H
#define MICRO_H

#ifndef C
#define C(c) (0x1f & (c))
#endif
#ifndef M
#define M(c) (((char) 0x80) | (c))
#endif
#define ABORT C('a')

#endif /* MICRO_H */
