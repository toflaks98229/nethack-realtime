/* NetHack 5.0	mkroom.c	$NHDT-Date: 1781973056 2026/06/20 16:30:56 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.81 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file mkroom.c
 * @brief 특수 방(상점·동물원·궁정·묘지·신전·늪 등)의 생성과 배치.
 *
 * 주어진 유형의 방을 만들어 몬스터·아이템으로 채우고, 방 관련 질의(문 인접,
 * 계단 포함, 내부 좌표 선택), 특수 방 검색, 궁정/분대 몬스터 생성, 방 정보의
 * 저장/복원, 심볼→지형 코드 변환 등을 제공한다.
 */

/*
 * Entry points:
 *      do_mkroom() -- make and stock a room of a given type
 *      nexttodoor() -- return TRUE if adjacent to a door
 *      has_dnstairs() -- return TRUE if given room has a down staircase
 *      has_upstairs() -- return TRUE if given room has an up staircase
 *      courtmon() -- generate a court monster
 *      save_rooms() -- save rooms into file fd
 *      rest_rooms() -- restore rooms from file fd
 *      cmap_to_type() -- convert S_xxx symbol to XXX topology code
 */

#include "hack.h"

#ifndef SFCTOOL
staticfn boolean isbig(struct mkroom *);
staticfn struct mkroom *pick_room(boolean);
staticfn void mkshop(void), mkzoo(int), mkswamp(void);
staticfn void mk_zoo_thronemon(coordxy, coordxy);
staticfn void mktemple(void);
staticfn coord *shrine_pos(int);
staticfn struct permonst *morguemon(void);
staticfn struct permonst *squadmon(void);
#endif /* SFCTOOL */

staticfn void save_room(NHFILE *, struct mkroom *);
staticfn void rest_room(NHFILE *, struct mkroom *);

#ifndef SFCTOOL
staticfn boolean invalid_shop_shape(struct mkroom *sroom);

#define sq(x) ((x) * (x))

extern const struct shclass shtypes[]; /* defined in shknam.c */

/**
 * @brief 방이 "큰" 방인지(면적 > 20) 판정한다.
 * @param[in] sroom 대상 방.
 * @return 큰 방이면 TRUE, 아니면 FALSE.
 */
staticfn boolean
isbig(struct mkroom *sroom)
{
    int area = (sroom->hx - sroom->lx + 1)
                        * (sroom->hy - sroom->ly + 1);

    return (boolean) (area > 20);
}

/**
 * @brief 주어진 유형의 방을 만들어 배치·초기화한다.
 * @param[in] roomtype 방 유형(@c SHOPBASE 이상=상점, @c COURT/ZOO/TEMPLE 등).
 */
/* make and stock a room of a given type */
void
do_mkroom(int roomtype)
{
    if (roomtype >= SHOPBASE) {
        mkshop(); /* someday, we should be able to specify shop type */
    } else {
        switch (roomtype) {
        case COURT:
            mkzoo(COURT);
            break;
        case ZOO:
            mkzoo(ZOO);
            break;
        case BEEHIVE:
            mkzoo(BEEHIVE);
            break;
        case MORGUE:
            mkzoo(MORGUE);
            break;
        case BARRACKS:
            mkzoo(BARRACKS);
            break;
        case SWAMP:
            mkswamp();
            break;
        case TEMPLE:
            mktemple();
            break;
        case LEPREHALL:
            mkzoo(LEPREHALL);
            break;
        case COCKNEST:
            mkzoo(COCKNEST);
            break;
        case ANTHOLE:
            mkzoo(ANTHOLE);
            break;
        default:
            impossible("Tried to make a room of type %d.", roomtype);
        }
    }
}

/**
 * @brief 상점(shop)을 생성한다.
 * @note 적당한 방을 골라 상점 유형을 지정하고 나중에 채워지도록 표시한다.
 */
