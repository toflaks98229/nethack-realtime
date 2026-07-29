/* NetHack 5.0	priest.c	$NHDT-Date: 1781973062 2026/06/20 16:31:02 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.110 $ */
/* Copyright (c) Izchak Miller, Steve Linhart, 1989.              */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file priest.c
 * @brief 신전 사제(temple priest) 및 정렬 성직자/천사(roamer) 관련 로직.
 *
 * 사제의 이동, 신전/제단 판정, 사제 생성(priestini)과 이름 표기, 사제와의 대화·
 * 헌금, 신전 침입 처리, 성역(sanctuary) 판정, 신의 응징(ghod_hitsu), 사제 분노
 * 등을 담당한다.
 */

#include "hack.h"
#include "mfndpos.h"

/* these match the categorizations shown by enlightenment */
/** @brief 정렬 점수: "죄지음"(strayed -1..-3 보다 나쁨) 경계값. */
#define ALGN_SINNED (-4) /* worse than strayed (-1..-3) */
/** @brief 정렬 점수: "독실함"(fervent 9..13 보다 좋음) 경계값. */
#define ALGN_DEVOUT 14   /* better than fervent (9..13) */

staticfn boolean histemple_at(struct monst *, coordxy, coordxy);
staticfn boolean has_shrine(struct monst *);

/**
 * @brief 몬스터에 사제(epri) 확장 구조체를 할당한다.
 * @param[in,out] mtmp 대상 몬스터.
 * @note 이미 할당되어 있으면 아무 동작도 하지 않는다.
 */
void
newepri(struct monst *mtmp)
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!EPRI(mtmp)) {
        EPRI(mtmp) = (struct epri *) alloc(sizeof(struct epri));
        (void) memset((genericptr_t) EPRI(mtmp), 0, sizeof(struct epri));
        EPRI(mtmp)->parentmid = mtmp->m_id;
    }
}

/**
 * @brief 몬스터의 사제(epri) 확장 구조체를 해제한다.
 * @param[in,out] mtmp 대상 몬스터.
 * @note 사제 플래그(@c ispriest)도 함께 해제한다.
 */
void
free_epri(struct monst *mtmp)
{
    if (mtmp->mextra && EPRI(mtmp)) {
        free((genericptr_t) EPRI(mtmp));
        EPRI(mtmp) = (struct epri *) 0;
    }
    mtmp->ispriest = 0;
}

/**
 * @brief 사제와 상점 주인의 공통 이동 로직(목표 지점을 향해 이동).
 * @param[in,out] mtmp        이동하는 몬스터.
 * @param[in]     in_his_shop 상점 주인이 자기 가게 안에 있는지.
 * @param[in]     appr        접근 성향(양수=접근, 음수=회피).
 * @param[in]     uondoor     영웅이 문 위에 있는지.
 * @param[in]     avoid       영웅을 피할지 여부.
 * @param[in]     omx,omy     현재 위치.
 * @param[in]     ggx,ggy     목표 위치.
 * @retval 1  이동했다.
 * @retval 0  이동하지 않았다.
 * @retval -1 일반 이동(@c m_move)에 맡긴다.
 * @retval -2 죽었다.
 */
/*
 * Move for priests and shopkeepers.  Called from shk_move() and pri_move().
 * Valid returns are  1: moved  0: didn't  -1: let m_move do it  -2: died.
 */
int
move_special(struct monst *mtmp, boolean in_his_shop, schar appr,
             boolean uondoor, boolean avoid,
             coordxy omx, coordxy omy, coordxy ggx, coordxy ggy)
{
    coordxy nx, ny, nix, niy;
    schar i;
    schar chcnt, cnt;
    struct mfndposdata mfp;
    long ninfo = 0;
    long allowflags;
#if 0 /* dead code; see below */
    struct obj *ib = (struct obj *) 0;
#endif

    if (omx == ggx && omy == ggy)
        return 0;
    if (mtmp->mconf) {
        avoid = FALSE;
        appr = 0;
    }

    nix = omx;
    niy = omy;
    allowflags = mon_allowflags(mtmp);
    cnt = mfndpos(mtmp, &mfp, allowflags);

    if (mtmp->isshk && avoid && uondoor) { /* perhaps we cannot avoid him */
        for (i = 0; i < cnt; i++)
            if (!(mfp.info[i] & NOTONL))
                goto pick_move;
        avoid = FALSE;
    }

#define GDIST(x, y) (dist2(x, y, ggx, ggy))
 pick_move:
    chcnt = 0;
    for (i = 0; i < cnt; i++) {
        nx = mfp.poss[i].x;
        ny = mfp.poss[i].y;
        if (IS_ROOM(levl[nx][ny].typ)
            || (mtmp->isshk && (!in_his_shop || ESHK(mtmp)->following))) {
            if (avoid && (mfp.info[i] & NOTONL) && !(mfp.info[i] & ALLOW_M))
                continue;
            if ((!appr && !rn2(++chcnt))
                || (appr && GDIST(nx, ny) < GDIST(nix, niy))
                || (mfp.info[i] & ALLOW_M)) {
                nix = nx;
                niy = ny;
                ninfo = mfp.info[i];
            }
        }
    }
#undef GDIST
    if (mtmp->ispriest && avoid && nix == omx && niy == omy
        && onlineu(omx, omy)) {
        /* might as well move closer as long it's going to stay
         * lined up */
        avoid = FALSE;
        goto pick_move;
    }

    if (nix != omx || niy != omy) {

        if (ninfo & ALLOW_ROCK) {
            m_break_boulder(mtmp, nix, niy);
            return 1;
        } else if (ninfo & ALLOW_M) {
            /* mtmp is deciding it would like to attack this turn.
             * Returns from m_move_aggress don't correspond to the same things
             * as this function should return, so we need to translate. */
            switch (m_move_aggress(mtmp, nix, niy)) {
            case 2:
                return -2; /* died making the attack */
            case 3:
                return 1; /* attacked and spent this move */
            }
        }

        if (MON_AT(nix, niy) || u_at(nix, niy))
            return 0;
        remove_monster(omx, omy);
        place_monster(mtmp, nix, niy);
        newsym(nix, niy);
        if (mtmp->isshk && !in_his_shop && inhishop(mtmp))
            check_special_room(FALSE);
#if 0 /* dead code; maybe someday someone will track down why... */
        if (ib) {
            if (cansee(mtmp->mx, mtmp->my))
                pline("%s picks up %s.", Monnam(mtmp),
                      distant_name(ib, doname));
            obj_extract_self(ib);
            (void) mpickobj(mtmp, ib);
        }
#endif
        return 1;
    }
    return 0;
}

