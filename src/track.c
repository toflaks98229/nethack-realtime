/* NetHack 5.0	track.c	$NHDT-Date: 1781973070 2026/06/20 16:31:10 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.24 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Kenneth Lorber, Kensington, Maryland, 2015. */
/* NetHack may be freely redistributed.  See license for details. */
/* track.c - version 1.0.2 */

/**
 * @file track.c
 * @brief 영웅(플레이어)의 이동 자취(track)를 기록하고 조회하는 모듈.
 *
 * 영웅이 지나온 최근 좌표들을 고정 크기 원형 버퍼(@c utrack)에 저장한다.
 * 몬스터가 영웅의 냄새/발자국을 추적하는 등의 로직에서 이 자취를 사용한다.
 *
 * @note 자취 상태는 파일 지역 static 변수로 유지되므로 스레드 안전하지 않다.
 */

#include "hack.h"

/** @brief 자취 원형 버퍼(@c utrack)의 크기(저장 가능한 최대 좌표 수). */
#define UTSZ 100

/** @brief 현재 저장된 자취 좌표의 개수(@c UTSZ 로 포화된다). */
static NEARDATA int utcnt,
/** @brief 원형 버퍼에서 다음에 기록할 위치(가장 최근 항목의 다음 인덱스). */
    utpnt;
/** @brief 영웅이 지나온 최근 좌표들을 담는 원형 버퍼. */
static NEARDATA coord utrack[UTSZ];

/**
 * @brief 자취 기록 상태를 초기화한다.
 *
 * 카운터를 0으로 되돌리고 버퍼 전체를 0으로 채운다.
 */
void
initrack(void)
{
    utcnt = utpnt = 0;
    (void) memset((genericptr_t) &utrack, 0, sizeof(utrack));
}

#ifndef SFCTOOL
/**
 * @brief 영웅의 현재 위치를 자취 버퍼에 추가한다.
 *
 * 원형 버퍼의 다음 슬롯에 현재 좌표(@c u.ux, @c u.uy)를 기록한다.
 *
 * @note 은신 반지(@c RIN_STEALTH)를 착용 중이면 자취를 남기지 않고 반환한다.
 */
void
settrack(void)
{
    if ((uleft && uleft->otyp == RIN_STEALTH)
        || (uright && uright->otyp == RIN_STEALTH))
        return;

    if (utcnt < UTSZ)
        utcnt++;
    if (utpnt == UTSZ)
        utpnt = 0;
    utrack[utpnt].x = u.ux;
    utrack[utpnt].y = u.uy;
    utpnt++;
}

/**
 * @brief 지정한 좌표 위 또는 인접한 곳의, 영웅이 가장 최근에 남긴 자취를 찾는다.
 *
 * 최신 항목부터 역순으로 탐색하여 @p x, @p y 와의 거리가 1 이하인
 * 자취 좌표를 반환한다.
 *
 * @param[in] x 기준 x 좌표.
 * @param[in] y 기준 y 좌표.
 * @return 조건을 만족하는 자취 좌표의 포인터(내부 버퍼를 가리킨다).
 * @retval NULL 조건을 만족하는 자취가 없거나, 가장 가까운 자취가
 *              기준 좌표와 정확히 겹치는 경우.
 * @note 반환된 포인터는 내부 버퍼를 가리키므로 호출자가 free 해서는 안 된다.
 */
coord *
gettrack(coordxy x, coordxy y)
{
    int cnt, ndist;
    coord *tc;
    cnt = utcnt;
    for (tc = &utrack[utpnt]; cnt--;) {
        if (tc == utrack)
            tc = &utrack[UTSZ - 1];
        else
            tc--;
        ndist = distmin(x, y, tc->x, tc->y);

        if (ndist <= 1)
            return (ndist ? tc : 0);
    }
    return (coord *) 0;
}
#endif /* !SFCTOOL */

/**
 * @brief 지정한 좌표에 영웅의 자취가 존재하는지 확인한다.
 *
 * @param[in] x 확인할 x 좌표.
 * @param[in] y 확인할 y 좌표.
 * @return 자취 존재 여부.
 * @retval TRUE  해당 좌표에 자취가 있다.
 * @retval FALSE 해당 좌표에 자취가 없다.
 */
boolean
hastrack(coordxy x, coordxy y)
{
    int i;

    for (i = 0; i < utcnt; i++)
        if (utrack[i].x == x && utrack[i].y == y)
            return TRUE;

    return FALSE;
}

/**
 * @brief 영웅의 자취 정보를 세이브 파일에 기록한다.
 *
 * @param[in,out] nhfp 저장 대상 NetHack 파일 핸들.
 * @note 데이터 해제 단계(@c release_data)에서는 저장 후 자취를 초기화한다.
 */
void
save_track(NHFILE *nhfp)
{
    if (update_file(nhfp)) {
        int i;

        Sfo_int(nhfp, &utcnt, "track-utcnt");
        Sfo_int(nhfp, &utpnt, "track-utpnt");
        for (i = 0; i < utcnt; i++) {
            Sfo_nhcoord(nhfp, &utrack[i], "utrack");
        }
    }
    if (release_data(nhfp))
        initrack();
}

/**
 * @brief 세이브 파일에서 영웅의 자취 정보를 복원한다.
 *
 * @param[in,out] nhfp 복원 원본 NetHack 파일 핸들.
 * @warning 저장된 카운터가 버퍼 크기(@c UTSZ)를 초과하면 @c panic() 으로
 *          프로그램을 중단시킨다(손상된 세이브 방어).
 */
void
rest_track(NHFILE *nhfp)
{
    int i;

    Sfi_int(nhfp, &utcnt, "track-utcnt");
    Sfi_int(nhfp, &utpnt, "track-utpnt");

    if (utcnt > UTSZ || utpnt > UTSZ)
        panic("rest_track: impossible pt counts");
    for (i = 0; i < utcnt; i++) {
        Sfi_nhcoord(nhfp, &utrack[i], "utrack");
    }
}

/*track.c*/
