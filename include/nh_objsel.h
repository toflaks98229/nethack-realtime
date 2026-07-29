/* NetHack 5.0	nh_objsel.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Consolidated 2026-07 from hack.h for the real-time fork (see
   MODIFICATIONS.md).  Content is unchanged; gathered at the position of the
   earliest constituent block, so every definition is available no later than
   before. */

/**
 * @file nh_objsel.h
 * @brief Building and ordering the menus from which the player picks objects.
 *
 * Choosing an object is not one operation but three: deciding which objects to
 * offer, deciding whether to offer whole categories instead of individual
 * items, and deciding what order to present them in. The flags here control all
 * three, and they are what distinguish "pick something from the floor" from
 * "pick something to sell" or "pick all your cursed items".
 *
 * @warning @c query_objlist() and @c query_category() share a flag word:
 *          @c BY_NEXTHERE and @c INCLUDE_VENOM mean the same thing to both, so
 *          the category flags deliberately start at 0x0004.
 * @note @c FEEL_COCKATRICE is not cosmetic -- listing such an object can touch
 *       it, so this flag engages the petrification checks.
 */

/**
 * @file nh_objsel.h
 * @brief 플레이어가 물건을 고르는 메뉴의 구성과 정렬.
 *
 * 물건 선택은 하나의 동작이 아니라 셋이다. 어떤 물건을 제시할지, 개별 물품
 * 대신 분류 단위로 제시할지, 그리고 어떤 순서로 보여 줄지다. 여기 플래그들이
 * 그 셋을 모두 제어하며, "바닥에서 무언가 고르기"와 "팔 물건 고르기",
 * "저주받은 물건 모두 고르기"를 구분 짓는 것이 바로 이 값들이다.
 *
 * @warning @c query_objlist() 와 @c query_category() 는 플래그 워드를 공유한다.
 *          @c BY_NEXTHERE 와 @c INCLUDE_VENOM 은 양쪽에서 같은 의미이므로,
 *          분류 플래그는 의도적으로 0x0004 부터 시작한다.
 * @note @c FEEL_COCKATRICE 는 표시상의 문제가 아니다. 그런 물건을 목록에 올리는
 *       행위가 접촉이 될 수 있으므로, 이 플래그가 석화 검사를 활성화한다.
 */

#ifndef NH_OBJSEL_H
#define NH_OBJSEL_H

/* flags for special ggetobj status returns */
#define ALL_FINISHED 0x01 /* called routine already finished the job */

/* flags to control query_objlist() */
#define BY_NEXTHERE       0x0001 /* follow objlist by nexthere field */
#define INCLUDE_VENOM     0x0002 /* include venom objects if present */
#define AUTOSELECT_SINGLE 0x0004 /* if only 1 object, don't ask */
#define USE_INVLET        0x0008 /* use object's invlet */
#define INVORDER_SORT     0x0010 /* sort objects by packorder */
#define SIGNAL_NOMENU     0x0020 /* return -1 rather than 0 if none allowed */
#define SIGNAL_ESCAPE     0x0040 /* return -2 rather than 0 for ESC */
#define FEEL_COCKATRICE   0x0080 /* engage cockatrice checks and react */
#define INCLUDE_HERO      0x0100 /* show hero among engulfer's inventory */

/* Flags to control query_category() */
/* BY_NEXTHERE and INCLUDE_VENOM are used by query_category() too, so
   skip 0x0001 and 0x0002 */
#define UNPAID_TYPES      0x0004
#define GOLD_TYPES        0x0008
#define WORN_TYPES        0x0010
#define ALL_TYPES         0x0020
#define BILLED_TYPES      0x0040
#define CHOOSE_ALL        0x0080
#define BUC_BLESSED       0x0100
#define BUC_CURSED        0x0200
#define BUC_UNCURSED      0x0400
#define BUC_UNKNOWN       0x0800
#define JUSTPICKED        0x1000
#define BUC_ALLBKNOWN (BUC_BLESSED | BUC_CURSED | BUC_UNCURSED)
#define BUCX_TYPES (BUC_ALLBKNOWN | BUC_UNKNOWN)
#define ALL_TYPES_SELECTED -2

/* control flags for sortloot() */
#define SORTLOOT_PACK   0x01
#define SORTLOOT_INVLET 0x02
#define SORTLOOT_LOOT   0x04
#define SORTLOOT_INUSE  0x08 /* for inventory, in-use items first */
#define SORTLOOT_PETRIFY 0x20 /* override filter func for c-trice corpses */

#endif /* NH_OBJSEL_H */
