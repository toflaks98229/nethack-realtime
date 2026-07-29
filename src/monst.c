/* NetHack 5.0	monst.c	$NHDT-Date: 1781973056 2026/06/20 16:30:56 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.106 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file monst.c
 * @brief 게임 내 모든 몬스터 종류(permonst)의 정적 정의 테이블을 구성한다.
 *
 * 지역 매크로(@c MON, @c LVL, @c SIZ, @c ATTK 등)와 @c monsters.h 포함을 통해
 * 몬스터 초기 테이블(@c mons_init)을 만들고, 게임 시작 시 이를 런타임 전역
 * 배열(@c mons)로 복사한다. 배열 끝에는 종료용(terminator) 항목을 둔다.
 *
 * @note 테이블 구성용 매크로의 @c #define / @c #undef 순서가 필수적이므로
 *       선언 재배치를 적용하지 않는다.
 */

#include "config.h"
#include "weight.h"
#include "permonst.h"
#include "wintype.h"
#include "sym.h"

#include "color.h"

extern const struct attack c_sa_yes[NATTK];
extern const struct attack c_sa_no[NATTK];

/** @brief 공격 없음을 나타내는 빈 공격 초기화자. */
#define NO_ATTK { 0, 0, 0, 0 }

/**
 * @brief 단일 이름을 갖는 몬스터 종류 항목을 구성하는 매크로.
 * @note @c bn 은 종 번호(@c PM_##bn)로 전개되어 permonst 항목을 초기화한다.
 */
/* monster type with single name */
#define MON(nam, sym, lvl, gen, atk, siz, mr1, mr2, \
            flg1, flg2, flg3, d, col, bn)           \
    {                                                                   \
        nam, PM_##bn,                                                   \
        sym, lvl, gen, atk, siz, mr1, mr2, flg1, flg2, flg3, d, col     \
    }

/* LVL() and SIZ() collect several fields to cut down on number of args
 * for MON().  Using more than 15 would fail to conform to the C Standard.
 * ATTK() and A() are to avoid braces and commas within args to MON().
 * NAM() and NAMS() are used for both reasons.
 */
#define NAM(name) { (const char *) 0, (const char *) 0, name }
#define NAMS(namm, namf, namn) { namm, namf, namn }
#define LVL(lvl, mov, ac, mr, aln) lvl, mov, ac, mr, aln
#define SIZ(wt, nut, snd, siz) wt, nut, snd, siz
#define ATTK(at, ad, n, d) { at, ad, n, d }
#define A(a1, a2, a3, a4, a5, a6) { a1, a2, a3, a4, a5, a6 }

/** @brief 몬스터 종류 초기 테이블. 런타임 복사의 원본이며 종료 항목을 포함한다. */
static struct permonst mons_init[NUMMONS + 1] = {
#include "monsters.h"
    /*
     * Array terminator, added to the end of the entries in monsters.h.
     *
     * mons[NUMMONS] used to be all zero except "" instead of Null for
     * the name field.  Then the index field was added and the terminator
     * uses NON_PM for that.  Now, a few monster flags also get set.
     */
#undef MON
#define MON(nam, sym, lvl, gen, atk, siz, mr1, mr2, \
            flg1, flg2, flg3, d, col, bn)           \
    {                                                                   \
        nam, NON_PM,                                                    \
        sym, lvl, gen, atk, siz, mr1, mr2, flg1, flg2, flg3, d, col     \
    }
    MON(NAM(""), 0,
        LVL(0, 0, 0, 0, 0), G_NOGEN | G_NOCORPSE,
        A(NO_ATTK, NO_ATTK, NO_ATTK, NO_ATTK, NO_ATTK, NO_ATTK),
        SIZ(0, 0, 0, 0), 0, 0,
        0L,  M2_NOPOLY, 0,
        0, 0, 0),
};

#undef MON
#undef NAM
#undef NAMS

void monst_globals_init(void); /* in hack.h but we're using config.h */

/** @brief 런타임에 사용되는 몬스터 종류 전역 배열. */
struct permonst mons[SIZE(mons_init)];

/**
 * @brief 몬스터 전역 배열을 초기 테이블 값으로 채운다.
 *
 * @c mons_init 의 내용을 런타임 전역 배열 @c mons 로 복사한다.
 * 게임 초기화 시 호출된다.
 */
void
monst_globals_init(void)
{
    memcpy(mons, mons_init, sizeof mons);
    return;
}

/** @brief 유혹(seduction) 공격을 수행하는 몬스터용 공격 배열. */
const struct attack c_sa_yes[NATTK] = SEDUCTION_ATTACKS_YES;
/** @brief 유혹(seduction) 공격을 수행하지 않는 몬스터용 공격 배열. */
const struct attack c_sa_no[NATTK] = SEDUCTION_ATTACKS_NO;

/* for 'onefile' processing where end of this file isn't necessarily the
   end of the source code seen by the compiler */
#undef NO_ATTK
#undef LVL
#undef SIZ
#undef ATTK
#undef A

/*monst.c*/
