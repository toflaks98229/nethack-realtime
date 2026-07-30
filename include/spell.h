/* NetHack 5.0	spell.h	$NHDT-Date: 1781973088 2026/06/20 16:31:28 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.21 $ */
/* Copyright 1986, M. Stephenson                                  */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file spell.h
 * @brief What the hero knows of a spell, and how surely.
 *
 * A spell is not simply known or unknown. Knowledge decays from the moment it is
 * learned, so each entry carries how much is left -- which is why the spell list
 * can warn that something is going stale, and why a forgotten spell remains
 * listed rather than disappearing.
 *
 * @note A forgotten spell is a distinct state from an unknown one: it is still
 *       in the book of spells the hero carries in their head, and can be
 *       relearned to full strength.
 * @warning The accessors index the hero's spell list directly, so a caller must
 *          already know the index is in range.
 */

/**
 * @file spell.h
 * @brief 영웅이 어떤 주문을 얼마나 확실히 아는지.
 *
 * 주문은 단순히 알거나 모르는 것이 아니다. 배운 순간부터 지식이 흐려지므로 각 항목은
 * 얼마나 남았는지를 지닌다. 주문 목록이 무언가 흐려지고 있다고 경고할 수 있는 이유이며,
 * 잊은 주문이 사라지지 않고 목록에 남는 이유다.
 *
 * @note 잊은 주문은 모르는 주문과 구별되는 상태다. 영웅이 머릿속에 지닌 주문 목록에
 *       여전히 남아 있고, 다시 배워 온전한 상태로 되돌릴 수 있다.
 * @warning 접근 매크로들은 영웅의 주문 목록을 곧바로 색인한다. 호출자가 색인이 범위
 *          안임을 이미 알고 있어야 한다.
 */

#ifndef SPELL_H
#define SPELL_H

/** @brief Sentinels for "no spell here" and "not a spell the hero knows". */
/** @brief "여기에 주문 없음"과 "영웅이 아는 주문이 아님"을 나타내는 특수값. */
#define NO_SPELL 0
#define UNKNOWN_SPELL (-1)

/* spellbook re-use control; used when reading and when polymorphing */
#define MAX_SPELL_STUDY 3

/**
 * @brief One spell the hero has learned.
 * @note @c sp_id is the spellbook's object type, so a spell and the book that
 *       teaches it are the same identity rather than two tables to keep in step.
 * @note @c sp_know counts down; it is a remaining duration, not a proficiency.
 */
/**
 * @brief 영웅이 배운 주문 하나.
 * @note @c sp_id 는 주문서의 객체 타입이다. 그래서 주문과 그것을 가르치는 책이 서로
 *       맞춰야 할 두 개의 표가 아니라 같은 정체성을 갖는다.
 * @note @c sp_know 는 줄어드는 값이다. 숙련도가 아니라 남은 지속 시간이다.
 */
struct spell {
    short sp_id;  /* spell id (== object.otyp) */
    xint16 sp_lev; /* power level */
    int sp_know;  /* knowledge of spell */
};

/**
 * @brief How well a spell is known, as a state rather than a number.
 * @note @c spe_GoingStale exists so the hero can be warned before a spell is
 *       lost, which is the point of tracking decay at all.
 */
/**
 * @brief 주문을 얼마나 아는지. 숫자가 아니라 상태로 나타낸다.
 * @note @c spe_GoingStale 은 주문을 잃기 전에 영웅에게 경고할 수 있도록 존재한다.
 *       애초에 흐려짐을 추적하는 목적이 그것이다.
 */
enum spellknowledge {
    spe_Forgotten  = -1, /* known but no longer castable */
    spe_Unknown    =  0, /* not yet known */
    spe_Fresh      =  1, /* castable if various casting criteria are met */
    spe_GoingStale =  2  /* still castable but nearly forgotten */
};

/* levels of memory destruction with a scroll of amnesia */
#define ALL_MAP 0x1
#define ALL_SPELLS 0x2

/**
 * @brief Reach into the hero's spell list by index.
 * @param spell Index into the hero's known spells.
 * @note Lvalues, so @c spellknow() is how decay is applied as well as read.
 * @warning No range check. The index must already be known good.
 */
/**
 * @brief 영웅의 주문 목록을 색인으로 참조한다.
 * @param spell 영웅이 아는 주문 목록에서의 색인.
 * @note 좌변값이므로 @c spellknow() 는 읽기뿐 아니라 흐려짐을 적용하는 수단이기도 하다.
 * @warning 범위 검사가 없다. 색인이 유효함을 이미 알고 있어야 한다.
 */
#define decrnknow(spell) svs.spl_book[spell].sp_know--
#define spellid(spell) svs.spl_book[spell].sp_id
#define spellknow(spell) svs.spl_book[spell].sp_know

/**
 * @brief Energy cost of casting a spell of a given level.
 * @param lvl Spell level.
 * @return The energy required.
 * @note Cost is purely a function of level; role and skill affect the chance of
 *       success, not the price.
 */
/**
 * @brief 주어진 등급의 주문을 시전하는 마력 비용.
 * @param lvl 주문 등급.
 * @return 필요한 마력.
 * @note 비용은 등급만의 함수다. 직업과 숙련은 성공 확률에 영향을 주지만 가격에는
 *       영향을 주지 않는다.
 */
/* how much Pw a spell of level lvl costs to cast? */
#define SPELL_LEV_PW(lvl) ((lvl) * 5)

#endif /* SPELL_H */
