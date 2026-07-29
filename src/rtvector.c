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

/* interface documented in nh_rtvector.h */
boolean
rtv_hero_offset(double *ox, double *oy)
{
    struct rtv_slot *s = rtv_find(RTV_HERO_ID);

    if (!s)
        return FALSE;
    *ox = s->x - (double) u.ux;
    *oy = s->y - (double) u.uy;
    return (*ox != 0.0 || *oy != 0.0);
}

/*
 * Hero drive.
 *
 * Everything above lets a position trail the square its entity was moved to.
 * This does the opposite: intent accumulates under real time and decides when
 * the next step happens, so the grid follows the continuous side rather than
 * leading it.  Kept separate from the slot table because it is about a step
 * not yet taken, whereas the slots describe positions already occupied.
 */
static double rtv_intent = 0.0;      /* progress toward the next square */
static int rtv_dirx = 0, rtv_diry = 0; /* direction that progress is for */
static unsigned long rtv_intent_tick = 0;

/* interface documented in nh_rtvector.h */
boolean
rtv_hero_drive(int dx, int dy, coordxy *sx, coordxy *sy)
{
    unsigned long now = nt_ticks(), elapsed;

    if (dx == 0 && dy == 0) {
        /* nothing held: discard partial progress so releasing a key can never
           produce a step the player did not ask for */
        rtv_intent = 0.0;
        rtv_dirx = rtv_diry = 0;
        rtv_intent_tick = 0;
        return FALSE;
    }
    if (dx != rtv_dirx || dy != rtv_diry) {
        /* a new direction starts fresh rather than inheriting progress made
           toward a different square */
        rtv_dirx = dx;
        rtv_diry = dy;
        rtv_intent = 0.0;
        rtv_intent_tick = now ? now : 1;
        return FALSE;
    }
    if (rtv_intent_tick == 0) {
        rtv_intent_tick = now ? now : 1;
        return FALSE;
    }

    elapsed = now - rtv_intent_tick;
    rtv_intent_tick = now;
    /* a stall means the window was not being drawn; do not bank it up into a
       burst of steps once drawing resumes */
    if (elapsed > (unsigned long) RT_TURN_MS)
        elapsed = (unsigned long) RT_TURN_MS;
    rtv_intent += (double) elapsed / (double) RT_TURN_MS;

    if (rtv_intent < 1.0)
        return FALSE;

    rtv_intent -= 1.0;
    if (rtv_intent > 1.0)
        rtv_intent = 1.0; /* never owe more than one step */
    *sx = (coordxy) dx;
    *sy = (coordxy) dy;
    return TRUE;
}

/* interface documented in nh_rtvector.h */
void
rtv_reset(void)
{
    int i;

    for (i = 0; i < RTV_SLOTS; i++)
        rtv_slots[i].id = 0;
    rtv_last_advance = 0;
    rtv_intent = 0.0;
    rtv_dirx = rtv_diry = 0;
    rtv_intent_tick = 0;
}

#endif /* REALTIME_PROTO */

/*rtvector.c*/
