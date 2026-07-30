/* NetHack 5.0	vision.h	$NHDT-Date: 1781973090 2026/06/20 16:31:30 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.20 $ */
/* Copyright (c) Dean Luick, with acknowledgements to Dave Cohrs, 1990. */
/* NetHack may be freely redistributed.  See license for details.       */

/**
 * @file vision.h
 * @brief Whether a square can be seen, asked in the several ways it is meant.
 *
 * Line of sight and visibility are different questions. A dark room is in the
 * hero's line of sight but not seen; a lit corridor around a corner is neither.
 * Keeping them apart is what lets a light source reveal what was already within
 * reach of the eye.
 *
 * The answers are precomputed into an array and read through these macros, so the
 * rest of the game asks rather than recalculates.
 *
 * @note The array is indexed row-first while the macros take x then y, which is
 *       why callers should use the macros rather than the array.
 * @warning These concern sight only. Telepathy, warning and detection give
 *          knowledge without it, so "the hero knows" is a broader question
 *          answered elsewhere.
 */

/**
 * @file vision.h
 * @brief 어떤 칸을 볼 수 있는지를, 그 질문이 뜻하는 여러 방식으로 묻기.
 *
 * 시선과 가시성은 서로 다른 질문이다. 어두운 방은 영웅의 시선 안에 있지만 보이지 않고,
 * 모퉁이를 돈 밝은 복도는 둘 다 아니다. 이 둘을 구분해 두는 것이, 광원이 이미 눈이
 * 닿는 범위에 있던 것을 드러낼 수 있게 한다.
 *
 * 답은 미리 계산되어 배열에 담기고 이 매크로들을 통해 읽힌다. 그래서 게임의 나머지
 * 부분은 다시 계산하지 않고 묻는다.
 *
 * @note 배열은 행 우선으로 색인되지만 매크로는 x 다음 y 를 받는다. 호출자가 배열이
 *       아니라 매크로를 써야 하는 이유다.
 * @warning 이것들은 시각에 관한 것뿐이다. 텔레파시와 경고, 탐지는 시각 없이 앎을
 *          주므로, "영웅이 안다"는 더 넓은 질문이며 다른 곳에서 답한다.
 */

#ifndef VISION_H
#define VISION_H

/**
 * @brief What is known about a square's visibility.
 * @note @c COULD_SEE is about line of sight and @c IN_SIGHT about actually
 *       seeing; a square can have the former without the latter, which is what
 *       darkness means here.
 */
/**
 * @brief 어떤 칸의 가시성에 대해 알려진 것.
 * @note @c COULD_SEE 는 시선에 관한 것이고 @c IN_SIGHT 는 실제로 보이는지에 관한
 *       것이다. 앞의 것만 있고 뒤의 것이 없을 수 있으며, 여기서 어둠이 뜻하는 바가
 *       그것이다.
 */
#define COULD_SEE 0x1 /* location could be seen, if it were lit */
#define IN_SIGHT 0x2  /* location can be seen */
#define TEMP_LIT 0x4  /* location is temporarily lit */

/*
 * Light source sources
 */
enum ls_sources {
    LS_NONE = 0,
    LS_OBJECT = 1,
    LS_MONSTER = 2,
    NUM_LS_SOURCES
};

/*
 *  cansee()    - Returns true if the hero can see the location.
 *
 *  couldsee()  - Returns true if the hero has a clear line of sight to
 *                the location.
 */
#define cansee(x, y) ((gv.viz_array[y][x] & IN_SIGHT) != 0)
#define couldsee(x, y) ((gv.viz_array[y][x] & COULD_SEE) != 0)
#define templit(x, y) ((gv.viz_array[y][x] & TEMP_LIT) != 0)

/*
 *  The following assume the monster is not blind.
 *
 *  m_cansee()  - Returns true if the monster can see the given location.
 *
 *  m_canseeu() - Returns true if the monster could see the hero.  Assumes
 *                that if the hero has a clear line of sight to the monster's
 *                location and the hero is visible, then monster can see the
 *                hero.
 */
#define m_cansee(mtmp, x2, y2) clear_path((mtmp)->mx, (mtmp)->my, (x2), (y2))

#if 0
#define m_canseeu(m) \
    ((!Invis || perceives((m)->data))                      \
     && !(Underwater || u.uburied || (m)->mburied)         \
     && couldsee((m)->mx, (m)->my))
#else   /* without 'uburied' and 'mburied' */
#define m_canseeu(m) \
    ((!Invis || perceives((m)->data))                      \
     && !Underwater                                        \
     && couldsee((m)->mx, (m)->my))
#endif

/*
 *  Circle information
 */
#define MAX_RADIUS 15 /* this is in points from the source */

/* Use this macro to get a list of distances of the edges (see vision.c). */
#define circle_ptr(z) (&circle_data[(int) circle_start[z]])

/* howmonseen() bitmask values */
#define MONSEEN_NORMAL   0x0001 /* normal vision */
#define MONSEEN_SEEINVIS 0x0002 /* seeing invisible */
#define MONSEEN_INFRAVIS 0x0004 /* via infravision */
#define MONSEEN_TELEPAT  0x0008 /* via telepathy */
#define MONSEEN_XRAYVIS  0x0010 /* via Xray vision */
#define MONSEEN_DETECT   0x0020 /* via extended monster detection */
#define MONSEEN_WARNMON  0x0040 /* via type-specific warning */

#endif /* VISION_H */
