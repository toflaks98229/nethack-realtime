/* NetHack 5.0	nh_bubble.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Consolidated 2026-07 from hack.h for the real-time fork (see
   MODIFICATIONS.md).  Content is unchanged; gathered at the position of the
   earliest constituent block, so every definition is available no later than
   before. */

/**
 * @file nh_bubble.h
 * @brief Drifting bubbles and their contents on the Plane of Water and Air.
 *
 * The endgame water and air levels are not static maps: the passable area is a
 * set of bubbles that drift, and whatever occupies a bubble -- objects,
 * monsters, traps, the hero -- must travel with it. A bubble therefore carries
 * a bitmask describing its shape plus a list of the things riding inside it.
 *
 * @note @c struct container is generic (it holds a @c genericptr_t list and a
 *       @c what tag), and @c enum bubble_contains_types names the kinds that
 *       tag can take.
 */

/**
 * @file nh_bubble.h
 * @brief 물·공기의 세계에서 떠다니는 거품과 그 내용물.
 *
 * 엔드게임의 물·공기 레벨은 정적인 지도가 아니다. 통행 가능한 영역은 떠다니는
 * 거품들의 집합이며, 거품 안에 있는 것 -- 물건, 몬스터, 함정, 영웅 -- 은 거품을
 * 따라 함께 이동해야 한다. 그래서 거품은 자신의 형태를 나타내는 비트마스크와
 * 안에 실린 것들의 목록을 함께 지닌다.
 *
 * @note @c struct container 는 범용 구조체이며(@c genericptr_t 목록과 @c what
 *       태그를 가짐), @c enum bubble_contains_types 가 그 태그에 올 수 있는
 *       종류를 정의한다.
 */

#ifndef NH_BUBBLE_H
#define NH_BUBBLE_H

#define MAX_BMASK 4

struct bubble {
    coordxy x, y;   /* coordinates of the upper left corner */
    schar dx, dy; /* the general direction of the bubble's movement */
    uchar bm[MAX_BMASK + 2];    /* bubble bit mask */
    struct bubble *prev, *next; /* need to traverse the list up and down */
    struct container *cons;
};

enum bubble_contains_types {
    CONS_OBJ = 0,
    CONS_MON,
    CONS_HERO,
    CONS_TRAP
};

struct container {
    struct container *next;
    coordxy x, y;
    short what;
    genericptr_t list;
};

#endif /* NH_BUBBLE_H */
