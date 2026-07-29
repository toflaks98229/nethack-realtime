/* NetHack 5.0	nh_makemon.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_makemon.h
 * @brief Flags controlling monster creation and candidate-position checks.
 *
 * A single flag word serves both @c makemon(), which decides how a new monster
 * is built (its inventory, sex, extension structures, whether it announces
 * itself), and @c goodpos(), which decides whether a square is an acceptable
 * place to put one. The @c MM_ bits belong to the former, the @c GP_ bits to
 * the latter, and they share one numbering space so a single argument can
 * carry both.
 *
 * @warning The @c MM_ and @c GP_ values must not collide: they are consecutive
 *          bits in one word (25 bits used). Adding an @c MM_ flag means
 *          renumbering the @c GP_ flags above it.
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_makemon.h
 * @brief 몬스터 생성과 배치 위치 검사를 제어하는 플래그.
 *
 * 하나의 플래그 워드가 두 곳에 쓰인다. 새 몬스터를 어떻게 만들지(소지품, 성별,
 * 확장 구조체, 등장 알림 여부) 결정하는 @c makemon() 과, 어떤 칸이 몬스터를
 * 놓기에 적합한지 판단하는 @c goodpos() 다. @c MM_ 비트는 전자에, @c GP_ 비트는
 * 후자에 속하며, 인자 하나로 둘 다 전달할 수 있도록 번호 공간을 공유한다.
 *
 * @warning @c MM_ 와 @c GP_ 값은 충돌해서는 안 된다. 한 워드 안의 연속된
 *          비트이며(25비트 사용), @c MM_ 플래그를 추가하려면 그 위의 @c GP_
 *          플래그들을 다시 번호 매겨야 한다.
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_MAKEMON_H
#define NH_MAKEMON_H

typedef uint32_t mmflags_nht;     /* makemon MM_ flags */


/* flags to control makemon(); goodpos() uses some plus has some of its own*/
#define NO_MM_FLAGS     0x00000000L /* use this rather than plain 0 */
#define NO_MINVENT      0x00000001L /* suppress minvent when creating mon */
#define MM_NOWAIT       0x00000002L /* don't set STRAT_WAITMASK flags */
#define MM_NOCOUNTBIRTH 0x00000004L /* don't incr born count (for revival) */
#define MM_IGNOREWATER  0x00000008L /* ignore water when positioning */
#define MM_ADJACENTOK   0x00000010L /* ok to use adjacent coordinates */
#define MM_ANGRY        0x00000020L /* monster is created angry */
#define MM_NONAME       0x00000040L /* monster is not christened */
#define MM_EGD          0x00000080L /* add egd structure */
#define MM_EPRI         0x00000100L /* add epri structure */
#define MM_ESHK         0x00000200L /* add eshk structure */
#define MM_EMIN         0x00000400L /* add emin structure */
#define MM_EDOG         0x00000800L /* add edog structure */
#define MM_ASLEEP       0x00001000L /* monsters should be generated asleep */
#define MM_NOGRP        0x00002000L /* suppress creation of monster groups */
#define MM_NOTAIL       0x00004000L /* if a long worm, don't give it a tail */
#define MM_MALE         0x00008000L /* male variation */
#define MM_FEMALE       0x00010000L /* female variation */
#define MM_NOMSG        0x00020000L /* no appear message */
#define MM_NOEXCLAM     0x00040000L /* more sedate "<mon> appears."
                                     * mesg for ^G */
#define MM_IGNORELAVA   0x00080000L /* ignore lava when positioning */
#define MM_MINVIS       0x00100000L /* for ^G/create_particular */
/* if more MM_ flag masks are added, skip or renumber the GP_ one(s) */
#define GP_ALLOW_XY     0x00200000L /* [actually used by enexto() to decide
                                     * whether to make an extra call to
                                     * goodpos()] */
#define GP_ALLOW_U      0x00400000L /* don't reject hero's location */
#define GP_CHECKSCARY   0x00800000L /* check monster for onscary() */
#define GP_AVOID_MONPOS 0x01000000L /* don't accept existing mon location */
/* 25 bits used */

#endif /* NH_MAKEMON_H */