staticfn void
mkshop(void)
{
    struct mkroom *sroom;
    int i = -1;
    char *ep = (char *) 0; /* (init == lint suppression) */

    /* first determine shoptype */
    if (wizard) {
        ep = nh_getenv("SHOPTYPE");
        if (ep) {
            if (*ep == 'z' || *ep == 'Z') {
                mkzoo(ZOO);
                return;
            }
            if (*ep == 'm' || *ep == 'M') {
                mkzoo(MORGUE);
                return;
            }
            if (*ep == 'b' || *ep == 'B') {
                mkzoo(BEEHIVE);
                return;
            }
            if (*ep == 't' || *ep == 'T' || *ep == '\\') {
                mkzoo(COURT);
                return;
            }
            if (*ep == 's' || *ep == 'S') {
                mkzoo(BARRACKS);
                return;
            }
            if (*ep == 'a' || *ep == 'A') {
                mkzoo(ANTHOLE);
                return;
            }
            if (*ep == 'c' || *ep == 'C') {
                mkzoo(COCKNEST);
                return;
            }
            if (*ep == 'l' || *ep == 'L') {
                mkzoo(LEPREHALL);
                return;
            }
            if (*ep == '_') {
                mktemple();
                return;
            }
            if (*ep == '}') {
                mkswamp();
                return;
            }
            for (i = 0; shtypes[i].name; i++)
                if (*ep == def_oc_syms[(int) shtypes[i].symb].sym)
                    goto gottype;
            if (*ep == 'g' || *ep == 'G')
                i = 0;
            else if (*ep == 'v' || *ep == 'V')
                i = FODDERSHOP - SHOPBASE; /* veggy food */
            else
                i = -1;
        }
    }

 gottype:
    for (sroom = &svr.rooms[0];; sroom++) {
        /* return from this loop: cannot find any eligible room to be a shop
         * continue: sroom is ineligible
         * break: sroom is eligible
         */
        if (sroom->hx < 0)
            return;
        if (sroom - svr.rooms >= svn.nroom) {
            impossible("rooms[] not closed by -1?");
            return;
        }
        if (sroom->rtype != OROOM)
            continue;
        if (has_dnstairs(sroom) || has_upstairs(sroom))
            continue;
        if (sroom->doorct == 1 || (wizard && ep && sroom->doorct != 0)) {
            if (invalid_shop_shape(sroom))
                continue;
            else
                break;
        }
    }
    if (!sroom->rlit) {
        coordxy x, y;

        for (x = sroom->lx - 1; x <= sroom->hx + 1; x++)
            for (y = sroom->ly - 1; y <= sroom->hy + 1; y++)
                levl[x][y].lit = 1;
        sroom->rlit = 1;
    }

    if (i < 0) { /* shoptype not yet determined */
        int j;

        /* pick a shop type at random */
        for (j = rnd(100), i = 0; (j -= shtypes[i].prob) > 0; i++)
            continue;

        /* big rooms cannot be wand or book shops,
         * - so make them general stores
         */
        if (isbig(sroom) && (shtypes[i].symb == WAND_CLASS
                             || shtypes[i].symb == SPBOOK_CLASS))
            i = 0;
    }
    sroom->rtype = SHOPBASE + i;

    /* set room bits before stocking the shop */
#ifdef SPECIALIZATION
    topologize(sroom, FALSE); /* doesn't matter - this is a special room */
#else
    topologize(sroom);
#endif

    /* The shop used to be stocked here, but this no longer happens--all we do
       is set its rtype, and it gets stocked at the end of makelevel() along
       with other special rooms. */
    sroom->needfill = FILL_NORMAL;
}

/**
 * @brief 특수 방으로 쓸 미사용 방을 고른다(가능하면 문이 하나뿐인 방).
 * @param[in] strict TRUE 이면 계단이 있는 방을 완전히 배제한다.
 * @return 선택된 방, 없으면 NULL.
 */
/* pick an unused room, preferably with only one door */
staticfn struct mkroom *
pick_room(boolean strict)
{
    struct mkroom *sroom;
    int i = svn.nroom;

    for (sroom = &svr.rooms[rn2(svn.nroom)]; i--; sroom++) {
        if (sroom == &svr.rooms[svn.nroom])
            sroom = &svr.rooms[0];
        if (sroom->hx < 0)
            return (struct mkroom *) 0;
        if (sroom->rtype != OROOM)
            continue;
        if (!strict) {
            if (has_upstairs(sroom) || (has_dnstairs(sroom) && rn2(3)))
                continue;
        } else if (has_upstairs(sroom) || has_dnstairs(sroom))
            continue;
        if (sroom->doorct == 1 || !rn2(5) || wizard)
            return sroom;
    }
    return (struct mkroom *) 0;
}

/**
 * @brief 지정한 유형의 "동물원류" 특수 방을 지정한다(나중에 채워짐).
 * @param[in] type 방 유형(@c COURT/ZOO/BEEHIVE/MORGUE 등).
 */
staticfn void
mkzoo(int type)
{
    struct mkroom *sroom;

    if ((sroom = pick_room(FALSE)) != 0) {
        sroom->rtype = type;
        /* room does not get stocked at this time - it will get stocked at the
         * end of makelevel() */
        sroom->needfill = FILL_NORMAL;
    }
}

/**
 * @brief 궁정(throne room) 중앙에 왕좌에 앉을 지배자 몬스터를 만든다.
 * @param[in] x,y 왕좌(지배자) 좌표.
 * @note 레벨 난이도에 따라 노움/드워프/엘프/오거 지배자가 등장한다.
 */
staticfn void
mk_zoo_thronemon(coordxy x, coordxy y)
{
    int i = rnd(level_difficulty());
    int pm = (i > 9) ? PM_OGRE_TYRANT
        : (i > 5) ? PM_ELVEN_MONARCH
        : (i > 2) ? PM_DWARF_RULER
        : PM_GNOME_RULER;
    struct monst *mon = makemon(&mons[pm], x, y, NO_MM_FLAGS);

    if (mon) {
        mon->msleeping = 1;
        mon->mpeaceful = 0;
        set_malign(mon);
        /* Give him a sceptre to pound in judgment */
        (void) mongets(mon, MACE);
    }
}

/**
 * @brief 동물원류 특수 방을 유형에 맞는 몬스터·아이템으로 채운다.
 * @param[in,out] sroom 채울 방(needfill 검사는 호출자 책임).
 * @note 궁정·동물원·벌집·묘지·병영·레프러콘 홀·코카트리스 둥지·개미굴 등을
 *       각각 다른 방식으로 채운다.
 */
