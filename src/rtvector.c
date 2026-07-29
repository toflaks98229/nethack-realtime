/* NetHack 5.0	rtvector.c	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Added 2026-07 for the real-time fork (see MODIFICATIONS.md).  New file;
   nothing here existed in the upstream NetHack distribution. */

/**
 * @file rtvector.c
 * @brief Continuous positions that chase the grid squares entities occupy.
 *
 * Each tracked entity keeps a real-valued point and the square it belongs to.
 * When the square changes the point stays where it was, and every frame it
 * travels toward the new square at a fixed rate, arriving in about the time one
 * game turn takes. That is the whole mechanism: the game keeps moving entities
 * a square at a time, and these points trail behind, which is what gives motion
 * something to draw between squares.
 *
 * @note Slots are keyed by identifier and reclaimed by age, so entities that
 *       die or leave the level need not announce themselves.
 * @warning This is observation only. Nothing here is read by game logic, and
 *          the positions must never be allowed to influence it: the rules
 *          assume entities occupy whole squares simultaneously.
 */

/**
 * @file rtvector.c
 * @brief 엔티티가 차지한 격자 칸을 뒤쫓는 연속 위치.
 *
 * 추적되는 엔티티마다 실수 좌표 하나와 자신이 속한 칸을 보관한다. 칸이 바뀌어도
 * 좌표는 있던 자리에 남고, 매 프레임 일정한 속도로 새 칸을 향해 이동해 대략 한
 * 게임 턴에 해당하는 시간에 도착한다. 기제는 이것이 전부다. 게임은 여전히
 * 엔티티를 한 칸씩 옮기고, 이 좌표들이 그 뒤를 따라가며, 그 덕분에 칸과 칸
 * 사이에 그릴 것이 생긴다.
 *
 * @note 슬롯은 식별자를 키로 하며 오래된 것부터 회수되므로, 죽거나 레벨을 떠나는
 *       엔티티가 따로 알릴 필요가 없다.
 * @warning 이것은 관찰 전용이다. 여기의 어떤 것도 게임 로직이 읽지 않으며, 이
 *          위치들이 게임 로직에 영향을 주게 두어서는 안 된다. 규칙은 엔티티들이
 *          온전한 칸을 동시에 차지한다고 전제한다.
 */

#include "hack.h"
#include "nh_rtvector.h"

#ifdef REALTIME_PROTO

/** @brief Number of entities that can be interpolated at once. */
/** @brief 동시에 보간할 수 있는 엔티티의 수. */
#define RTV_SLOTS 256

/** @brief How long an untouched slot survives before it may be reused, in ms. */
/** @brief 갱신되지 않은 슬롯이 재사용되기까지 살아남는 시간(밀리초). */
#define RTV_STALE_MS (RT_TURN_MS * 8)

/** @brief Below this distance, in grid units, a position counts as arrived. */
/** @brief 이 거리(격자 단위) 미만이면 위치가 도착한 것으로 간주한다. */
#define RTV_EPSILON 0.02

/**
 * @brief Round a real coordinate to the square that contains it.
 * @note Written out rather than using @c floor() so the file needs no floating
 *       point library beyond what it already uses, and so the halfway case is
 *       defined rather than implementation-dependent.
 */
/**
 * @brief 실수 좌표를 그것이 속한 칸으로 반올림한다.
 * @note @c floor() 대신 직접 계산한다. 이 파일이 이미 쓰는 것 이상의 부동소수점
 *       라이브러리를 요구하지 않기 위해서이고, 정확히 중간인 경우가 구현에
 *       따라 달라지지 않고 정의되도록 하기 위해서다.
 */
#define RT_ROUND(v) ((int) (((v) < 0.0) ? ((v) - 0.5) : ((v) + 0.5)))

/**
 * @brief One tracked entity: where it is drawn, and where it belongs.
 * @note @c id of zero marks a free slot; no monster has @c m_id zero and the
 *       hero uses @c RTV_HERO_ID.
 */
/**
 * @brief 추적 중인 엔티티 하나. 어디에 그려지는지와 어디에 속하는지.
 * @note @c id 가 0이면 빈 슬롯이다. @c m_id 가 0인 몬스터는 없으며 영웅은
 *       @c RTV_HERO_ID 를 쓴다.
 */
