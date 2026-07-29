/* NetHack 5.0	were.c	$NHDT-Date: 1781973073 2026/06/20 16:31:13 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.46 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file were.c
 * @brief 라이칸스로프(were-creature)의 변신 및 소환 관련 로직.
 *
 * 늑대인간/자칼인간/쥐인간 등의 인간 형태와 짐승 형태 사이의 변신, 짐승
 * 무리 소환, 그리고 영웅 자신의 라이칸스로프 상태 변화를 처리한다.
 * 변신 확률은 밤/보름달 여부 및 형태 변경 방지(Protection) 여부에 좌우된다.
 */

#include "hack.h"

/**
 * @brief 라이칸스로프 몬스터의 주기적 변신 시도를 처리한다.
 *
 * 인간 형태이면 밤/보름달 등에 따른 확률로 짐승 형태로 변하고, 짐승 형태이면
 * 낮은 확률로 인간 형태로 되돌아간다. 근처에 있으면 울음소리를 들려준다.
 *
 * @param[in,out] mon 변신을 시도할 몬스터.
 * @note 라이칸스로프가 아닌 몬스터에게는 아무 동작도 하지 않는다.
 */
void
were_change(struct monst *mon)
{
    if (!is_were(mon->data))
        return;

    if (is_human(mon->data)) {
        if (!Protection_from_shape_changers
            && !rn2(night() ? (flags.moonphase == FULL_MOON ? 3 : 30)
                            : (flags.moonphase == FULL_MOON ? 10 : 50))) {
            new_were(mon); /* change into animal form */
            gw.were_changes++;
            if (!Deaf && !canseemon(mon)) {
                const char *howler;

                switch (monsndx(mon->data)) {
                case PM_WEREWOLF:
                    howler = "wolf";
                    break;
                case PM_WEREJACKAL:
                    howler = "jackal";
                    break;
                default:
                    howler = (char *) 0;
                    break;
                }
                if (howler) {
                    Soundeffect(se_canine_howl, 50);
                    You_hear("a %s howling at the moon.", howler);
                    wake_nearto(mon->mx, mon->my, 4 * 4);
                }
            }
        }
    } else if (!rn2(30) || Protection_from_shape_changers) {
        new_were(mon); /* change back into human form */
        gw.were_changes++;
    }
}

/**
 * @brief 라이칸스로프의 반대(짝) 형태 종 번호를 반환한다.
 *
 * 짐승 형태는 대응하는 인간 형태로, 인간 형태는 대응하는 짐승 형태로 매핑한다.
 *
 * @param[in] pm 현재 형태의 몬스터 종 번호.
 * @return 반대 형태의 종 번호.
 * @retval NON_PM 라이칸스로프 형태가 아닐 경우.
 */
int
counter_were(int pm)
{
    switch (pm) {
    case PM_WEREWOLF:
        return PM_HUMAN_WEREWOLF;
    case PM_HUMAN_WEREWOLF:
        return PM_WEREWOLF;
    case PM_WEREJACKAL:
        return PM_HUMAN_WEREJACKAL;
    case PM_HUMAN_WEREJACKAL:
        return PM_WEREJACKAL;
    case PM_WERERAT:
        return PM_HUMAN_WERERAT;
    case PM_HUMAN_WERERAT:
        return PM_WERERAT;
    default:
        return NON_PM;
    }
}

/**
 * @brief 라이칸스로프와 유사한 몬스터를 대응하는 were-짐승 종으로 변환한다.
 *
 * 예: 하수구 쥐/거대 쥐 → 쥐인간, 자칼/여우 → 자칼인간, 늑대/워그 → 늑대인간.
 *
 * @param[in] pm 변환할 몬스터 종 번호.
 * @return 대응하는 were-짐승 종 번호.
 * @retval NON_PM 대응하는 were-짐승이 없을 경우.
 */