/**
 * @brief 방 목록 중 신전(temple)에 해당하는 방 문자를 찾는다.
 * @param[in] array 방 문자 배열(예: @c u.urooms).
 * @return 신전 방 문자, 없으면 '\0'.
 */
char
temple_occupied(char *array)
{
    char *ptr;

    for (ptr = array; *ptr; ptr++)
        if (svr.rooms[*ptr - ROOMOFFSET].rtype == TEMPLE)
            return *ptr;
    return '\0';
}

/**
 * @brief 지정 좌표가 이 사제의 신전(같은 레벨·같은 방)인지 판정한다.
 * @param[in] priest 대상 사제.
 * @param[in] x,y    확인할 좌표.
 * @return 사제의 신전 위치이면 TRUE, 아니면 FALSE.
 */
staticfn boolean
histemple_at(struct monst *priest, coordxy x, coordxy y)
{
    return (boolean) (priest && priest->ispriest
                      && (EPRI(priest)->shroom == *in_rooms(x, y, TEMPLE))
                      && on_level(&(EPRI(priest)->shrlevel), &u.uz));
}

/**
 * @brief 사제가 자신의 신전 안(정렬 맞는 제단이 있는)에 있는지 판정한다.
 * @param[in] priest 대상 사제.
 * @return 자신의 온전한 신전 안에 있으면 TRUE, 아니면 FALSE.
 */
boolean
inhistemple(struct monst *priest)
{
    /* make sure we have a priest */
    if (!priest || !priest->ispriest)
        return FALSE;
    /* priest must be on right level and in right room */
    if (!histemple_at(priest, priest->mx, priest->my))
        return FALSE;
    /* temple room must still contain properly aligned altar */
    return has_shrine(priest);
}

/**
 * @brief 신전 사제의 한 턴 이동을 처리한다.
 * @param[in,out] priest 대상 사제.
 * @retval 1  이동했다.
 * @retval 0  이동하지 않았다(또는 공격했다).
 * @retval -1 일반 이동(@c m_move)에 맡긴다.
 * @retval -2 죽었다.
 * @note 평상시엔 제단 주변을 배회하고, 적대적이면 영웅을 추격/공격한다.
 */
/*
 * pri_move: return 1: moved  0: didn't  -1: let m_move do it  -2: died
 */
int
pri_move(struct monst *priest)
{
    coordxy ggx, ggy, omx, omy;
    schar temple;
    boolean avoid = TRUE;

    omx = priest->mx;
    omy = priest->my;

    if (!histemple_at(priest, omx, omy))
        return -1;

    temple = EPRI(priest)->shroom;

    ggx = EPRI(priest)->shrpos.x;
    ggy = EPRI(priest)->shrpos.y;

    ggx += rn1(3, -1); /* mill around the altar */
    ggy += rn1(3, -1);

    if (!priest->mpeaceful
        || (Conflict && !resist_conflict(priest))) {
        if (monnear(priest, u.ux, u.uy)) {
            if (Displaced)
                Your("displaced image doesn't fool %s!", mon_nam(priest));
            (void) mattacku(priest);
            return 0;
        } else if (strchr(u.urooms, temple)) {
            /* chase player if inside temple & can see him */
            if (priest->mcansee && m_canseeu(priest)) {
                ggx = u.ux;
                ggy = u.uy;
            }
            avoid = FALSE;
        }
    } else if (Invis)
        avoid = FALSE;

    return move_special(priest, FALSE, TRUE, FALSE, avoid, omx, omy, ggx, ggy);
}

