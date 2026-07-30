/* NetHack 5.0	trap.h	$NHDT-Date: 1781973090 2026/06/20 16:31:30 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.38 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

/* note for 3.1.0 and later: no longer manipulated by 'makedefs' */

/**
 * @file trap.h
 * @brief A trap on the map, and the extra fact each kind needs.
 *
 * Traps differ in what they must remember. A trapdoor needs a destination level, a
 * teleport trap a destination square, a rolling boulder a second launch point, a
 * squeaky board its note, a pit whether it joins the one beside it. Rather than a
 * field per kind, one union holds whichever applies -- so the trap's type decides
 * which member is the live one.
 *
 * @note Whether the hero made the trap is recorded, and not merely for blame: a
 *       monster caught in a trap the hero set reacts to that, and untrapping it
 *       would otherwise be a free way to make it peaceful, as the comment inside
 *       explains.
 * @warning The union members are reached through macros that do not check the
 *          trap's type. Reading the wrong one yields a plausible number that means
 *          something else.
 */

/**
 * @file trap.h
 * @brief 지도 위의 함정과, 종류마다 필요한 추가 정보.
 *
 * 함정은 무엇을 기억해야 하는지가 서로 다르다. 뚜껑문은 목적지 레벨이, 순간이동 함정은 목적지
 * 칸이, 굴러오는 바위는 두 번째 발사 지점이, 삐걱이는 널판은 그 음이, 구덩이는 옆의 것과
 * 이어져 있는지가 필요하다. 종류마다 필드를 두는 대신 하나의 공용체가 해당하는 것을 담으므로,
 * 어느 멤버가 유효한지는 함정의 종류가 정한다.
 *
 * @note 영웅이 그 함정을 만들었는지가 기록되며, 단지 책임을 묻기 위한 것이 아니다. 영웅이 놓은
 *       함정에 걸린 몬스터는 그것에 반응하고, 그러지 않으면 함정을 풀어 주는 것이 손쉽게
 *       평화롭게 만드는 수단이 되어 버린다. 안쪽 주석이 설명하고 있다.
 * @warning 공용체 멤버들은 함정 종류를 검사하지 않는 매크로로 접근된다. 잘못된 것을 읽으면
 *          그럴듯하지만 다른 것을 뜻하는 숫자가 나온다.
 */

#ifndef TRAP_H
#define TRAP_H

/**
 * @brief The one extra fact a trap needs, whichever kind it is.
 * @warning Which member is meaningful depends entirely on the trap's type; nothing
 *          here records that, so the type must be consulted first.
 */
/**
 * @brief 함정이 필요로 하는 단 하나의 추가 정보. 종류가 무엇이든.
 * @warning 어느 멤버가 유효한지는 전적으로 함정의 종류에 달려 있다. 여기에는 그것을 기록하는
 *          것이 없으므로, 먼저 종류를 확인해야 한다.
 */
union vlaunchinfo {
    short v_launch_otyp; /* type of object to be triggered */
    coord v_launch2;     /* secondary launch point (for boulders) */
    uchar v_conjoined;   /* conjoined pit locations */
    short v_tnote;       /* boards: 12 notes        */
};

struct trap {
    struct trap *ntrap;
    coordxy tx, ty;
    d_level dst; /* destination for portals/holes/trapdoors */
    coord launch;
#define teledest launch /* x,y destination for teleport traps, if > 0 */
    Bitfield(ttyp, 5);
    Bitfield(tseen, 1);
    Bitfield(once, 1);
    Bitfield(madeby_u, 1); /* So monsters may take offence when you trap
                            * them.  Recognizing who made the trap isn't
                            * completely unreasonable; everybody has
                            * their own style.  This flag is also needed
                            * when you untrap a monster.  It would be too
                            * easy to make a monster peaceful if you could
                            * set a trap for it and then untrap it. */
    union vlaunchinfo vl;
#define launch_otyp vl.v_launch_otyp
#define launch2 vl.v_launch2
#define conjoined vl.v_conjoined
#define tnote vl.v_tnote
};