struct rtv_slot {
    unsigned id;         /* owning entity, 0 when free */
    double x, y;         /* where it is drawn, in grid units */
    coordxy sqx, sqy;    /* the square it belongs to */
    unsigned long tick;  /* nt_ticks() when last touched */
};

static struct rtv_slot rtv_slots[RTV_SLOTS];
static unsigned long rtv_last_advance = 0;

/*
 * Free-movement state.  The hero is the one entity whose position is driven by
 * input rather than chasing the square it was placed on, so it lives here
 * rather than in the slot table.
 */
static double rtv_fx, rtv_fy;         /* the hero's real position */
static boolean rtv_free_init = FALSE;
static unsigned long rtv_free_tick = 0;
static boolean rtv_step_pending = FALSE;
static coordxy rtv_step_sqx, rtv_step_sqy;     /* square the step asked for */
static coordxy rtv_step_fromx, rtv_step_fromy; /* square it was asked from */
static long rtv_step_moves = 0;                /* svm.moves when it was asked */
static coordxy rtv_blocked_x, rtv_blocked_y;   /* square that just refused us */
static unsigned long rtv_blocked_tick = 0;     /* when it refused */

staticfn struct rtv_slot *rtv_find(unsigned);
staticfn struct rtv_slot *rtv_claim(unsigned);

/**
 * @brief Locate the slot tracking an entity.
 * @param[in] id Entity identifier.
 * @return The slot, or NULL if the entity is not tracked.
 */
/**
 * @brief 어떤 엔티티를 추적 중인 슬롯을 찾는다.
 * @param[in] id 엔티티 식별자.
 * @return 해당 슬롯, 추적 중이 아니면 NULL.
 */
staticfn struct rtv_slot *
rtv_find(unsigned id)
{
    int i;

    for (i = 0; i < RTV_SLOTS; i++)
        if (rtv_slots[i].id == id)
            return &rtv_slots[i];
    return (struct rtv_slot *) 0;
}

/**
 * @brief Obtain a slot for an entity, evicting the stalest one if need be.
 * @param[in] id Entity identifier.
 * @return A slot to use; never NULL.
 * @note Eviction is by age, so the entity sacrificed is the one that has gone
 *       longest without moving -- which is the one least likely to be mid-glide.
 */
/**
 * @brief 엔티티를 위한 슬롯을 얻으며, 필요하면 가장 오래된 것을 밀어낸다.
 * @param[in] id 엔티티 식별자.
 * @return 사용할 슬롯. NULL 이 되는 일은 없다.
 * @note 회수 기준은 나이다. 따라서 희생되는 엔티티는 가장 오래 움직이지 않은
 *       쪽이며, 이는 이동 중일 가능성이 가장 낮은 엔티티이기도 하다.
 */
staticfn struct rtv_slot *
rtv_claim(unsigned id)
{
    struct rtv_slot *s = rtv_find(id), *oldest;
    int i;

    if (s)
        return s;
    for (i = 0; i < RTV_SLOTS; i++)
        if (rtv_slots[i].id == 0)
            return &rtv_slots[i];

    oldest = &rtv_slots[0];
    for (i = 1; i < RTV_SLOTS; i++)
        if (rtv_slots[i].tick < oldest->tick)
            oldest = &rtv_slots[i];
    return oldest;
}

/* interface documented in nh_rtvector.h */
void
rtv_set_square(unsigned id, coordxy x, coordxy y)
{
    struct rtv_slot *s;
    boolean tracked;
    int dx, dy;

    if (!id || !isok(x, y))
        return;

    s = rtv_find(id);
    tracked = (s != 0);
    if (!tracked)
        s = rtv_claim(id);

    if (tracked) {
        dx = (int) x - (int) s->sqx;
        dy = (int) y - (int) s->sqy;
        /* a step keeps the drawn position where it was so it can travel;
           anything further is a teleport and simply appears */
        if (dx < -1 || dx > 1 || dy < -1 || dy > 1) {
            s->x = (double) x;
            s->y = (double) y;
        }
    } else {
        s->x = (double) x;
        s->y = (double) y;
    }

    s->id = id;
    s->sqx = x;
    s->sqy = y;
    s->tick = nt_ticks();
}

