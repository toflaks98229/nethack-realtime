/* NetHack 5.0	minion.c	$NHDT-Date: 1781973054 2026/06/20 16:30:54 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.88 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2008. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file minion.c
 * @brief 하수인(minion)·악마·천사의 소환, 협상, 수호천사 관련 로직.
 *
 * 정렬(alignment)에 따른 하수인/원소/악마/천사 소환, 악마의 통행료 협상
 * (@c demon_talk, @c bribe), 갈등(Conflict)으로 인한 수호천사 상실/획득,
 * 악마 군주·왕자 선택 등을 처리한다.
 */

#include "hack.h"

/** @brief 네 가지 기본 원소 몬스터 목록(재정렬·추가에 영향받지 않게 하기 위함). */
/* used to pick among the four basic elementals without worrying whether
   they've been reordered (difficulty reassessment?) or any new ones have
   been introduced (hybrid types added to 'E'-class?) */
static const int elementals[4] = {
    PM_AIR_ELEMENTAL, PM_FIRE_ELEMENTAL,
    PM_EARTH_ELEMENTAL, PM_WATER_ELEMENTAL
};

/**
 * @brief 몬스터에 하수인(emin) 확장 구조체를 할당한다.
 * @param[in,out] mtmp 대상 몬스터.
 * @note 이미 할당되어 있으면 아무 동작도 하지 않으며, 부모 몬스터 ID를 기록한다.
 */
void
newemin(struct monst *mtmp)
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!EMIN(mtmp)) {
        EMIN(mtmp) = (struct emin *) alloc(sizeof(struct emin));
        (void) memset((genericptr_t) EMIN(mtmp), 0, sizeof(struct emin));
        EMIN(mtmp)->parentmid = mtmp->m_id;
    }
}

/**
 * @brief 몬스터의 하수인(emin) 확장 구조체를 해제한다.
 * @param[in,out] mtmp 대상 몬스터.
 * @note 하수인 플래그(@c isminion)도 함께 해제한다.
 */
void
free_emin(struct monst *mtmp)
{
    if (mtmp->mextra && EMIN(mtmp)) {
        free((genericptr_t) EMIN(mtmp));
        EMIN(mtmp) = (struct emin *) 0;
    }
    mtmp->isminion = 0;
}

/**
 * @brief 현재 레벨의 몬스터 수를 센다.
 * @param[in] spotted TRUE 이면 영웅이 보거나 감지한 몬스터만, FALSE 이면 전부.
 * @return 조건에 맞는 살아 있는 몬스터의 수.
 */
/* count the number of monsters on the level */
int
monster_census(boolean spotted) /* seen|sensed vs all */
{
    struct monst *mtmp;
    int count = 0;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp) || PARKEDMONSTER(mtmp))
            continue;
        if (spotted && !canspotmon(mtmp))
            continue;
        ++count;
    }
    return count;
}

/**
 * @brief 몬스터가 다른 몬스터를 소환한다.
 *
 * 소환 주체의 종류·정렬에 따라 악마 왕자/군주/일반 악마, 하수인, 천사, 원소
 * 등을 결정하여 영웅 위치 부근에 생성한다.
 *
 * @param[in] mon 소환 주체 몬스터. NULL 이면 옌더의 마법사가 소환하는 것으로 간주.
 * @return 새로 늘어난 몬스터의 수(무리 소환 고려).
 * @note 데몬베인(@c ART_DEMONBANE)을 든 영웅 앞에서는 악마가 소환에 실패한다.
 */