/* convert monsters similar to werecritters into appropriate werebeast */
int
were_beastie(int pm)
{
    switch (pm) {
    case PM_WERERAT:
    case PM_SEWER_RAT:
    case PM_GIANT_RAT:
    case PM_RABID_RAT:
        return PM_WERERAT;
    case PM_WEREJACKAL:
    case PM_JACKAL:
    case PM_FOX:
    case PM_COYOTE:
        return PM_WEREJACKAL;
    case PM_WEREWOLF:
    case PM_WOLF:
    case PM_WARG:
    case PM_WINTER_WOLF:
    case PM_WINTER_WOLF_CUB:
        return PM_WEREWOLF;
    default:
        break;
    }
    return NON_PM;
}

/**
 * @brief 라이칸스로프 몬스터를 반대 형태로 실제 변신시킨다.
 *
 * 종 데이터를 교체하고, 변신에 따른 각성/체력 일부 회복/장비 파손/무장 해제
 * 등을 처리하며 화면을 갱신한다.
 *
 * @param[in,out] mon 변신시킬 몬스터.
 * @note 형태 변경 방지(Protection_from_shape_changers) 상태에서 인간 형태인
 *       경우에는 변신하지 않는다.
 * @warning 알 수 없는 라이칸스로프이면 @c impossible() 경고를 낸다.
 */
void
new_were(struct monst *mon)
{
    int pm;

    /* neither hero nor werecreature can change from human form to
       critter form if hero has Protection_from_shape_changers extrinsic;
       if already in critter form, always change to human form for that */
    if (Protection_from_shape_changers && is_human(mon->data))
        return;

    pm = counter_were(monsndx(mon->data));
    if (pm < LOW_PM) {
        impossible("unknown lycanthrope %s.",
                    mon->data->pmnames[NEUTRAL]);
        return;
    }

    if (canseemon(mon) && !Hallucination)
        pline("%s changes into a %s.", Monnam(mon),
              is_human(&mons[pm]) ? "human"
                                  /* pmname()+4: skip past "were" prefix */
                                  : pmname(&mons[pm], Mgender(mon)) + 4);

    set_mon_data(mon, &mons[pm]);
    if (helpless(mon)) {
        /* transformation wakens and/or revitalizes */
        mon->msleeping = 0;
        mon->mfrozen = 0; /* not asleep or paralyzed */
        mon->mcanmove = 1;
    }
    /* regenerate by 1/4 of the lost hit points */
    healmon(mon, (mon->mhpmax - mon->mhp) / 4, 0);
    newsym(mon->mx, mon->my);
    mon_break_armor(mon, FALSE);
    possibly_unwield(mon, FALSE);

    /* vision capability isn't changing so we don't call set_apparxy() to
       update mon's idea of where hero is; peaceful check is redundant */
    if (svc.context.mon_moving && !mon->mpeaceful
        && onscary(mon->mux, mon->muy, mon)
        && monnear(mon, mon->mux, mon->muy))
        monflee(mon, rn1(9, 2), TRUE, TRUE); /* 2..10 turns */
}

/**
 * @brief 라이칸스로프(영웅 포함)가 짐승 무리를 소환한다.
 *
 * 종류에 따라 쥐/자칼/늑대 계열 몬스터를 무작위 수만큼 생성한다.
 *
 * @param[in]  ptr     소환 주체의 종 데이터.
 * @param[in]  yours   TRUE 이면 소환된 짐승이 영웅의 애완동물이 된다.
 * @param[out] visible 생성된 몬스터 중 눈에 보이는 수를 저장한다.
 * @param[out] genbuf  NULL 이 아니면 소환된 무리의 종류 이름을 기록한다.
 * @return 실제로 생성된 몬스터의 총 수.
 * @note 형태 변경 방지 상태이고 영웅의 소환이 아니면 아무것도 소환하지 않는다.
 */
