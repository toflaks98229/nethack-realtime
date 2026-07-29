/* NetHack 5.0	rect.c	$NHDT-Date: 1781973063 2026/06/20 16:31:03 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.26 $ */
/* Copyright (c) 1990 by Jean-Christophe Collet                   */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file rect.c
 * @brief 레벨 생성에 사용되는 사각형(NhRect) 목록 관리 유틸리티.
 *
 * 이 파일은 방 생성 과정에서 필요한 각종 사각형 연산을 담당한다.
 * 아직 사용되지 않은(비어 있는) 사각형들의 목록을 내부 static 배열로
 * 유지하며, 사각형의 추가/삭제/검색/분할 기능을 제공한다.
 *
 * @note 모든 함수는 파일 지역 static 상태(@c rect, @c n_rects, @c rect_cnt)를
 *       공유하므로 스레드 안전하지 않다. 레벨 생성은 단일 스레드에서만
 *       수행된다는 전제를 가진다.
 */

#include "hack.h"

/** @brief 사각형 좌우 여백(열 방향)에 대한 최소 간격 한계값. */
#define XLIM 4
/** @brief 사각형 상하 여백(행 방향)에 대한 최소 간격 한계값. */
#define YLIM 3

/** @brief 비어 있는 사각형들을 담는 동적 배열. 미할당 상태에서는 NULL. */
static NhRect *rect = (NhRect *) 0;
/** @brief @c rect 배열에 할당된 원소 개수(수용 가능한 최대 사각형 수). */
static int n_rects = 0;
/** @brief 현재 목록에 들어 있는 유효한 사각형의 개수. */
static int rect_cnt;

/* public prototype (see extern.h); forward-declared here for local use */
int get_rect_ind(NhRect *);

staticfn boolean intersect(NhRect *, NhRect *, NhRect *);

/**
 * @brief 사각형 목록을 새 레벨 생성용으로 초기화한다.
 *
 * 최초 호출 시 내부 배열을 할당하고, 이후에는 목록을 레벨 전체를 덮는
 * 단일 사각형 하나만 담도록 재설정한다.
 *
 * @note 새로운 레벨을 생성할 때마다 호출해야 한다.
 * @warning 배열 할당에 실패하면 @c panic() 으로 프로그램을 중단시킨다.
 */
void
init_rect(void)
{
    if (!rect) {
        n_rects = (COLNO * ROWNO) / 30;
        rect = (NhRect *) alloc(sizeof(NhRect) * n_rects);
        if (!rect)
            panic("Could not alloc rect");
    }

    rect_cnt = 1;
    rect[0].lx = rect[0].ly = 0;
    rect[0].hx = COLNO - 1;
    rect[0].hy = ROWNO - 1;
}

/**
 * @brief 사각형 목록에 사용된 자원을 해제한다.
 *
 * 내부 배열을 free 하고 관련 카운터를 0으로 되돌린다.
 *
 * @note 해제 후 다시 사용하려면 @c init_rect() 를 호출해야 한다.
 */
void
free_rect(void)
{
    if (rect)
        free(rect);
    rect = 0;
    n_rects = rect_cnt = 0;
}

/**
 * @brief 특정 사각형과 정확히 일치하는 항목의 목록 내 인덱스를 찾는다.
 *
 * @param[in] r 검색 대상 사각형. 네 좌표(lx, ly, hx, hy)가 모두 일치해야 한다.
 * @return 일치하는 사각형의 인덱스.
 * @retval -1 일치하는 사각형이 없을 경우.
 */
int
get_rect_ind(NhRect *r)
{
    NhRect *rectp;
    int lx, ly, hx, hy;
    int i;

    lx = r->lx;
    ly = r->ly;
    hx = r->hx;
    hy = r->hy;
    for (i = 0, rectp = &rect[0]; i < rect_cnt; i++, rectp++)
        if (lx == rectp->lx && ly == rectp->ly && hx == rectp->hx
            && hy == rectp->hy)
            return i;
    return -1;
}

/**
 * @brief 주어진 사각형을 완전히 포함하는 비어 있는 사각형을 찾는다.
 *
 * @param[in] r 포함되어야 할 기준 사각형.
 * @return 목록에서 @p r 을 완전히 포함하는 첫 번째 사각형의 포인터.
 * @retval NULL 그러한 사각형이 없을 경우.
 * @note 반환된 포인터는 내부 배열을 가리키므로 호출자가 free 해서는 안 되며,
 *       목록 변경 연산 이후에는 무효가 될 수 있다.
 */
NhRect *
get_rect(NhRect *r)
{
    NhRect *rectp;
    int lx, ly, hx, hy;
    int i;

    lx = r->lx;
    ly = r->ly;
    hx = r->hx;
    hy = r->hy;
    for (i = 0, rectp = &rect[0]; i < rect_cnt; i++, rectp++)
        if (lx >= rectp->lx && ly >= rectp->ly && hx <= rectp->hx
            && hy <= rectp->hy)
            return rectp;
    return 0;
}

/**
 * @brief 목록에서 임의의 사각형 하나를 무작위로 선택해 반환한다.
 *
 * @return 무작위로 선택된 사각형의 포인터.
 * @retval NULL 목록이 비어 있을 경우.
 * @note 반환된 포인터는 내부 배열을 가리키므로 호출자가 free 해서는 안 된다.
 */
NhRect *
rnd_rect(void)
{
    return rect_cnt > 0 ? &rect[rn2(rect_cnt)] : 0;
}

