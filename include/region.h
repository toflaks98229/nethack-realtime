/* NetHack 5.0	region.h	$NHDT-Date: 1781973086 2026/06/20 16:31:26 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.21 $ */
/* Copyright (c) 1996 by Jean-Christophe Collet                   */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file region.h
 * @brief An area that acts on whoever is in it, described as a set of
 *        rectangles.
 *
 * A region is not a property of squares. It is a thing with a shape, a lifetime
 * and behaviour, so a gas cloud can drift, expire, and be blamed for a death
 * after it has gone.
 *
 * The shape is a list of rectangles with a bounding box in front of it, so the
 * common answer -- "not even close" -- costs one comparison, and the exact test
 * only runs for positions that could plausibly be inside.
 *
 * @note The behaviours are stored as small indices rather than function
 *       pointers, because a region is saved and a pointer could not be written;
 *       the index is resolved back to a function on restore.
 * @warning The comment below is load-bearing: the save and restore code writes
 *          these fields individually, so changing the layout without updating
 *          both leaves a save that reads as nonsense rather than failing.
 */

/**
 * @file region.h
 * @brief 안에 있는 자에게 작용하는 영역. 직사각형들의 집합으로 기술된다.
 *
 * 영역은 칸의 속성이 아니다. 형태와 수명, 행동을 지닌 하나의 사물이다. 그래서 가스
 * 구름이 떠다니고, 만료되고, 사라진 뒤에도 죽음의 원인으로 지목될 수 있다.
 *
 * 형태는 직사각형 목록이며 그 앞에 경계 상자가 있다. 그래서 가장 흔한 답인 "전혀
 * 아니다"가 비교 한 번으로 끝나고, 정확한 검사는 안에 있을 법한 위치에 대해서만
 * 실행된다.
 *
 * @note 행동들은 함수 포인터가 아니라 작은 색인으로 저장된다. 영역은 저장되는데
 *       포인터는 쓸 수 없기 때문이며, 복원 시 색인이 다시 함수로 해석된다.
 * @warning 아래 주석은 동작을 좌우한다. 저장·복원 코드가 이 필드들을 하나하나 쓰므로,
 *          양쪽을 함께 고치지 않고 배치를 바꾸면 실패하는 대신 엉뚱하게 읽히는 저장이
 *          남는다.
 */

#ifndef REGION_H
#define REGION_H

/**
 * @brief A region's behaviour, called with the region and the affected party.
 * @note Both arguments are untyped because the affected party may be the hero or
 *       a monster; the callback knows which it expects.
 */
/**
 * @brief 영역의 행동. 영역과 영향을 받는 대상을 받아 호출된다.
 * @note 두 인자가 모두 타입이 없는 것은 영향을 받는 대상이 영웅일 수도 몬스터일 수도
 *       있기 때문이다. 어느 쪽을 기대하는지는 콜백이 안다.
 */
/* generic callback function */

typedef boolean (*callback_proc)(genericptr_t, genericptr_t);

/*
 * player_flags
 */
#define REG_HERO_INSIDE 0x01
#define REG_NOT_HEROS 0x02
#define hero_inside(r) ((r)->player_flags & REG_HERO_INSIDE)
#define heros_fault(r) (!((r)->player_flags & REG_NOT_HEROS))
#define set_hero_inside(r) ((r)->player_flags |= REG_HERO_INSIDE)
#define clear_hero_inside(r) ((r)->player_flags &= ~REG_HERO_INSIDE)
#define set_heros_fault(r) ((r)->player_flags &= ~REG_NOT_HEROS)
#define clear_heros_fault(r) ((r)->player_flags |= REG_NOT_HEROS)

/*
 * Note: if you change the size/type of any of the fields below,
 *       or add any/remove any fields, you must update the
 *       bwrite() calls in save_regions(), and the
 *       mread() calls in rest_regions() in src/region.c
 *       to reflect the changes.
 */

typedef struct {
    NhRect bounding_box;   /* Bounding box of the region */
    NhRect *rects;         /* Rectangles composing the region */
    short nrects;          /* Number of rectangles  */
    boolean attach_2_u;    /* Region attached to player ? */
    unsigned attach_2_m;   /* Region attached to monster ? */
    /*struct obj *attach_2_o;*/ /* Region attached to object ? UNUSED YET */
    const char *enter_msg; /* Message when entering */
    const char *leave_msg; /* Message when leaving */
    long ttl;              /* Time to live. -1 is forever */
    short expire_f;        /* Function to call when region's ttl expire */
    short can_enter_f;     /* Function to call to check whether the player
                            * can, or can not, enter the region */
    short enter_f;         /* Function to call when the player enters*/
    short can_leave_f;     /* Function to call to check whether the player
                            * can, or can not, leave the region */
    short leave_f;         /* Function to call when the player leaves */
    short inside_f;        /* Function to call every turn if player's inside */
    unsigned player_flags; /* (see above) */
    unsigned *monsters;    /* Monsters currently inside this region */
    short n_monst;         /* Number of monsters inside this region */
    short max_monst;       /* Maximum number of monsters that can be
                            * listed without having to grow the array */
#define MONST_INC 5

    /* Should probably do the same thing about objects */

    boolean visible;       /* Is the region visible? */
    int glyph;             /* Which glyph to use if visible */
    anything arg;          /* Optional user argument (Ex: strength of
                            * force field, damage of a fire zone, ...*/
} NhRegion;

#endif /* REGION_H */
