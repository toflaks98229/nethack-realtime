/* NetHack 5.0  botl.h  $NHDT-Date: 1781973077 2026/06/20 16:31:17 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.45 $ */
/* Copyright (c) Michael Allison, 2003                            */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file botl.h
 * @brief The status fields, and how an interface is told about them.
 *
 * Two arrangements exist at once. The older one formats the status into text
 * lines itself; the newer hands each field to the interface separately, so a
 * graphical port can lay them out however it likes. Both are supported, which is
 * why this header describes fields as well as line widths.
 *
 * Fields are identified by index rather than by name at the boundary, so adding
 * one is a change both sides must agree on.
 *
 * @note The comment below records why the buffer is sized as it is, working from
 *       the longest status line the game can actually produce -- it is a
 *       calculation, not a guess.
 * @warning When the line will not fit, less important fields are moved to the
 *          end so that truncation removes the least significant information.
 *          Field order is therefore a display decision, not an arbitrary one.
 */

/**
 * @file botl.h
 * @brief 상태 항목들과, 인터페이스에 그것을 알리는 방식.
 *
 * 두 가지 방식이 동시에 존재한다. 예전 방식은 상태를 스스로 글줄로 조립하고, 새 방식은
 * 항목마다 따로 인터페이스에 건네므로 그래픽 포팅이 원하는 대로 배치할 수 있다. 둘 다
 * 지원되며, 그래서 이 헤더가 줄 너비와 함께 항목까지 기술한다.
 *
 * 경계에서 항목은 이름이 아니라 색인으로 식별된다. 그래서 항목을 추가하는 일은 양쪽이
 * 함께 합의해야 하는 변경이다.
 *
 * @note 아래 주석은 버퍼 크기를 그렇게 잡은 이유를 기록한다. 게임이 실제로 만들어 낼 수
 *       있는 가장 긴 상태 줄에서 역산한 것이며, 어림짐작이 아니라 계산이다.
 * @warning 줄이 들어가지 않을 때는 덜 중요한 항목이 뒤로 밀린다. 잘라 낼 때 가장 덜
 *          중요한 정보가 사라지게 하기 위함이다. 따라서 항목 순서는 임의가 아니라 표시에
 *          관한 결정이다.
 */

#ifndef BOTL_H
#define BOTL_H

/* Note: this comment is about the pre-VIA_WINDOWPORT two line status
 * which is still available but has not added a bunch of conditional
 * extra status conditions (Grab, InLava, Held, Zzz and many others)
 * or the new fields Weapon, Armor, and Terrain.
 *
 * MAXCO must hold longest uncompressed status line, and must be larger
 * than COLNO
 *
 * longest practical second status line at the moment is
Astral Plane \GXXXXNNNN:123456 HP:1234(1234) Pw:1234(1234) AC:-127
 Xp:30/123456789 T:123456  Stone Slime Strngl FoodPois TermIll
 Satiated Overloaded Blind Deaf Stun Conf Hallu Lev Ride
 * -- or about 185 characters.  '$' gets encoded even when it
 * could be used as-is.  The first five status conditions are fatal
 * so it's rare to have more than one at a time.
 *
 * When the full line is wider than the map, the basic status line
 * formatting will move less important fields to the end, so if/when
 * truncation is necessary, it will chop off the least significant
 * information.
 */
#if COLNO <= 160
#define MAXCO 200
#else
#define MAXCO (COLNO + 40)
#endif

/* limit of the player's name in the status window */
#define BOTL_NSIZ 16

struct condmap {
    const char *id;
    unsigned long bitmask;
};

enum statusfields {
    BL_CHARACTERISTICS = -3, /* alias for BL_STR..BL_CH */
    BL_RESET = -2,           /* Force everything to redisplay */
    BL_FLUSH = -1,           /* Finished cycling through bot fields */
    /*
     * Note: status_sanity_check() in wintty.c has strings for the rest
     * of these, so if any get renumbered or more get added, be sure to
     * keep those in sync.
     */
    BL_TITLE = 0,
    BL_STR, BL_DX, BL_CO, BL_IN, BL_WI, BL_CH,  /*  1.. 6 */
    BL_ALIGN, BL_SCORE, BL_CAP, BL_GOLD,        /*  7..10 */
    BL_ENE, BL_ENEMAX, BL_XP, BL_AC, BL_HD,     /* 11..15 */
    BL_TIME, BL_HUNGER, BL_HP, BL_HPMAX,        /* 16..19 */
    BL_LEVELDESC, BL_EXP, BL_CONDITION,         /* 20..22 */
    BL_WEAPON, BL_ARMOR, BL_TERRAIN,            /* 23..25 */
    BL_VERS,                                    /*   26   */
    MAXBLSTATS /* [27] */
};

