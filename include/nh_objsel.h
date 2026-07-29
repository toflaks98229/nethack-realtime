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

/** @brief The called routine already completed the work, so stop iterating. */
/** @brief 호출된 루틴이 이미 작업을 끝냈으므로 반복을 멈추라는 표시. */
/* flags for special ggetobj status returns */
#define ALL_FINISHED 0x01 /* called routine already finished the job */

/**
 * @brief Which objects to offer, how to label them, and what an empty or
 *        cancelled selection means.
 *
 * These decide both the contents of the menu and how its outcome is reported,
 * since a caller often needs to tell "nothing was eligible" apart from "the
 * player declined".
 *
 * @note @c BY_NEXTHERE walks the pile at a map square rather than an
 *       inventory chain; the two use different link fields.
 * @warning @c FEEL_COCKATRICE is not presentational. Listing such an object can
 *          amount to touching it, and this flag engages the petrification
 *          checks that go with that.
 */
/**
 * @brief 어떤 물건을 제시할지, 어떻게 표시할지, 그리고 아무것도 없거나 취소된
 *        선택이 무엇을 뜻하는지.
 *
 * 메뉴의 내용과 결과 보고 방식을 함께 정한다. 호출자는 흔히 "해당하는 물건이
 * 없었다"와 "플레이어가 거절했다"를 구분해야 하기 때문이다.
 *
 * @note @c BY_NEXTHERE 는 소지품 사슬이 아니라 맵 칸에 쌓인 더미를 순회한다.
 *       둘은 서로 다른 연결 필드를 쓴다.
 * @warning @c FEEL_COCKATRICE 는 표시상의 문제가 아니다. 그런 물건을 목록에
 *          올리는 것이 접촉에 해당할 수 있으며, 이 플래그가 그에 따르는 석화
 *          검사를 활성화한다.
 */
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

/**
 * @brief Which groupings to offer when the player picks by category.
 *
 * Categories are not only object classes: unpaid, worn, and blessed/cursed
 * status are equally useful ways to say "all of those", which is what makes
 * bulk operations practical.
 *
 * @warning Shares a flag word with @c query_objlist(). @c BY_NEXTHERE and
 *          @c INCLUDE_VENOM keep their meaning here, which is why these values
 *          begin at 0x0004.
 * @note @c BUC_ALLBKNOWN and @c BUCX_TYPES are unions of the individual
 *       blessed/cursed bits, not additional categories.
 */
/**
 * @brief 플레이어가 분류 단위로 고를 때 어떤 묶음을 제시할지.
 *
 * 분류는 물건의 종류만이 아니다. 미지불, 착용 중, 축복/저주 상태 역시 "그것들
 * 전부"를 뜻하는 유용한 기준이며, 그 덕분에 일괄 작업이 실용적이 된다.
 *
 * @warning @c query_objlist() 와 플래그 워드를 공유한다. @c BY_NEXTHERE 와
 *          @c INCLUDE_VENOM 이 여기서도 같은 의미를 유지하므로, 이 값들은
 *          0x0004 부터 시작한다.
 * @note @c BUC_ALLBKNOWN 과 @c BUCX_TYPES 는 개별 축복/저주 비트들의 합집합이며,
 *       추가적인 분류가 아니다.
 */
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

/**
 * @brief What order to list objects in, and what to bring to the front.
 * @note @c SORTLOOT_INUSE floats worn, wielded, and lit items to the top,
 *       which is usually what the player is looking for in a long inventory.
 * @warning @c SORTLOOT_PETRIFY overrides the caller's filter so that
 *          cockatrice corpses are still listed; it exists for safety, not
 *          convenience.
 */
/**
 * @brief 물건을 어떤 순서로 나열할지, 그리고 무엇을 앞으로 끌어올릴지.
 * @note @c SORTLOOT_INUSE 는 착용 중이거나 들고 있거나 불이 붙은 물건을 위로
 *       올린다. 소지품이 길 때 플레이어가 대개 찾는 것이 그것이기 때문이다.
 * @warning @c SORTLOOT_PETRIFY 는 호출자의 필터를 무시하고 코카트리스 시체를
 *          목록에 남긴다. 편의가 아니라 안전을 위한 것이다.
 */
/* control flags for sortloot() */
#define SORTLOOT_PACK   0x01
#define SORTLOOT_INVLET 0x02
#define SORTLOOT_LOOT   0x04
#define SORTLOOT_INUSE  0x08 /* for inventory, in-use items first */
#define SORTLOOT_PETRIFY 0x20 /* override filter func for c-trice corpses */

#endif /* NH_OBJSEL_H */
