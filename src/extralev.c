/* NetHack 5.0	extralev.c	$NHDT-Date: 1781973049 2026/06/20 16:30:49 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.30 $ */
/*      Copyright 1988, 1989 by Ken Arromdee                      */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file extralev.c
 * @brief "로그(Rogue)" 스타일 레벨 생성 지원 코드.
 *
 * 레벨을 3x3 격자로 나눠 각 칸에 방 또는 교차로를 배치하고, 미로형 알고리즘
 * (@c miniwalk)으로 칸들을 연결한 뒤 복도(@c roguecorr, @c roguejoin)를
 * 그린다. 로그 레벨 특유의 유령(ghost)과 유품 배치도 담당한다.
 *
 * @note 서로 강하게 결합된 static 헬퍼 블록이므로 선언 재배치는 적용하지 않고
 *       정의 위치에서 문서화한다.
 */

#include "hack.h"

/** @brief 인접 칸 연결 방향 비트: 위쪽. */
#define XL_UP 1
/** @brief 인접 칸 연결 방향 비트: 아래쪽. */
#define XL_DOWN 2
/** @brief 인접 칸 연결 방향 비트: 왼쪽. */
#define XL_LEFT 4
/** @brief 인접 칸 연결 방향 비트: 오른쪽. */
#define XL_RIGHT 8

staticfn void roguejoin(coordxy, coordxy, coordxy, coordxy, int);
staticfn void roguecorr(coordxy, coordxy, int);
staticfn void miniwalk(coordxy, coordxy);

/**
 * @brief 두 지점을 ㄷ자(꺾인) 복도로 잇는다.
 * @param[in] x1,y1 시작 지점 좌표.
 * @param[in] x2,y2 도착 지점 좌표.
 * @param[in] horiz TRUE 이면 수평 우선, FALSE 이면 수직 우선으로 꺾는다.
 */
staticfn void
roguejoin(coordxy x1, coordxy y1, coordxy x2, coordxy y2, int horiz)
{
    coordxy x, y, middle;

    if (horiz) {
        middle = x1 + rn2(x2 - x1 + 1);
        for (x = min(x1, middle); x <= max(x1, middle); x++)
            corr(x, y1);
        for (y = min(y1, y2); y <= max(y1, y2); y++)
            corr(middle, y);
        for (x = min(middle, x2); x <= max(middle, x2); x++)
            corr(x, y2);
    } else {
        middle = y1 + rn2(y2 - y1 + 1);
        for (y = min(y1, middle); y <= max(y1, middle); y++)
            corr(x1, y);
        for (x = min(x1, x2); x <= max(x1, x2); x++)
            corr(x, middle);
        for (y = min(middle, y2); y <= max(middle, y2); y++)
            corr(x2, y);
    }
}

/**
 * @brief 지정한 칸에서 인접 칸으로 이어지는 복도(문 포함)를 뚫는다.
 * @param[in] x,y 시작 칸의 격자 좌표.
 * @param[in] dir 연결 방향(@c XL_DOWN 또는 @c XL_RIGHT).
 * @warning @p dir 이 @c XL_DOWN/XL_RIGHT 가 아니면 @c impossible() 경고를 낸다.
 */