/**
 * @brief 신전 생성 시 제단을 지키는 사제를 만든다(mktemple 전용).
 * @param[in] lvl     신전이 위치한 던전 레벨.
 * @param[in] sroom   신전 방.
 * @param[in] sx,sy   제단 좌표.
 * @param[in] sanctum TRUE 이면 대사제(성소)의 자리.
 * @note 사제에게 주문서·로브 등을 지급하며, 성소의 무정렬 제단이면 옌더의 부적을
 *       준다.
 */
/* exclusively for mktemple() */
void
priestini(
    d_level *lvl,
    struct mkroom *sroom,
    int sx, int sy,
    boolean sanctum) /* is it the seat of the high priest? */
{
    struct monst *priest;
    struct obj *otmp;
    int cnt;
    int px = 0, py = 0, i, si = rn2(N_DIRS);
    struct permonst *prim = &mons[sanctum ? PM_HIGH_CLERIC
                                          : PM_ALIGNED_CLERIC];

    for (i = 0; i < N_DIRS; i++) {
        px = sx + xdir[DIR_CLAMP(i+si)];
        py = sy + ydir[DIR_CLAMP(i+si)];
        if (pm_good_location(px, py, prim))
            break;
    }
    if (i == N_DIRS)
        px = sx, py = sy;

    if (MON_AT(px, py))
        (void) rloc(m_at(px, py), RLOC_NOMSG); /* insurance */

    priest = makemon(prim, px, py, MM_EPRI);
    if (priest) {
        EPRI(priest)->shroom = (schar) ((sroom - svr.rooms) + ROOMOFFSET);
        EPRI(priest)->shralign = Amask2align(levl[sx][sy].altarmask);
        EPRI(priest)->shrpos.x = sx;
        EPRI(priest)->shrpos.y = sy;
        assign_level(&(EPRI(priest)->shrlevel), lvl);
        mon_learns_traps(priest, ALL_TRAPS); /* traps are known */
        priest->mpeaceful = 1;
        priest->ispriest = 1;
        priest->isminion = 0;
        priest->msleeping = 0;
        set_malign(priest); /* mpeaceful may have changed */

        /* now his/her goodies... */
        if (sanctum && EPRI(priest)->shralign == A_NONE
            && on_level(&sanctum_level, &u.uz)) {
            (void) mongets(priest, AMULET_OF_YENDOR);
        }
        /* 2 to 4 spellbooks */
        for (cnt = rn1(3, 2); cnt > 0; --cnt) {
            (void) mpickobj(priest, mkobj(SPBOOK_no_NOVEL, FALSE));
        }
        /* robe [via makemon()] */
        if (rn2(2) && (otmp = which_armor(priest, W_ARMC)) != 0) {
            if (p_coaligned(priest))
                uncurse(otmp);
            else
                curse(otmp);
        }
    }
}

/**
 * @brief 몬스터의 정렬 유형을 반환한다(사제/하수인/일반 구분 없이).
 * @param[in] mon 대상 몬스터.
 * @return @c A_LAWFUL / @c A_NEUTRAL / @c A_CHAOTIC / @c A_NONE.
 */
/* get a monster's alignment type without caller needing EPRI & EMIN */
aligntyp
mon_aligntyp(struct monst *mon)
{
    aligntyp algn = mon->ispriest ? EPRI(mon)->shralign
                                  : mon->isminion ? EMIN(mon)->min_align
                                                  : mon->data->maligntyp;

    if (algn == A_NONE)
        return A_NONE; /* negative but differs from chaotic */
    return (algn > 0) ? A_LAWFUL : (algn < 0) ? A_CHAOTIC : A_NEUTRAL;
}

/*
 * Specially aligned monsters are named specially.
 *      - aligned priests with ispriest and high priests have shrines
 *              they retain ispriest and epri when polymorphed
 *      - aligned priests without ispriest are roamers
 *              they have isminion set and use emin rather than epri
 *      - minions do not have ispriest but have isminion and emin
 *      - caller needs to inhibit Hallucination if it wants to force
 *              the true name even when under that influence
 */
/**
 * @brief 사제·성직자·천사 등의 정렬을 반영한 이름 문자열을 만든다.
 * @param[in]  mon                 대상 몬스터.
 * @param[in]  article             붙일 관사(@c ARTICLE_THE 등).
 * @param[in]  reveal_high_priest  TRUE 이면 대사제의 정체(신 이름)를 드러낸다.
 * @param[out] pname               결과를 저장할 버퍼.
 * @return 결과가 기록된 @p pname.
 */