void
fill_zoo(struct mkroom *sroom)
{
    struct monst *mon;
    int sx, sy, i;
    int sh, goldlim = 0, type = sroom->rtype;
    coordxy tx = 0, ty = 0;
    int rmno = (int) ((sroom - svr.rooms) + ROOMOFFSET);
    coord mm;

    /* Note: This doesn't check needfill; it assumes the caller has already
       done that. */
    sh = sroom->fdoor;
    switch (type) {
    case COURT:
        if (svl.level.flags.is_maze_lev) {
            for (tx = sroom->lx; tx <= sroom->hx; tx++)
                for (ty = sroom->ly; ty <= sroom->hy; ty++)
                    if (IS_THRONE(levl[tx][ty].typ))
                        goto throne_placed;
        }
        i = 100;
        do { /* don't place throne on top of stairs */
            (void) somexyspace(sroom, &mm);
            tx = mm.x;
            ty = mm.y;
        } while (occupied(tx, ty) && --i > 0);
 throne_placed:
        mk_zoo_thronemon(tx, ty);
        break;
    case BEEHIVE:
        tx = sroom->lx + (sroom->hx - sroom->lx + 1) / 2;
        ty = sroom->ly + (sroom->hy - sroom->ly + 1) / 2;
        if (sroom->irregular) {
            /* center might not be valid, so put queen elsewhere */
            if ((int) levl[tx][ty].roomno != rmno || levl[tx][ty].edge) {
                (void) somexyspace(sroom, &mm);
                tx = mm.x;
                ty = mm.y;
            }
        }
        break;
    case ZOO:
    case LEPREHALL:
        goldlim = 500 * level_difficulty();
        break;
    }

    for (sx = sroom->lx; sx <= sroom->hx; sx++)
        for (sy = sroom->ly; sy <= sroom->hy; sy++) {
            if (sroom->irregular) {
                if ((int) levl[sx][sy].roomno != rmno || levl[sx][sy].edge
                    || (sroom->doorct
                        && (distmin(sx, sy, svd.doors[sh].x, svd.doors[sh].y)
                            <= 1)))
                    continue;
            } else if (!SPACE_POS(levl[sx][sy].typ)
                       || (sroom->doorct
                           && ((sx == sroom->lx && svd.doors[sh].x == sx - 1)
                               || (sx == sroom->hx && svd.doors[sh].x
                                   == sx + 1)
                               || (sy == sroom->ly && svd.doors[sh].y
                                   == sy - 1)
                               || (sy == sroom->hy
                                   && svd.doors[sh].y == sy + 1))))
                continue;
            /* don't place monster on explicitly placed throne */
            if (type == COURT && IS_THRONE(levl[sx][sy].typ))
                continue;
            mon = makemon((type == COURT)
                           ? courtmon()
                           : (type == BARRACKS)
                              ? squadmon()
                              : (type == MORGUE)
                                 ? morguemon()
                                 : (type == BEEHIVE)
                                     ? (sx == tx && sy == ty
                                         ? &mons[PM_QUEEN_BEE]
                                         : &mons[PM_KILLER_BEE])
                                     : (type == LEPREHALL)
                                         ? &mons[PM_LEPRECHAUN]
                                         : (type == COCKNEST)
                                             ? &mons[PM_COCKATRICE]
                                             : (type == ANTHOLE)
                                                 ? antholemon()
                                                 : (struct permonst *) 0,
                          sx, sy, MM_ASLEEP | MM_NOGRP);
            if (mon) {
                mon->msleeping = 1;
                if (type == COURT && mon->mpeaceful) {
                    mon->mpeaceful = 0;
                    set_malign(mon);
                }
            }
            switch (type) {
            case ZOO:
            case LEPREHALL:
                if (sroom->doorct) {
                    int distval = dist2(sx, sy,
                                        svd.doors[sh].x, svd.doors[sh].y);
                    i = sq(distval);
                } else
                    i = goldlim;
                if (i >= goldlim)
                    i = 5 * level_difficulty();
                goldlim -= i;
                (void) mkgold((long) rn1(i, 10), sx, sy);
                break;
            case MORGUE:
                if (!rn2(5))
                    (void) mk_tt_object(CORPSE, sx, sy);
                if (!rn2(10)) /* lots of treasure buried with dead */
                    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST, sx, sy,
                                     TRUE, FALSE);
                if (!rn2(5))
                    make_grave(sx, sy, (char *) 0);
                break;
            case BEEHIVE:
                if (!rn2(3))
                    (void) mksobj_at(LUMP_OF_ROYAL_JELLY, sx, sy, TRUE,
                                     FALSE);
                break;
            case BARRACKS:
                if (!rn2(20)) /* the payroll and some loot */
                    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST, sx, sy,
                                     TRUE, FALSE);
                break;
            case COCKNEST:
                if (!rn2(3)) {
                    struct obj *sobj = mk_tt_object(STATUE, sx, sy);

                    if (sobj) {
                        for (i = rn2(5); i; i--)
                            (void) add_to_container(
                                sobj, mkobj(RANDOM_CLASS, FALSE));
                        sobj->owt = weight(sobj);
                    }
                }
                break;
            case ANTHOLE:
                if (!rn2(3))
                    (void) mkobj_at(FOOD_CLASS, sx, sy, FALSE);
                break;
            }
        }
    switch (type) {
    case COURT: {
        struct obj *chest, *gold;
        levl[tx][ty].typ = THRONE;
        (void) somexyspace(sroom, &mm);
        gold = mksobj(GOLD_PIECE, TRUE, FALSE);
        gold->quan = (long) rn1(50 * level_difficulty(), 10);
        gold->owt = weight(gold);
        /* the royal coffers */
        chest = mksobj_at(CHEST, mm.x, mm.y, TRUE, FALSE);
        add_to_container(chest, gold);
        chest->owt = weight(chest);
        chest->spe = 2; /* so it can be found later */
        svl.level.flags.has_court = 1;
        break;
    }
    case BARRACKS:
        svl.level.flags.has_barracks = 1;
        break;
    case ZOO:
        svl.level.flags.has_zoo = 1;
        break;
    case MORGUE:
        svl.level.flags.has_morgue = 1;
        break;
    case SWAMP:
        svl.level.flags.has_swamp = 1;
        break;
    case BEEHIVE:
        svl.level.flags.has_beehive = 1;
        break;
    }
}

