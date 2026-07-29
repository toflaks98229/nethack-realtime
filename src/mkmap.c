/* NetHack 5.0	mkmap.c	$NHDT-Date: 1781973055 2026/06/20 16:30:55 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.47 $ */
/* Copyright (c) J. C. Collet, M. Stephenson and D. Cohrs, 1992   */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file mkmap.c
 * @brief Cellular-automaton cave generator and flood-fill room joiner.
 *
 * Builds organic, cavern-style levels: an initial random scatter of
 * foreground terrain is smoothed over several cellular-automaton passes,
 * then flood filling identifies the resulting open regions as rooms and
 * digs corridors to join them.  The public entry point is @c mkmap().
 *
 * @note Function definition order is intentionally preserved (only file-local
 *       constants are grouped at the top) to avoid any risk of altering
 *       behavior; this matches the codebase's conservative reorder policy.
 */

/**
 * @file mkmap.c
 * @brief 셀룰러 오토마타 동굴 생성기 및 플러드필 방 결합기.
 *
 * 무작위로 흩뿌린 전경 지형을 여러 번의 셀룰러 오토마타 패스로 다듬어
 * 유기적인 동굴형 레벨을 만든다. 이후 플러드필로 열린 영역들을 방으로
 * 인식하고 통로로 연결한다. 공개 진입점은 @c mkmap() 이다.
 *
 * @note 동작 변경 위험을 피하기 위해 함수 정의 순서는 의도적으로 보존하며
 *       (파일 지역 상수만 상단에 모음), 이는 코드베이스의 보수적 재배치
 *       방침과 일치한다.
 */

#include "hack.h"
#include "sp_lev.h"

/** @brief Usable map height, excluding the bottom border row. */
/** @brief 하단 경계 행을 제외한 사용 가능한 맵 높이. */
#define HEIGHT (ROWNO - 1)

/** @brief Usable map width, excluding the left/right border columns. */
/** @brief 좌우 경계 열을 제외한 사용 가능한 맵 너비. */
#define WIDTH (COLNO - 2)

/** @brief Eight-neighbour offset pairs (dx, dy) used by the automaton passes. */
/** @brief 오토마타 패스가 사용하는 8방향 이웃 오프셋 쌍 (dx, dy). */
staticfn const int dirs[16] = {
    -1, -1 /**/, -1,  0 /**/, -1, 1 /**/, 0, -1 /**/,
     0,  1 /**/,  1, -1 /**/,  1, 0 /**/, 1,  1
};

/**
 * @brief Address a cell in the scratch buffer @c gn.new_locations.
 * @param i Column index.
 * @param j Row index.
 */
/**
 * @brief 스크래치 버퍼 @c gn.new_locations 의 한 칸을 가리킨다.
 * @param i 열 인덱스.
 * @param j 행 인덱스.
 */
#define new_loc(i, j) *(gn.new_locations + ((j) * (WIDTH + 1)) + (i))

/** @brief Iteration counts that tune map generation and smoothing. */
/** @brief 맵 생성 및 평활화를 조정하는 반복 횟수. */
#define N_P1_ITER 1 /* tune map generation via this value */
#define N_P2_ITER 1 /* tune map generation via this value */
#define N_P3_ITER 2 /* tune map smoothing via this value */

staticfn void init_map(schar);
staticfn void init_fill(schar, schar);
staticfn schar get_map(coordxy, coordxy, schar);
staticfn void pass_one(schar, schar);
staticfn void pass_two(schar, schar);
staticfn void pass_three(schar, schar);
staticfn void join_map_cleanup(void);
staticfn void join_map(schar, schar);
staticfn void finish_map(schar, schar, boolean, boolean, boolean);
staticfn void remove_room(unsigned);
void mkmap(lev_init *);

/**
 * @brief Reset every map cell to the background terrain, unlit and roomless.
 * @param[in] bg_typ Background terrain type to fill with.
 */
/**
 * @brief 모든 맵 칸을 배경 지형으로 초기화하고, 소등·무소속 상태로 만든다.
 * @param[in] bg_typ 채울 배경 지형 타입.
 */
staticfn void
init_map(schar bg_typ)
{
    coordxy x, y;

    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++) {
            levl[x][y].roomno = NO_ROOM;
            levl[x][y].typ = bg_typ;
            levl[x][y].lit = FALSE;
        }
}