char *
priestname(
    struct monst *mon,
    int article,
    boolean reveal_high_priest,
    char *pname) /* caller-supplied output buffer */
{
    boolean do_hallu = Hallucination,
            aligned_priest = mon->data == &mons[PM_ALIGNED_CLERIC],
            high_priest = mon->data == &mons[PM_HIGH_CLERIC];
    char whatcode = '\0';
    const char *what = do_hallu ? rndmonnam(&whatcode) : mon_pmname(mon);

    if (!mon->ispriest && !mon->isminion) /* should never happen...  */
        return strcpy(pname, what);       /* caller must be confused */

    /* for high priest(ess), "high" (or "grand" for poohbah) will be inserted
       [this was done near the end but we want 'what' to be updated sooner] */
    if (mon->ispriest || aligned_priest || high_priest)
        what = do_hallu ? "poohbah" : mon->female ? "priestess" : "priest";

    *pname = '\0';
    if (article != ARTICLE_NONE && (!do_hallu || !bogon_is_pname(whatcode))) {
        if (article == ARTICLE_YOUR || (article == ARTICLE_A && high_priest))
            article = ARTICLE_THE;
        if (article == ARTICLE_THE) {
            Strcpy(pname, "the ");
        } else if (!strcmp(what, "Angel")) {
            /* bypass just_an(); it would yield "" due to treating capital A
               as indicating a personal name */
            Strcpy(pname, "an ");
        } else {
            (void) just_an(pname, what);
        }
    }
    /* pname[] contains "" or {"a ","an ","the "} */
    if (mon->minvis) {
        /* avoid "a invisible priest" */
        if (!strcmp(pname, "a "))
            Strcpy(pname, "an ");
        Strcat(pname, "invisible ");
    }
    if (mon->isminion && EMIN(mon)->renegade) {
        /* avoid "an renegade Angel" */
        if (!strcmp(pname, "an ") && !mon->minvis)
            Strcpy(pname, "a ");
        Strcat(pname, "renegade ");
    }

    if (mon->ispriest || aligned_priest) {
        if (high_priest)
            Strcat(pname, do_hallu ? "grand " : "high ");
    } else {
        if (mon->mtame && !strcmpi(what, "Angel"))
            Strcat(pname, "guardian ");
    }

    Strcat(pname, what);
    /* same as distant_monnam(), more or less... */
    if (do_hallu || !high_priest || reveal_high_priest
        || !Is_astralevel(&u.uz)
        || m_next2u(mon) || program_state.gameover) {
        Strcat(pname, " of ");
        Strcat(pname, halu_gname(mon_aligntyp(mon)));
    }
    return pname;
}

/**
 * @brief 사제가 영웅과 같은 정렬인지 판정한다.
 * @param[in] priest 대상 사제.
 * @return 같은 정렬이면 TRUE, 아니면 FALSE.
 */
boolean
p_coaligned(struct monst *priest)
{
    return (boolean) (u.ualign.type == mon_aligntyp(priest));
}

/**
 * @brief 사제의 제단이 온전한 성소(shrine)로 남아 있는지 판정한다.
 * @param[in] pri 대상 사제.
 * @return 정렬이 맞는 성소 제단이 있으면 TRUE, 아니면 FALSE.
 */
staticfn boolean
has_shrine(struct monst *pri)
{
    struct rm *lev;
    struct epri *epri_p;

    if (!pri || !pri->ispriest)
        return FALSE;
    epri_p = EPRI(pri);
    lev = &levl[epri_p->shrpos.x][epri_p->shrpos.y];
    if (!IS_ALTAR(lev->typ) || !(lev->altarmask & AM_SHRINE))
        return FALSE;
    return (boolean) (epri_p->shralign
                      == (Amask2align(lev->altarmask & ~AM_SHRINE)));
}

/**
 * @brief 지정한 방 번호의 신전을 지키는 사제를 찾는다.
 * @param[in] roomno 신전 방 번호.
 * @return 해당 신전의 사제, 없으면 NULL.
 */
struct monst *
findpriest(char roomno)
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->ispriest && (EPRI(mtmp)->shroom == roomno)
            && histemple_at(mtmp, mtmp->mx, mtmp->my))
            return mtmp;
    }
    return (struct monst *) 0;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/**
 * @brief 영웅이 신전 방에 들어왔을 때의 반응을 처리한다.
 * @param[in] roomno 진입한 신전 방 번호.
 * @note 사제의 인사·경고 등을 정렬/신앙 상태에 따라 출력한다.
 */