/**
 * @brief 지정 지점 주위에 언데드 무리를 생성한다.
 * @param[in] mm             무리의 중심 좌표.
 * @param[in] revive_corpses TRUE 이면 근처 시체를 되살리는 것을 우선한다.
 * @param[in] mm_flags       makemon 에 전달할 플래그.
 */
/* make a swarm of undead around mm */
void
mkundead(
    coord *mm,
    boolean revive_corpses,
    int mm_flags)
{
    int cnt = (level_difficulty() + 1) / 10 + rnd(5);
    struct permonst *mdat;
    struct obj *otmp;
    coord cc;

    while (cnt--) {
        mdat = morguemon();
        if (mdat && enexto(&cc, mm->x, mm->y, mdat)
            && (!revive_corpses
                || !(otmp = sobj_at(CORPSE, cc.x, cc.y))
                || !revive(otmp, FALSE)))
            (void) makemon(mdat, cc.x, cc.y, mm_flags);
    }
    svl.level.flags.graveyard = TRUE; /* reduced chance for undead corpse */
}

/**
 * @brief 묘지(morgue)에 어울리는 언데드/악마 몬스터 종을 무작위로 고른다.
 * @return 선택된 몬스터 종 데이터.
 */
staticfn struct permonst *
morguemon(void)
{
    int i = rn2(100), hd = rn2(level_difficulty());

    if (hd > 10 && i < 10) {
        if (Inhell || In_endgame(&u.uz)) {
            return mkclass(S_DEMON, 0);
        } else {
            int ndemon_res = ndemon(A_NONE);
            if (ndemon_res != NON_PM)
                return &mons[ndemon_res];
            /* else do what? As is, it will drop to ghost/wraith/zombie */
        }
    }

    if (hd > 8 && i > 85)
        return mkclass(S_VAMPIRE, 0);

    return ((i < 20) ? &mons[PM_GHOST]
                     : (i < 40) ? &mons[PM_WRAITH]
                                : mkclass(S_ZOMBIE, 0));
}

/**
 * @brief 개미굴(anthole)에 채울 개미 종류를 고른다.
 * @return 선택된 개미 종 데이터, 모두 멸종했으면 NULL.
 * @note 같은 레벨 안에서는 동일 종, 레벨 간에는 다른 종이 나오도록 한다.
 */
struct permonst *
antholemon(void)
{
    int mtyp, indx, trycnt = 0;

    /* casts are for dealing with time_t */
    indx = (int) ((long) ubirthday % 3L);
    indx += level_difficulty();
    /* Same monsters within a level, different ones between levels */
    do {
        switch ((indx + trycnt) % 3) {
        case 0:
            mtyp = PM_SOLDIER_ANT;
            break;
        case 1:
            mtyp = PM_FIRE_ANT;
            break;
        default:
            mtyp = PM_GIANT_ANT;
            break;
        }
        /* try again if chosen type has been genocided or used up */
    } while (++trycnt < 3 && (svm.mvitals[mtyp].mvflags & G_GONE));

    return ((svm.mvitals[mtyp].mvflags & G_GONE) ? (struct permonst *) 0
                                             : &mons[mtyp]);
}

/**
 * @brief 일부 방을 늪(swamp)으로 만든다(웅덩이와 뱀장어·곰팡이 배치).
 * @note 최대 5개 방을 늪으로 바꾸며, 웅덩이에는 뱀장어/피라냐를 놓는다.
 */
