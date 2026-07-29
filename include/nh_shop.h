/* NetHack 5.0	nh_shop.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Consolidated 2026-07 from hack.h for the real-time fork (see
   MODIFICATIONS.md).  Content is unchanged.  Unlike the earlier extractions,
   these blocks were scattered across hack.h; they are gathered here and
   included at the position of the *earliest* of them, so every definition
   becomes available no later than it was before. */

/**
 * @file nh_shop.h
 * @brief Shopkeeper billing: what the hero owes, and why.
 *
 * A shopkeeper charges not only for items carried out but for damage done and
 * for value destroyed in place. These definitions cover all three: the selling
 * disposition of an object being dropped, the price list for structural damage,
 * the catalogue of ways an object can be devalued while still owned by the
 * shop, and the repossession context used when the bill goes unpaid.
 *
 * @warning @c enum cost_alteration_types must stay in step with
 *          @c costly_alteration() in @c mkobj.c; the enum is the index into
 *          that function's parallel handling.
 * @note @c SHOP_WALL_DMG is not a constant: it scales with the hero's current
 *       strength, and is evaluated where it is used.
 */

/**
 * @file nh_shop.h
 * @brief 상점 주인의 청구: 영웅이 무엇을, 왜 빚지는가.
 *
 * 상점 주인은 들고 나간 물건뿐 아니라 입힌 손상과 그 자리에서 파괴한 가치에
 * 대해서도 값을 매긴다. 여기 정의들은 그 셋을 모두 다룬다. 내려놓는 물건의
 * 판매 처리 방식, 구조물 손상에 대한 가격표, 상점 소유인 채로 물건의 가치가
 * 떨어질 수 있는 경우들의 목록, 그리고 대금이 지불되지 않았을 때 쓰이는 회수
 * 컨텍스트다.
 *
 * @warning @c enum cost_alteration_types 는 @c mkobj.c 의
 *          @c costly_alteration() 과 항상 일치해야 한다. 이 열거값이 해당
 *          함수의 대응 처리에 대한 색인이기 때문이다.
 * @note @c SHOP_WALL_DMG 는 상수가 아니다. 영웅의 현재 힘에 비례하며 사용
 *       시점에 평가된다.
 */

#ifndef NH_SHOP_H
#define NH_SHOP_H

/* sellobj_state() states */
#define SELL_NORMAL (0)
#define SELL_DELIBERATE (1)
#define SELL_DONTSELL (2)

#define SHOP_DOOR_COST 400L /* cost of a destroyed shop door */
#define SHOP_BARS_COST 300L /* cost of iron bars */
#define SHOP_HOLE_COST 200L /* cost of making hole/trapdoor */
#define SHOP_WALL_COST 200L /* cost of destroying a wall */
#define SHOP_WALL_DMG  (10L * ACURRSTR) /* damaging a wall */
#define SHOP_PIT_COST  100L /* cost of making a pit */
#define SHOP_WEB_COST   30L /* cost of removing a web */

/* alteration types--keep in synch with costly_alteration(mkobj.c) */
enum cost_alteration_types {
    COST_CANCEL  =  0, /* standard cancellation */
    COST_DRAIN   =  1, /* drain life upon an object */
    COST_UNCHRG  =  2, /* cursed charging */
    COST_UNBLSS  =  3, /* unbless (devalues holy water) */
    COST_UNCURS  =  4, /* uncurse (devalues unholy water) */
    COST_DECHNT  =  5, /* disenchant weapons or armor */
    COST_DEGRD   =  6, /* removal of rustproofing, dulling via engraving */
    COST_DILUTE  =  7, /* potion dilution */
    COST_ERASE   =  8, /* scroll or spellbook blanking */
    COST_BURN    =  9, /* dipped into flaming oil */
    COST_NUTRLZ  = 10, /* neutralized via unicorn horn */
    COST_DSTROY  = 11, /* wand breaking (bill first, useup later) */
    COST_SPLAT   = 12, /* cream pie to own face (ditto) */
    COST_BITE    = 13, /* start eating food */
    COST_OPEN    = 14, /* open tin */
    COST_BRKLCK  = 15, /* break box/chest's lock */
    COST_RUST    = 16, /* rust damage */
    COST_ROT     = 17, /* rotting attack */
    COST_CORRODE = 18, /* acid damage */
    COST_CRACK   = 19, /* damage to crystal armor */
};

/* used by unpaid_cost(shk.h) */
enum unpaid_cost_flags {
    COST_NOCONTENTS = 0,
    COST_CONTENTS   = 1,
    COST_SINGLEOBJ  = 2,
};

struct repo { /* repossession context */
    struct monst *shopkeeper;
    coord location;
};

#endif /* NH_SHOP_H */