staticfn void
roguecorr(coordxy x, coordxy y, int dir)
{
    coordxy fromx, fromy, tox, toy;

    if (dir == XL_DOWN) {
        gr.r[x][y].doortable &= ~XL_DOWN;
        if (!gr.r[x][y].real) {
            fromx = gr.r[x][y].rlx;
            fromy = gr.r[x][y].rly;
            fromx += 1 + 26 * x;
            fromy += 7 * y;
        } else {
            fromx = gr.r[x][y].rlx + rn2(gr.r[x][y].dx);
            fromy = gr.r[x][y].rly + gr.r[x][y].dy;
            fromx += 1 + 26 * x;
            fromy += 7 * y;
            if (!IS_WALL(levl[fromx][fromy].typ))
                impossible("down: no wall at %d,%d?", fromx, fromy);
            dodoor(fromx, fromy, &svr.rooms[gr.r[x][y].nroom]);
            levl[fromx][fromy].doormask = D_NODOOR;
            fromy++;
        }
        if (y >= 2) {
            impossible("down door from %d,%d going nowhere?", x, y);
            return;
        }
        y++;
        gr.r[x][y].doortable &= ~XL_UP;
        if (!gr.r[x][y].real) {
            tox = gr.r[x][y].rlx;
            toy = gr.r[x][y].rly;
            tox += 1 + 26 * x;
            toy += 7 * y;
        } else {
            tox = gr.r[x][y].rlx + rn2(gr.r[x][y].dx);
            toy = gr.r[x][y].rly - 1;
            tox += 1 + 26 * x;
            toy += 7 * y;
            if (!IS_WALL(levl[tox][toy].typ))
                impossible("up: no wall at %d,%d?", tox, toy);
            dodoor(tox, toy, &svr.rooms[gr.r[x][y].nroom]);
            levl[tox][toy].doormask = D_NODOOR;
            toy--;
        }
        roguejoin(fromx, fromy, tox, toy, FALSE);
        return;
    } else if (dir == XL_RIGHT) {
        gr.r[x][y].doortable &= ~XL_RIGHT;
        if (!gr.r[x][y].real) {
            fromx = gr.r[x][y].rlx;
            fromy = gr.r[x][y].rly;
            fromx += 1 + 26 * x;
            fromy += 7 * y;
        } else {
            fromx = gr.r[x][y].rlx + gr.r[x][y].dx;
            fromy = gr.r[x][y].rly + rn2(gr.r[x][y].dy);
            fromx += 1 + 26 * x;
            fromy += 7 * y;
            if (!IS_WALL(levl[fromx][fromy].typ))
                impossible("down: no wall at %d,%d?", fromx, fromy);
            dodoor(fromx, fromy, &svr.rooms[gr.r[x][y].nroom]);
            levl[fromx][fromy].doormask = D_NODOOR;
            fromx++;
        }
        if (x >= 2) {
            impossible("right door from %d,%d going nowhere?", x, y);
            return;
        }
        x++;
        gr.r[x][y].doortable &= ~XL_LEFT;
        if (!gr.r[x][y].real) {
            tox = gr.r[x][y].rlx;
            toy = gr.r[x][y].rly;
            tox += 1 + 26 * x;
            toy += 7 * y;
        } else {
            tox = gr.r[x][y].rlx - 1;
            toy = gr.r[x][y].rly + rn2(gr.r[x][y].dy);
            tox += 1 + 26 * x;
            toy += 7 * y;
            if (!IS_WALL(levl[tox][toy].typ))
                impossible("left: no wall at %d,%d?", tox, toy);
            dodoor(tox, toy, &svr.rooms[gr.r[x][y].nroom]);
            levl[tox][toy].doormask = D_NODOOR;
            tox--;
        }
        roguejoin(fromx, fromy, tox, toy, TRUE);
        return;
    } else
        impossible("corridor in direction %d?", dir);
}

/**
 * @brief 3x3 격자를 미로처럼 탐색하며 칸들 사이의 연결(문)을 만든다.
 * @param[in] x,y 탐색을 시작할 격자 좌표.
 * @note @c mkmaze.c 의 @c walkfrom() 을 변형한 것으로, 재귀적으로 동작하며
 *       1/10 확률로 여분의 연결을 추가한다.
 */