staticfn void
mkswamp(void) /* Michiel Huisjes & Fred de Wilde */
{
    struct mkroom *sroom;
    int i, eelct = 0;
    coordxy sx, sy;
    int rmno;

    for (i = 0; i < 5; i++) { /* turn up to 5 rooms swampy */
        sroom = &svr.rooms[rn2(svn.nroom)];
        if (sroom->hx < 0 || sroom->rtype != OROOM || has_upstairs(sroom)
            || has_dnstairs(sroom))
            continue;

        rmno = (int)(sroom - svr.rooms) + ROOMOFFSET;

        /* satisfied; make a swamp */
        sroom->rtype = SWAMP;
        for (sx = sroom->lx; sx <= sroom->hx; sx++)
            for (sy = sroom->ly; sy <= sroom->hy; sy++) {
                if (!IS_ROOM(levl[sx][sy].typ)
                    || (int) levl[sx][sy].roomno != rmno)
                    continue;
                if (!OBJ_AT(sx, sy) && !MON_AT(sx, sy) && !t_at(sx, sy)
                    && !nexttodoor(sx, sy)) {
                    if ((sx + sy) % 2) {
                        del_engr_at(sx, sy);
                        levl[sx][sy].typ = POOL;
                        if (!eelct || !rn2(4)) {
                            /* mkclass() won't do, as we might get kraken */
                            (void) makemon(rn2(5)
                                              ? &mons[PM_GIANT_EEL]
                                              : rn2(2)
                                                 ? &mons[PM_PIRANHA]
                                                 : &mons[PM_ELECTRIC_EEL],
                                           sx, sy, NO_MM_FLAGS);
                            eelct++;
                        }
                    } else if (!rn2(4)) /* swamps tend to be moldy */
                        (void) makemon(mkclass(S_FUNGUS, 0), sx, sy,
                                       NO_MM_FLAGS);
                }
            }
        svl.level.flags.has_swamp = 1;
    }
}

/**
 * @brief 신전 방의 제단(성소) 위치를 구한다.
 * @param[in] roomno 대상 신전 방 번호.
 * @return 제단 좌표(정적 버퍼). 중앙이 막혔으면 임의의 빈 칸.
 */
/* return center of room, or a random free location
   if center is blocked */
staticfn coord *
shrine_pos(int roomno)
{
    static coord buf;
    int delta;
    struct mkroom *troom = &svr.rooms[roomno - ROOMOFFSET];

    /* if width and height are odd, placement will be the exact center;
       if either or both are even, center point is a hypothetical spot
       between map locations and placement will be adjacent to that */
    delta = troom->hx - troom->lx;
    buf.x = troom->lx + delta / 2;
    if ((delta % 2) && rn2(2))
        buf.x++;
    delta = troom->hy - troom->ly;
    buf.y = troom->ly + delta / 2;
    if ((delta % 2) && rn2(2))
        buf.y++;

    /* irregular room or the location is blocked */
    if (roomno != (int) levl[buf.x][buf.y].roomno
        || (levl[buf.x][buf.y].typ != ROOM
            && levl[buf.x][buf.y].typ != ICE
            && levl[buf.x][buf.y].typ != CLOUD))
        (void) somexyspace(troom, &buf);

    return &buf;
}

/**
 * @brief 신전(temple) 방을 만들고 중앙에 성소 제단과 사제를 배치한다.
 */
staticfn void
mktemple(void)
{
    struct mkroom *sroom;
    coord *shrine_spot;
    struct rm *lev;

    if (!(sroom = pick_room(TRUE)))
        return;

    /* set up Priest and shrine */
    sroom->rtype = TEMPLE;
    /*
     * In temples, shrines are blessed altars
     * located in the center of the room
     */
    shrine_spot = shrine_pos((int) ((sroom - svr.rooms) + ROOMOFFSET));
    lev = &levl[shrine_spot->x][shrine_spot->y];
    lev->typ = ALTAR;
    lev->altarmask = induced_align(80);
    priestini(&u.uz, sroom, shrine_spot->x, shrine_spot->y, FALSE);
    lev->altarmask |= AM_SHRINE;
    svl.level.flags.has_temple = 1;
}

/**
 * @brief 지정 좌표가 문(또는 비밀문)에 인접해 있는지 판정한다.
 * @param[in] sx,sy 확인할 좌표.
 * @return 문에 인접하면 TRUE, 아니면 FALSE.
 */
boolean
nexttodoor(int sx, int sy)
{
    int dx, dy;
    struct rm *lev;

    for (dx = -1; dx <= 1; dx++)
        for (dy = -1; dy <= 1; dy++) {
            if (!isok(sx + dx, sy + dy))
                continue;
            lev = &levl[sx + dx][sy + dy];
            if (IS_DOOR(lev->typ) || lev->typ == SDOOR)
                return TRUE;
        }
    return FALSE;
}

/**
 * @brief 방 안에 내려가는 계단이 있는지 판정한다.
 * @param[in] sroom 대상 방.
 * @return 내려가는 계단이 있으면 TRUE, 아니면 FALSE.
 */
boolean
has_dnstairs(struct mkroom *sroom)
{
    stairway *stway = gs.stairs;

    while (stway) {
        if (!stway->up && inside_room(sroom, stway->sx, stway->sy))
            return TRUE;
        stway = stway->next;
    }
    return FALSE;
}

/**
 * @brief 방 안에 올라가는 계단이 있는지 판정한다.
 * @param[in] sroom 대상 방.
 * @return 올라가는 계단이 있으면 TRUE, 아니면 FALSE.
 */
boolean
has_upstairs(struct mkroom *sroom)
{
    stairway *stway = gs.stairs;

    while (stway) {
        if (stway->up && inside_room(sroom, stway->sx, stway->sy))
            return TRUE;
        stway = stway->next;
    }
    return FALSE;
}

/**
 * @brief 방 내부의 무작위 x 좌표를 반환한다.
 * @param[in] croom 대상 방.
 * @return 방 범위 내의 무작위 x 좌표.
 */