/**
 * @brief Randomly seed foreground cells across the interior of the map.
 * @param[in] bg_typ Background type currently filling the map.
 * @param[in] fg_typ Foreground type to scatter over roughly 40% of the area.
 */
/**
 * @brief 맵 내부에 전경 칸을 무작위로 흩뿌려 초기 씨앗을 만든다.
 * @param[in] bg_typ 현재 맵을 채우고 있는 배경 타입.
 * @param[in] fg_typ 면적의 약 40% 에 흩뿌릴 전경 타입.
 */
staticfn void
init_fill(schar bg_typ, schar fg_typ)
{
    coordxy x, y;
    long limit, count;

    limit = (WIDTH * HEIGHT * 2) / 5;
    count = 0;
    while (count < limit) {
        x = (coordxy) rn1(WIDTH - 1, 2);
        y = (coordxy) rnd(HEIGHT - 1);
        if (levl[x][y].typ == bg_typ) {
            levl[x][y].typ = fg_typ;
            count++;
        }
    }
}

/**
 * @brief Read a cell's terrain type, treating out-of-bounds as background.
 * @param[in] col    Column to sample.
 * @param[in] row    Row to sample.
 * @param[in] bg_typ Value to return for out-of-bounds coordinates.
 * @return The terrain type at (col,row), or @p bg_typ if out of bounds.
 */
/**
 * @brief 칸의 지형 타입을 읽되, 범위를 벗어나면 배경으로 간주한다.
 * @param[in] col    표본 열.
 * @param[in] row    표본 행.
 * @param[in] bg_typ 범위를 벗어난 좌표에 대해 반환할 값.
 * @return (col,row) 의 지형 타입, 범위 밖이면 @p bg_typ.
 */
staticfn schar
get_map(coordxy col, coordxy row, schar bg_typ)
{
    if (col <= 0 || row < 0 || col > WIDTH || row >= HEIGHT)
        return bg_typ;
    return levl[col][row].typ;
}

/**
 * @brief First automaton pass: kill sparse cells and grow dense ones in place.
 * @param[in] bg_typ Background terrain type.
 * @param[in] fg_typ Foreground terrain type.
 * @note Writes results directly into @c levl, so it reads partly updated
 *       neighbours; this is intentional for this pass.
 */
/**
 * @brief 1차 오토마타 패스: 희박한 칸은 제거하고 밀집한 칸은 즉석에서 키운다.
 * @param[in] bg_typ 배경 지형 타입.
 * @param[in] fg_typ 전경 지형 타입.
 * @note 결과를 @c levl 에 직접 쓰므로 일부 갱신된 이웃을 읽으며, 이 패스에서는
 *       의도된 동작이다.
 */
staticfn void
pass_one(schar bg_typ, schar fg_typ)
{
    coordxy x, y;
    short count, dr;

    for (x = 2; x <= WIDTH; x++)
        for (y = 1; y < HEIGHT; y++) {
            for (count = 0, dr = 0; dr < 8; dr++)
                if (get_map(x + dirs[dr * 2], y + dirs[(dr * 2) + 1], bg_typ)
                    == fg_typ)
                    count++;

            switch (count) {
            case 0: /* death */
            case 1:
            case 2:
                levl[x][y].typ = bg_typ;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                levl[x][y].typ = fg_typ;
                break;
            default:
                break;
            }
        }
}

/**
 * @brief Second automaton pass: thin exactly-five-neighbour cells, buffered.
 * @param[in] bg_typ Background terrain type.
 * @param[in] fg_typ Foreground terrain type.
 * @note Computes into the @c new_loc scratch buffer first, then commits, so
 *       neighbour reads are from the pre-pass state.
 */
/**
 * @brief 2차 오토마타 패스: 이웃이 정확히 5인 칸을 솎아내되 버퍼로 처리한다.
 * @param[in] bg_typ 배경 지형 타입.
 * @param[in] fg_typ 전경 지형 타입.
 * @note @c new_loc 스크래치 버퍼에 먼저 계산한 뒤 반영하므로, 이웃 읽기는
 *       패스 이전 상태를 기준으로 한다.
 */
