/* NetHack 5.0	dbridge.c	$NHDT-Date: 1781973044 2026/06/20 16:30:44 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.74 $ */
/*      Copyright (c) 1989 by Jean-Christophe Collet              */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file dbridge.c
 * @brief 도개교(drawbridge) 조작과 지형 판정 및 엔티티 처리.
 *
 * 도개교의 생성/열기/닫기/파괴와, 물/용암/얼음/해자 등 지형 판정 함수를
 * 제공한다. 도개교가 오르내리거나 파괴될 때 그 위/아래에 있는 영웅과
 * 몬스터를 공통으로 다루기 위해 "엔티티(entity)" 구조를 사용한다.
 *
 * @note 엔티티 처리용 static 헬퍼가 다수 있으나 재배치는 적용하지 않고 정의
 *       위치에서 문서화한다. 도개교 개폐/파괴 시 해당 위치의 함정·조각은 제거된다.
 */

/*
 * This file contains the drawbridge manipulation (create, open, close,
 * destroy).
 *
 * Added comprehensive monster-handling, and the "entity" structure to
 * deal with players as well. - 11/89
 *
 * Any traps and/or engravings at either the portcullis or span location
 * are destroyed whenever the bridge is lowered, raised, or destroyed.
 * (Engraving handling could be extended to flag whether an engraving on
 * the DB_UNDER surface is hidden by the lowered bridge, or one on the
 * bridge itself is hidden because the bridge has been raised, but that
 * seems like an awful lot of effort for very little gain.)
 */

#include "hack.h"

staticfn void get_wall_for_db(coordxy *, coordxy *);
staticfn struct entity *e_at(coordxy, coordxy);
staticfn void m_to_e(struct monst *, coordxy, coordxy, struct entity *);
staticfn void u_to_e(struct entity *);
staticfn void set_entity(coordxy, coordxy, struct entity *);
staticfn const char *e_nam(struct entity *);
staticfn const char *E_phrase(struct entity *, const char *);
staticfn boolean e_survives_at(struct entity *, coordxy, coordxy);
staticfn void e_died(struct entity *, int, int);
staticfn boolean automiss(struct entity *);
staticfn boolean e_missed(struct entity *, boolean);
staticfn boolean e_jumps(struct entity *);
staticfn void do_entity(struct entity *);
staticfn void nokiller(void);

/**
 * @brief 지정 좌표가 물벽(water wall)인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 물벽이면 TRUE, 아니면 FALSE.
 */
boolean
is_waterwall(coordxy x, coordxy y)
{
    if (isok(x, y) && IS_WATERWALL(levl[x][y].typ))
        return TRUE;
    return FALSE;
}

/**
 * @brief 지정 좌표가 물(웅덩이/해자/물)인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 물이면 TRUE, 아니면 FALSE.
 */
boolean
is_pool(coordxy x, coordxy y)
{
    schar ltyp;

    if (!isok(x, y))
        return FALSE;
    ltyp = levl[x][y].typ;
    /* The ltyp == MOAT is not redundant with is_moat, because the
     * Juiblex level does not have moats, although it has MOATs. There
     * is probably a better way to express this. */
    if (ltyp == POOL || ltyp == MOAT || ltyp == WATER || is_moat(x, y))
        return TRUE;
    return FALSE;
}

/**
 * @brief 지정 좌표가 용암인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 용암이면 TRUE, 아니면 FALSE.
 */
boolean
is_lava(coordxy x, coordxy y)
{
    schar ltyp;

    if (!isok(x, y))
        return FALSE;
    ltyp = levl[x][y].typ;
    if (ltyp == LAVAPOOL || ltyp == LAVAWALL
        || (ltyp == DRAWBRIDGE_UP
            && (levl[x][y].drawbridgemask & DB_UNDER) == DB_LAVA))
        return TRUE;
    return FALSE;
}

/**
 * @brief 지정 좌표가 물 또는 용암인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 물이거나 용암이면 TRUE, 아니면 FALSE.
 */
boolean
is_pool_or_lava(coordxy x, coordxy y)
{
    if (is_pool(x, y) || is_lava(x, y))
        return TRUE;
    else
        return FALSE;
}

/**
 * @brief 지정 좌표가 얼음인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 얼음이면 TRUE, 아니면 FALSE.
 */
boolean
is_ice(coordxy x, coordxy y)
{
    schar ltyp;

    if (!isok(x, y))
        return FALSE;
    ltyp = levl[x][y].typ;
    if (ltyp == ICE || (ltyp == DRAWBRIDGE_UP
                        && (levl[x][y].drawbridgemask & DB_UNDER) == DB_ICE))
        return TRUE;
    return FALSE;
}

/**
 * @brief 지정 좌표가 해자(moat)인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 해자이면 TRUE, 아니면 FALSE(주이블렉스 레벨에서는 항상 FALSE).
 */
boolean
is_moat(coordxy x, coordxy y)
{
    schar ltyp;

    if (!isok(x, y))
        return FALSE;
    ltyp = levl[x][y].typ;
    if (!Is_juiblex_level(&u.uz)
        && (ltyp == MOAT
            || (ltyp == DRAWBRIDGE_UP
                && (levl[x][y].drawbridgemask & DB_UNDER) == DB_MOAT)))
        return TRUE;
    return FALSE;
}

/**
 * @brief 도개교 마스크의 DB_UNDER 값에 대응하는 지형 타입을 반환한다.
 * @param[in] mask 도개교 마스크(@c drawbridgemask).
 * @return 아래 지형 타입(@c ICE/LAVAPOOL/MOAT/STONE).
 */