int
somex(struct mkroom *croom)
{
    return rn1(croom->hx - croom->lx + 1, croom->lx);
}

/**
 * @brief 방 내부의 무작위 y 좌표를 반환한다.
 * @param[in] croom 대상 방.
 * @return 방 범위 내의 무작위 y 좌표.
 */
int
somey(struct mkroom *croom)
{
    return rn1(croom->hy - croom->ly + 1, croom->ly);
}

/**
 * @brief 지정 좌표가 방 내부에 있는지 판정한다(불규칙 방·서브룸 고려).
 * @param[in] croom 대상 방.
 * @param[in] x,y   확인할 좌표.
 * @return 방 내부이면 TRUE, 아니면 FALSE.
 */
boolean
inside_room(struct mkroom *croom, coordxy x, coordxy y)
{
    if (croom->irregular) {
        int i = (int) ((croom - svr.rooms) + ROOMOFFSET);
        return (!levl[x][y].edge && (int) levl[x][y].roomno == i);
    }

    return (boolean) (x >= croom->lx - 1 && x <= croom->hx + 1
                      && y >= croom->ly - 1 && y <= croom->hy + 1);
}

/**
 * @brief 방 내부의 임의 좌표를 하나 고른다(서브룸 제외).
 * @param[in]  croom 대상 방.
 * @param[out] c     선택된 좌표.
 * @return 적당한 좌표를 찾으면 TRUE, 못 찾으면 FALSE.
 * @note 접근 불가한 위치(벽 안 등)를 반환할 수도 있다.
 */
/* return a coord c inside mkroom croom, but not in a subroom.
   returns TRUE if any such space found.
   can return a non-accessible location, eg. inside a wall
   if a themed room is not irregular, but has some non-room terrain */
boolean
somexy(struct mkroom *croom, coord *c)
{
    int try_cnt = 0;
    int i;

    if (croom->irregular) {
        i = (int) ((croom - svr.rooms) + ROOMOFFSET);

        while (try_cnt++ < 100) {
            c->x = somex(croom);
            c->y = somey(croom);
            if (!levl[c->x][c->y].edge && (int) levl[c->x][c->y].roomno == i)
                return TRUE;
        }
        /* try harder; exhaustively search until one is found */
        for (c->x = croom->lx; c->x <= croom->hx; c->x++)
            for (c->y = croom->ly; c->y <= croom->hy; c->y++)
                if (!levl[c->x][c->y].edge
                    && (int) levl[c->x][c->y].roomno == i)
                    return TRUE;
        return FALSE;
    }

    if (!croom->nsubrooms) {
        c->x = somex(croom);
        c->y = somey(croom);
        return TRUE;
    }

    /* Check that coords doesn't fall into a subroom or into a wall */

    while (try_cnt++ < 100) {
        c->x = somex(croom);
        c->y = somey(croom);
        if (IS_WALL(levl[c->x][c->y].typ))
            continue;
        for (i = 0; i < croom->nsubrooms; i++)
            if (inside_room(croom->sbrooms[i], c->x, c->y))
                goto you_lose;
        break;
 you_lose:
        ;
    }
    if (try_cnt >= 100)
        return FALSE;
    return TRUE;
}

/**
 * @brief @c somexy() 와 같지만 접근 가능한(설 수 있는) 위치를 반환한다.
 * @param[in]  croom 대상 방.
 * @param[out] c     선택된 좌표.
 * @return 접근 가능한 좌표를 찾으면 TRUE, 못 찾으면 FALSE.
 */
/* like somexy(), but returns an accessible location */
boolean
somexyspace(struct mkroom* croom, coord *c)
{
    int trycnt = 0;
    boolean okay;

    do {
        okay = somexy(croom, c) && isok(c->x, c->y) && !occupied(c->x, c->y)
            && (levl[c->x][c->y].typ == ROOM
                || levl[c->x][c->y].typ == CORR
                || levl[c->x][c->y].typ == ICE);
    } while (trycnt++ < 100 && !okay);
    return okay;
}

/**
 * @brief 유형으로 특수 방을 검색한다.
 * @param[in] type 찾을 방 유형(@c ANY_SHOP, @c ANY_TYPE 특수값 포함).
 * @return 조건에 맞는 첫 방(서브룸 포함), 없으면 NULL.
 */
/*
 * Search for a special room given its type (zoo, court, etc...)
 *      Special values :
 *              - ANY_SHOP
 *              - ANY_TYPE
 */
struct mkroom *
search_special(schar type)
{
    struct mkroom *croom;

    for (croom = &svr.rooms[0]; croom->hx >= 0; croom++)
        if ((type == ANY_TYPE && croom->rtype != OROOM)
            || (type == ANY_SHOP && croom->rtype >= SHOPBASE)
            || croom->rtype == type)
            return croom;
    for (croom = &gs.subrooms[0]; croom->hx >= 0; croom++)
        if ((type == ANY_TYPE && croom->rtype != OROOM)
            || (type == ANY_SHOP && croom->rtype >= SHOPBASE)
            || croom->rtype == type)
            return croom;
    return (struct mkroom *) 0;
}

/**
 * @brief 궁정(court)에 어울리는 몬스터 종을 무작위로 고른다.
 * @return 선택된 몬스터 종 데이터.
 */