staticfn void
pass_two(schar bg_typ, schar fg_typ)
{
    coordxy x, y;
    short count, dr;

    for (x = 2; x <= WIDTH; x++)
        for (y = 1; y < HEIGHT; y++) {
            for (count = 0, dr = 0; dr < 8; dr++)
                if (get_map(x + dirs[dr * 2], y + dirs[(dr * 2) + 1], bg_typ)
                    == fg_typ)
                    count++;
            if (count == 5)
                new_loc(x, y) = bg_typ;
            else
                new_loc(x, y) = get_map(x, y, bg_typ);
        }

    for (x = 2; x <= WIDTH; x++)
        for (y = 1; y < HEIGHT; y++)
            levl[x][y].typ = new_loc(x, y);
}

/**
 * @brief Third automaton pass: smooth away cells with fewer than three
 *        foreground neighbours, buffered.
 * @param[in] bg_typ Background terrain type.
 * @param[in] fg_typ Foreground terrain type.
 */
/**
 * @brief 3차 오토마타 패스: 전경 이웃이 3 미만인 칸을 버퍼로 다듬어 없앤다.
 * @param[in] bg_typ 배경 지형 타입.
 * @param[in] fg_typ 전경 지형 타입.
 */
staticfn void
pass_three(schar bg_typ, schar fg_typ)
{
    coordxy x, y;
    short count, dr;

    for (x = 2; x <= WIDTH; x++)
        for (y = 1; y < HEIGHT; y++) {
            for (count = 0, dr = 0; dr < 8; dr++)
                if (get_map(x + dirs[dr * 2], y + dirs[(dr * 2) + 1], bg_typ)
                    == fg_typ)
                    count++;
            if (count < 3)
                new_loc(x, y) = bg_typ;
            else
                new_loc(x, y) = get_map(x, y, bg_typ);
        }

    for (x = 2; x <= WIDTH; x++)
        for (y = 1; y < HEIGHT; y++)
            levl[x][y].typ = new_loc(x, y);
}

/**
 * @brief Flood fill a connected region, assigning it a room number.
 *
 * Recursively spreads @p rmno across cells matching the seed terrain (or any
 * room terrain when @p anyroom is set), updating the region bounding box in
 * @c gm.min_rx..max_ry and counting filled cells in @c gn.n_loc_filled.
 *
 * @param[in] sx      Seed column.
 * @param[in] sy      Seed row.
 * @param[in] rmno    Room number to stamp onto the region.
 * @param[in] lit     Whether the filled cells should be lit.
 * @param[in] anyroom If true, match any room terrain and include walls;
 *                    otherwise match the seed cell's exact type.
 * @warning Recursive; relies on the caller having initialized the
 *          @c gm.min_rx..max_ry bounds before the top-level call.
 */
/**
 * @brief 연결된 영역을 플러드필하여 방 번호를 부여한다.
 *
 * 씨앗 지형과 일치하는 칸들(또는 @p anyroom 시 임의의 방 지형)에 걸쳐
 * @p rmno 를 재귀적으로 확산하며, @c gm.min_rx..max_ry 경계 상자와
 * @c gn.n_loc_filled 채운 칸 수를 갱신한다.
 *
 * @param[in] sx      씨앗 열.
 * @param[in] sy      씨앗 행.
 * @param[in] rmno    영역에 새길 방 번호.
 * @param[in] lit     채운 칸을 밝힐지 여부.
 * @param[in] anyroom 참이면 임의의 방 지형을 매칭하고 벽도 포함하며, 아니면
 *                    씨앗 칸의 정확한 타입만 매칭한다.
 * @warning 재귀적이며, 최상위 호출 전에 호출자가 @c gm.min_rx..max_ry
 *          경계를 초기화해 두었다고 가정한다.
 */