schar
db_under_typ(int mask)
{
    switch (mask & DB_UNDER) {
    case DB_ICE:
        return ICE;
    case DB_LAVA:
        return LAVAPOOL;
    case DB_MOAT:
        return MOAT;
    default:
        return STONE;
    }
}

/**
 * @brief 벽(또는 문)이 도개교의 통로(portcullis)인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 도개교가 있는 방향(@c DB_WEST/EAST/SOUTH/NORTH), 아니면 -1.
 */
/*
 * We want to know whether a wall (or a door) is the portcullis (passageway)
 * of an eventual drawbridge.
 *
 * Return value:  the direction of the drawbridge, or -1 if not valid
 */
int
is_drawbridge_wall(coordxy x, coordxy y)
{
    struct rm *lev;

    if (!isok(x, y))
        return -1;

    lev = &levl[x][y];
    if (lev->typ != DOOR && lev->typ != DBWALL)
        return -1;

    if (isok(x + 1, y) && IS_DRAWBRIDGE(levl[x + 1][y].typ)
        && (levl[x + 1][y].drawbridgemask & DB_DIR) == DB_WEST)
        return DB_WEST;
    if (isok(x - 1, y) && IS_DRAWBRIDGE(levl[x - 1][y].typ)
        && (levl[x - 1][y].drawbridgemask & DB_DIR) == DB_EAST)
        return DB_EAST;
    if (isok(x, y - 1) && IS_DRAWBRIDGE(levl[x][y - 1].typ)
        && (levl[x][y - 1].drawbridgemask & DB_DIR) == DB_SOUTH)
        return DB_SOUTH;
    if (isok(x, y + 1) && IS_DRAWBRIDGE(levl[x][y + 1].typ)
        && (levl[x][y + 1].drawbridgemask & DB_DIR) == DB_NORTH)
        return DB_NORTH;

    return -1;
}

/**
 * @brief 지정 좌표의 도개교 "벽"이 올라간(UP) 상태인지 판정한다.
 * @param[in] x,y 확인할 좌표.
 * @return 올라간 도개교 벽(@c DBWALL)이면 TRUE, 아니면 FALSE.
 * @note 올라감/내려감을 모두 보는 @c is_drawbridge_wall 과 달리 UP만 확인한다.
 */
/*
 * Use is_db_wall where you want to verify that a
 * drawbridge "wall" is UP in the location x, y
 * (instead of UP or DOWN, as with is_drawbridge_wall).
 */
boolean
is_db_wall(coordxy x, coordxy y)
{
    return (boolean) (levl[x][y].typ == DBWALL);
}

/**
 * @brief 도개교 또는 그 벽 좌표를 실제 도개교 좌표로 보정한다.
 * @param[in,out] x,y 입력 좌표(도개교/벽), 성공 시 도개교 좌표로 갱신된다.
 * @return 도개교(또는 그 벽)를 가리키면 TRUE, 아니면 FALSE.
 */
/*
 * Return true with x,y pointing to the drawbridge if x,y initially indicate
 * a drawbridge or drawbridge wall.
 */