struct permonst *
courtmon(void)
{
    int i = rn2(60) + rn2(3 * level_difficulty());

    if (i > 100)
        return mkclass(S_DRAGON, 0);
    else if (i > 95)
        return mkclass(S_GIANT, 0);
    else if (i > 85)
        return mkclass(S_TROLL, 0);
    else if (i > 75)
        return mkclass(S_CENTAUR, 0);
    else if (i > 60)
        return mkclass(S_ORC, 0);
    else if (i > 45)
        return &mons[PM_BUGBEAR];
    else if (i > 30)
        return &mons[PM_HOBGOBLIN];
    else if (i > 15)
        return mkclass(S_GNOME, 0);
    else
        return mkclass(S_KOBOLD, 0);
}

static const struct {
    unsigned pm;
    unsigned prob;
} squadprob[] = { { PM_SOLDIER, 80 },
                  { PM_SERGEANT, 15 },
                  { PM_LIEUTENANT, 4 },
                  { PM_CAPTAIN, 1 } };

/**
 * @brief 병영(barracks)에 배치할 병사 종류를 확률에 따라 고른다.
 * @return 선택된 병사 종 데이터, 해당 종이 멸종했으면 NULL.
 */
/* return soldier types. */
staticfn struct permonst *
squadmon(void)
{
    int sel_prob, i, cpro, mndx;

    sel_prob = rnd(80 + level_difficulty());

    cpro = 0;
    for (i = 0; i < SIZE(squadprob); i++) {
        cpro += squadprob[i].prob;
        if (cpro > sel_prob) {
            mndx = squadprob[i].pm;
            goto gotone;
        }
    }
    mndx = ROLL_FROM(squadprob).pm;
 gotone:
    if (!(svm.mvitals[mndx].mvflags & G_GONE))
        return &mons[mndx];
    else
        return (struct permonst *) 0;
}

/**
 * @brief 방과 그 서브룸을 재귀적으로 파일에 저장한다.
 * @param[in,out] nhfp 저장 대상 파일 핸들.
 * @param[in]     r    저장할 방.
 */
/*
 * save_room : A recursive function that saves a room and its subrooms
 * (if any).
 */
staticfn void
save_room(NHFILE *nhfp, struct mkroom *r)
{
    short i;

    /*
     * Well, I really should write only useful information instead
     * of writing the whole structure. That is I should not write
     * the gs.subrooms pointers, but who cares ?
     */
    Sfo_mkroom(nhfp, r, "room-mkroom");
    for (i = 0; i < r->nsubrooms; i++) {
        save_room(nhfp, r->sbrooms[i]);
    }
}

/**
 * @brief 현재 레벨의 모든 방을 파일에 저장한다.
 * @param[in,out] nhfp 저장 대상 파일 핸들.
 */
/*
 * save_rooms : Save all the rooms on disk!
 */
void
save_rooms(NHFILE *nhfp)
{
    short i;

    /* First, write the number of rooms */
    Sfo_int(nhfp, &svn.nroom, "room-nroom");
    for (i = 0; i < svn.nroom; i++)
        save_room(nhfp, &svr.rooms[i]);
}
#endif /* !SFCTOOL */

/**
 * @brief 방과 그 서브룸을 파일에서 재귀적으로 복원한다.
 * @param[in,out] nhfp 복원 원본 파일 핸들.
 * @param[out]    r    복원할 방.
 */
staticfn void
rest_room(NHFILE *nhfp, struct mkroom *r)
{
    short i;

    Sfi_mkroom(nhfp, r, "room-mkroom");

    for (i = 0; i < r->nsubrooms; i++) {
        r->sbrooms[i] = &gs.subrooms[gn.nsubroom];
        rest_room(nhfp, &gs.subrooms[gn.nsubroom]);
        gs.subrooms[gn.nsubroom++].resident = (struct monst *) 0;
    }
}

/**
 * @brief 파일에서 현재 레벨의 모든 방을 복원한다.
 * @param[in,out] nhfp 복원 원본 파일 핸들.
 */
/*
 * rest_rooms : That's for restoring rooms. Read the rooms structure from
 * the disk.
 */
void
rest_rooms(NHFILE *nhfp)
{
    short i;

    Sfi_int(nhfp, &svn.nroom, "room-nroom");

    gn.nsubroom = 0;
    for (i = 0; i < svn.nroom; i++) {
        rest_room(nhfp, &svr.rooms[i]);
        svr.rooms[i].resident = (struct monst *) 0;
    }
    svr.rooms[svn.nroom].hx = -1; /* restore ending flags */
    gs.subrooms[gn.nsubroom].hx = -1;
}

#ifndef SFCTOOL
/**
 * @brief 지형 표시 심볼(@c S_xxx)을 지형 타입 코드로 변환한다.
 * @param[in] sym 지형 표시 심볼.
 * @return 대응하는 지형 타입 코드(알 수 없으면 @c STONE).
 * @note 흉내쟁이가 지형으로 위장할 때 기억된 지형을 복원하는 데 쓰인다.
 */
/* convert a display symbol for terrain into topology type;
   used for remembered terrain when mimics pose as furniture */