#define newtrap() (struct trap *) alloc(sizeof(struct trap))
#define dealloc_trap(trap) free((genericptr_t)(trap))

/* reasons for statue animation */
#define ANIMATE_NORMAL 0
#define ANIMATE_SHATTER 1
#define ANIMATE_SPELL 2

/* reasons for animate_statue's failure */
#define AS_OK 0            /* didn't fail */
#define AS_NO_MON 1        /* makemon failed */
#define AS_MON_IS_UNIQUE 2 /* statue monster is unique */

/* Note: if adding/removing a trap, adjust trap_engravings[] in mklev.c */

/* unconditional traps */
enum trap_types {
    ALL_TRAPS    = -1, /* mon_knows_traps(), mon_learns_traps() */
    NO_TRAP      =  0,
    ARROW_TRAP   =  1,
    DART_TRAP    =  2,
    ROCKTRAP     =  3,
    SQKY_BOARD   =  4,
    BEAR_TRAP    =  5,
    LANDMINE     =  6,
    ROLLING_BOULDER_TRAP = 7,
    SLP_GAS_TRAP =  8,
    RUST_TRAP    =  9,
    FIRE_TRAP    = 10,
    PIT          = 11,
    SPIKED_PIT   = 12,
    HOLE         = 13,
    TRAPDOOR     = 14,
    TELEP_TRAP   = 15,
    LEVEL_TELEP  = 16,
    MAGIC_PORTAL = 17,
    WEB          = 18,
    STATUE_TRAP  = 19,
    MAGIC_TRAP   = 20,
    ANTI_MAGIC   = 21,
    POLY_TRAP    = 22,
    VIBRATING_SQUARE = 23, /* not a trap but shown/remembered as if one
                            * once it has been discovered */

    /* trapped door and trapped chest aren't traps on the map, but they
       might be shown/remembered as such after trap detection until hero
       comes in view of them and sees the feature or object;
       key-using or door-busting monsters who survive a door trap learn
       to avoid other such doors [not implemented] */
    TRAPPED_DOOR = 24, /* part of door; not present on map as a trap */
    TRAPPED_CHEST = 25, /* part of object; not on map */

    TRAPNUM = 26
};

/* some trap-related function return results */
enum trap_result {
    Trap_Effect_Finished = 0,
    Trap_Is_Gone = 0,
    Trap_Caught_Mon = 1,
    Trap_Killed_Mon = 2,
    Trap_Moved_Mon = 3, /* new location, or new level */
};

/* return codes from immune_to_trap() */
enum trap_immunities {
    TRAP_NOT_IMMUNE = 0,
    TRAP_CLEARLY_IMMUNE = 1,
    TRAP_HIDDEN_IMMUNE = 2,
};


#define is_pit(ttyp) ((ttyp) == PIT || (ttyp) == SPIKED_PIT)
#define is_hole(ttyp)  ((ttyp) == HOLE || (ttyp) == TRAPDOOR)
#define unhideable_trap(ttyp) ((ttyp) == HOLE) /* visible traps */
#define undestroyable_trap(ttyp) ((ttyp) == MAGIC_PORTAL         \
                                  || (ttyp) == VIBRATING_SQUARE)
#define is_magical_trap(ttyp) ((ttyp) == TELEP_TRAP     \
                               || (ttyp) == LEVEL_TELEP \
                               || (ttyp) == MAGIC_TRAP  \
                               || (ttyp) == ANTI_MAGIC  \
                               || (ttyp) == POLY_TRAP)
/* "transportation" traps */
#define is_xport(ttyp) ((ttyp) >= TELEP_TRAP && (ttyp) <= MAGIC_PORTAL)
#define fixed_tele_trap(t) ((t)->ttyp == TELEP_TRAP \
                            && isok((t)->teledest.x,(t)->teledest.y))

#endif /* TRAP_H */