enum relationships {
    NO_LTEQGT = -1,
    EQ_VALUE, LT_VALUE, LE_VALUE,
    GE_VALUE, GT_VALUE, TXT_VALUE
};

enum blconditions {
    bl_bareh, /* deprecated -- bl_weapon encompasses this */
    bl_blind,
    bl_busy,
    bl_conf,
    bl_deaf,
    bl_elf_iron,
    bl_fly,
    bl_foodpois,
    bl_glowhands,
    bl_grab,
    bl_hallu,
    bl_held,
    bl_icy, /* bl_terrain encompasses this */
    bl_inlava,
    bl_lev,
    bl_parlyz,
    bl_ride,
    bl_sleeping,
    bl_slime,
    bl_slippery,
    bl_stone,
    bl_strngl,
    bl_stun,
    bl_submerged, /* bl_terrain encompasses this */
    bl_termill,
    bl_tethered,
    bl_trapped,
    bl_unconsc,
    bl_woundedl,
    bl_holding,

    CONDITION_COUNT
};

/* Boolean condition bits for the condition mask */

/* clang-format off */
#define BL_MASK_BAREH        0x00000001L
#define BL_MASK_BLIND        0x00000002L
#define BL_MASK_BUSY         0x00000004L
#define BL_MASK_CONF         0x00000008L
#define BL_MASK_DEAF         0x00000010L
#define BL_MASK_ELF_IRON     0x00000020L
#define BL_MASK_FLY          0x00000040L
#define BL_MASK_FOODPOIS     0x00000080L
#define BL_MASK_GLOWHANDS    0x00000100L
#define BL_MASK_GRAB         0x00000200L
#define BL_MASK_HALLU        0x00000400L
#define BL_MASK_HELD         0x00000800L
#define BL_MASK_ICY          0x00001000L
#define BL_MASK_INLAVA       0x00002000L
#define BL_MASK_LEV          0x00004000L
#define BL_MASK_PARLYZ       0x00008000L
#define BL_MASK_RIDE         0x00010000L
#define BL_MASK_SLEEPING     0x00020000L
#define BL_MASK_SLIME        0x00040000L
#define BL_MASK_SLIPPERY     0x00080000L
#define BL_MASK_STONE        0x00100000L
#define BL_MASK_STRNGL       0x00200000L
#define BL_MASK_STUN         0x00400000L
#define BL_MASK_SUBMERGED    0x00800000L
#define BL_MASK_TERMILL      0x01000000L
#define BL_MASK_TETHERED     0x02000000L
#define BL_MASK_TRAPPED      0x04000000L
#define BL_MASK_UNCONSC      0x08000000L
#define BL_MASK_WOUNDEDL     0x10000000L
#define BL_MASK_HOLDING      0x20000000L
#define BL_MASK_BITS            30 /* number of mask bits that can be set */
/* clang-format on */

struct conditions_t {
    int ranking;
    long mask;
    enum blconditions c;
    const char *text[3];
};
extern const struct conditions_t conditions[CONDITION_COUNT];

struct condtests_t {
    enum blconditions c;
    const char *useroption;
    enum optchoice opt;
    boolean enabled;
    boolean choice;
    boolean test;
};

extern struct condtests_t condtests[CONDITION_COUNT];
extern int cond_idx[CONDITION_COUNT];

#define BEFORE  0
#define NOW     1

/*
 * Possible additional conditions:
 *  major:
 *      grab   - grabbed by eel so about to be drowned ("wrapd"? damage type
 *               is AD_WRAP but message is "<mon> swings itself around you")
 *      digst  - swallowed and being digested
 *      lava   - trapped sinking into lava
 *  in_between: (potentially severe but don't necessarily lead to death;
 *               explains to player why he isn't getting to take any turns)
 *      unconc - unconscious
 *      parlyz - (multi < 0 && (!strncmp(multi_reason, "paralyzed", 9)
 *                              || !strncmp(multi_reason, "frozen", 6)))
 *      asleep - (multi < 0 && !strncmp(multi_reason, "sleeping", 8))
 *      busy   - other multi < 0
 *  minor:
 *      held   - grabbed by non-eel or by eel but not susceptible to drowning
 *      englf  - engulfed or swallowed but not being digested (usually
 *               obvious but the blank symbol set makes that uncertain)
 *      vomit  - vomiting (causes confusion and stun late in countdown)
 *      trap   - trapped in pit, bear trap, web, or floor (solidified lava)
 *      teth   - tethered to buried iron ball
 *      chain  - punished
 *      slip   - slippery fingers
 *      ice    - standing on ice (movement becomes uncertain)
 *     [underwater - movement uncertain, vision truncated, equipment at risk]
 *  other:
 *     [hold      - poly'd into grabber and holding adjacent monster]
 *      Stormbringer - wielded weapon poses risks
 *      Cleaver   - wielded weapon risks unintended consequences
 *      barehand  - not wielding any weapon nor wearing gloves
 *      no-weapon - not wielding any weapon
 *      bow/xbow/sling - wielding a missile launcher of specified type
 *      pole      - wielding a polearm
 *      pick      - wielding a pickaxe
 *      junk      - wielding non-weapon, non-weptool
 *      naked     - no armor
 *      no-gloves - self-explanatory
 *      no-cloak  - ditto
 *     [no-{other armor slots?} - probably much too verbose]
 *  conduct?
 *      [maybe if third status line is added]
 *
 *  Can't add all of these and probably don't want to.  But maybe we
 *  can add some of them and it's not as many as first appears:
 *  lava/trap/teth are mutually exclusive;
 *  digst/grab/englf/held/hold are also mutually exclusive;
 *  Stormbringer/Cleaver/barehand/no-weapon/bow&c/pole/pick/junk too;
 *  naked/no-{any armor slot} likewise.
 */