int
cmap_to_type(int sym)
{
    int typ = STONE; /* catchall */

    switch (sym) {
    case S_stone:
        typ = STONE;
        break;
    case S_vwall:
        typ = VWALL;
        break;
    case S_hwall:
        typ = HWALL;
        break;
    case S_tlcorn:
        typ = TLCORNER;
        break;
    case S_trcorn:
        typ = TRCORNER;
        break;
    case S_blcorn:
        typ = BLCORNER;
        break;
    case S_brcorn:
        typ = BRCORNER;
        break;
    case S_crwall:
        typ = CROSSWALL;
        break;
    case S_tuwall:
        typ = TUWALL;
        break;
    case S_tdwall:
        typ = TDWALL;
        break;
    case S_tlwall:
        typ = TLWALL;
        break;
    case S_trwall:
        typ = TRWALL;
        break;
    case S_ndoor:  /* no door (empty doorway) */
    case S_vodoor: /* open door in vertical wall */
    case S_hodoor: /* open door in horizontal wall */
    case S_vcdoor: /* closed door in vertical wall */
    case S_hcdoor:
        typ = DOOR;
        break;
    case S_bars:
        typ = IRONBARS;
        break;
    case S_tree:
        typ = TREE;
        break;
    case S_room:
    case S_darkroom:
        typ = ROOM;
        break;
    case S_corr:
    case S_litcorr:
        typ = CORR;
        break;
    case S_upstair:
    case S_dnstair:
        typ = STAIRS;
        break;
    case S_upladder:
    case S_dnladder:
        typ = LADDER;
        break;
    case S_altar:
        typ = ALTAR;
        break;
    case S_grave:
        typ = GRAVE;
        break;
    case S_throne:
        typ = THRONE;
        break;
    case S_sink:
        typ = SINK;
        break;
    case S_fountain:
        typ = FOUNTAIN;
        break;
    case S_pool:
        typ = POOL;
        break;
    case S_ice:
        typ = ICE;
        break;
    case S_lava:
        typ = LAVAPOOL;
        break;
    case S_vodbridge: /* open drawbridge spanning north/south */
    case S_hodbridge:
        typ = DRAWBRIDGE_DOWN;
        break;        /* east/west */
    case S_vcdbridge: /* closed drawbridge in vertical wall */
    case S_hcdbridge:
        typ = DBWALL;
        break;
    case S_air:
        typ = AIR;
        break;
    case S_cloud:
        typ = CLOUD;
        break;
    case S_water:
        typ = WATER;
        break;
    case S_lavawall:
        typ = LAVAWALL;
        break;
    default:
        break; /* not a cmap symbol? */
    }
    return typ;
}

/* With the introduction of themed rooms, there are certain room shapes that
 * may generate a door, the square just inside the door, and only one other
 * ROOM square touching that one. E.g.
 *   ---
 * ---..
 * +....
 * ---..
 *   ---
 * This means that if the room becomes a shop, the shopkeeper will move
 * between those two squares nearest the door without ever allowing the
 * player to get past them.
 * Before approving sroom as a shop, check for this circumstance, and if it
 * exists, don't consider it as valid for a shop.
 *
 * Note that the invalidity of the shape derives from the position of its door
 * already being chosen. It's quite possible that if the door were somewhere
 * else on the perimeter of this room, it would work fine as a shop.*/
/**
 * @brief 방의 모양이 상점으로 쓰기에 부적합한지 판정한다.
 * @param[in] sroom 대상 방.
 * @return 상점으로 부적합한 모양이면 TRUE, 적합하면 FALSE.
 * @note 문 안쪽 칸에 인접한 ROOM 칸이 하나뿐이면 상점 주인이 갇힐 수 있어
 *       부적합으로 본다.
 */
staticfn boolean
invalid_shop_shape(struct mkroom *sroom)
{
    coordxy x, y;
    coordxy doorx = svd.doors[sroom->fdoor].x;
    coordxy doory = svd.doors[sroom->fdoor].y;
    coordxy insidex = 0, insidey = 0, insidect = 0;

    /* First, identify squares inside the room and next to the door. */
    for (x = max(doorx - 1, sroom->lx);
         x <= min(doorx + 1, sroom->hx); x++) {
        for (y = max(doory - 1, sroom->ly);
             y <= min(doory + 1, sroom->hy); y++) {
            if (levl[x][y].typ == ROOM) {
                insidex = x;
                insidey = y;
                insidect++;
            }
        }
    }
    if (insidect < 1) {
        impossible("invalid_shop_shape: no squares inside door?");
        return TRUE;
    }
    /* if insidect > 1, then the shopkeeper already has alternate
     * squares to move to so we don't need to check further. */
    if (insidect == 1) {
        /* But if it is 1, scan all adjacent squares for other squares
         * that are part of this room. */
        insidect = 0;
        for (x = max(insidex - 1, sroom->lx);
             x <= min(insidex + 1, sroom->hx); x++) {
            for (y = max(insidey - 1, sroom->ly);
                 y <= min(insidey + 1, sroom->hy); y++) {
                if (x == insidex && y == insidey)
                    continue;
                if (levl[x][y].typ == ROOM)
                    insidect++;
            }
        }
        if (insidect == 1) {
            /* shopkeeper standing just inside the door can only move
             * to one other square; this cannot be a shop. */
            return TRUE;
        }
    }
    return FALSE;
}
#endif /* !SFCTOOL */

/*mkroom.c*/