/* were-creature (even you) summons a horde */
int
were_summon(
    struct permonst *ptr,
    boolean yours,
    int *visible, /* number of visible helpers created */
    char *genbuf)
{
    int i, typ, pm = monsndx(ptr);
    struct monst *mtmp;
    int total = 0;

    *visible = 0;
    if (Protection_from_shape_changers && !yours)
        return 0;
    for (i = rnd(5); i > 0; i--) {
        switch (pm) {
        case PM_WERERAT:
        case PM_HUMAN_WERERAT:
            typ = rn2(3) ? PM_SEWER_RAT
                         : rn2(3) ? PM_GIANT_RAT : PM_RABID_RAT;
            if (genbuf)
                Strcpy(genbuf, "rat");
            break;
        case PM_WEREJACKAL:
        case PM_HUMAN_WEREJACKAL:
            typ = rn2(7) ? PM_JACKAL : rn2(3) ? PM_COYOTE : PM_FOX;
            if (genbuf)
                Strcpy(genbuf, "jackal");
            break;
        case PM_WEREWOLF:
        case PM_HUMAN_WEREWOLF:
            typ = rn2(5) ? PM_WOLF : rn2(2) ? PM_WARG : PM_WINTER_WOLF;
            if (genbuf)
                Strcpy(genbuf, "wolf");
            break;
        default:
            continue;
        }
        mtmp = makemon(&mons[typ], u.ux, u.uy, NO_MM_FLAGS);
        if (mtmp) {
            total++;
            if (canseemon(mtmp))
                *visible += 1;
        }
        if (yours && mtmp)
            (void) tamedog(mtmp, (struct obj *) 0, FALSE);
    }
    return total;
}

/**
 * @brief 영웅이 라이칸스로프 짐승 형태로 변신한다.
 *
 * 변신 제어(Polymorph_control)가 가능하면 변신 여부를 묻고, 그렇지 않으면
 * 근처에 몬스터가 없을 때 자동으로 변신한다.
 *
 * @note 변신 불가(Unchanging) 상태이거나 이미 짐승 형태이면 아무 동작도 하지
 *       않는다.
 */
void
you_were(void)
{
    char qbuf[QBUFSZ];
    boolean controllable_poly = Polymorph_control && !(Stunned || Unaware);

    if (Unchanging || u.umonnum == u.ulycn)
        return;
    if (controllable_poly) {
        /* `+4' => skip "were" prefix to get name of beast */
        Sprintf(qbuf, "Do you want to change into %s?",
                an(mons[u.ulycn].pmnames[NEUTRAL] + 4));
        if (!paranoid_query(ParanoidWerechange, qbuf))
            return;
    } else if (monster_nearby()) {
        return;
    }
    gw.were_changes++;
    (void) polymon(u.ulycn);
}

/**
 * @brief 영웅을 짐승 형태에서 인간 형태로 되돌리거나 라이칸스로프를 치유한다.
 *
 * @param[in] purify TRUE 이면 라이칸스로프 자체를 치유한다(정화).
 * @note 변신 제어가 가능하면 짐승 형태 유지 여부를 물을 수 있으며, 되돌리지
 *       않을 경우 다음 변신까지의 타이머를 설정한다.
 */
void
you_unwere(boolean purify)
{
    boolean controllable_poly = Polymorph_control && !(Stunned || Unaware);

    if (purify) {
        You_feel("purified.");
        set_ulycn(NON_PM); /* cure lycanthropy */
    }
    if (!Unchanging && is_were(gy.youmonst.data)
        && !monster_nearby()
        && (!controllable_poly
            || !paranoid_query(ParanoidWerechange, "Remain in beast form?")))
        rehumanize();
    else if (is_were(gy.youmonst.data) && !u.mtimedone)
        u.mtimedone = rn1(200, 200); /* 40% of initial were change */
}

/**
 * @brief 영웅의 라이칸스로프 종류를 설정하고 내재 능력을 갱신한다.
 *
 * 형태 변화 없이 라이칸스로피에 걸리거나 치유될 때 호출된다.
 *
 * @param[in] which 새 라이칸스로프 종 번호(치유 시 @c NON_PM).
 * @note 라이칸스로프의 내재 내성(Drain_resistance)을 추가/제거한다.
 */
/* lycanthropy is being caught or cured, but no shape change is involved */
void
set_ulycn(int which)
{
    u.ulycn = which;
    /* add or remove lycanthrope's innate intrinsics (Drain_resistance) */
    set_uasmon();
}

/*were.c*/