void
flood_fill_rm(
    coordxy sx,
    coordxy sy,
    int rmno,
    boolean lit,
    boolean anyroom)
{
    coordxy i, nx;
    schar fg_typ = levl[sx][sy].typ;

    /* back up to find leftmost uninitialized location */
    while (sx > 0 && (anyroom ? IS_ROOM(levl[sx][sy].typ)
                              : levl[sx][sy].typ == fg_typ)
           && (int) levl[sx][sy].roomno != rmno)
        sx--;
    sx++; /* compensate for extra decrement */

    /* assume sx,sy is valid */
    if (sx < gm.min_rx)
        gm.min_rx = sx;
    if (sy < gm.min_ry)
        gm.min_ry = sy;

    for (i = sx; i <= WIDTH && levl[i][sy].typ == fg_typ; i++) {
        levl[i][sy].roomno = rmno;
        levl[i][sy].lit = lit;
        if (anyroom) {
            /* add walls to room as well */
            coordxy ii, jj;
            for (ii = (i == sx ? i - 1 : i); ii <= i + 1; ii++)
                for (jj = sy - 1; jj <= sy + 1; jj++)
                    if (isok(ii, jj) && (IS_WALL(levl[ii][jj].typ)
                                         || IS_DOOR(levl[ii][jj].typ)
                                         || levl[ii][jj].typ == SDOOR)) {
                        levl[ii][jj].edge = 1;
                        if (lit)
                            levl[ii][jj].lit = lit;

                        if (levl[ii][jj].roomno == NO_ROOM)
                            levl[ii][jj].roomno = rmno;
                        else if ((int) levl[ii][jj].roomno != rmno)
                            levl[ii][jj].roomno = SHARED;
                    }
        }
        gn.n_loc_filled++;
    }
    nx = i;

    if (isok(sx, sy - 1)) {
        for (i = sx; i < nx; i++)
            if (levl[i][sy - 1].typ == fg_typ) {
                if ((int) levl[i][sy - 1].roomno != rmno)
                    flood_fill_rm(i, sy - 1, rmno, lit, anyroom);
            } else {
                if ((i > sx || isok(i - 1, sy - 1))
                    && levl[i - 1][sy - 1].typ == fg_typ) {
                    if ((int) levl[i - 1][sy - 1].roomno != rmno)
                        flood_fill_rm(i - 1, sy - 1, rmno, lit, anyroom);
                }
                if ((i < nx - 1 || isok(i + 1, sy - 1))
                    && levl[i + 1][sy - 1].typ == fg_typ) {
                    if ((int) levl[i + 1][sy - 1].roomno != rmno)
                        flood_fill_rm(i + 1, sy - 1, rmno, lit, anyroom);
                }
            }
    }
    if (isok(sx, sy + 1)) {
        for (i = sx; i < nx; i++)
            if (levl[i][sy + 1].typ == fg_typ) {
                if ((int) levl[i][sy + 1].roomno != rmno)
                    flood_fill_rm(i, sy + 1, rmno, lit, anyroom);
            } else {
                if ((i > sx || isok(i - 1, sy + 1))
                    && levl[i - 1][sy + 1].typ == fg_typ) {
                    if ((int) levl[i - 1][sy + 1].roomno != rmno)
                        flood_fill_rm(i - 1, sy + 1, rmno, lit, anyroom);
                }
                if ((i < nx - 1 || isok(i + 1, sy + 1))
                    && levl[i + 1][sy + 1].typ == fg_typ) {
                    if ((int) levl[i + 1][sy + 1].roomno != rmno)
                        flood_fill_rm(i + 1, sy + 1, rmno, lit, anyroom);
                }
            }
    }

    if (nx > gm.max_rx)
        gm.max_rx = nx - 1; /* nx is just past valid region */
    if (sy > gm.max_ry)
        gm.max_ry = sy;
}

/**
 * @brief Discard the temporary rooms created while joining the map.
 * @note Clears every cell's @c roomno and resets the room/subroom counts and
 *       sentinel entries; call once @c join_map() has dug its corridors.
 */
/**
 * @brief 맵을 결합하는 동안 만들어진 임시 방들을 정리한다.
 * @note 모든 칸의 @c roomno 를 지우고 방/서브룸 카운트와 감시 항목을
 *       재설정한다. @c join_map() 이 통로를 다 판 뒤 한 번 호출한다.
 */
staticfn void
join_map_cleanup(void)
{
    coordxy x, y;

    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++)
            levl[x][y].roomno = NO_ROOM;
    svn.nroom = gn.nsubroom = 0;
    svr.rooms[svn.nroom].hx = gs.subrooms[gn.nsubroom].hx = -1;
}

/**
 * @brief Identify open regions as rooms and dig corridors to connect them.
 *
 * Flood fills each foreground region into a temporary room (erasing tiny
 * pockets that would trap the player), then walks the sorted room list and
 * digs a corridor between successive regions.
 *
 * @param[in] bg_typ Background terrain type.
 * @param[in] fg_typ Foreground terrain type carved into corridors.
 */