#define VIA_WINDOWPORT() \
    ((windowprocs.wincap2 & (WC2_HILITE_STATUS | WC2_FLUSH_STATUS)) != 0)

#define REASSESS_ONLY TRUE

/* #ifdef STATUS_HILITES */
/* hilite status field behavior - coloridx values */
#define BL_HILITE_NONE    -1    /* no hilite of this field */

#if 0
#define BL_HILITE_BOLD    -2    /* bold hilite */
#define BL_HILITE_DIM     -3    /* dim hilite */
#define BL_HILITE_ITALIC  -4    /* italic hilite */
#define BL_HILITE_ULINE   -5    /* underline hilite */
#define BL_HILITE_BLINK   -6    /* blink hilite */
#define BL_HILITE_INVERSE -7    /* inverse hilite */
                                /* or any CLR_ index (0 - 15) */
#endif

#define BL_TH_NONE 0
#define BL_TH_VAL_PERCENTAGE 100 /* threshold is percentage */
#define BL_TH_VAL_ABSOLUTE 101   /* threshold is particular value */
#define BL_TH_UPDOWN 102         /* threshold is up or down change */
#define BL_TH_CONDITION 103      /* threshold is bitmask of conditions */
#define BL_TH_TEXTMATCH 104      /* threshold text value to match against */
#define BL_TH_ALWAYS_HILITE 105  /* highlight regardless of value */
#define BL_TH_CRITICALHP 106     /* highlight critically low HP */

#define HL_ATTCLR_NONE    CLR_MAX + 1
#define HL_ATTCLR_BOLD    CLR_MAX + 2
#define HL_ATTCLR_DIM     CLR_MAX + 3
#define HL_ATTCLR_ITALIC  CLR_MAX + 4
#define HL_ATTCLR_ULINE   CLR_MAX + 5
#define HL_ATTCLR_BLINK   CLR_MAX + 6
#define HL_ATTCLR_INVERSE CLR_MAX + 7
#define BL_ATTCLR_MAX     CLR_MAX + 8

enum hlattribs {
    HL_UNDEF   = 0x00,
    HL_NONE    = 0x01,
    HL_BOLD    = 0x02,
    HL_DIM     = 0x04,
    HL_ITALIC  = 0x08,
    HL_ULINE   = 0x10,
    HL_BLINK   = 0x20,
    HL_INVERSE = 0x40
};

#define MAXVALWIDTH 80 /* actually less, but was using 80 to allocate title
                        * and leveldesc then using QBUFSZ everywhere else   */
#ifdef STATUS_HILITES
struct hilite_s {
    enum statusfields fld;
    boolean set;
    enum any_types anytype;
    anything value;
    int behavior;
    char textmatch[MAXVALWIDTH];
    enum relationships rel;
    int coloridx;
    struct hilite_s *next;
};
#endif

/*
 * Note: If you add/change/remove fields in istat_s, you need to
 * update the initialization of the istat_s struct blstats[][]
 * array in instance_globals_b (decl.c).
 */
struct istat_s {
    const char *fldname;
    const char *fldfmt;
    long time;  /* moves when this field hilite times out */
    boolean chg; /* need to recalc time? */
    boolean percent_matters;
    short percent_value;
    enum any_types anytype;
    anything a, rawval;
    char *val;
    int valwidth;
    enum statusfields idxmax;
    enum statusfields fld;
#ifdef STATUS_HILITES
    struct hilite_s *hilite_rule; /* the entry, if any, in 'thresholds'
                                   * list that currently applies        */
    struct hilite_s *thresholds;
#endif
};

extern const char *status_fieldnames[]; /* in botl.c */

#endif /* BOTL_H */
