/* NetHack 5.0	stairs.c	$NHDT-Date: 1704043695 2023/12/31 17:28:15 $  $NHDT-Branch: keni-luabits2 $:$NHDT-Revision: 1.207 $ */
/* Copyright (c) 2024 by Pasi Kallinen */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file stairs.c
 * @brief 현재 레벨의 계단·사다리(stairway) 목록 관리 및 조회.
 *
 * 각 레벨의 계단/사다리를 연결 리스트(@c gs.stairs)로 유지하며, 추가/해제와
 * 위치·방향·목적지 기준의 다양한 검색, 영웅 배치, 계단 설명 문자열 생성을
 * 제공한다.
 */

#include "hack.h"

/**
 * @brief 현재 레벨의 계단/사다리 목록에 새 항목을 추가한다.
 *
 * @param[in] x        계단의 x 좌표.
 * @param[in] y        계단의 y 좌표.
 * @param[in] up       TRUE 이면 올라가는 계단, FALSE 이면 내려가는 계단.
 * @param[in] isladder TRUE 이면 사다리, FALSE 이면 계단.
 * @param[in] dest     이 계단이 연결되는 목적지 레벨.
 * @note 항목을 새로 할당하며, 목록은 @c stairway_free_all() 로 해제한다.
 */
void
stairway_add(
    coordxy x, coordxy y,
    boolean up, boolean isladder,
    d_level *dest)
{
    stairway *tmp = (stairway *) alloc(sizeof (stairway));

    (void) memset((genericptr_t) tmp, 0, sizeof (stairway));
    tmp->sx = x;
    tmp->sy = y;
    tmp->up = up;
    tmp->isladder = isladder;
    tmp->u_traversed = FALSE;
    assign_level(&(tmp->tolev), dest);
    tmp->next = gs.stairs;
    gs.stairs = tmp;
}

/**
 * @brief 현재 레벨의 계단/사다리 목록 전체를 해제한다.
 * @note 목록 포인터(@c gs.stairs)를 NULL 로 재설정한다.
 */
void
stairway_free_all(void)
{
    stairway *tmp = gs.stairs;

    while (tmp) {
        stairway *tmp2 = tmp->next;
        free(tmp);
        tmp = tmp2;
    }
    gs.stairs = NULL;
}

/**
 * @brief 지정한 좌표에 있는 계단/사다리를 찾는다.
 * @param[in] x 검색할 x 좌표.
 * @param[in] y 검색할 y 좌표.
 * @return 해당 좌표의 계단 항목 포인터, 없으면 NULL.
 * @note 반환 포인터는 목록 내부를 가리키므로 free 하지 말 것.
 */
stairway *
stairway_at(coordxy x, coordxy y)
{
    stairway *tmp = gs.stairs;

    while (tmp && !(tmp->sx == x && tmp->sy == y))
        tmp = tmp->next;
    return tmp;
}

/**
 * @brief 지정한 목적지 레벨로 이어지는 계단/사다리를 찾는다.
 * @param[in] fromdlev 목적지로 삼을 레벨.
 * @return 목적지가 일치하는 계단 항목 포인터, 없으면 NULL.
 */
stairway *
stairway_find(d_level *fromdlev)
{
    stairway *tmp = gs.stairs;

    while (tmp) {
        if (tmp->tolev.dnum == fromdlev->dnum
            && tmp->tolev.dlevel == fromdlev->dlevel)
            break; /* return */
        tmp = tmp->next;
    }
    return tmp;
}

/**
 * @brief 목적지 레벨과 종류(계단/사다리)가 모두 일치하는 항목을 찾는다.
 * @param[in] fromdlev 목적지로 삼을 레벨.
 * @param[in] isladder TRUE 이면 사다리, FALSE 이면 계단으로 한정한다.
 * @return 조건이 일치하는 항목 포인터, 없으면 NULL.
 */
stairway *
stairway_find_from(d_level *fromdlev, boolean isladder)
{
    stairway *tmp = gs.stairs;

    while (tmp) {
        if (tmp->tolev.dnum == fromdlev->dnum
            && tmp->tolev.dlevel == fromdlev->dlevel
            && tmp->isladder == isladder)
            break; /* return */
        tmp = tmp->next;
    }
    return tmp;
}