boolean
find_drawbridge(coordxy *x, coordxy *y)
{
    int dir;

    if (IS_DRAWBRIDGE(levl[*x][*y].typ))
        return TRUE;
    dir = is_drawbridge_wall(*x, *y);
    if (dir >= 0) {
        switch (dir) {
        case DB_NORTH:
            (*y)++;
            break;
        case DB_SOUTH:
            (*y)--;
            break;
        case DB_EAST:
            (*x)--;
            break;
        case DB_WEST:
            (*x)++;
            break;
        }
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief 도개교에 연결된 도개교 벽(통로) 좌표를 구한다.
 * @param[in,out] x,y 입력은 도개교 좌표, 출력은 연결된 벽 좌표.
 */
/*
 * Find the drawbridge wall associated with a drawbridge.
 */
staticfn void
get_wall_for_db(coordxy *x, coordxy *y)
{
    switch (levl[*x][*y].drawbridgemask & DB_DIR) {
    case DB_NORTH:
        (*y)--;
        break;
    case DB_SOUTH:
        (*y)++;
        break;
    case DB_EAST:
        (*x)++;
        break;
    case DB_WEST:
        (*x)--;
        break;
    }
}

/**
 * @brief 지정 위치에 도개교를 생성한다.
 * @param[in] x,y  도개교(span) 위치.
 * @param[in] dir  도개교 방향(@c DB_NORTH/SOUTH/EAST/WEST).
 * @param[in] flag TRUE 이면 열린(내려간) 상태로, FALSE 이면 닫힌 상태로 만든다.
 * @return 생성에 성공하면 TRUE, 인접 위치가 벽이 아니면 FALSE.
 */
/*
 * Creation of a drawbridge at pos x,y.
 *     dir is the direction.
 *     flag must be put to TRUE if we want the drawbridge to be opened.
 */
boolean
create_drawbridge(coordxy x, coordxy y, int dir, boolean flag)
{
    coordxy x2, y2;
    boolean horiz;
    boolean lava = levl[x][y].typ == LAVAPOOL; /* assume initialized map */

    x2 = x;
    y2 = y;
    switch (dir) {
    case DB_NORTH:
        horiz = TRUE;
        y2--;
        break;
    case DB_SOUTH:
        horiz = TRUE;
        y2++;
        break;
    case DB_EAST:
        horiz = FALSE;
        x2++;
        break;
    default:
        impossible("bad direction in create_drawbridge");
        FALLTHROUGH;
        /*FALLTHRU*/
    case DB_WEST:
        horiz = FALSE;
        x2--;
        break;
    }
    if (!IS_WALL(levl[x2][y2].typ))
        return FALSE;
    if (flag) { /* We want the bridge open */
        levl[x][y].typ = DRAWBRIDGE_DOWN;
        levl[x2][y2].typ = DOOR;
        levl[x2][y2].doormask = D_NODOOR;
    } else {
        levl[x][y].typ = DRAWBRIDGE_UP;
        levl[x2][y2].typ = DBWALL;
        /* Drawbridges are non-diggable. */
        levl[x2][y2].wall_info = W_NONDIGGABLE;
    }
    levl[x][y].horizontal = !horiz;
    levl[x2][y2].horizontal = horiz;
    levl[x][y].drawbridgemask = dir;
    if (lava)
        levl[x][y].drawbridgemask |= DB_LAVA;
    return  TRUE;
}

/**
 * @brief 지정 좌표에 있는 엔티티(occupants 중 하나)를 찾는다.
 * @param[in] x,y 확인할 좌표.
 * @return 해당 좌표의 엔티티 포인터, 없으면 NULL.
 */
staticfn struct entity *
e_at(coordxy x, coordxy y)
{
    int entitycnt;

    for (entitycnt = 0; entitycnt < ENTITIES; entitycnt++)
        if (go.occupants[entitycnt].edata
            && go.occupants[entitycnt].ex == x
            && go.occupants[entitycnt].ey == y)
            break;
    debugpline1("entitycnt = %d", entitycnt);
#ifdef D_DEBUG
    wait_synch();
#endif
    return (entitycnt == ENTITIES) ? (struct entity *) 0
                                   : &(go.occupants[entitycnt]);
}

/**
 * @brief 몬스터를 엔티티 구조체로 채운다.
 * @param[in]  mtmp 대상 몬스터(NULL 이면 빈 엔티티).
 * @param[in]  x,y  엔티티 위치.
 * @param[out] etmp 채울 엔티티.
 * @note 긴 벌레의 꼬리 위치이면 꼬리 종 데이터를 사용한다.
 */
staticfn void
m_to_e(struct monst *mtmp, coordxy x, coordxy y, struct entity *etmp)
{
    etmp->emon = mtmp;
    if (mtmp) {
        etmp->ex = x;
        etmp->ey = y;
        if (mtmp->wormno && (x != mtmp->mx || y != mtmp->my))
            etmp->edata = &mons[PM_LONG_WORM_TAIL];
        else
            etmp->edata = mtmp->data;
    } else {
        etmp->edata = (struct permonst *) 0;
        etmp->ex = etmp->ey = 0;
    }
}

/**
 * @brief 영웅을 엔티티 구조체로 채운다.
 * @param[out] etmp 채울 엔티티.
 */
staticfn void
u_to_e(struct entity *etmp)
{
    etmp->emon = &gy.youmonst;
    etmp->ex = u.ux;
    etmp->ey = u.uy;
    etmp->edata = gy.youmonst.data;
}

/**
 * @brief 지정 좌표의 점유자(영웅 또는 몬스터)로 엔티티를 설정한다.
 * @param[in]  x,y  span 또는 portcullis 위치.
 * @param[out] etmp 채울 엔티티(occupants[0] 또는 [1]).
 */
staticfn void
set_entity(
    coordxy x, coordxy y, /* location of span or portcullis */
    struct entity *etmp)  /* pointer to occupants[0] or occupants[1] */
{
    if (u_at(x, y))
        u_to_e(etmp);
    else /* m_at() might yield Null; that's ok */
        m_to_e(m_at(x, y), x, y, etmp);
}

#define is_u(etmp) (etmp->emon == &gy.youmonst)
#define e_canseemon(etmp) (is_u(etmp) || canseemon(etmp->emon))

/*
 * e_strg is a utility routine which is not actually in use anywhere, since
 * the specialized routines below suffice for all current purposes.
 */

/* #define e_strg(etmp, func) (is_u(etmp) ? (char *) 0 : func(etmp->emon)) */

/**
 * @brief 엔티티의 이름 문자열을 반환한다.
 * @param[in] etmp 대상 엔티티.
 * @return 영웅이면 "you", 몬스터면 해당 이름.
 */
staticfn const char *
e_nam(struct entity *etmp)
{
    return is_u(etmp) ? "you" : mon_nam(etmp->emon);
}

/**
 * @brief 대문자로 시작하는 "엔티티 + 동사" 구절을 만든다.
 * @param[in] etmp 대상 엔티티.
 * @param[in] verb 동사(필요 시 2인칭→3인칭 변환).
 * @return 조합된 구절(정적 버퍼).
 */
/*
 * Generates capitalized entity name, makes 2nd -> 3rd person conversion on
 * verb, where necessary.
 */
staticfn const char *
E_phrase(struct entity *etmp, const char *verb)
{
    static char wholebuf[80];

    Strcpy(wholebuf, is_u(etmp) ? "You" : Monnam(etmp->emon));
    if (!verb || !*verb)
        return wholebuf;
    Strcat(wholebuf, " ");
    if (is_u(etmp))
        Strcat(wholebuf, verb);
    else
        Strcat(wholebuf, vtense((char *) 0, verb));
    return wholebuf;
}

/**
 * @brief 엔티티가 지정 좌표에서 생존할 수 있는지 판정한다.
 * @param[in] etmp 대상 엔티티.
 * @param[in] x,y  확인할 좌표.
 * @return 그곳에서 생존 가능하면 TRUE, 아니면 FALSE.
 * @note 물/용암/도개교 벽 등 지형과 엔티티의 비행·수영·통과 능력을 고려한다.
 */
/*
 * Simple-minded "can it be here?" routine
 */
staticfn boolean
e_survives_at(struct entity *etmp, coordxy x, coordxy y)
{
    if (noncorporeal(etmp->edata))
        return TRUE;
    if (is_pool(x, y))
        return (boolean) ((is_u(etmp) && (Wwalking || Amphibious || Breathless
                                          || Swimming || Flying || Levitation))
                          || is_swimmer(etmp->edata)
                          || is_flyer(etmp->edata)
                          || is_floater(etmp->edata));
    /* must force call to lava_effects in e_died if is_u */
    if (is_lava(x, y))
        return (boolean) ((is_u(etmp) && (Levitation || Flying))
                          || likes_lava(etmp->edata)
                          || is_flyer(etmp->edata));
    if (is_db_wall(x, y))
        return (boolean) (is_u(etmp) ? Passes_walls
                          : passes_walls(etmp->edata));
    return TRUE;
}

/**
 * @brief 도개교/통로에 의해 엔티티가 죽는 처리를 수행한다.
 * @param[in,out] etmp        죽는 엔티티.
 * @param[in]     xkill_flags 처치 처리 플래그(메시지/시체 억제 등).
 * @param[in]     how         사인(@c DROWNING/BURNING/CRUSHING 등).
 * @note 영웅이면 익사/용암 처리로 위임하거나 생존 시 안전한 곳으로 텔레포트하며,
 *       긴 벌레의 여러 엔티티를 함께 정리한다.
 */
staticfn void
e_died(
    struct entity *etmp,
    int xkill_flags, int how)
{
    if (is_u(etmp)) {
        if (how == DROWNING) {
            svk.killer.name[0] = 0; /* drown() sets its own killer */
            (void) drown();
        } else if (how == BURNING) {
            svk.killer.name[0] = 0; /* lava_effects() sets own killer */
            (void) lava_effects();
        } else {
            coord xy;

            /* use more specific killer if specified */
            if (!svk.killer.name[0]) {
                svk.killer.format = KILLED_BY_AN;
                Strcpy(svk.killer.name, "falling drawbridge");
            }
            done(how);
            /* So, you didn't die */
            if (!e_survives_at(etmp, etmp->ex, etmp->ey)) {
                if (enexto(&xy, etmp->ex, etmp->ey, etmp->edata)) {
                    pline("A %s force teleports you away...",
                          Hallucination ? "normal" : "strange");
                    teleds(xy.x, xy.y, TELEDS_NO_FLAGS);
                }
                /* otherwise on top of the drawbridge is the
                 * only viable spot in the dungeon, so stay there
                 */
            }
        }
        /* we might have crawled out of the moat to survive */
        etmp->ex = u.ux, etmp->ey = u.uy;
    } else {
        int entitycnt;

        svk.killer.name[0] = 0;
/* fake "digested to death" damage-type suppresses corpse */
#define mk_message(dest) (((dest & XKILL_NOMSG) != 0) ? (char *) 0 : "")
#define mk_corpse(dest) (((dest & XKILL_NOCORPSE) != 0) ? AD_DGST : AD_PHYS)
        /* if monsters are moving, one of them caused the destruction */
        if (svc.context.mon_moving)
            monkilled(etmp->emon,
                      mk_message(xkill_flags), mk_corpse(xkill_flags));
        else /* you caused it */
            xkilled(etmp->emon, xkill_flags);

        /* if etmp gets life-saved, kill it again; otherwise we might end up
           trying to place another monster (probably a xorn) on same spot */
        if (!DEADMONSTER(etmp->emon)) {
            int seeit = canspotmon(etmp->emon);

            xkill_flags |= XKILL_NOMSG | XKILL_NOCONDUCT;
            if (svc.context.mon_moving)
                monkilled(etmp->emon, "", mk_corpse(xkill_flags));
            else /* you caused it */
                xkilled(etmp->emon, xkill_flags);

            if (DEADMONSTER(etmp->emon)) {
                if (seeit)
                    pline("Unfortunately for %s, %s is still crushed.",
                          mon_nam(etmp->emon), mhe(etmp->emon));
            } else {
                ; /* FIXME: still not dead?  What should we do now? */
            }
        }
        etmp->edata = (struct permonst *) 0;

        /* dead long worm handling */
        for (entitycnt = 0; entitycnt < ENTITIES; entitycnt++) {
            if (etmp != &(go.occupants[entitycnt])
                && etmp->emon == go.occupants[entitycnt].emon)
                go.occupants[entitycnt].edata = (struct permonst *) 0;
        }
#undef mk_message
#undef mk_corpse
    }
}

/**
 * @brief 엔티티가 도개교/통로의 영향을 원천적으로 받지 않는지 판정한다.
 * @param[in] etmp 대상 엔티티.
 * @return 벽을 통과하거나 비물질(noncorporeal)이면 TRUE, 아니면 FALSE.
 */
/*
 * These are never directly affected by a bridge or portcullis.
 */
staticfn boolean
automiss(struct entity *etmp)
{
    return (boolean) ((is_u(etmp) ? Passes_walls : passes_walls(etmp->edata))
                      || noncorporeal(etmp->edata));
}

/**
 * @brief 떨어지는 도개교/통로(또는 파편)가 엔티티를 빗나가는지 판정한다.
 * @param[in] etmp   대상 엔티티.
 * @param[in] chunks TRUE 이면 파괴 시 흩날리는 금속 파편에 대한 판정.
 * @return 빗나가면 TRUE, 맞으면 FALSE.
 * @note 비행/부유/통과 능력과 위치(통로 안 등)에 따라 회피 확률이 달라진다.
 */
/*
 * Does falling drawbridge or portcullis miss etmp?
 */
staticfn boolean
e_missed(struct entity *etmp, boolean chunks)
{
    int misses;

    if (chunks) {
        debugpline0("Do chunks miss?");
    }
    if (automiss(etmp))
        return TRUE;

    if (is_flyer(etmp->edata)
        && (is_u(etmp) ? !Unaware
                       : !helpless(etmp->emon)))
        /* flying requires mobility */
        misses = 5; /* out of 8 */
    else if (is_floater(etmp->edata)
             || (is_u(etmp) && Levitation)) /* doesn't require mobility */
        misses = 3;
    else if (chunks && is_pool(etmp->ex, etmp->ey))
        misses = 2; /* sitting ducks */
    else
        misses = 0;

    if (is_db_wall(etmp->ex, etmp->ey))
        misses -= 3; /* less airspace */

    debugpline1("Miss chance = %d (out of 8)", misses);

    return (misses >= rnd(8)) ? TRUE : FALSE;
}

/**
 * @brief 엔티티가 죽음을 피해 (통로 밖으로) 뛰어내릴 수 있는지 판정한다.
 * @param[in] etmp 대상 엔티티.
 * @return 뛰어내리기에 성공하면 TRUE, 아니면 FALSE.
 * @note 무력/혼란/기절 상태와 위치에 따라 성공 확률이 낮아진다.
 */
/*
 * Can etmp jump from death?
 */
staticfn boolean
e_jumps(struct entity *etmp)
{
    int tmp = 4; /* out of 10 */

    if (is_u(etmp) ? (Unaware || Fumbling)
                   : (helpless(etmp->emon)
                      || !etmp->edata->mmove || etmp->emon->wormno))
        return FALSE;

    if (is_u(etmp) ? Confusion : etmp->emon->mconf)
        tmp -= 2;

    if (is_u(etmp) ? Stunned : etmp->emon->mstun)
        tmp -= 3;

    if (is_db_wall(etmp->ex, etmp->ey))
        tmp -= 2; /* less room to maneuver */

    debugpline2("%s to jump (%d chances in 10)", E_phrase(etmp, "try"), tmp);
    return (tmp >= rnd(10)) ? TRUE : FALSE;
}

/**
 * @brief 도개교 개폐/파괴 시 한 엔티티의 운명을 처리한다.
 *
 * 회피/점프 판정을 거쳐 짓눌림, 이동(relocation), 물·용암 낙하 등을 결정하고
 * 필요 시 다른 엔티티와의 자리 충돌을 재귀적으로 해소한다.
 *
 * @param[in,out] etmp 처리할 엔티티.
 */
staticfn void
do_entity(struct entity *etmp)
{
    coordxy newx, newy, oldx, oldy;
    int at_portcullis;
    boolean must_jump = FALSE, relocates = FALSE, e_inview;
    struct rm *crm;

    if (!etmp->edata)
        return;

    e_inview = e_canseemon(etmp);
    oldx = etmp->ex;
    oldy = etmp->ey;
    at_portcullis = is_db_wall(oldx, oldy);
    crm = &levl[oldx][oldy];

    if (automiss(etmp) && e_survives_at(etmp, oldx, oldy)) {
        if (e_inview && (at_portcullis || IS_DRAWBRIDGE(crm->typ)))
            pline_The("%s passes through %s!",
                      at_portcullis ? "portcullis" : "drawbridge",
                      e_nam(etmp));
        if (is_u(etmp))
            spoteffects(FALSE);
        return;
    }
    if (e_missed(etmp, FALSE)) {
        if (at_portcullis) {
            pline_The("portcullis misses %s!", e_nam(etmp));
        } else {
            debugpline1("The drawbridge misses %s!", e_nam(etmp));
        }
        if (e_survives_at(etmp, oldx, oldy)) {
            return;
        } else {
            debugpline0("Mon can't survive here");
            if (at_portcullis)
                must_jump = TRUE;
            else
                relocates = TRUE; /* just ride drawbridge in */
        }
    } else {
        if (crm->typ == DRAWBRIDGE_DOWN) {
            if (is_u(etmp)) {
                svk.killer.format = NO_KILLER_PREFIX;
                Strcpy(svk.killer.name,
                       "crushed to death underneath a drawbridge");
            }
            pline("%s crushed underneath the drawbridge.",
                  E_phrase(etmp, "are"));             /* no jump */
            e_died(etmp,
                   XKILL_NOCORPSE | (e_inview ? XKILL_GIVEMSG : XKILL_NOMSG),
                   CRUSHING); /* no corpse */
            return;       /* Note: Beyond this point, we know we're  */
        }                 /* not at an opened drawbridge, since all  */
        must_jump = TRUE; /* *missable* creatures survive on the     */
    }                     /* square, and all the unmissed ones die.  */
    if (must_jump) {
        if (at_portcullis) {
            if (e_jumps(etmp)) {
                relocates = TRUE;
                debugpline0("Jump succeeds!");
            } else {
                if (e_inview) {
                    pline("%s crushed by the falling portcullis!",
                          E_phrase(etmp, "are"));
                } else if (!Deaf) {
                    Soundeffect(se_crushing_sound, 100);
                    You_hear("a crushing sound.");
                }
                e_died(etmp,
                       XKILL_NOCORPSE | (e_inview ? XKILL_GIVEMSG
                                                  : XKILL_NOMSG),
                       CRUSHING);
                /* no corpse */
                return;
            }
        } else { /* tries to jump off bridge to original square */
            relocates = !e_jumps(etmp);
            debugpline1("Jump %s!", (relocates) ? "fails" : "succeeds");
        }
    }

    /*
     * Here's where we try to do relocation.  Assumes that etmp is not
     * arriving at the portcullis square while the drawbridge is
     * falling, since this square would be inaccessible (i.e. etmp
     * started on drawbridge square) or unnecessary (i.e. etmp started
     * here) in such a situation.
     */
    debugpline0("Doing relocation.");
    newx = oldx;
    newy = oldy;
    (void) find_drawbridge(&newx, &newy);
    if ((newx == oldx) && (newy == oldy))
        get_wall_for_db(&newx, &newy);
    debugpline0("Checking new square for occupancy.");
    if (relocates && (e_at(newx, newy))) {
        /*
         * Standoff problem: one or both entities must die, and/or
         * both switch places.  Avoid infinite recursion by checking
         * first whether the other entity is staying put.  Clean up if
         * we happen to move/die in recursion.
         */
        struct entity *other;

        other = e_at(newx, newy);
        debugpline1("New square is occupied by %s", e_nam(other));
        if (e_survives_at(other, newx, newy) && automiss(other)) {
            relocates = FALSE; /* "other" won't budge */
            debugpline1("%s suicide.", E_phrase(etmp, "commit"));
        } else {
            debugpline1("Handling %s", e_nam(other));
            while ((e_at(newx, newy) != 0) && (e_at(newx, newy) != etmp))
                do_entity(other);
            debugpline1("Checking existence of %s", e_nam(etmp));
#ifdef D_DEBUG
            wait_synch();
#endif
            if (e_at(oldx, oldy) != etmp) {
                debugpline1("%s moved or died in recursion somewhere",
                            E_phrase(etmp, "have"));
#ifdef D_DEBUG
                wait_synch();
#endif
                return;
            }
        }
    }
    if (relocates && !e_at(newx, newy)) { /* if e_at() entity = worm tail */
        debugpline1("Moving %s", e_nam(etmp));
        if (!is_u(etmp)) {
            remove_monster(etmp->ex, etmp->ey);
            place_monster(etmp->emon, newx, newy);
            update_monster_region(etmp->emon);
        } else {
            u.ux = newx;
            u.uy = newy;
        }
        etmp->ex = newx;
        etmp->ey = newy;
        e_inview = e_canseemon(etmp);
    }
    debugpline1("Final disposition of %s", e_nam(etmp));
#ifdef D_DEBUG
    wait_synch();
#endif
    if (is_db_wall(etmp->ex, etmp->ey)) {
        debugpline1("%s in portcullis chamber", E_phrase(etmp, "are"));
#ifdef D_DEBUG
        wait_synch();
#endif
        if (e_inview) {
            if (is_u(etmp)) {
                You("tumble towards the closed portcullis!");
                if (automiss(etmp))
                    You("pass through it!");
                else
                    pline_The("drawbridge closes in...");
            } else
                pline("%s behind the drawbridge.",
                      E_phrase(etmp, "disappear"));
        }
        if (!e_survives_at(etmp, etmp->ex, etmp->ey)) {
            svk.killer.format = KILLED_BY_AN;
            Strcpy(svk.killer.name, "closing drawbridge");
            e_died(etmp, XKILL_NOMSG, CRUSHING);
            return;
        }
        debugpline1("%s in here", E_phrase(etmp, "survive"));
    } else {
        debugpline1("%s on drawbridge square", E_phrase(etmp, "are"));
        if (is_pool(etmp->ex, etmp->ey) && !e_inview)
            if (!Deaf) {
                Soundeffect(se_splash, 100);
                You_hear("a splash.");
            }
        if (e_survives_at(etmp, etmp->ex, etmp->ey)) {
            if (e_inview && !is_flyer(etmp->edata)
                && !is_floater(etmp->edata))
                pline("%s from the bridge.", E_phrase(etmp, "fall"));
            return;
        }
        debugpline1("%s cannot survive on the drawbridge square",
                    E_phrase(etmp, NULL));
        if (is_pool(etmp->ex, etmp->ey) || is_lava(etmp->ex, etmp->ey))
            if (e_inview && !is_u(etmp)) {
                /* drown() will supply msgs if nec. */
                boolean lava = is_lava(etmp->ex, etmp->ey);

                if (Hallucination)
                    pline("%s the %s and disappears.",
                          E_phrase(etmp, "drink"), lava ? "lava" : "moat");
                else
                    pline("%s into the %s.", E_phrase(etmp, "fall"),
                          lava ? hliquid("lava") : "moat");
            }
        svk.killer.format = NO_KILLER_PREFIX;
        Strcpy(svk.killer.name, "fell from a drawbridge");
        e_died(etmp, /* CRUSHING is arbitrary */
               XKILL_NOCORPSE | (e_inview ? XKILL_GIVEMSG : XKILL_NOMSG),
               is_pool(etmp->ex, etmp->ey) ? DROWNING
                 : is_lava(etmp->ex, etmp->ey) ? BURNING
                   : CRUSHING); /*no corpse*/
        return;
    }
}

/**
 * @brief 사인(killer) 정보와 두 엔티티 슬롯을 초기화한다.
 * @note 도개교 처리 종료 전에 오래된 사망 원인과 점유자 정보를 지운다.
 */
/* clear stale reason for death and both 'entities' before returning */
staticfn void
nokiller(void)
{
    svk.killer.name[0] = '\0';
    svk.killer.format = 0;
    m_to_e((struct monst *) 0, 0, 0, &go.occupants[0]);
    m_to_e((struct monst *) 0, 0, 0, &go.occupants[1]);
}

/**
 * @brief 지정 위치의 도개교를 닫는다(들어 올린다).
 * @param[in] x,y 도개교(span) 위치.
 * @note 도개교 위/아래의 엔티티를 처리하고 해당 위치의 오브젝트·함정·조각을
 *       제거한다.
 */
/*
 * Close the drawbridge located at x,y
 */
void
close_drawbridge(coordxy x, coordxy y)
{
    struct rm *lev1, *lev2;
    struct trap *t;
    coordxy x2, y2;

    lev1 = &levl[x][y];
    if (lev1->typ != DRAWBRIDGE_DOWN)
        return;
    x2 = x;
    y2 = y;
    get_wall_for_db(&x2, &y2);
    if (cansee(x, y) || cansee(x2, y2)) {
        You_see("a drawbridge %s up!",
                (((u.ux == x || u.uy == y) && !Underwater)
                 || distu(x2, y2) < distu(x, y))
                    ? "coming"
                    : "going");
    } else { /* "5 gears turn" for castle drawbridge tune */
        Soundeffect(se_chains_rattling_gears_turning, 75);
        You_hear("chains rattling and gears turning.");
    }
    lev1->typ = DRAWBRIDGE_UP;
    lev2 = &levl[x2][y2];
    lev2->typ = DBWALL;
    switch (lev1->drawbridgemask & DB_DIR) {
    case DB_NORTH:
    case DB_SOUTH:
        lev2->horizontal = TRUE;
        break;
    case DB_WEST:
    case DB_EAST:
        lev2->horizontal = FALSE;
        break;
    }
    lev2->wall_info = W_NONDIGGABLE;
    set_entity(x, y, &(go.occupants[0]));
    set_entity(x2, y2, &(go.occupants[1]));
    do_entity(&(go.occupants[0]));          /* Do set_entity after first */
    set_entity(x2, y2, &(go.occupants[1])); /* do_entity for worm tail */
    do_entity(&(go.occupants[1]));
    if (OBJ_AT(x, y) && !Deaf) {
        Soundeffect(se_smashing_and_crushing, 75);
        You_hear("smashing and crushing.");
    }
    (void) revive_nasty(x, y, (char *) 0);
    (void) revive_nasty(x2, y2, (char *) 0);
    delallobj(x, y);
    delallobj(x2, y2);
    if ((t = t_at(x, y)) != 0)
        deltrap(t);
    if ((t = t_at(x2, y2)) != 0)
        deltrap(t);
    del_engr_at(x, y);
    del_engr_at(x2, y2);
    newsym(x, y);
    newsym(x2, y2);
    block_point(x2, y2); /* vision */
    nokiller();
}

/**
 * @brief 지정 위치의 도개교를 연다(내린다).
 * @param[in] x,y 도개교(span) 위치.
 * @note 도개교 위/아래의 엔티티를 처리하며, 요새(Stronghold)에서는 개방
 *       이벤트를 기록한다.
 */
/*
 * Open the drawbridge located at x,y
 */
void
open_drawbridge(coordxy x, coordxy y)
{
    struct rm *lev1, *lev2;
    struct trap *t;
    coordxy x2, y2;

    lev1 = &levl[x][y];
    if (lev1->typ != DRAWBRIDGE_UP)
        return;
    x2 = x;
    y2 = y;
    get_wall_for_db(&x2, &y2);
    if (cansee(x, y) || cansee(x2, y2)) {
        You_see("a drawbridge %s down!",
                (distu(x2, y2) < distu(x, y)) ? "going" : "coming");
    } else { /* "5 gears turn" for castle drawbridge tune */
        Soundeffect(se_gears_turning_chains_rattling, 100);
        You_hear("gears turning and chains rattling.");
    }
    lev1->typ = DRAWBRIDGE_DOWN;
    lev2 = &levl[x2][y2];
    lev2->typ = DOOR;
    lev2->doormask = D_NODOOR;
    set_entity(x, y, &(go.occupants[0]));
    set_entity(x2, y2, &(go.occupants[1]));
    do_entity(&(go.occupants[0]));          /* do set_entity after first */
    set_entity(x2, y2, &(go.occupants[1])); /* do_entity for worm tails */
    do_entity(&(go.occupants[1]));
    (void) revive_nasty(x, y, (char *) 0);
    delallobj(x, y);
    if ((t = t_at(x, y)) != 0)
        deltrap(t);
    if ((t = t_at(x2, y2)) != 0)
        deltrap(t);
    del_engr_at(x, y);
    del_engr_at(x2, y2);
    newsym(x, y);
    newsym(x2, y2);
    unblock_point(x2, y2); /* vision */
    if (Is_stronghold(&u.uz))
        u.uevent.uopened_dbridge = TRUE;
    nokiller();
}

/**
 * @brief 지정 위치의 도개교를 파괴한다.
 * @param[in] x,y 도개교(span) 위치.
 * @note 아래에 해자/용암이 있으면 그 지형으로, 없으면 바닥/얼음으로 바뀌며,
 *       쇠사슬 파편이 흩어지고 위/아래 엔티티가 파편에 피해를 입을 수 있다.
 */
/*
 * Let's destroy the drawbridge located at x,y
 */
void
destroy_drawbridge(coordxy x, coordxy y)
{
    struct rm *lev1, *lev2;
    struct trap *t;
    struct obj *otmp;
    coordxy x2, y2;
    int i;
    boolean e_inview;
    struct entity *etmp1 = &(go.occupants[0]), *etmp2 = &(go.occupants[1]);

    lev1 = &levl[x][y];
    if (!IS_DRAWBRIDGE(lev1->typ))
        return;
    x2 = x;
    y2 = y;
    get_wall_for_db(&x2, &y2);
    lev2 = &levl[x2][y2];
    if ((lev1->drawbridgemask & DB_UNDER) == DB_MOAT
        || (lev1->drawbridgemask & DB_UNDER) == DB_LAVA) {
        struct obj *otmp2;
        boolean lava = (lev1->drawbridgemask & DB_UNDER) == DB_LAVA;

        Soundeffect(se_loud_splash, 100);  /* Deaf-aware */
        if (lev1->typ == DRAWBRIDGE_UP) {
            if (cansee(x2, y2) || u_at(x2, y2))
                pline_The("portcullis of the drawbridge falls into the %s!",
                          lava ? hliquid("lava") : "moat");
            else
                You_hear("a loud *SPLASH*!");  /* Deaf-aware */
        } else {
            if (cansee(x, y) || u_at(x, y))
                pline_The("drawbridge collapses into the %s!",
                          lava ? hliquid("lava") : "moat");
            else
                You_hear("a loud *SPLASH*!");  /* Deaf-aware */
        }
        lev1->typ = lava ? LAVAPOOL : MOAT;
        lev1->drawbridgemask = 0;
        if ((otmp2 = sobj_at(BOULDER, x, y)) != 0) {
            obj_extract_self(otmp2);
            (void) flooreffects(otmp2, x, y, "fall");
        }
    } else {
        /* no moat beneath */
        Soundeffect(se_loud_crash, 100);  /* Deaf-aware */
        if (cansee(x, y) || u_at(x, y))
            pline_The("drawbridge disintegrates!");
        else
            You_hear("a loud *CRASH*!");  /* Deaf-aware */
        lev1->typ = ((lev1->drawbridgemask & DB_ICE) ? ICE : ROOM);
        lev1->icedpool = ((lev1->drawbridgemask & DB_ICE) ? ICED_MOAT : 0);
    }
    wake_nearto(x, y, 500);
    lev2->typ = DOOR;
    lev2->doormask = D_NODOOR;
    if ((t = t_at(x, y)) != 0)
        deltrap(t);
    if ((t = t_at(x2, y2)) != 0)
        deltrap(t);
    del_engr_at(x, y);
    del_engr_at(x2, y2);
    for (i = rn2(6); i > 0; --i) { /* scatter some debris */
        /* doesn't matter if we happen to pick <x,y2> or <x2,y>;
           since drawbridges are never placed diagonally, those
           pairings will always match one of <x,y> or <x2,y2> */
        otmp = mksobj_at(IRON_CHAIN, rn2(2) ? x : x2, rn2(2) ? y : y2, TRUE,
                         FALSE);
        /* a force of 5 here would yield a radius of 2 for
           iron chain; anything less produces a radius of 1 */
        (void) scatter(otmp->ox, otmp->oy, 1, MAY_HIT, otmp);
    }
    newsym(x, y);
    newsym(x2, y2);
    if (!does_block(x2, y2, lev2))
        unblock_point(x2, y2); /* vision */
    vision_recalc(0);
    if (Is_stronghold(&u.uz))
        u.uevent.uopened_dbridge = TRUE;

    set_entity(x2, y2, etmp2); /* currently only automissers can be here */
    if (etmp2->edata) {
        e_inview = e_canseemon(etmp2);
        if (!automiss(etmp2)) {
            if (e_inview)
                pline("%s blown apart by flying debris.",
                      E_phrase(etmp2, "are"));
            svk.killer.format = KILLED_BY_AN;
            Strcpy(svk.killer.name, "exploding drawbridge");
            e_died(etmp2,
                   XKILL_NOCORPSE | (e_inview ? XKILL_GIVEMSG : XKILL_NOMSG),
                   CRUSHING); /*no corpse*/
        } /* nothing which is vulnerable can survive this */
    }
    set_entity(x, y, etmp1);
    if (etmp1->edata) {
        e_inview = e_canseemon(etmp1);
        if (e_missed(etmp1, TRUE)) {
            debugpline1("%s spared!", E_phrase(etmp1, "are"));
            /* if there is water or lava here, fall in now */
            if (is_u(etmp1))
                spoteffects(FALSE);
            else
                (void) minliquid(etmp1->emon);
        } else {
            if (e_inview) {
                if (!is_u(etmp1) && Hallucination)
                    pline("%s into some heavy metal!",
                          E_phrase(etmp1, "get"));
                else
                    pline("%s hit by a huge chunk of metal!",
                          E_phrase(etmp1, "are"));
            } else {
                if (!Deaf && !is_u(etmp1) && !is_pool(x, y)) {
                    Soundeffect(se_crushing_sound, 75);
                    You_hear("a crushing sound.");
                } else {
                    debugpline1("%s from shrapnel", E_phrase(etmp1, "die"));
                }
            }
            svk.killer.format = KILLED_BY_AN;
            Strcpy(svk.killer.name, "collapsing drawbridge");
            e_died(etmp1,
                   XKILL_NOCORPSE | (e_inview ? XKILL_GIVEMSG : XKILL_NOMSG),
                   CRUSHING); /*no corpse*/
            if (levl[etmp1->ex][etmp1->ey].typ == MOAT)
                do_entity(etmp1);
        }
    }
    nokiller();
    if (Is_stronghold(&u.uz))
        u.uevent.uheard_tune = 3; /* bridge is gone so tune is now useless */
}

/*dbridge.c*/