/* interface documented in nh_rtvector.h */
void
rtv_advance(void)
{
    unsigned long now = nt_ticks(), elapsed;
    double step;
    int i;

    if (rtv_last_advance == 0) {
        rtv_last_advance = now ? now : 1;
        return;
    }
    elapsed = now - rtv_last_advance;
    rtv_last_advance = now;
    if (elapsed == 0)
        return;
    /* a long stall means the window was not being drawn; do not let positions
       lurch across the map to catch up */
    if (elapsed > (unsigned long) (RT_TURN_MS * 4))
        elapsed = (unsigned long) (RT_TURN_MS * 4);

    /* one square per turn-time, so motion matches the pace of the world */
    step = (double) elapsed / (double) RT_TURN_MS;

    for (i = 0; i < RTV_SLOTS; i++) {
        struct rtv_slot *s = &rtv_slots[i];
        double dx, dy, dist;

        if (!s->id)
            continue;
        if ((unsigned long) (now - s->tick) > (unsigned long) RTV_STALE_MS) {
            s->id = 0; /* gone: died, left the level, or never drawn */
            continue;
        }
        dx = (double) s->sqx - s->x;
        dy = (double) s->sqy - s->y;
        dist = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        if (dist <= RTV_EPSILON) {
            s->x = (double) s->sqx;
            s->y = (double) s->sqy;
            continue;
        }
        if (step >= 1.0 || step >= dist) {
            s->x = (double) s->sqx;
            s->y = (double) s->sqy;
        } else {
            s->x += dx * (step / dist);
            s->y += dy * (step / dist);
        }
    }
}

/* interface documented in nh_rtvector.h */
boolean
rtv_offset_at(coordxy x, coordxy y, double *ox, double *oy)
{
    struct monst *mtmp;
    struct rtv_slot *s;

    if (!isok(x, y))
        return FALSE;
    mtmp = svl.level.monsters[x][y];
    if (!mtmp)
        return FALSE;
    s = rtv_find(mtmp->m_id);
    if (!s)
        return FALSE;

    *ox = s->x - (double) x;
    *oy = s->y - (double) y;
    return (*ox != 0.0 || *oy != 0.0);
}

/* interface documented in nh_rtvector.h
   The hero's position is driven by input rather than chasing its square, so
   this reports the free position rather than a slot in the chase table. */
boolean
rtv_hero_offset(double *ox, double *oy)
{
    if (!rtv_free_init)
        return FALSE;
    *ox = rtv_fx - (double) u.ux;
    *oy = rtv_fy - (double) u.uy;
    return (*ox != 0.0 || *oy != 0.0);
}

/*
 * Free movement.
 *
 * The hero's position is a real point that input moves in any direction; the
 * square the game works with is that point rounded.  Crossing into a different
 * square is what asks the game for a step, and because the game may refuse
 * (walls, doors, an occupant) the position is predicted and then reconciled
 * against where the hero actually ended up.
 */

/** @brief How close to a square's edge the position may sit, in grid units. */
/** @brief 위치가 칸의 가장자리에 얼마나 다가갈 수 있는지, 격자 단위. */
#define RTV_EDGE 0.48