/* called from check_special_room() when the player enters the temple room */
void
intemple(int roomno)
{
    struct monst *priest, *mtmp;
    struct epri *epri_p;
    boolean shrined, sanctum, can_speak;
    long *this_time, *other_time;
    const char *msg1, *msg2;
    char buf[BUFSZ];

    /* don't do anything if hero is already in the room */
    if (temple_occupied(u.urooms0))
        return;

    if ((priest = findpriest((char) roomno)) != 0) {
        /* tended */
        record_achievement(ACH_TMPL);

        epri_p = EPRI(priest);
        shrined = has_shrine(priest);
        sanctum = (priest->data == &mons[PM_HIGH_CLERIC]
                   && (Is_sanctum(&u.uz) || In_endgame(&u.uz)));
        can_speak = !helpless(priest);
        if (can_speak && !Deaf && svm.moves >= epri_p->intone_time) {
            unsigned save_priest = priest->ispriest;

            /* don't reveal the altar's owner upon temple entry in
               the endgame; for the Sanctum, the next message names
               Moloch so suppress the "of Moloch" for him here too */
            if (sanctum && !Hallucination)
                priest->ispriest = 0;
            pline("%s intones:",
                  canseemon(priest) ? Monnam(priest) : "A nearby voice");
            priest->ispriest = save_priest;
            epri_p->intone_time = svm.moves + (long) d(10, 500); /* ~2505 */
            /* make sure that we don't suppress entry message when
               we've just given its "priest intones" introduction */
            epri_p->enter_time = 0L;
        }
        msg1 = msg2 = 0;
        if (sanctum && Is_sanctum(&u.uz)) {
            if (priest->mpeaceful) {
                /* first time inside */
                msg1 = "Infidel, you have entered Moloch's Sanctum!";
                msg2 = "Be gone!";
                priest->mpeaceful = 0;
                /* became angry voluntarily; no penalty for attacking him */
                set_malign(priest);
            } else {
                /* repeat visit, or attacked priest before entering */
                msg1 = "You desecrate this place by your presence!";
            }
        } else if (svm.moves >= epri_p->enter_time) {
            Sprintf(buf, "Pilgrim, you enter a %s place!",
                    !shrined ? "desecrated" : "sacred");
            msg1 = buf;
        }
        if (msg1 && can_speak && !Deaf) {
            SetVoice(priest, 0, 80, 0);
            verbalize1(msg1);
            if (msg2)
                verbalize1(msg2);
            epri_p->enter_time = svm.moves + (long) d(10, 100); /* ~505 */
        }
        if (!sanctum) {
            if (!shrined || !p_coaligned(priest)
                || u.ualign.record <= ALGN_SINNED) {
                msg1 = "have a%s forbidding feeling...";
                msg2 = (!shrined || !p_coaligned(priest)) ? "" : " strange";
                this_time = &epri_p->hostile_time;
                other_time = &epri_p->peaceful_time;
            } else {
                msg1 = "experience %s sense of peace.";
                msg2 = (u.ualign.record >= ALGN_DEVOUT) ? "a" : "an unusual";
                this_time = &epri_p->peaceful_time;
                other_time = &epri_p->hostile_time;
            }
            /* give message if we haven't seen it recently or
               if alignment update has caused it to switch from
               forbidding to sense-of-peace or vice versa */
            if (svm.moves >= *this_time || *other_time >= *this_time) {
                You(msg1, msg2);
                *this_time = svm.moves + (long) d(10, 20); /* ~55 */
                /* avoid being tricked by the RNG:  switch might have just
                   happened and previous random threshold could be larger */
                if (*this_time <= *other_time)
                    *other_time = *this_time - 1L;
            }
        }
        /* recognize the Valley of the Dead and Moloch's Sanctum
           once hero has encountered the temple priest on those levels */
        mapseen_temple(priest);
    } else {
        /* untended */

        switch (rn2(4)) {
        case 0:
            You("have an eerie feeling...");
            break;
        case 1:
            You_feel("like you are being watched.");
            break;
        case 2:
            pline("A shiver runs down your %s.", body_part(SPINE));
            break;
        default:
            break; /* no message; unfortunately there's no
                      EPRI(priest)->eerie_time available to
                      make sure we give one the first time */
        }
        if (!rn2(5)
            && (mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, MM_NOMSG))
                   != 0) {
            int ngen = svm.mvitals[PM_GHOST].born;
            if (canspotmon(mtmp))
                pline("A%s ghost appears next to you%c",
                      ngen < 5 ? "n enormous" : "",
                      ngen < 10 ? '!' : '.');
            else
                You("sense a presence close by!");
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
            if (flags.verbose)
                You("are frightened to death, and unable to move.");
            nomul(-3);
            gm.multi_reason = "being terrified of a ghost";
            gn.nomovemsg = "You regain your composure.";
        }
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* reset the move counters used to limit temple entry feedback;
   leaving the level and then returning yields a fresh start */
/**
 * @brief 사제의 신전 진입 관련 타이머 기록을 초기화한다.
 * @param[in,out] priest 대상 사제.
 * @note 레벨을 떠났다 돌아오면 신전 반응이 새로 시작되도록 한다.
 */
void
forget_temple_entry(struct monst *priest)
{
    struct epri *epri_p = priest->ispriest ? EPRI(priest) : 0;

    if (!epri_p) {
        impossible("attempting to manipulate shrine data for non-priest?");
        return;
    }
    epri_p->intone_time = epri_p->enter_time = epri_p->peaceful_time =
        epri_p->hostile_time = 0L;
}

/**
 * @brief 영웅이 사제에게 말을 걸었을 때(#chat)의 대화를 처리한다.
 * @param[in,out] priest 대화 상대 사제.
 * @note 헌금 요청, 정렬/신앙 상태에 따른 반응, 축복/저주 등을 다룬다.
 */
void
priest_talk(struct monst *priest)
{
    boolean coaligned = p_coaligned(priest);
    boolean strayed = (u.ualign.record < 0);
    unsigned *cheapskate = NULL;
    if (EPRI(priest)) cheapskate = &EPRI(priest)->cheapskate_count;

    /*
     * Note: we won't be called if hero is Deaf [since dochat() will
     * return before calling domonnoise()], so we don't need to check
     * for that before the various calls to verbalize() here.
     */

    /* KMH, conduct */
    if (!u.uconduct.gnostic++)
        livelog_printf(LL_CONDUCT,
                       "rejected atheism by consulting with %s",
                       mon_nam(priest));

    if (priest->mflee || (!priest->ispriest && coaligned && strayed)) {
        pline("%s doesn't want anything to do with you!", Monnam(priest));
        priest->mpeaceful = 0;
        return;
    }

    /* priests don't chat unless peaceful and in their own temple */
    if (!inhistemple(priest) || !priest->mpeaceful || helpless(priest)) {
        static const char *const cranky_msg[3] = {
            "Thou wouldst have words, eh?  I'll give thee a word or two!",
            "Talk?  Here is what I have to say!",
            "Pilgrim, I would speak no longer with thee."
        };

        if (helpless(priest)) {
            pline("%s breaks out of %s reverie!", Monnam(priest),
                  mhis(priest));
            priest->mfrozen = priest->msleeping = 0;
            priest->mcanmove = 1;
        }
        priest->mpeaceful = 0;
        SetVoice(priest, 0, 80, 0);
        verbalize1(cranky_msg[rn2(3)]);
        return;
    }

    /* you desecrated the temple and now you want to chat? */
    if (priest->mpeaceful && *in_rooms(priest->mx, priest->my, TEMPLE)
        && !has_shrine(priest)) {
        SetVoice(priest, 0, 80, 0);
        verbalize(
              "Begone!  Thou desecratest this holy place with thy presence.");
        priest->mpeaceful = 0;
        return;
    }
    if (!money_cnt(gi.invent)) {
        if (coaligned && !strayed) {
            long pmoney = money_cnt(priest->minvent);
            if (pmoney > 0L) {
                const char *bits;
                bits = (Hallucination) ? currency(pmoney)
                                       : (pmoney == 1L) ? "bit" : "bits";
                /* Note: two bits is actually 25 cents.  Hmm. */
                pline("%s gives you %s%s for an ale.", Monnam(priest),
                      (pmoney == 1L) ? "one " : "two ", bits);
                money2u(priest, pmoney > 1L ? 2 : 1);
            } else
                pline("%s preaches the virtues of poverty.", Monnam(priest));
            exercise(A_WIS, TRUE);
        } else
            pline("%s is not interested.", Monnam(priest));
        return;
    } else {
        /* there's now some randomization in how much you need to donate, but
           you are given suggested donation values that will guarantee
           clairvoyance and protection respectively; with more gold visible
           you need to donate more but get a greater effect; and if you
           cheapskate out to rerandomize the donation amounts they will be
           higher next time */
        long offer;
        long suggested = (u.ulevelpeak ? u.ulevelpeak : 1 ) *
            rn1(101, 150 + (cheapskate ? *cheapskate : 0) * 40);
        long quan = money_cnt(gi.invent) / (suggested * 3);
        char buf[BUFSZ];

        if (quan < 1)
            quan = 1;

        Sprintf(buf, "How much will you offer (suggested: %ld or %ld)?",
                suggested * quan, suggested * quan * 2);

        if (flags.debug)
            pline("%s asks you for a contribution for the temple (base %ld).",
                  Monnam(priest), suggested);
        else
            pline("%s asks you for a contribution for the temple.",
                  Monnam(priest));
        if ((offer = bribe(priest, buf)) == 0) {
            SetVoice(priest, 0, 80, 0);
            verbalize("Thou shalt regret thine action!");
            if (coaligned)
                adjalign(-1);
            if (cheapskate) ++*cheapskate;
        } else if (offer < suggested * quan) {
            if (money_cnt(gi.invent) > (offer * 2L)) {
                SetVoice(priest, 0, 80, 0);
                verbalize("Cheapskate.");
                if (cheapskate) ++*cheapskate;
            } else {
                SetVoice(priest, 0, 80, 0);
                verbalize("I thank thee for thy contribution.");
                /* give player some token */
                exercise(A_WIS, TRUE);
            }
        } else if (offer < suggested * quan * 2) {
            SetVoice(priest, 0, 80, 0);
            verbalize("Thou art indeed a pious individual.");
            if (money_cnt(gi.invent) < (offer * 2L)) {
                if (coaligned && u.ualign.record <= ALGN_SINNED)
                    adjalign(1);
            }
            verbalize("I bestow upon thee a blessing.");
            incr_itimeout(&HClairvoyant, rn1(500 * offer / suggested,
                                             500 * offer / suggested));
        } else if (offer < suggested * quan * 3) {
            int orig_ublessed = u.ublessed;

            /* u.ublessed is only active when Protection is enabled via
               something other than worn gear (theft by gremlin clears the
               intrinsic but not its former magnitude, making it
               recoverable) */
            if (!(HProtection & INTRINSIC)) {
                HProtection |= FROMOUTSIDE;
                orig_ublessed = -1; /* force "rewarded" message */
            }

            for (; offer >= (2 * suggested); offer -= (2 * suggested)) {
                if (!u.ublessed)
                    u.ublessed = rn1(3, 2);
                else if (u.ublessed < 20 &&
                         (u.ublessed < 9 || !rn2(u.ublessed)))
                    u.ublessed++;
            }
            SetVoice(priest, 0, 80, 0);
            if (u.ublessed > orig_ublessed) {
                verbalize("Thou hast been rewarded for thy devotion.");
            } else {
                verbalize("Thy selfless generosity is deeply appreciated.");
            }
        } else {
            SetVoice(priest, 0, 80, 0);
            verbalize("Thy selfless generosity is deeply appreciated.");
            /* money_cnt check is preserved for futureproofing but probably
               can't fail in the current code */
            if (money_cnt(gi.invent) < (offer * 2L) && coaligned) {
                if (strayed && (svm.moves - u.ucleansed) > 5000L) {
                    u.ualign.record = 0; /* cleanse thee */
                    u.ucleansed = svm.moves;
                } else {
                    adjalign(2);
                }
            }
        }
    }
}

/**
 * @brief 배회하는 성직자/천사(roamer) 몬스터를 생성한다.
 * @param[in] ptr       생성할 종 데이터(정렬 성직자/천사 등).
 * @param[in] alignment 생성될 몬스터의 정렬.
 * @param[in] x,y       생성 위치.
 * @param[in] peaceful  TRUE 이면 평화적으로 생성한다.
 * @return 생성된 몬스터, 실패 시 NULL.
 */
struct monst *
mk_roamer(struct permonst *ptr, aligntyp alignment, coordxy x, coordxy y,
          boolean peaceful)
{
    struct monst *roamer;
    boolean coaligned = (u.ualign.type == alignment);

#if 0 /* this was due to permonst's pxlth field which is now gone */
    if (ptr != &mons[PM_ALIGNED_CLERIC] && ptr != &mons[PM_ANGEL])
        return (struct monst *) 0;
#endif

    if (MON_AT(x, y))
        (void) rloc(m_at(x, y), RLOC_NOMSG); /* insurance */

    if (!(roamer = makemon(ptr, x, y, MM_ADJACENTOK | MM_EMIN | MM_NOMSG)))
        return (struct monst *) 0;

    EMIN(roamer)->min_align = alignment;
    EMIN(roamer)->renegade = (coaligned && !peaceful);
    roamer->ispriest = 0;
    roamer->isminion = 1;
    mon_learns_traps(roamer, ALL_TRAPS); /* traps are known */
    roamer->mpeaceful = peaceful;
    roamer->msleeping = 0;
    set_malign(roamer); /* peaceful may have changed */

    /* MORE TO COME */
    return roamer;
}

/**
 * @brief 배회자(roamer)의 적대 여부를 영웅 정렬 변화에 맞춰 재설정한다.
 * @param[in,out] roamer 대상 배회자(성직자/천사).
 * @note 정렬이 영웅과 달라지면 적대적으로 바뀐다.
 */
void
reset_hostility(struct monst *roamer)
{
    if (!roamer->isminion)
        return;
    if (roamer->data != &mons[PM_ALIGNED_CLERIC]
        && roamer->data != &mons[PM_ANGEL])
        return;

    if (EMIN(roamer)->min_align != u.ualign.type) {
        roamer->mpeaceful = roamer->mtame = 0;
        set_malign(roamer);
    }
    newsym(roamer->mx, roamer->my);
}

/**
 * @brief 지정 위치가 영웅에게 우호적인 성역(sanctuary)인지 판정한다.
 * @param[in] mon NULL 이 아니면 이 몬스터의 위치로 @p x,y 를 대체.
 * @param[in] x,y 확인할 좌표.
 * @return 우호적 성역(정렬 맞는 평화적 사제의 온전한 성소)이면 TRUE.
 * @note 영웅이 죄를 지은 상태이면 성역으로 인정되지 않는다.
 */
boolean
in_your_sanctuary(
    struct monst *mon, /* if non-null, <mx,my> overrides <x,y> */
    coordxy x, coordxy y)
{
    char roomno;
    struct monst *priest;

    if (mon) {
        if (is_minion(mon->data) || is_rider(mon->data))
            return FALSE;
        x = mon->mx, y = mon->my;
    }
    if (u.ualign.record <= ALGN_SINNED) /* sinned or worse */
        return FALSE;
    if ((roomno = temple_occupied(u.urooms)) == 0
        || roomno != *in_rooms(x, y, TEMPLE))
        return FALSE;
    if ((priest = findpriest(roomno)) == 0)
        return FALSE;
    return (boolean) (has_shrine(priest) && p_coaligned(priest)
                      && priest->mpeaceful);
}

/**
 * @brief 신전 안에서 사제를 공격했을 때 신의 응징을 내린다.
 * @param[in] priest 공격받은 사제.
 * @note 제단 부근에서 벼락 등 신성한 공격이 영웅을 향해 발동한다.
 */
/* when attacking "priest" in his temple */
void
ghod_hitsu(struct monst *priest)
{
    struct mkroom *troom;
    struct monst *oldbuzzer;
    struct obj *oldcurrwand;
    coordxy x, y, ax, ay;
    int roomno = (int) temple_occupied(u.urooms);

    if (!roomno || !has_shrine(priest))
        return;

    ax = x = EPRI(priest)->shrpos.x;
    ay = y = EPRI(priest)->shrpos.y;
    troom = &svr.rooms[roomno - ROOMOFFSET];

    if (u_at(x, y) || !linedup(u.ux, u.uy, x, y, 1)) {
        if (IS_DOOR(levl[u.ux][u.uy].typ)) {
            if (u.ux == troom->lx - 1) {
                x = troom->hx;
                y = u.uy;
            } else if (u.ux == troom->hx + 1) {
                x = troom->lx;
                y = u.uy;
            } else if (u.uy == troom->ly - 1) {
                x = u.ux;
                y = troom->hy;
            } else if (u.uy == troom->hy + 1) {
                x = u.ux;
                y = troom->ly;
            }
        } else {
            switch (rn2(4)) {
            case 0:
                x = u.ux;
                y = troom->ly;
                break;
            case 1:
                x = u.ux;
                y = troom->hy;
                break;
            case 2:
                x = troom->lx;
                y = u.uy;
                break;
            default:
                x = troom->hx;
                y = u.uy;
                break;
            }
        }
        if (!linedup(u.ux, u.uy, x, y, 1))
            return;
    }

    switch (rn2(3)) {
    case 0:
        pline("%s roars in anger:  \"Thou shalt suffer!\"",
              a_gname_at(ax, ay));
        break;
    case 1:
        pline("%s voice booms:  \"How darest thou harm my servant!\"",
              s_suffix(a_gname_at(ax, ay)));
        break;
    default:
        pline("%s roars:  \"Thou dost profane my shrine!\"",
              a_gname_at(ax, ay));
        break;
    }

    /* bolt of lightning cast by unspecified monster */
    oldcurrwand = gc.current_wand;
    gc.current_wand = 0;
    oldbuzzer = gb.buzzer;
    gb.buzzer = 0;
    buzz(BZ_M_SPELL(BZ_OFS_AD(AD_ELEC)), 6, x, y, sgn(gt.tbx), sgn(gt.tby));
    gb.buzzer = oldbuzzer;
    gc.current_wand = oldcurrwand;
    exercise(A_WIS, FALSE);
}

/**
 * @brief 현재 신전의 사제를 분노시킨다(적대화).
 * @note 제단이 파괴/개종되었으면 사제는 신전을 떠나 배회 하수인이 된다.
 */
void
angry_priest(void)
{
    struct monst *priest;
    struct rm *lev;

    if ((priest = findpriest(temple_occupied(u.urooms))) != 0) {
        struct epri *eprip = EPRI(priest);

        wakeup(priest, FALSE);
        setmangry(priest, FALSE);
        /*
         * If the altar has been destroyed or converted, let the
         * priest run loose.
         * (When it's just a conversion and there happens to be
         * a fresh corpse nearby, the priest ought to have an
         * opportunity to try converting it back; maybe someday...)
         */
        lev = &levl[eprip->shrpos.x][eprip->shrpos.y];
        if (!IS_ALTAR(lev->typ)
            || ((aligntyp) Amask2align(lev->altarmask & AM_MASK)
                != eprip->shralign)) {
            if (!EMIN(priest))
                newemin(priest);
            priest->ispriest = 0; /* now a roaming minion */
            priest->isminion = 1;
            assert(has_emin(priest));
            EMIN(priest)->min_align = eprip->shralign;
            EMIN(priest)->renegade = FALSE;
            /* discard priest's memory of his former shrine;
               if we ever implement the re-conversion mentioned
               above, this will need to be removed */
            free_epri(priest);
        }
    }
}

/**
 * @brief bones 저장 시 자신의 성소 레벨에 없는 사제들을 제거한다.
 * @note bones 복원 시의 문제를 방지하기 위한 정리 작업이다.
 */
/*
 * When saving bones, find priests that aren't on their shrine level,
 * and remove them.  This avoids big problems when restoring bones.
 * [Perhaps we should convert them into roamers instead?]
 */
void
clearpriests(void)
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->ispriest && !on_level(&(EPRI(mtmp)->shrlevel), &u.uz))
            mongone(mtmp);
    }
}

/**
 * @brief 복원 시 사제 고유 구조체(성소 레벨 등)를 보정한다.
 * @param[in,out] mtmp    복원 중인 사제.
 * @param[in]     ghostly bones 파일에서 복원하는 경우 TRUE.
 */
/* munge priest-specific structure when restoring -dlc */
void
restpriest(struct monst *mtmp, boolean ghostly)
{
    if (u.uz.dlevel) {
        if (ghostly)
            assign_level(&(EPRI(mtmp)->shrlevel), &u.uz);
    }
}

#undef ALGN_SINNED
#undef ALGN_DEVOUT

/*priest.c*/
