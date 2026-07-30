/* NetHack 5.0	attrib.h	$NHDT-Date: 1781973077 2026/06/20 16:31:17 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.18 $ */
/* Copyright 1988, Mike Stephenson                                */
/* NetHack may be freely redistributed.  See license for details. */

/*      attrib.h - Header file for character class processing. */

/**
 * @file attrib.h
 * @brief The hero's six attributes, and the several values each one has at once.
 *
 * An attribute is not a single number. There is what it naturally is, what it is
 * after temporary changes, the maximum this hero may reach, and the exercise
 * accumulated toward changing it -- so code must be explicit about which of
 * those it means.
 *
 * @note The attributes are ordered so that a random one can be drawn by index,
 *       which is why @c A_MAX is a count rather than a value.
 */

/**
 * @file attrib.h
 * @brief 영웅의 여섯 능력치와, 각각이 동시에 지니는 여러 값.
 *
 * 능력치는 하나의 숫자가 아니다. 본래의 값, 일시적 변화가 반영된 값, 이 영웅이
 * 도달할 수 있는 최대치, 그리고 그것을 바꾸기 위해 쌓인 단련이 있다. 그래서 코드는
 * 그중 무엇을 뜻하는지 분명히 해야 한다.
 *
 * @note 능력치는 색인으로 무작위 하나를 뽑을 수 있도록 정렬되어 있다. @c A_MAX 가
 *       값이 아니라 개수인 이유다.
 */

#ifndef ATTRIB_H
#define ATTRIB_H

/**
 * @brief The six attributes, in the order they are indexed by.
 * @note @c A_MAX is the count and is used as the bound when choosing one at
 *       random; it is not itself an attribute.
 */
/**
 * @brief 여섯 능력치. 색인되는 순서대로다.
 * @note @c A_MAX 는 개수이며 무작위로 하나를 고를 때 상한으로 쓰인다. 그 자체가
 *       능력치는 아니다.
 */
enum attrib_types {
    A_STR = 0,
    A_INT,
    A_WIS,
    A_DEX,
    A_CON,
    A_CHA,

    A_MAX /* used in rn2() selection of attrib */
};

#define ABASE(x) (u.acurr.a[x])
#define ABON(x) (u.abon.a[x])
#define AEXE(x) (u.aexe.a[x])
#define ACURR(x) (acurr(x))
#define ACURRSTR (acurrstr())
/* should be: */
/* #define ACURR(x) (ABON(x) + ATEMP(x) + (Upolyd  ? MBASE(x) : ABASE(x)) */
#define MCURR(x) (u.macurr.a[x])
#define AMAX(x) (u.amax.a[x])
#define MMAX(x) (u.mamax.a[x])

#define ATEMP(x) (u.atemp.a[x])
#define ATIME(x) (u.atime.a[x])

/* KMH -- Conveniences when dealing with strength constants */
#define STR18(x) (18 + (x))  /* 18/xx */
#define STR19(x) (100 + (x)) /* For 19 and above */

struct attribs {
    schar a[A_MAX];
};

#define ATTRMAX(x) \
    ((x == A_STR && Upolyd) ? uasmon_maxStr() : gu.urace.attrmax[x])
#define ATTRMIN(x) (gu.urace.attrmin[x])

#endif /* ATTRIB_H */