/**
 * @brief 지정한 방향(위/아래)의 계단/사다리를 찾는다.
 * @param[in] up TRUE 이면 올라가는 것, FALSE 이면 내려가는 것.
 * @return 방향이 일치하는 첫 항목 포인터, 없으면 NULL.
 */
stairway *
stairway_find_dir(boolean up)
{
    stairway *tmp = gs.stairs;

    while (tmp && !(tmp->up == up))
        tmp = tmp->next;
    return tmp;
}

/**
 * @brief 지정한 종류와 방향이 모두 일치하는 계단/사다리를 찾는다.
 * @param[in] isladder TRUE 이면 사다리, FALSE 이면 계단.
 * @param[in] up       TRUE 이면 올라가는 것, FALSE 이면 내려가는 것.
 * @return 조건이 일치하는 첫 항목 포인터, 없으면 NULL.
 */
stairway *
stairway_find_type_dir(boolean isladder, boolean up)
{
    stairway *tmp = gs.stairs;

    while (tmp && !(tmp->isladder == isladder && tmp->up == up))
        tmp = tmp->next;
    return tmp;
}

/**
 * @brief 특수 계단(다른 던전 분기로 이어지는 계단)을 방향 기준으로 찾는다.
 * @param[in] up 기준 방향(이 방향과 반대이면서 다른 던전으로 가는 계단을 찾음).
 * @return 조건이 일치하는 항목 포인터, 없으면 NULL.
 */
stairway *
stairway_find_special_dir(boolean up)
{
    stairway *tmp = gs.stairs;

    while (tmp) {
        if (tmp->tolev.dnum != u.uz.dnum && tmp->up != up)
            return tmp;
        tmp = tmp->next;
    }
    return tmp;
}

/**
 * @brief 영웅을 특수 계단 위에 배치한다.
 * @param[in] upflag 배치 방향 플래그(위/아래).
 * @note 특수 계단이 없으면 무작위 지점(@c u_on_rndspot)에 배치한다.
 */
/* place you on the special staircase */
void
u_on_sstairs(int upflag)
{
    stairway *stway = stairway_find_special_dir(upflag);

    if (stway)
        u_on_newpos(stway->sx, stway->sy);
    else
        u_on_rndspot(upflag);
}

/**
 * @brief 영웅을 올라가는 계단(또는 그에 상응하는 특수 계단)에 배치한다.
 */
/* place you on upstairs (or special equivalent) */
void
u_on_upstairs(void)
{
    stairway *stway = stairway_find_dir(TRUE);

    if (stway)
        u_on_newpos(stway->sx, stway->sy);
    else
        u_on_sstairs(0); /* destination upstairs implies moving down */
}

/**
 * @brief 영웅을 내려가는 계단(또는 그에 상응하는 특수 계단)에 배치한다.
 */
/* place you on dnstairs (or special equivalent) */
void
u_on_dnstairs(void)
{
    stairway *stway = stairway_find_dir(FALSE);

    if (stway)
        u_on_newpos(stway->sx, stway->sy);
    else
        u_on_sstairs(1); /* destination dnstairs implies moving up */
}

/**
 * @brief 지정한 좌표가 계단/사다리 위인지 판별한다.
 * @param[in] x,y 확인할 좌표.
 * @return 계단/사다리가 있으면 TRUE, 아니면 FALSE.
 */
boolean
On_stairs(coordxy x, coordxy y)
{
    return (stairway_at(x, y) != NULL);
}

/**
 * @brief 지정한 좌표가 사다리 위인지 판별한다.
 * @param[in] x,y 확인할 좌표.
 * @return 사다리가 있으면 TRUE, 아니면 FALSE.
 */
boolean
On_ladder(coordxy x, coordxy y)
{
    stairway *stway = stairway_at(x, y);

    return (boolean) (stway && stway->isladder);
}

/**
 * @brief 지정한 좌표가 올라가는 계단/사다리 위인지 판별한다.
 * @param[in] x,y 확인할 좌표.
 * @return 올라가는 것이 있으면 TRUE, 아니면 FALSE.
 */
