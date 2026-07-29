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

/** @brief Number of mask bytes describing a bubble's shape. */
/** @brief 거품의 형태를 서술하는 마스크 바이트 수. */
#define MAX_BMASK 4

/**
 * @brief One drifting pocket of habitable space, and what rides inside it.
 *
 * A bubble is stored by its upper-left corner plus a bitmask giving its shape,
 * rather than as a rectangle, because bubbles are irregular. It is linked in
 * both directions since collision handling walks the list from either end.
 *
 * @note @c dx and @c dy are a general heading, not a per-turn displacement;
 *       the actual drift is derived from them.
 * @warning Everything in @c cons moves with the bubble. Repositioning a bubble
 *          without carrying its contents leaves monsters and objects stranded
 *          outside the habitable area.
 */
/**
 * @brief 떠다니는 거주 가능 공간 하나와 그 안에 실린 것들.
 *
 * 거품은 불규칙한 모양이므로 사각형이 아니라 좌상단 모서리와 형태를 나타내는
 * 비트마스크로 저장한다. 충돌 처리가 목록의 양쪽 끝에서 순회하므로 양방향으로
 * 연결된다.
 *
 * @note @c dx 와 @c dy 는 턴당 이동량이 아니라 대략적인 진행 방향이며, 실제
 *       표류는 이로부터 유도된다.
 * @warning @c cons 안의 모든 것이 거품과 함께 이동한다. 내용물을 함께 옮기지
 *          않고 거품만 이동시키면 몬스터와 물건이 거주 가능 영역 밖에 고립된다.
 */
struct bubble {
    coordxy x, y;   /* coordinates of the upper left corner */
    schar dx, dy; /* the general direction of the bubble's movement */
    uchar bm[MAX_BMASK + 2];    /* bubble bit mask */
    struct bubble *prev, *next; /* need to traverse the list up and down */
    struct container *cons;
};

/**
 * @brief What a container node's untyped list actually points at.
 * @note This is the tag that makes @c struct container's @c genericptr_t safe
 *       to dereference; nothing else records the type.
 */
/**
 * @brief 컨테이너 노드의 타입 없는 목록이 실제로 가리키는 대상.
 * @note @c struct container 의 @c genericptr_t 를 안전하게 역참조할 수 있게
 *       해 주는 태그다. 타입을 기록하는 다른 수단은 없다.
 */
enum bubble_contains_types {
    CONS_OBJ = 0,
    CONS_MON,
    CONS_HERO,
    CONS_TRAP
};

/**
 * @brief One thing riding inside a bubble, with its own position.
 *
 * Deliberately untyped: a single list can hold objects, monsters, traps, and
 * the hero together, which is what lets bubble movement relocate all of them
 * in one pass.
 *
 * @warning @c list is a @c genericptr_t; cast it according to @c what. Reading
 *          it as the wrong kind is unchecked.
 */
/**
 * @brief 거품 안에 실린 것 하나와 그 자신의 위치.
 *
 * 의도적으로 타입을 두지 않았다. 하나의 목록이 물건, 몬스터, 함정, 영웅을 함께
 * 담을 수 있어야 거품 이동이 한 번의 순회로 그 모두를 옮길 수 있기 때문이다.
 *
 * @warning @c list 는 @c genericptr_t 다. @c what 에 따라 캐스팅할 것. 잘못된
 *          종류로 읽는 것은 검사되지 않는다.
 */
struct container {
    struct container *next;
    coordxy x, y;
    short what;
    genericptr_t list;
};

#endif /* NH_BUBBLE_H */