/**
 * @brief 열린 영역들을 방으로 인식하고 통로를 파서 서로 연결한다.
 *
 * 각 전경 영역을 임시 방으로 플러드필하고(플레이어를 가둘 만한 작은 구멍은
 * 지운다), 정렬된 방 목록을 따라 이웃한 영역들 사이에 통로를 판다.
 *
 * @param[in] bg_typ 배경 지형 타입.
 * @param[in] fg_typ 통로로 파낼 전경 지형 타입.
 */
staticfn void
join_map(schar bg_typ, schar fg_typ)
{
    struct mkroom *croom, *croom2;

    coordxy x, y, sx, sy;
    coord sm, em;

    /* first, use flood filling to find all of the regions that need joining
     */
    for (x = 2; x <= WIDTH; x++)
        for (y = 1; y < HEIGHT; y++) {
            if (levl[x][y].typ == fg_typ && levl[x][y].roomno == NO_ROOM) {
                gm.min_rx = gm.max_rx = x;
                gm.min_ry = gm.max_ry = y;
                gn.n_loc_filled = 0;
                flood_fill_rm(x, y, svn.nroom + ROOMOFFSET, FALSE, FALSE);
                if (gn.n_loc_filled > 3) {
                    add_room(gm.min_rx, gm.min_ry, gm.max_rx, gm.max_ry,
                             FALSE, OROOM, TRUE);
                    svr.rooms[svn.nroom - 1].irregular = TRUE;
                    if (svn.nroom >= (MAXNROFROOMS * 2))
                        goto joinm;
                } else {
                    /*
                     * it's a tiny hole; erase it from the map to avoid
                     * having the player end up here with no way out.
                     */
                    for (sx = gm.min_rx; sx <= gm.max_rx; sx++)
                        for (sy = gm.min_ry; sy <= gm.max_ry; sy++)
                            if ((int) levl[sx][sy].roomno
                                == svn.nroom + ROOMOFFSET) {
                                levl[sx][sy].typ = bg_typ;
                                levl[sx][sy].roomno = NO_ROOM;
                            }
                }
            }
        }

 joinm:
    /*
     * Ok, now we can actually join the regions with fg_typ's.
     * The rooms are already sorted due to the previous loop,
     * so don't call sort_rooms(), which can screw up the roomno's
     * validity in the levl structure.
     */
    for (croom = &svr.rooms[0], croom2 = croom + 1;
         croom2 < &svr.rooms[svn.nroom]; ) {
        /* pick random starting and end locations for "corridor" */
        if (!somexy(croom, &sm) || !somexy(croom2, &em)) {
            /* ack! -- the level is going to be busted */
            /* arbitrarily pick centers of both rooms and hope for the best */
            impossible("No start/end room loc in join_map.");
            sm.x = croom->lx + ((croom->hx - croom->lx) / 2);
            sm.y = croom->ly + ((croom->hy - croom->ly) / 2);
            em.x = croom2->lx + ((croom2->hx - croom2->lx) / 2);
            em.y = croom2->ly + ((croom2->hy - croom2->ly) / 2);
        }

        (void) dig_corridor(&sm, &em, NULL, FALSE, fg_typ, bg_typ);

        /* choose next region to join */
        /* only increment croom if croom and croom2 are non-overlapping */
        if (croom2->lx > croom->hx
            || ((croom2->ly > croom->hy || croom2->hy < croom->ly)
                && rn2(3))) {
            croom = croom2;
        }
        croom2++; /* always increment the next room */
    }
    join_map_cleanup();
}

/**
 * @brief Apply final touches: optional walls, lighting, and lava/ice state.
 * @param[in] fg_typ    Foreground terrain type.
 * @param[in] bg_typ    Background terrain type.
 * @param[in] lit       Whether open terrain and walls should be lit.
 * @param[in] walled    Whether to wall off the generated cavern.
 * @param[in] icedpools Whether ICE cells came from pools (else moats).
 */