boolean
On_stairs_up(coordxy x, coordxy y)
{
    stairway *stway = stairway_at(x, y);

    return (boolean) (stway && stway->up);
}

/**
 * @brief 지정한 좌표가 내려가는 계단/사다리 위인지 판별한다.
 * @param[in] x,y 확인할 좌표.
 * @return 내려가는 것이 있으면 TRUE, 아니면 FALSE.
 */
boolean
On_stairs_dn(coordxy x, coordxy y)
{
    stairway *stway = stairway_at(x, y);

    return (boolean) (stway && !stway->up);
}

/**
 * @brief 분기 계단이며 영웅이 이미 그 분기를 방문했는지 판별한다.
 * @param[in] sway 판별할 계단 항목.
 * @return 분기 계단이고 영웅이 통과한 적이 있으면 TRUE, 아니면 FALSE.
 */
/* return True if 'sway' is a branch staircase and hero has used these stairs
   to visit the branch */
boolean
known_branch_stairs(stairway *sway)
{
    return (sway && sway->tolev.dnum != u.uz.dnum && sway->u_traversed);
}

/**
 * @brief 계단/사다리에 대한 설명 문자열을 생성한다.
 *
 * 영웅이 목적지를 아는지 여부에 따라 목적지 레벨/분기 이름 등을 포함한
 * 설명을 만든다. 레벨 1의 올라가는 계단 등 특수 상황을 별도 처리한다.
 *
 * @param[in]  sway   설명할 계단/사다리 항목.
 * @param[out] outbuf 결과 문자열을 저장할 버퍼.
 * @param[in]  stcase TRUE 이면 항상 단수형("staircase"/"ladder"), FALSE 이면
 *                    "stairs"/"ladder"(단·복수 처리는 호출자 책임).
 * @return 결과가 기록된 @p outbuf 를 그대로 반환한다.
 */
/* describe staircase 'sway' based on whether hero knows the destination */
char *
stairs_description(
    stairway *sway, /* stairs/ladder to describe */
    char *outbuf,   /* result buffer */
    boolean stcase) /* True: "staircase" or "ladder", always singular;
                     * False: "stairs" or "ladder"; caller needs to deal
                     * with singular vs plural when forming a sentence */
{
    d_level tolev;
    const char *stairs, *updown;

    tolev = sway->tolev;
    stairs = sway->isladder ? "ladder" : stcase ? "staircase" : "stairs";
    updown = sway->up ? "up" : "down";

    if (!known_branch_stairs(sway)) {
        /* ordinary stairs or branch stairs to not-yet-visited branch */
        Sprintf(outbuf, "%s %s", stairs, updown);
        if (sway->u_traversed) {
            boolean specialdepth = (tolev.dnum == quest_dnum
                                    || single_level_branch(&tolev)); /* knox */
            int to_dlev = specialdepth ? dunlev(&tolev) : depth(&tolev);

            Sprintf(eos(outbuf), " to level %d", to_dlev);
        }
    } else if (u.uz.dnum == 0 && u.uz.dlevel == 1 && sway->up) {
        /* stairs up from level one are a special case; they are marked
           as having been traversed because the hero obviously started
           the game by coming down them, but the remote side varies
           depending on whether the Amulet is being carried */
        Sprintf(outbuf, "%s%s %s %s",
                !u.uhave.amulet ? "" : "branch ",
                stairs, updown,
                !u.uhave.amulet ? "out of the dungeon"
                /* minimize our expectations about what comes next */
                : (on_level(&tolev, &earth_level)
                   || on_level(&tolev, &air_level)
                   || on_level(&tolev, &fire_level)
                   || on_level(&tolev, &water_level))
                  ? "to the Elemental Planes"
                  : "to the end game");
    } else {
        /* known branch stairs; tacking on destination level is too verbose */
        Sprintf(outbuf, "branch %s %s to %s",
                stairs, updown, svd.dungeons[tolev.dnum].dname);
        /* dungeons[].dname is capitalized; undo that for "The <Branch>" */
        (void) strsubst(outbuf, "The ", "the ");
    }
    return outbuf;
}

/*stairs.c*/