/**
 * @brief 두 사각형을 모두 포함하는 최소 경계 사각형을 계산한다.
 *
 * @param[in]  r1 첫 번째 사각형(값 전달).
 * @param[in]  r2 두 번째 사각형(값 전달).
 * @param[out] r3 @p r1 과 @p r2 를 모두 감싸는 경계 사각형을 저장할 위치.
 */
void
rect_bounds(NhRect r1, NhRect r2, NhRect *r3)
{
    r3->lx = min(r1.lx, r2.lx);
    r3->ly = min(r1.ly, r2.ly);
    r3->hx = max(r1.hx, r2.hx);
    r3->hy = max(r1.hy, r2.hy);
}

/**
 * @brief 목록에서 지정한 사각형을 제거한다.
 *
 * 일치하는 항목을 찾아 목록의 마지막 항목으로 덮어써서 제거한다.
 *
 * @param[in] r 제거할 사각형. @c get_rect_ind() 로 일치 항목을 검색한다.
 * @note 일치하는 사각형이 없으면 아무 동작도 하지 않는다.
 */
void
remove_rect(NhRect *r)
{
    int ind;

    ind = get_rect_ind(r);
    if (ind >= 0)
        rect[ind] = rect[--rect_cnt];
}

/**
 * @brief 새 사각형을 목록에 추가한다.
 *
 * 이미 다른 사각형에 포함되는 사각형은 추가하지 않는다.
 *
 * @param[in] r 추가할 사각형.
 * @warning 목록이 가득 찬 상태에서 호출하면 @c impossible() 경고를 내고
 *          아무것도 추가하지 않은 채 반환한다.
 */
void
add_rect(NhRect *r)
{
    if (rect_cnt >= n_rects) {
        impossible("n_rects may be too small.");
        return;
    }
    /* Check that this NhRect is not included in another one */
    if (get_rect(r))
        return;
    rect[rect_cnt] = *r;
    rect_cnt++;
}

/**
 * @brief 한 사각형 안에 포함된 영역을 할당하기 위해 사각형을 분할한다.
 *
 * @p r1 은 이미 목록에 존재하는 사각형이고 @p r2 는 그 안에 포함된다.
 * @p r2 영역을 사용 처리하기 위해, @p r1 을 @p r2 를 둘러싼 더 작은
 * 사각형들로 나눈 뒤 @p r1 자신을 목록에서 제거한다. 분할된 조각은
 * 여백 한계(@c XLIM, @c YLIM)를 만족할 만큼 충분히 클 때만 다시 추가된다.
 *
 * @param[in] r1 목록에 존재하며 분할될 원본 사각형.
 * @param[in] r2 @p r1 내부에 포함된, 할당하고자 하는 영역.
 * @note 분할 과정에서 @p r2 와 겹치는 다른 사각형에 대해 재귀적으로 호출된다.
 */
void
split_rects(NhRect *r1, NhRect *r2)
{
    NhRect r, old_r;
    int i;

    old_r = *r1;
    remove_rect(r1);

    /* Walk down since rect_cnt & rect[] will change... */
    for (i = rect_cnt - 1; i >= 0; i--)
        if (intersect(&rect[i], r2, &r))
            split_rects(&rect[i], &r);

    if (r2->ly - old_r.ly - 1
        > (old_r.hy < ROWNO - 1 ? 2 * YLIM : YLIM + 1) + 4) {
        r = old_r;
        r.hy = r2->ly - 2;
        add_rect(&r);
    }
    if (r2->lx - old_r.lx - 1
        > (old_r.hx < COLNO - 1 ? 2 * XLIM : XLIM + 1) + 4) {
        r = old_r;
        r.hx = r2->lx - 2;
        add_rect(&r);
    }
    if (old_r.hy - r2->hy - 1 > (old_r.ly > 0 ? 2 * YLIM : YLIM + 1) + 4) {
        r = old_r;
        r.ly = r2->hy + 2;
        add_rect(&r);
    }
    if (old_r.hx - r2->hx - 1 > (old_r.lx > 0 ? 2 * XLIM : XLIM + 1) + 4) {
        r = old_r;
        r.lx = r2->hx + 2;
        add_rect(&r);
    }
}

/**
 * @brief 두 사각형(@p r1, @p r2)의 교집합을 계산한다.
 *
 * @param[in]  r1 첫 번째 사각형.
 * @param[in]  r2 두 번째 사각형.
 * @param[out] r3 교집합이 존재할 경우 그 결과를 저장할 위치.
 * @return 교집합의 존재 여부.
 * @retval TRUE  교집합이 존재하며 그 결과를 @p r3 에 기록했다.
 * @retval FALSE 교집합이 존재하지 않는다(@p r3 의 내용은 정의되지 않음).
 */
staticfn boolean
intersect(NhRect *r1, NhRect *r2, NhRect *r3)
{
    if (r2->lx > r1->hx || r2->ly > r1->hy || r2->hx < r1->lx
        || r2->hy < r1->ly)
        return FALSE;

    r3->lx = (r2->lx > r1->lx ? r2->lx : r1->lx);
    r3->ly = (r2->ly > r1->ly ? r2->ly : r1->ly);
    r3->hx = (r2->hx > r1->hx ? r1->hx : r2->hx);
    r3->hy = (r2->hy > r1->hy ? r1->hy : r2->hy);

    if (r3->lx > r3->hx || r3->ly > r3->hy)
        return FALSE;
    return TRUE;
}

/*rect.c*/