/**
 * @brief 마무리 처리: 선택적 벽, 조명, 용암/얼음 상태를 적용한다.
 * @param[in] fg_typ    전경 지형 타입.
 * @param[in] bg_typ    배경 지형 타입.
 * @param[in] lit       열린 지형과 벽을 밝힐지 여부.
 * @param[in] walled    생성된 동굴을 벽으로 둘러쌀지 여부.
 * @param[in] icedpools ICE 칸이 웅덩이 출신인지(아니면 해자인지) 여부.
 */
staticfn void
finish_map(
    schar fg_typ,
    schar bg_typ,
    boolean lit,
    boolean walled,
    boolean icedpools)
{
    coordxy x, y;

    if (walled)
        wallify_map(1, 0, COLNO-1, ROWNO-1);

    if (lit) {
        for (x = 1; x < COLNO; x++)
            for (y = 0; y < ROWNO; y++)
                if ((!IS_OBSTRUCTED(fg_typ) && levl[x][y].typ == fg_typ)
                    || (!IS_OBSTRUCTED(bg_typ) && levl[x][y].typ == bg_typ)
                    || (bg_typ == TREE && levl[x][y].typ == bg_typ)
                    || (walled && IS_WALL(levl[x][y].typ)))
                    levl[x][y].lit = TRUE;
        for (x = 0; x < svn.nroom; x++)
            svr.rooms[x].rlit = 1;
    }
    /* light lava even if everything's otherwise unlit;
       ice might be frozen pool rather than frozen moat */
    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++) {
            if (levl[x][y].typ == LAVAPOOL)
                levl[x][y].lit = TRUE;
            else if (levl[x][y].typ == ICE)
                levl[x][y].icedpool = icedpools ? ICED_POOL : ICED_MOAT;
        }
}

/**
 * @brief Remove every room that falls inside a rectangular region.
 *
 * Rooms fully inside the region [lx,hx) x [ly,hy) are removed; rooms only
 * partially inside are truncated (currently just validated).  Must run before
 * the overlaid MAP's REGIONs or ROOMs are processed.
 *
 * @param[in] lx Left bound (inclusive).
 * @param[in] ly Top bound (inclusive).
 * @param[in] hx Right bound (exclusive).
 * @param[in] hy Bottom bound (exclusive).
 * @note Assumes roomno fields inside the region are cleared and those outside
 *       are set; see the block comment for the full contract.
 */
/**
 * @brief 직사각형 영역 안에 들어오는 모든 방을 제거한다.
 *
 * 영역 [lx,hx) x [ly,hy) 에 완전히 포함된 방은 제거하고, 일부만 걸친 방은
 * 잘라낸다(현재는 검증만). 덮어씌운 MAP 의 REGION/ROOM 처리 전에 호출해야
 * 한다.
 *
 * @param[in] lx 왼쪽 경계(포함).
 * @param[in] ly 위쪽 경계(포함).
 * @param[in] hx 오른쪽 경계(제외).
 * @param[in] hy 아래쪽 경계(제외).
 * @note 영역 내부 roomno 는 지워지고 외부는 설정되어 있다고 가정한다. 전체
 *       계약은 상단 블록 주석을 참고한다.
 */
void
remove_rooms(coordxy lx, coordxy ly, coordxy hx, coordxy hy)
{
    int i;
    struct mkroom *croom;

    for (i = svn.nroom - 1; i >= 0; --i) {
        croom = &svr.rooms[i];
        if (croom->hx < lx || croom->lx >= hx || croom->hy < ly
            || croom->ly >= hy)
            continue; /* no overlap */

        if (croom->lx < lx || croom->hx >= hx || croom->ly < ly
            || croom->hy >= hy) { /* partial overlap */
            /* TODO: ensure remaining parts of room are still joined */

            if (!croom->irregular)
                impossible("regular room in joined map");
        } else {
            /* total overlap, remove the room */
            remove_room((unsigned) i);
        }
    }
}

/**
 * @brief Remove a single subroom-free room from the rooms array.
 *
 * Swaps the last room into the removed slot and rewrites the affected cells'
 * @c roomno so the array stays compact.
 *
 * @param[in] roomno Index of the room to remove.
 * @warning Only handles rooms with no subrooms; assumes the room's level
 *          contents have already been reset.
 */
/**
 * @brief 서브룸이 없는 방 하나를 방 배열에서 제거한다.
 *
 * 마지막 방을 제거되는 슬롯으로 옮기고, 영향을 받는 칸들의 @c roomno 를
 * 다시 써서 배열을 조밀하게 유지한다.
 *
 * @param[in] roomno 제거할 방의 인덱스.
 * @warning 서브룸이 없는 방만 처리하며, 해당 방의 레벨 내용이 이미
 *          재설정되었다고 가정한다.
 */