/* Modified walkfrom() from mkmaze.c */
staticfn void
miniwalk(coordxy x, coordxy y)
{
    int q, dir;
    int dirs[4];

    while (1) {
        q = 0;
#define doorhere (gr.r[x][y].doortable)
        if (x > 0 && (!(doorhere & XL_LEFT))
            && (!gr.r[x - 1][y].doortable || !rn2(10)))
            dirs[q++] = 0;
        if (x < 2 && (!(doorhere & XL_RIGHT))
            && (!gr.r[x + 1][y].doortable || !rn2(10)))
            dirs[q++] = 1;
        if (y > 0 && (!(doorhere & XL_UP))
            && (!gr.r[x][y - 1].doortable || !rn2(10)))
            dirs[q++] = 2;
        if (y < 2 && (!(doorhere & XL_DOWN))
            && (!gr.r[x][y + 1].doortable || !rn2(10)))
            dirs[q++] = 3;
        /* Rogue levels aren't just 3 by 3 mazes; they have some extra
         * connections, thus that 1/10 chance
         */
        if (!q)
            return;
        dir = dirs[rn2(q)];
        switch (dir) { /* Move in direction */
        case 0:
            doorhere |= XL_LEFT;
            x--;
            doorhere |= XL_RIGHT;
            break;
        case 1:
            doorhere |= XL_RIGHT;
            x++;
            doorhere |= XL_LEFT;
            break;
        case 2:
            doorhere |= XL_UP;
            y--;
            doorhere |= XL_DOWN;
            break;
        case 3:
            doorhere |= XL_DOWN;
            y++;
            doorhere |= XL_UP;
            break;
        }
        miniwalk(x, y);
    }
#undef doorhere
}

/**
 * @brief 로그 스타일 레벨의 방과 연결 복도를 생성한다.
 *
 * 3x3 격자 각 칸에 실제 방 또는 더미(교차로)를 배치하고, 미로 탐색으로 칸을
 * 연결한 뒤 실제 방을 만들고 방 사이를 복도로 잇는다.
 */
void
makeroguerooms(void)
{
    coordxy x, y;
    /* Rogue levels are structured 3 by 3, with each section containing
     * a room or an intersection.  The minimum width is 2 each way.
     * One difference between these and "real" Rogue levels: real Rogue
     * uses 24 rows and NetHack only 23.  So we cheat a bit by making the
     * second row of rooms not as deep.
     *
     * Each normal space has 6/7 rows and 25 columns in which a room may
     * actually be placed.  Walls go from rows 0-5/6 and columns 0-24.
     * Not counting walls, the room may go in
     * rows 1-5 and columns 1-23 (numbering starting at 0).  A room
     * coordinate of this type may be converted to a level coordinate
     * by adding 1+28*x to the column, and 7*y to the row.  (The 1
     * is because column 0 isn't used [we only use 1-78]).
     * Room height may be 2-4 (2-5 on last row), length 2-23 (not
     * counting walls).
     */
#define here gr.r[x][y]

    svn.nroom = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 3; x++) {
            /* Note: we want to insure at least 1 room.  So, if the
             * first 8 are all dummies, force the last to be a room.
             */
            if (!rn2(5) && (svn.nroom || (x < 2 && y < 2))) {
                /* Arbitrary: dummy rooms may only go where real
                 * ones do.
                 */
                here.real = FALSE;
                here.rlx = rn1(22, 2);
                here.rly = rn1((y == 2) ? 4 : 3, 2);
            } else {
                here.real = TRUE;
                here.dx = rn1(22, 2); /* 2-23 long, plus walls */
                here.dy = rn1((y == 2) ? 4 : 3, 2); /* 2-5 high, plus walls */

                /* boundaries of room floor */
                here.rlx = rnd(23 - here.dx + 1);
                here.rly = rnd(((y == 2) ? 5 : 4) - here.dy + 1);
                svn.nroom++;
            }
            here.doortable = 0;
        }
    miniwalk(rn2(3), rn2(3));
    svn.nroom = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 3; x++) {
            if (here.real) { /* Make a room */
                coordxy lowx, lowy, hix, hiy;

                gr.r[x][y].nroom = svn.nroom;
                gs.smeq[svn.nroom] = svn.nroom;

                lowx = 1 + 26 * x + here.rlx;
                lowy = 7 * y + here.rly;
                hix = 1 + 26 * x + here.rlx + here.dx - 1;
                hiy = 7 * y + here.rly + here.dy - 1;
                /* Strictly speaking, it should be lit only if above
                 * level 10, but since Rogue rooms are only
                 * encountered below level 10, use !rn2(7).
                 */
                add_room(lowx, lowy, hix, hiy, (boolean) !rn2(7), OROOM,
                         FALSE);
            }
        }

    /* Now, add connecting corridors. */
    for (y = 0; y < 3; y++)
        for (x = 0; x < 3; x++) {
            if (here.doortable & XL_DOWN)
                roguecorr(x, y, XL_DOWN);
            if (here.doortable & XL_RIGHT)
                roguecorr(x, y, XL_RIGHT);
            if (here.doortable & XL_LEFT)
                impossible("left end of %d, %d never connected?", x, y);
            if (here.doortable & XL_UP)
                impossible("up end of %d, %d never connected?", x, y);
        }