/* mon summons a monster */
int
msummon(struct monst *mon)
{
    struct permonst *ptr;
    int dtype = NON_PM, cnt = 0, result = 0, census;
    boolean xlight;
    aligntyp atyp;
    struct monst *mtmp;

    if (mon) {
        ptr = mon->data;

        if (u_wield_art(ART_DEMONBANE) && is_demon(ptr)) {
            if (canseemon(mon))
                pline("%s looks puzzled for a moment.", Monnam(mon));
            return 0;
        }

        atyp = mon->ispriest ? EPRI(mon)->shralign
               : mon->isminion ? EMIN(mon)->min_align
                 : (ptr->maligntyp == A_NONE) ? A_NONE
                   : sgn(ptr->maligntyp);
    } else {
        ptr = &mons[PM_WIZARD_OF_YENDOR];
        atyp = (ptr->maligntyp == A_NONE) ? A_NONE : sgn(ptr->maligntyp);
    }

    if (is_dprince(ptr) || (ptr == &mons[PM_WIZARD_OF_YENDOR])) {
        dtype = (!rn2(20)) ? dprince(atyp) : (!rn2(4)) ? dlord(atyp)
                                                       : ndemon(atyp);
        cnt = ((dtype != NON_PM)
               && !rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
    } else if (is_dlord(ptr)) {
        dtype = (!rn2(50)) ? dprince(atyp) : (!rn2(20)) ? dlord(atyp)
                                                        : ndemon(atyp);
        cnt = ((dtype != NON_PM)
               && !rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
    } else if (ptr == &mons[PM_BONE_DEVIL]) {
        dtype = PM_SKELETON;
        cnt = 1;
    } else if (is_ndemon(ptr)) {
        dtype = (!rn2(20)) ? dlord(atyp) : (!rn2(6)) ? ndemon(atyp)
                                                     : monsndx(ptr);
        cnt = 1;
    } else if (is_lminion(mon)) {
        dtype = (is_lord(ptr) && !rn2(20))
                    ? llord()
                    : (is_lord(ptr) || !rn2(6)) ? lminion() : monsndx(ptr);
        cnt = ((dtype != NON_PM)
               && !rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
    } else if (ptr == &mons[PM_ANGEL]) {
        /* non-lawful angels can also summon */
        if (!rn2(6)) {
            switch (atyp) { /* see summon_minion */
            case A_NEUTRAL:
                dtype = ROLL_FROM(elementals);
                break;
            case A_CHAOTIC:
            case A_NONE:
                dtype = ndemon(atyp);
                break;
            }
        } else {
            dtype = PM_ANGEL;
        }
        cnt = ((dtype != NON_PM)
               && !rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
    }

    if (dtype == NON_PM)
        return 0;

    /* sanity checks */
    if (cnt > 1 && (mons[dtype].geno & G_UNIQ) != 0)
        cnt = 1;
    /*
     * If this daemon is unique and being re-summoned (the only way we
     * could get this far with an extinct dtype), try another.
     */
    if ((svm.mvitals[dtype].mvflags & G_GONE) != 0) {
        dtype = ndemon(atyp);
        if (dtype == NON_PM)
            return 0;
    }

    /* some candidates can generate a group of monsters, so simple
       count of non-null makemon() result is not sufficient */
    census = monster_census(FALSE);
    xlight = FALSE;

    while (cnt > 0) {
        mtmp = makemon(&mons[dtype], u.ux, u.uy, MM_EMIN|MM_NOMSG);
        if (mtmp) {
            result++;
            /* an angel's alignment should match the summoner */
            if (dtype == PM_ANGEL) {
                mtmp->isminion = 1;
                EMIN(mtmp)->min_align = atyp;
                /* renegade if same alignment but not peaceful
                   or peaceful but different alignment */
                EMIN(mtmp)->renegade =
                    (atyp != u.ualign.type) ^ !mtmp->mpeaceful;
            }

            if (mtmp->data->mlet == S_ANGEL && !Blind) {
                /* for any 'A', 'cloud of smoke' will be 'flash of light';
                   if more than one monster is being created, that message
                   might be skipped for this monster but show 'mtmp' anyway */
                show_transient_light((struct obj *) 0, mtmp->mx, mtmp->my);
                xlight = TRUE;
                /* we don't do this for 'burst of flame' (fire elemental)
                   because those monsters become their own light source */
            }

            if (cnt == 1 && canseemon(mtmp)) {
                const char *cloud = 0,
                           *what = msummon_environ(mtmp->data, &cloud);

                pline("%s appears in a %s of %s!", Amonnam(mtmp),
                      cloud, what);
            }
        }
        cnt--;
    }

    if (xlight) {
        /* Note: if we forced --More-- here, the 'A's would be visible for
           long enough to be seen, but like with clairvoyance, some players
           would be annoyed at the disruption of having to acknowledge it */
        transient_light_cleanup();
    }

    /* how many monsters exist now compared to before? */
    if (result)
        result = monster_census(FALSE) - census;

    return result;
}

/**
 * @brief 지정한 정렬의 하수인을 하나 소환한다(신의 징벌 등).
 * @param[in] alignment 소환할 하수인의 정렬.
 * @param[in] talk      TRUE 이면 신의 목소리 및 등장 메시지를 출력한다.
 */
void
summon_minion(aligntyp alignment, boolean talk)
{
    struct monst *mon;
    int mnum;

    switch ((int) alignment) {
    case A_LAWFUL:
        mnum = lminion();
        break;
    case A_NEUTRAL:
        mnum = ROLL_FROM(elementals);
        break;
    case A_CHAOTIC:
    case A_NONE:
        mnum = ndemon(alignment);
        break;
    default:
        impossible("unaligned player?");
        mnum = ndemon(A_NONE);
        break;
    }
    if (mnum == NON_PM) {
        mon = 0;
    } else if (mnum == PM_ANGEL) {
        mon = makemon(&mons[mnum], u.ux, u.uy, MM_EMIN|MM_NOMSG);
        if (mon) {
            mon->isminion = 1;
            EMIN(mon)->min_align = alignment;
            EMIN(mon)->renegade = FALSE;
        }
    } else if (mnum != PM_SHOPKEEPER && mnum != PM_GUARD
               && mnum != PM_ALIGNED_CLERIC && mnum != PM_HIGH_CLERIC) {
        /* This was mons[mnum].pxlth == 0 but is this restriction
           appropriate or necessary now that the structures are separate? */
        mon = makemon(&mons[mnum], u.ux, u.uy, MM_EMIN|MM_NOMSG);
        if (mon) {
            mon->isminion = 1;
            EMIN(mon)->min_align = alignment;
            EMIN(mon)->renegade = FALSE;
        }
    } else {
        mon = makemon(&mons[mnum], u.ux, u.uy, MM_NOMSG);
    }
    if (mon) {
        if (talk) {
            if (!Deaf)
                pline_The("voice of %s booms:", align_gname(alignment));
            else
                You_feel("%s booming voice:",
                         s_suffix(align_gname(alignment)));
            SetVoice(mon, 0, 80, 0);
            verbalize("Thou shalt pay for thine indiscretion!");
            if (canspotmon(mon))
                pline("%s appears before you.", Amonnam(mon));
            mon->mstrategy &= ~STRAT_APPEARMSG;
        }
        mon->mpeaceful = FALSE;
        /* don't call set_malign(); player was naughty */
    }
}

/** @brief 악마가 자기 고향(지옥)에 있는지 여부(통행료 협상 강도에 영향). */
#define Athome (Inhell && (mtmp->cham == NON_PM))

/**
 * @brief 악마와의 통행료 협상을 처리한다.
 *
 * 영웅이 데몬베인/엑스칼리버를 들었거나 뇌물 협상이 결렬되면 전투로 이어지고,
 * 충분한 뇌물을 지불하거나 악마가 물러나면 사라진다.
 *
 * @param[in,out] mtmp 협상 상대 악마.
 * @return 악마가 공격하지 않고 물러나면 1, 전투로 이어지면 0.
 */
/* returns 1 if it won't attack. */
int
demon_talk(struct monst *mtmp)
{
    long cash, demand, offer;

    if (u_wield_art(ART_EXCALIBUR) || u_wield_art(ART_DEMONBANE)) {
        if (canspotmon(mtmp))
            pline("%s looks very angry.", Amonnam(mtmp));
        else
            You_feel("tension building.");
        mtmp->mpeaceful = mtmp->mtame = 0;
        set_malign(mtmp);
        newsym(mtmp->mx, mtmp->my);
        return 0;
    }

    if (is_fainted()) {
        reset_faint(); /* if fainted - wake up */
    } else {
        stop_occupation();
        if (gm.multi > 0) {
            nomul(0);
            unmul((char *) 0);
        }
    }

    /* Slight advantage given. */
    if (is_dprince(mtmp->data) && mtmp->minvis) {
        boolean wasunseen = !canspotmon(mtmp);

        mtmp->minvis = mtmp->perminvis = 0;
        if (wasunseen && canspotmon(mtmp)) {
            pline("%s appears before you.", Amonnam(mtmp));
            mtmp->mstrategy &= ~STRAT_APPEARMSG;
        }
        newsym(mtmp->mx, mtmp->my);
    }
    if (gy.youmonst.data->mlet == S_DEMON) { /* Won't blackmail their own. */
        if (!Deaf)
            pline("%s says, \"Good hunting, %s.\"", Amonnam(mtmp),
                  flags.female ? "Sister" : "Brother");
        else if (canseemon(mtmp))
            pline("%s %s something.", Amonnam(mtmp),
                  says());
        if (!tele_restrict(mtmp))
            (void) rloc(mtmp, RLOC_MSG);
        return 1;
    }
    cash = money_cnt(gi.invent);
    demand = (cash * (rnd(80) + 20 * Athome))
           / (100 * (1 + (sgn(u.ualign.type) == sgn(mtmp->data->maligntyp))));

    if (!demand || gm.multi < 0) { /* you have no gold or can't move */
        mtmp->mpeaceful = 0;
        set_malign(mtmp);
        return 0;
    } else {
        /* make sure that the demand is unmeetable if the monster
           has the Amulet, preventing monster from being satisfied
           and removed from the game (along with said Amulet...) */
        /* [actually the Amulet is safe; it would be dropped when
           mongone() gets rid of the monster; force combat anyway;
           also make it unmeetable if the player is Deaf, to simplify
           handling that case as player-won't-pay] */
        if (mon_has_amulet(mtmp) || Deaf)
            /* 125: 5*25 in case hero has maximum possible charisma */
            demand = cash + (long) rn1(1000, 125);

        if (!Deaf)
            pline("%s demands %ld %s for safe passage.",
                  Amonnam(mtmp), demand, currency(demand));
        else if (canseemon(mtmp))
            pline("%s seems to be demanding something.", Amonnam(mtmp));
        offer = 0L;
        if (!Deaf &&
            ((offer = bribe(mtmp, "How much will you offer?")) >= demand)) {
            pline("%s vanishes, laughing about cowardly mortals.",
                  Amonnam(mtmp));
        } else if (offer > 0L
                   && (long) rnd(5 * ACURR(A_CHA)) > (demand - offer)) {
            pline("%s scowls at you menacingly, then vanishes.",
                  Amonnam(mtmp));
        } else {
            pline("%s gets angry...", Amonnam(mtmp));
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
            return 0;
        }
    }
    /* if 'mtmp' is unrecognizable due to hero's hallucination,
       #chronicle will reveal its true identity -- just live with that;
       also, avoid random hallucinatory currency() units */
    livelog_printf(LL_UMONST, "bribed %s with %ld %s for safe passage",
                   x_monnam(mtmp, ARTICLE_A, (char *) 0, EXACT_NAME, FALSE),
                   offer, (offer == 1L) ? "zorkmid" : "zorkmids");
    mongone(mtmp);
    return 1;
}

/**
 * @brief 영웅에게 뇌물 액수를 입력받아 몬스터에게 지불한다.
 * @param[in,out] mtmp   뇌물을 받을 몬스터.
 * @param[in]     prompt 입력 프롬프트 문자열.
 * @return 실제로 지불한 금액(거절 시 0, 소지금 초과 시 소지금 전액).
 */
long
bribe(struct monst *mtmp, const char *prompt)
{
    char buf[BUFSZ] = DUMMY;
    long offer;
    long umoney = money_cnt(gi.invent);

    getlin(prompt, buf);
    if (sscanf(buf, "%ld", &offer) != 1)
        offer = 0L;

    /*Michael Paddon -- fix for negative offer to monster*/
    /*JAR880815 - */
    if (offer < 0L) {
        You("try to shortchange %s, but fumble.", mon_nam(mtmp));
        return 0L;
    } else if (offer == 0L) {
        You("refuse.");
        return 0L;
    } else if (offer >= umoney) {
        You("give %s all your gold.", mon_nam(mtmp));
        offer = umoney;
    } else {
        You("give %s %ld %s.", mon_nam(mtmp), offer, currency(offer));
    }
    (void) money2mon(mtmp, offer);
    disp.botl = TRUE;
    return offer;
}

/**
 * @brief 지정한 정렬에 맞는 악마 왕자(demon prince) 종을 고른다.
 * @param[in] atyp 원하는 정렬(@c A_NONE 이면 아무 정렬이나 허용).
 * @return 조건에 맞는 악마 왕자 종 번호. 못 찾으면 @c dlord() 결과로 근사.
 */
int
dprince(aligntyp atyp)
{
    int tryct, pm;

    for (tryct = !In_endgame(&u.uz) ? 20 : 0; tryct > 0; --tryct) {
        pm = rn1(PM_DEMOGORGON + 1 - PM_ORCUS, PM_ORCUS);
        if (!(svm.mvitals[pm].mvflags & G_GONE)
            && (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
            return pm;
    }
    return dlord(atyp); /* approximate */
}

/**
 * @brief 지정한 정렬에 맞는 악마 군주(demon lord) 종을 고른다.
 * @param[in] atyp 원하는 정렬(@c A_NONE 이면 아무 정렬이나 허용).
 * @return 조건에 맞는 악마 군주 종 번호. 못 찾으면 @c ndemon() 결과로 근사.
 */
int
dlord(aligntyp atyp)
{
    int tryct, pm;

    for (tryct = !In_endgame(&u.uz) ? 20 : 0; tryct > 0; --tryct) {
        pm = rn1(PM_YEENOGHU + 1 - PM_JUIBLEX, PM_JUIBLEX);
        if (!(svm.mvitals[pm].mvflags & G_GONE)
            && (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
            return pm;
    }
    return ndemon(atyp); /* approximate */
}

/**
 * @brief 질서(선) 진영의 군주를 고른다.
 * @return 아콘(@c PM_ARCHON) 종 번호. 이미 멸종했으면 @c lminion() 으로 근사.
 */
/* create lawful (good) lord */
int
llord(void)
{
    if (!(svm.mvitals[PM_ARCHON].mvflags & G_GONE))
        return PM_ARCHON;

    return lminion(); /* approximate */
}

/**
 * @brief 질서 진영의 일반 하수인(천사류, 군주 제외)을 고른다.
 * @return 조건에 맞는 하수인 종 번호. 못 찾으면 @c NON_PM.
 */
int
lminion(void)
{
    int tryct;
    struct permonst *ptr;

    for (tryct = 0; tryct < 20; tryct++) {
        ptr = mkclass(S_ANGEL, 0);
        if (ptr && !is_lord(ptr))
            return monsndx(ptr);
    }

    return NON_PM;
}

/**
 * @brief 지정한 정렬에 맞는 일반 악마(demon) 종을 고른다.
 * @param[in] atyp 원하는 정렬(@c A_NONE 이면 아무 정렬이나 허용).
 * @return 조건에 맞는 악마 종 번호. 못 찾으면 @c NON_PM.
 */
int
ndemon(aligntyp atyp) /* A_NONE is used for 'any alignment' */
{
    struct permonst *ptr;

    /*
     * 3.6.2:  [fixed #H2204, 22-Dec-2010, eight years later...]
     * pick a correctly aligned demon in one try.  This used to
     * use mkclass() to choose a random demon type and keep trying
     * (up to 20 times) until it got one with the desired alignment.
     * mkclass_aligned() skips wrongly aligned potential candidates.
     * [The only neutral demons are djinni and mail daemon and
     * mkclass() won't pick them, but call it anyway in case either
     * aspect of that changes someday.]
     */
#if 0
    if (atyp == A_NEUTRAL)
        return NON_PM;
#endif
    ptr = mkclass_aligned(S_DEMON, 0, atyp);
    return (ptr && is_ndemon(ptr)) ? monsndx(ptr) : NON_PM;
}

/**
 * @brief 갈등(Conflict)의 영향으로 수호천사가 영웅을 떠나 적대적으로 변한다.
 *
 * 기존 수호천사를 사라지게 하고, 그 자리에 2~4마리의 적대적 천사를 만든다.
 *
 * @param[in,out] mon 떠나갈 수호천사. NULL 이면 아직 천사가 생성되지 않은 상태.
 */
/* guardian angel has been affected by conflict so is abandoning hero */
void
lose_guardian_angel(
    struct monst *mon) /* if Null, angel hasn't been created yet */
{
    coord mm;
    int i;

    if (mon) {
        if (canspotmon(mon)) {
            if (!Deaf) {
                pline("%s rebukes you, saying:", Monnam(mon));
                SetVoice(mon, 0, 80, 0);
                verbalize("Since you desire conflict, have some more!");
            } else {
                pline("%s vanishes!", Monnam(mon));
            }
        }
        mongone(mon);
    }
    /* create 2 to 4 hostile angels to replace the lost guardian */
    for (i = rn1(3, 2); i > 0; --i) {
        mm.x = u.ux;
        mm.y = u.uy;
        if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
            (void) mk_roamer(&mons[PM_ANGEL], u.ualign.type, mm.x, mm.y,
                             FALSE);
    }
}

/**
 * @brief 아스트랄 평면 진입 시 자격이 있으면 길들여진 수호천사를 받는다.
 *
 * 갈등 상태이면 오히려 적대적 천사가 나타나고, 신앙심(record)이 높으면
 * 강력한 무장을 갖춘 길들여진 천사를 얻는다.
 *
 * @note 무애완(petless) 관습을 게임 막판에 깨뜨리지 않도록, 애완동물 관습을
 *       지켜온 경우에만 천사를 실제로 길들인다.
 */
/* just entered the Astral Plane; receive tame guardian angel if worthy */
void
gain_guardian_angel(void)
{
    struct monst *mtmp;
    struct obj *otmp;
    coord mm;

    Hear_again(); /* attempt to cure any deafness now (divine
                     message will be heard even if that fails) */
    if (Conflict) {
       if (!Deaf)
            pline("A voice booms:");
        else
            You_feel("a booming voice:");
        SetVoice((struct monst *) 0, 0, 80, voice_deity);
        verbalize("Thy desire for conflict shall be fulfilled!");
        /* send in some hostile angels instead */
        lose_guardian_angel((struct monst *) 0);
    } else if (u.ualign.record > 8) { /* fervent */
        if (!Deaf)
            pline("A voice whispers:");
        else
            You_feel("a soft voice:");
        SetVoice((struct monst *) 0, 0, 80, voice_deity);
        verbalize("Thou hast been worthy of me!");
        mm.x = u.ux;
        mm.y = u.uy;
        if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL])
            && (mtmp = mk_roamer(&mons[PM_ANGEL], u.ualign.type, mm.x, mm.y,
                                 TRUE)) != 0) {
            mtmp->mstrategy &= ~STRAT_APPEARMSG;
            /* guardian angel -- the one case mtame doesn't imply an
             * edog structure, so we don't want to call tamedog().
             * [Note: this predates mon->mextra which allows a monster
             * to have both emin and edog at the same time.]
             */
            /* Too nasty for the game to unexpectedly break petless conduct on
             * the final level of the game. The angel will still appear, but
             * won't be tamed. */
            if (u.uconduct.pets) {
                mtmp->mtame = 10;
                u.uconduct.pets++;
            }
            /* for 'hilite_pet'; after making tame, before next message */
            newsym(mtmp->mx, mtmp->my);
            if (!Blind)
                pline("An angel appears near you.");
            else
                You_feel("the presence of a friendly angel near you.");
            /* make him strong enough vs. endgame foes */
            mtmp->m_lev = rn1(8, 15);
            mtmp->mhp = mtmp->mhpmax =
                d((int) mtmp->m_lev, 10) + 30 + rnd(30);
            if ((otmp = select_hwep(mtmp)) == 0) {
                otmp = mksobj(SILVER_SABER, FALSE, FALSE);
                if (mpickobj(mtmp, otmp))
                    panic("merged weapon?");
            }
            bless(otmp);
            if (otmp->spe < 4)
                otmp->spe += rnd(4);
            if ((otmp = which_armor(mtmp, W_ARMS)) == 0
                || otmp->otyp != SHIELD_OF_REFLECTION) {
                (void) mongets(mtmp, AMULET_OF_REFLECTION);
                m_dowear(mtmp, TRUE);
            }
        }
    }
}

/*minion.c*/