staticfn void
remove_room(unsigned int roomno)
{
    struct mkroom *croom = &svr.rooms[roomno];
    struct mkroom *maxroom = &svr.rooms[--svn.nroom];
    coordxy x, y;
    unsigned oroomno;

    if (croom != maxroom) {
        /* since the order in the array only matters for making corridors,
         * copy the last room over the one being removed on the assumption
         * that corridors have already been dug. */
        *croom = *maxroom;

        /* since maxroom moved, update affected level roomno values */
        oroomno = svn.nroom + ROOMOFFSET;
        roomno += ROOMOFFSET;
        for (x = croom->lx; x <= croom->hx; ++x)
            for (y = croom->ly; y <= croom->hy; ++y) {
                if (levl[x][y].roomno == oroomno)
                    levl[x][y].roomno = roomno;
            }
    }

    maxroom->hx = -1; /* just like add_room */
}

/**
 * @brief Resolve a lit-state request, rolling randomly when unspecified.
 * @param[in] litstate Explicit 0/1 lit flag, or negative to randomize by depth.
 * @retval TRUE  The region should be lit.
 * @retval FALSE The region should be dark.
 */
/**
 * @brief 조명 상태 요청을 해석하며, 미지정 시 무작위로 결정한다.
 * @param[in] litstate 명시적 0/1 조명 플래그, 음수면 깊이에 따라 무작위.
 * @retval TRUE  영역을 밝혀야 함.
 * @retval FALSE 영역을 어둡게 둬야 함.
 */
boolean
litstate_rnd(int litstate)
{
    if (litstate < 0)
        return (rnd(1 + abs(depth(&u.uz))) < 11 && rn2(77)) ? TRUE : FALSE;
    return (boolean) litstate;
}

/**
 * @brief Generate a cavern level from a level-initialization descriptor.
 *
 * Orchestrates the full pipeline: allocate the scratch buffer, seed and smooth
 * the cave via the automaton passes, optionally join regions with corridors,
 * finish lighting/walls, and free the scratch buffer.
 *
 * @param[in] init_lev Descriptor holding terrain types and generation flags.
 * @note Allocates @c gn.new_locations for the duration of the call and frees
 *       it before returning.
 */
/**
 * @brief 레벨 초기화 서술자로부터 동굴 레벨을 생성한다.
 *
 * 전체 파이프라인을 지휘한다: 스크래치 버퍼 할당, 오토마타 패스로 동굴 씨앗과
 * 평활화, 선택적 통로 결합, 조명/벽 마무리, 스크래치 버퍼 해제.
 *
 * @param[in] init_lev 지형 타입과 생성 플래그를 담은 서술자.
 * @note 호출 동안 @c gn.new_locations 를 할당하고 반환 전에 해제한다.
 */
void
mkmap(lev_init *init_lev)
{
    schar bg_typ = init_lev->bg, fg_typ = init_lev->fg;
    boolean smooth = init_lev->smoothed, join = init_lev->joined;
    xint16 lit = init_lev->lit, walled = init_lev->walled;
    int i;

    lit = litstate_rnd(lit);

    gn.new_locations = (char *) alloc((WIDTH + 1) * HEIGHT);

    init_map(bg_typ);
    init_fill(bg_typ, fg_typ);

    for (i = 0; i < N_P1_ITER; i++)
        pass_one(bg_typ, fg_typ);

    for (i = 0; i < N_P2_ITER; i++)
        pass_two(bg_typ, fg_typ);

    if (smooth)
        for (i = 0; i < N_P3_ITER; i++)
            pass_three(bg_typ, fg_typ);

    if (join)
        join_map(bg_typ, fg_typ);

    finish_map(fg_typ, bg_typ, (boolean) lit, (boolean) walled,
               init_lev->icedpools);
    /* a walled, joined level is cavernous, not mazelike -dlc */
    if (walled && join) {
        svl.level.flags.is_maze_lev = FALSE;
        svl.level.flags.is_cavernous_lev = TRUE;
    }
    free(gn.new_locations);
}

/*mkmap.c*/