/* interface documented in nh_rtvector.h */
boolean
rtv_hero_free_move(double dx, double dy, coordxy *sx, coordxy *sy)
{
    unsigned long now = nt_ticks(), elapsed;
    double len, speed, nx, ny;
    coordxy tx, ty;
    int i;

    if (!rtv_free_init || !isok((coordxy) rtv_fx, (coordxy) rtv_fy)) {
        rtv_fx = (double) u.ux;
        rtv_fy = (double) u.uy;
        rtv_free_init = TRUE;
        rtv_free_tick = now ? now : 1;
        rtv_step_pending = FALSE;
        return FALSE;
    }

    /* Reconcile: see what the game did with the step we asked for.
       While it is outstanding the position is left exactly where it was.  It
       must not be pulled back toward the square the hero has not left yet --
       doing that made the hero visibly jump backwards on every step. */
    if (rtv_step_pending) {
        if (u.ux == rtv_step_sqx && u.uy == rtv_step_sqy) {
            rtv_step_pending = FALSE; /* accepted; keep the predicted position */
        } else if (u.ux != rtv_step_fromx || u.uy != rtv_step_fromy) {
            /* somewhere neither predicted nor left from: the hero was moved by
               something other than us, so the prediction describes nothing */
            rtv_fx = (double) u.ux;
            rtv_fy = (double) u.uy;
            rtv_step_pending = FALSE;
        } else if (svm.moves != rtv_step_moves) {
            /* the game has taken a turn and the hero is still where it was:
               refused.  Rest against whatever refused us rather than snapping
               back to the middle of the square -- walking into a wall should
               stop the hero at the wall, not a square short of it. */
            rtv_fx = (double) u.ux
                     + (double) (rtv_step_sqx - u.ux) * RTV_EDGE;
            rtv_fy = (double) u.uy
                     + (double) (rtv_step_sqy - u.uy) * RTV_EDGE;
            rtv_blocked_x = rtv_step_sqx;
            rtv_blocked_y = rtv_step_sqy;
            rtv_blocked_tick = now;
            rtv_step_pending = FALSE;
        }
    } else if ((coordxy) RT_ROUND(rtv_fx) != u.ux
               || (coordxy) RT_ROUND(rtv_fy) != u.uy) {
        /* the game moved the hero on its own (teleport, level change, being
           displaced); the prediction is meaningless now */
        rtv_fx = (double) u.ux;
        rtv_fy = (double) u.uy;
    }

    elapsed = now - rtv_free_tick;
    rtv_free_tick = now;
    if (elapsed > (unsigned long) RT_TURN_MS)
        elapsed = (unsigned long) RT_TURN_MS; /* a stall must not lurch */
    if (dx == 0.0 && dy == 0.0)
        return FALSE;

    /* a diagonal must not cover more ground than a straight line; input is
       eight-directional, so the only case to correct is both axes at once and
       the factor is known without needing a square root */
    len = (dx != 0.0 && dy != 0.0) ? 0.70710678 : 1.0;
    dx *= len;
    dy *= len;

    speed = (double) elapsed / (double) RT_TURN_MS; /* squares of travel */
    nx = rtv_fx + dx * speed;
    ny = rtv_fy + dy * speed;

    tx = (coordxy) RT_ROUND(nx);
    ty = (coordxy) RT_ROUND(ny);

    if (tx == u.ux && ty == u.uy) {
        rtv_fx = nx; /* still the same square: nothing to ask the game */
        rtv_fy = ny;
        return FALSE;
    }
    if (rtv_step_pending) {
        /* a step is already outstanding: hold position rather than run ahead
           of a move the game has not made yet.  Holding, not pulling back --
           the position is already legitimately past the boundary. */
        return FALSE;
    }
    if (!isok(tx, ty)
        || ((tx == rtv_blocked_x && ty == rtv_blocked_y)
            && now - rtv_blocked_tick < (unsigned long) RT_TURN_MS)) {
        /* the map ends here, or this square just refused us: rest against it.
           Retrying every frame would ask the game to walk into the same wall
           sixty times a second, and say so each time. */
        if (nx > (double) u.ux + RTV_EDGE)
            nx = (double) u.ux + RTV_EDGE;
        else if (nx < (double) u.ux - RTV_EDGE)
            nx = (double) u.ux - RTV_EDGE;
        if (ny > (double) u.uy + RTV_EDGE)
            ny = (double) u.uy + RTV_EDGE;
        else if (ny < (double) u.uy - RTV_EDGE)
            ny = (double) u.uy - RTV_EDGE;
        rtv_fx = nx;
        rtv_fy = ny;
        return FALSE;
    }

    /* crossing into a new square: report the step for the caller to deliver */
    for (i = 0; i < 8; i++) {
        if (xdir[i] == (schar) (tx - u.ux) && ydir[i] == (schar) (ty - u.uy)) {
            rtv_step_pending = TRUE;
            rtv_step_sqx = tx;
            rtv_step_sqy = ty;
            rtv_step_fromx = u.ux;
            rtv_step_fromy = u.uy;
            rtv_step_moves = svm.moves;
            rtv_fx = nx;
            rtv_fy = ny;
            *sx = (coordxy) (tx - u.ux);
            *sy = (coordxy) (ty - u.uy);
            return TRUE;
        }
    }
    return FALSE;
}

/* interface documented in nh_rtvector.h */
void
rtv_reset(void)
{
    int i;

    for (i = 0; i < RTV_SLOTS; i++)
        rtv_slots[i].id = 0;
    rtv_last_advance = 0;

    /* the hero's free position refers to the level being left, and any step
       asked for on it will never be answered */
    rtv_free_init = FALSE;
    rtv_free_tick = 0;
    rtv_step_pending = FALSE;
}

#endif /* REALTIME_PROTO */

/*rtvector.c*/