#undef here
}

/**
 * @brief 지정한 좌표를 복도 칸으로 만든다(드물게 숨은 복도).
 * @param[in] x,y 복도로 만들 좌표.
 * @note 50분의 1 확률로 일반 복도(@c CORR) 대신 숨은 복도(@c SCORR)가 된다.
 */
void
corr(coordxy x, coordxy y)
{
    if (rn2(50)) {
        levl[x][y].typ = CORR;
    } else {
        levl[x][y].typ = SCORR;
    }
}

/**
 * @brief 로그 레벨에 잠자는 유령과 그 유품 아이템들을 배치한다.
 *
 * 무작위 방에 유령을 만들어 이름을 부여하고, 무기·방어구·식량·부적 등을
 * 무작위로 함께 놓는다.
 *
 * @note 방이 하나도 없으면(정상적으로는 발생하지 않음) 아무 동작도 하지 않는다.
 */
void
makerogueghost(void)
{
    struct monst *ghost;
    struct obj *ghostobj;
    struct mkroom *croom;
    coordxy x, y;

    if (!svn.nroom)
        return; /* Should never happen */
    croom = &svr.rooms[rn2(svn.nroom)];
    x = somex(croom);
    y = somey(croom);
    if (!(ghost = makemon(&mons[PM_GHOST], x, y, NO_MM_FLAGS)))
        return;
    ghost->msleeping = 1;
    ghost = christen_monst(ghost, roguename());
    nhUse(ghost);

    if (rn2(4)) {
        ghostobj = mksobj_at(FOOD_RATION, x, y, FALSE, FALSE);
        ghostobj->quan = (long) rnd(7);
        ghostobj->owt = weight(ghostobj);
    }
    if (rn2(2)) {
        ghostobj = mksobj_at(MACE, x, y, FALSE, FALSE);
        ghostobj->spe = rnd(3);
        if (rn2(4))
            curse(ghostobj);
    } else {
        ghostobj = mksobj_at(TWO_HANDED_SWORD, x, y, FALSE, FALSE);
        ghostobj->spe = rnd(5) - 2;
        if (rn2(4))
            curse(ghostobj);
    }
    ghostobj = mksobj_at(BOW, x, y, FALSE, FALSE);
    ghostobj->spe = 1;
    if (rn2(4))
        curse(ghostobj);

    ghostobj = mksobj_at(ARROW, x, y, FALSE, FALSE);
    ghostobj->spe = 0;
    ghostobj->quan = (long) rn1(10, 25);
    ghostobj->owt = weight(ghostobj);
    if (rn2(4))
        curse(ghostobj);

    if (rn2(2)) {
        ghostobj = mksobj_at(RING_MAIL, x, y, FALSE, FALSE);
        ghostobj->spe = rn2(3);
        if (!rn2(3))
            ghostobj->oerodeproof = TRUE;
        if (rn2(4))
            curse(ghostobj);
    } else {
        ghostobj = mksobj_at(PLATE_MAIL, x, y, FALSE, FALSE);
        ghostobj->spe = rnd(5) - 2;
        if (!rn2(3))
            ghostobj->oerodeproof = TRUE;
        if (rn2(4))
            curse(ghostobj);
    }
    if (rn2(2)) {
        ghostobj = mksobj_at(FAKE_AMULET_OF_YENDOR, x, y, TRUE, FALSE);
        ghostobj->known = TRUE;
    }
}

/*extralev.c*/
