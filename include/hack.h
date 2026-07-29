/* NetHack 5.0	hack.h	$NHDT-Date: 1781973080 2026/06/20 16:31:20 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.299 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef HACK_H
#define HACK_H

#ifndef CONFIG_H
#include "config.h"
#endif
#include "lint.h"

#include "align.h"
#include "weight.h"
#include "dungeon.h"
#include "stairs.h"
#include "objclass.h"
#include "wintype.h"
#include "flag.h"
#include "rect.h"
#include "sym.h"
#include "trap.h"
#include "youprop.h"
#include "display.h"

#include "botl.h"
#include "context.h"
#include "engrave.h"
#include "mkroom.h"
#include "obj.h"
#include "quest.h"
#include "region.h"
#include "rm.h"
#include "selvar.h"
#include "sndprocs.h"
#include "spell.h"
#include "sys.h"
#include "timeout.h"
#include "winprocs.h"
#include "vision.h"
#include "you.h"

#define TELL 1
#define NOTELL 0
#define ON 1
#define OFF 0
#define BOLT_LIM 8        /* from this distance ranged attacks will be made */
#define DUMMY { 0 }       /* array initializer, letting [1..N-1] default */
#define DEF_NOTHING ' '   /* default symbol for NOTHING and UNEXPLORED  */

/* Macros for how a rumor was delivered in outrumor() */
#define BY_ORACLE 0
#define BY_COOKIE 1
#define BY_PAPER 2
#define BY_OTHER 9

/* bitmask flags for corpse_xname();
   PFX_THE takes precedence over ARTICLE, NO_PFX takes precedence over both */
#define CXN_NORMAL 0    /* no special handling */
#define CXN_SINGULAR 1  /* override quantity if greater than 1 */
#define CXN_NO_PFX 2    /* suppress "the" from "the Unique Monst */
#define CXN_PFX_THE 4   /* prefix with "the " (unless pname) */
#define CXN_ARTICLE 8   /* include a/an/the prefix */
#define CXN_NOCORPSE 16 /* suppress " corpse" suffix */
#define CXN_ADDGNDR 32  /* include a gender */

/* number of turns it takes for vault guard to show up */
#define VAULT_GUARD_TIME 30

/* shopkeeper billing: selling states, damage prices, devaluation, repossession
   [gathered from four places in this file; see nh_shop.h] */
#include "nh_shop.h"

/* flags for look_here() */
#define LOOKHERE_NOFLAGS       0U
#define LOOKHERE_PICKED_SOME   1U
#define LOOKHERE_SKIP_DFEATURE 2U

/* max size of a windowtype option */
#define WINTYPELEN 16

/* str_or_len from sp_lev.h */
typedef union str_or_len {
    char *str;
    int len;
} Str_or_Len;

enum artifacts_nums {
#define ARTI_ENUM
#include "artilist.h"
#undef ARTI_ENUM
    AFTER_LAST_ARTIFACT
};

enum misc_arti_nums {
    NROFARTIFACTS = (AFTER_LAST_ARTIFACT - 1)
};

/* related to breadcrumb struct */
enum bcargs {override_restriction = -1};

struct breadcrumbs {
    const char *funcnm;
    int linenum;
    boolean in_effect;
};

/* types of calls to bhit() */
enum bhit_call_types {
    ZAPPED_WAND   = 0,
    THROWN_WEAPON = 1,
    THROWN_TETHERED_WEAPON = 2,
    KICKED_WEAPON = 3,
    FLASHED_LIGHT = 4,
    INVIS_BEAM    = 5
};

/* Macros for messages referring to hands, eyes, feet, etc... */
enum bodypart_types {
    NO_PART   = -1,
    ARM       =  0,
    EYE       =  1,
    FACE      =  2,
    FINGER    =  3,
    FINGERTIP =  4,
    FOOT      =  5,
    HAND      =  6,
    HANDED    =  7,
    HEAD      =  8,
    LEG       =  9,
    LIGHT_HEADED = 10,
    NECK      = 11,
    SPINE     = 12,
    TOE       = 13,
    HAIR      = 14,
    BLOOD     = 15,
    LUNG      = 16,
    NOSE      = 17,
    STOMACH   = 18
};

/* drifting bubbles and their contents on the water/air levels
   [struct container gathered here too; see nh_bubble.h] */
#include "nh_bubble.h"

/* command queue, special keys, and the command dispatch table */
#include "nh_cmd.h"

struct c_color_names {
    const char *const c_black, *const c_amber, *const c_golden,
        *const c_light_blue, *const c_red, *const c_green, *const c_silver,
        *const c_blue, *const c_purple, *const c_white, *const c_orange;
};

struct c_common_strings {
    const char *const c_nothing_happens, *const c_nothing_seems_to_happen,
        *const c_thats_enough_tries, *const c_silly_thing_to,
        *const c_shudder_for_moment, *const c_something, *const c_Something,
        *const c_You_can_move_again, *const c_Never_mind,
        *const c_vision_clears, *const c_the_your[2], *const c_fakename[2];
};

/* [struct container moved to nh_bubble.h] */

/* [cost_alteration_types and unpaid_cost_flags moved to nh_shop.h] */

/* read.c, create_particular() & create_particular_parse() */
struct _create_particular_data {
    int quan;
    int which;
    int fem;        /* -1, MALE, FEMALE, NEUTRAL */
    int genderconf;    /* conflicting gender */
    char monclass;
    boolean randmonst;
    boolean maketame, makepeaceful, makehostile;
    boolean sleeping, saddled, invisible, hidden;
};

/* dig_check() results */

enum digcheck_result {
    DIGCHECK_PASSED                 = 1,
    DIGCHECK_PASSED_DESTROY_TRAP    = 2,
    DIGCHECK_PASSED_PITONLY         = 3,
    DIGCHECK_FAILED                 = 4,
    DIGCHECK_FAIL_ONSTAIRS          = DIGCHECK_FAILED,
    DIGCHECK_FAIL_ONLADDER,
    DIGCHECK_FAIL_THRONE,
    DIGCHECK_FAIL_ALTAR,
    DIGCHECK_FAIL_AIRLEVEL,
    DIGCHECK_FAIL_WATERLEVEL,
    DIGCHECK_FAIL_TOOHARD,
    DIGCHECK_FAIL_UNDESTROYABLETRAP,
    DIGCHECK_FAIL_CANTDIG,
    DIGCHECK_FAIL_BOULDER,
    DIGCHECK_FAIL_OBJ_POOL_OR_TRAP
};


/* Dismount: causes for why you are no longer riding */
enum dismount_types {
    DISMOUNT_GENERIC  = 0,
    DISMOUNT_FELL     = 1,
    DISMOUNT_THROWN   = 2,
    DISMOUNT_KNOCKED  = 3, /* hero hit for knockback effect */
    DISMOUNT_POLY     = 4,
    DISMOUNT_ENGULFED = 5,
    DISMOUNT_BONES    = 6,
    DISMOUNT_BYCHOICE = 7
};

/* special-level locations and the short names used to reach them */
#include "nh_dgntopo.h"

enum lua_theme_group {
    all_themes = 1,  /* for end of game */
    most_themes = 2, /* for entering endgame */
    tut_themes = 3,  /* for leaving tutorial */
};

enum earlyarg {
    ARG_DEBUG, ARG_VERSION, ARG_SHOWPATHS
#ifndef NODUMPENUMS
    , ARG_DUMPENUMS
#endif
    , ARG_DUMPGLYPHIDS
    , ARG_DUMPMONGEN
    , ARG_DUMPWEIGHTS
#ifdef WIN32
    , ARG_WINDOWS
#endif
#if defined(CRASHREPORT)
    , ARG_BIDSHOW
#endif
};

struct early_opt {
    enum earlyarg e;
    const char *name;
    int minlength;
    boolean valallowed;
};

/* symbolic names for capacity levels */
enum encumbrance_types {
    UNENCUMBERED = 0,
    SLT_ENCUMBER = 1, /* Burdened */
    MOD_ENCUMBER = 2, /* Stressed */
    HVY_ENCUMBER = 3, /* Strained */
    EXT_ENCUMBER = 4, /* Overtaxed */
    OVERLOADED   = 5  /* Overloaded */
};

struct entity {
    struct monst *emon;     /* youmonst for the player */
    struct permonst *edata; /* must be non-zero for record to be valid */
    int ex, ey;
};

struct enum_dump {
    int val;
    const char *nm;
};

/*
 * This is the way the game ends.  If these are rearranged, the arrays
 * in end.c and topten.c will need to be changed.  Some parts of the
 * code assume that PANICKED separates the deaths from the non-deaths.
 */
enum game_end_types {
    DIED         =  0,
    CHOKING      =  1,
    POISONING    =  2,
    STARVING     =  3,
    DROWNING     =  4,
    BURNING      =  5,
    DISSOLVED    =  6,
    CRUSHING     =  7,
    STONING      =  8,
    TURNED_SLIME =  9,
    GENOCIDED    = 10,
    PANICKED     = 11,
    TRICKED      = 12,
    QUIT         = 13,
    ESCAPED      = 14,
    ASCENDED     = 15
};

/* game events log */
struct gamelog_line {
    long turn; /* turn when this happened */
    long flags; /* LL_foo flags */
    char *text;
    struct gamelog_line *next;
};


/* values returned from getobj() callback functions */
enum getobj_callback_returns {
    /* generally invalid - can't be used for this purpose. will give a "silly
     * thing" message if the player tries to pick it, unless a more specific
     * failure message is in getobj itself - e.g. "You cannot foo gold". */
    GETOBJ_EXCLUDE = -3,
    /* invalid because it is not in inventory; used when the hands/self
     * possibility is queried and the player passed up something on the
     * floor before getobj. */
    GETOBJ_EXCLUDE_NONINVENT = -2,
    /* invalid because it is an inaccessible or unwanted piece of gear, but
     * pseudo-valid for the purposes of allowing the player to select it and
     * getobj to return it if there is a prompt instead of getting "silly
     * thing", in order for the getobj caller to present a specific failure
     * message. Other than that, the only thing this does differently from
     * GETOBJ_EXCLUDE is that it inserts an "else" in "You don't have anything
     * else to foo". */
    GETOBJ_EXCLUDE_INACCESS = -1,
    /* invalid for purposes of not showing a prompt if nothing is valid but
     * pseudo-valid for selecting - identical to GETOBJ_EXCLUDE_INACCESS but
     * without the "else" in "You don't have anything else to foo". */
    GETOBJ_EXCLUDE_SELECTABLE = 0,
    /* valid - invlet not presented in the summary or the ? menu as a
     * recommendation, but is selectable if the player enters it anyway.
     * Used for objects that are actually valid but unimportantly so, such
     * as shirts for reading. */
    GETOBJ_DOWNPLAY = 1,
    /* valid - will be shown in summary and ? menu */
    GETOBJ_SUGGEST  = 2,
};

/* getpos() return values */
enum getpos_retval {
    LOOK_TRADITIONAL = 0, /* '.' -- ask about "more info?" */
    LOOK_QUICK       = 1, /* ',' -- skip "more info?" */
    LOOK_ONCE        = 2, /* ';' -- skip and stop looping */
    LOOK_VERBOSE     = 3  /* ':' -- show more info w/o asking */
};

struct h2o_ctx {
    int dkn_boom, unk_boom; /* track dknown, !dknown separately */
    boolean ctx_valid;
};

/* attack mode for hmon() */
enum hmon_atkmode_types {
    HMON_MELEE   = 0, /* hand-to-hand */
    HMON_THROWN  = 1, /* normal ranged (or spitting while poly'd) */
    HMON_KICKED  = 2, /* alternate ranged */
    HMON_APPLIED = 3, /* polearm, treated as ranged */
    HMON_DRAGGED = 4  /* attached iron ball, pulled into mon */
};

/* hunger states - see hu_stat in eat.c */
enum hunger_state_types {
    SATIATED   = 0,
    NOT_HUNGRY = 1,
    HUNGRY     = 2,
    WEAK       = 3,
    FAINTING   = 4,
    FAINTED    = 5,
    STARVED    = 6
};

/* fake inventory letters, not 'a'..'z' or 'A'..'Z' */
#define NOINVSYM '#'      /* overflow because all 52 letters are in use */
#define CONTAINED_SYM '>' /* designator for inside a container */
#define HANDS_SYM '-'     /* hands|fingers|self depending on context */

/* inventory counts (slots in tty parlance)
 * a...zA..Z    invlet_basic (52)
 * $a...zA..Z#  2 special additions
 */
enum inventory_counts {
    invlet_basic = 52,
    invlet_gold = 1,
    invlet_overflow = 1,
    invlet_max = invlet_basic + invlet_gold + invlet_overflow,
    /* 2023/11/30 invlet_max is not yet used anywhere */
};

#ifndef IDLECHECKPOINT_WAIT_TIME
#define IDLECHECKPOINT_WAIT_TIME 10  /* seconds to wait before executing a checkpoint;
                                      * always #define'd but only has meaning if
                                      * IDLECHECKPOINT is defined.
                                      */
#endif

struct kinfo {
    struct kinfo *next; /* chain of delayed killers */
    int id;             /* uprop keys to ID a delayed killer */
    int format;         /* one of the killer formats */
#define KILLED_BY_AN 0
#define KILLED_BY 1
#define NO_KILLER_PREFIX 2
    char name[BUFSZ]; /* actual killer name */
};

struct launchplace {
    struct obj *obj;
    coordxy x, y;
};

/* light source */
typedef struct ls_t {
    struct ls_t *next;
    coordxy x, y;  /* source's position */
    short range; /* source's current range */
    short flags;
    short type;  /* type of light source */
    anything id; /* source's identifier */
} light_source;

struct menucoloring {
    struct nhregex *match;
    char *origstr;
    int color, attr;
    struct menucoloring *next;
};

/* directions, movement styles, and move-attempt outcomes
   [test_move and m_move values gathered here too; see nh_move.h] */
#include "nh_move.h"

struct multishot {
    int n, i;
    short o;
    boolean s;
};

struct musable {
    struct obj *offensive;
    struct obj *defensive;
    struct obj *misc;
    int has_offense, has_defense, has_misc;
    /* =0, no capability; otherwise, different numbers.
     * If it's an object, the object is also set (it's 0 otherwise).
     */
};

struct mvitals {
    uchar born;
    uchar died;
    uchar mvflags;
    Bitfield(seen_close, 1);
    Bitfield(photographed, 1);
};


/* Lua callback functions */
enum nhcore_calls {
    NHCORE_START_NEW_GAME = 0,
    NHCORE_RESTORE_OLD_GAME,
    NHCORE_MOVELOOP_TURN,
    NHCORE_GAME_EXIT,
    NHCORE_GETPOS_TIP,
    NHCORE_ENTER_TUTORIAL,
    NHCORE_LEAVE_TUTORIAL,

    NUM_NHCORE_CALLS
};

/* Lua callbacks. TODO: Merge with NHCORE */
enum nhcb_calls {
    NHCB_CMD_BEFORE = 0,
    NHCB_LVL_ENTER,
    NHCB_LVL_LEAVE,
    NHCB_END_TURN,

    NUM_NHCB
};

#define NHUUIDSZ 37

/* message classification, single-argument wrappers, yes/no queries, and
   custompline() flags [gathered from four places; see nh_msg.h] */
#include "nh_msg.h"

/* polyself flags */
enum polyself_flags {
    POLY_NOFLAGS    = 0x00,
    POLY_CONTROLLED = 0x01,
    POLY_MONSTER    = 0x02,
    POLY_REVERT     = 0x04,
    POLY_LOW_CTRL   = 0x08
};

/* [struct repo moved to nh_shop.h] */

struct restore_info {
    const char *name;
    int mread_flags;
};

enum restore_stages {
    REST_GSTATE = 1, /* restoring game state + first pass of current level */
    REST_LEVELS = 2, /* restoring remainder of dungeon */
    REST_CURRENT_LEVEL = 3, /* final pass of restoring current level */
};

struct rogueroom {
    coordxy rlx, rly;
    coordxy dx, dy;
    boolean real;
    uchar doortable;
    int nroom; /* Only meaningful for "real" rooms */
};

#define NUM_ROLES (13)
struct role_filter {
    boolean roles[NUM_ROLES + 1];
    short mask;
};
#define NUM_RACES (5)

struct selectionvar {
    int wid, hei;
    boolean bounds_dirty;
    NhRect bounds; /* use selection_getbounds() */
    char *map;
};

/* program_state / level_status phases and input-state enum */
#include "nh_progstate.h"

/* sortloot() return type; needed before extern.h */
struct sortloot_item {
    struct obj *obj;
    char *str; /* result of loot_xname(obj) in some cases, otherwise null */
    /* these need to be signed; 'indx' should be big enough to hold a count
       of the largest pile of items, the others fit within char */
    int indx;        /* index into original list (used as tie-breaker) */
    int8 orderclass; /* order rather than object class; 0 => not yet init'd */
    int8 subclass;   /* subclass for some classes */
    int8 disco;      /* discovery status */
    int8 inuse;      /* 0: not in-use or not sorting by inuse_only;
                      * 1: lit candle/lamp or attached leash; 2: worn armor;
                      * 3: wielded weapon (including uswapwep and uquiver);
                      * 4: worn accessory (amulet, rings, blindfold). */
};
typedef struct sortloot_item Loot;

typedef struct strbuf {
    int    len;
    char  *str;
    char   buf[256];
} strbuf_t;

enum stoning_checks {
    st_gloves    = 0x1,  /* wearing gloves? */
    st_corpse    = 0x2,  /* is it a corpse obj? */
    st_petrifies = 0x4,  /* does the corpse petrify on touch? */
    st_resists   = 0x8,  /* do you have stoning resistance? */
    st_all = (st_gloves | st_corpse | st_petrifies | st_resists)
};

struct throw_and_return_weapon {
    short otyp;
    int range;
    Bitfield(tethered, 1);
};

struct trapinfo {
    struct obj *tobj;
    coordxy tx, ty;
    int time_needed;
    boolean force_bungle;
};

/* values for rtype are defined in dungeon.h */
/* lev_region from sp_lev.h */
typedef struct {
    struct {
        coordxy x1, y1, x2, y2;
    } inarea;
    struct {
        coordxy x1, y1, x2, y2;
    } delarea;
    boolean in_islev, del_islev;
    coordxy rtype, padding;
    Str_or_Len rname;
} lev_region;

/* savefile compatibility flags, NHFILE handle, and serializer mode bits
   [uptodate/status flags gathered here with the handle; see nh_savefile.h] */
#include "nh_savefile.h"

#define ENTITIES 2
struct valuable_data {
    long count;
    int typ;
};

struct val_list {
    struct valuable_data *list;
    int size;
};

enum vanq_order_modes {
    VANQ_MLVL_MNDX = 0, /* t - traditional: by monster level */
    VANQ_MSTR_MNDX,     /* d - by difficulty rating */
    VANQ_ALPHA_SEP,     /* a - alphabetical, first uniques, then ordinary */
    VANQ_ALPHA_MIX,     /* A - alpha with uniques and ordinary intermixed */
    VANQ_MCLS_HTOL,     /* C - by class, high to low within class */
    VANQ_MCLS_LTOH,     /* c - by class, low to high within class */
    VANQ_COUNT_H_L,     /* n - by count, high to low */
    VANQ_COUNT_L_H,     /* z - by count, low to high */

    NUM_VANQ_ORDER_MODES
};

struct autopickup_exception {
    struct nhregex *regex;
    char *pattern;
    boolean grab;
    struct autopickup_exception *next;
};

/* at most one of `door' and `box' should be non-null at any given time */
struct xlock_s {
    struct rm *door;
    struct obj *box;
    int picktyp, /* key|pick|card for unlock, sharp vs blunt for #force */
        chance, usedtime;
    boolean magic_key;
};

#define MAX_BMASK 4

/* [NHFILE handle and mode bits now included earlier, with the savefile
   compatibility flags; see nh_savefile.h] */

/* articles and suppress masks used when naming a monster */
#include "nh_monnam.h"

/* [single-argument pline wrappers moved to nh_msg.h] */

/* directory classes used to locate data and state files */
#include "nh_fileprefix.h"

/* from options.c */
#define MAX_MENU_MAPPED_CMDS 32 /* some number */

/* player selection constants */
#define BP_ALIGN 0
#define BP_GEND 1
#define BP_RACE 2
#define BP_ROLE 3
#define NUM_BP 4

/* some array sizes for 'g?' */
#define WIZKIT_MAX 128
#define CVT_BUF_SIZE 64

#define LUA_VER_BUFSIZ 20
#define LUA_COPYRIGHT_BUFSIZ 120

/* Symbol offsets */
#define SYM_OFF_P (0)
#define SYM_OFF_O (SYM_OFF_P + MAXPCHARS)   /* MAXPCHARS from sym.h */
#define SYM_OFF_M (SYM_OFF_O + MAXOCLASSES) /* MAXOCLASSES from objclass.h */
#define SYM_OFF_W (SYM_OFF_M + MAXMCLASSES) /* MAXMCLASSES from sym.h*/
#define SYM_OFF_X (SYM_OFF_W + WARNCOUNT)
#define SYM_MAX (SYM_OFF_X + MAXOTHER)

/* The UNDEFINED macros are used to initialize variables whose
   initialized value is not relied upon.
   UNDEFINED_VALUE: used to initialize any scalar type except pointers.
   UNDEFINED_VALUES: used to initialize any non scalar type without pointers.
   UNDEFINED_PTR: can be used only on pointer types. */
#define UNDEFINED_VALUE 0
#define UNDEFINED_VALUES { 0 }
#define UNDEFINED_PTR NULL

/* The UNDEFINED_ROLE macro is used to initialize Role variables */
#define UNDEFINED_ROLE \
    {                                           \
      /* role name, set of rank names */        \
      { NULL, NULL }, { { NULL, NULL } },       \
      /* strings: pantheon deity names */       \
      NULL, NULL, NULL,                         \
      /* file code, quest home+goal names */    \
      NULL, NULL, NULL,                         \
      /* indices: base mon type, pet */         \
      NON_PM, NON_PM,                           \
      /* quest leader, guardians, nemesis */    \
      NON_PM, NON_PM, NON_PM,                   \
      /* quest enemy types (index, symbol) */   \
      NON_PM, NON_PM, '\0', '\0',               \
      /* quest artifact object index */         \
      STRANGE_OBJECT,                           \
      /* Bitmasks */                            \
      0,                                        \
      /* Attributes */                          \
      {0}, {0}, {0}, {0}, 0, 0,                 \
      /* spell statistics */                    \
      0, 0, 0, 0, 0, 0, 0 }

/* The UNDEFINED_RACE macro is used to initialize Race variables */
#define UNDEFINED_RACE \
    {                                           \
      /* strings */                             \
      NULL, NULL, NULL, NULL, { NULL, NULL },   \
      /* Indices: base race, mummy, zombie */   \
      NON_PM, NON_PM, NON_PM,                   \
      /* Bitmasks */                            \
      0, 0, 0, 0,                               \
      /* Characteristic limits */               \
      {0}, {0},                                 \
      /* Level change HP and Pw adjustments */  \
      {0}, {0}                                  \
    }

#define MATCH_WARN_OF_MON(mon) \
    (Warn_of_mon                                                        \
     && ((svc.context.warntype.obj & (mon)->data->mflags2) != 0           \
         || (svc.context.warntype.polyd & (mon)->data->mflags2) != 0      \
         || (svc.context.warntype.species                                 \
             && (svc.context.warntype.species == (mon)->data))))

/* makemon()/goodpos() control flags, sharing one bit space */
#include "nh_makemon.h"

/* flags for mhidden_description() (pager.c; used for mimics and hiders) */
#define MHID_PREFIX  1 /* include ", mimicking " prefix */
#define MHID_ARTICLE 2 /* include "a " or "an " after prefix */
#define MHID_ALTMON  4 /* if mimicking a monster, include that */
#define MHID_REGION  8 /* include region when mon is in one */

/* flags for that_is_a_mimic() */
#define MIM_REVEAL    1 /* seemimic() */
#define MIM_OMIT_WAIT 2 /* strip beginning from "Wait!  That is a <foo>" */

/* corpse/statue creation flags, low bits stored in obj->spe */
#include "nh_corpstat.h"

/* flag bits for collect_coords(); combining ring_pairs with unshuffled
   makes no sense--if both are specified unshuffled takes precedence */
#define CC_NO_FLAGS    0x00 /* skip center, collect in distinct rings and
                             * shuffle each ring, ignore monster occupants */
#define CC_INCL_CENTER 0x01 /* include center point as ring #0 */
#define CC_UNSHUFFLED  0x02 /* don't shuffle the rings */
#define CC_RING_PAIRS  0x04 /* shuffle w/ odd and next even rings together */
#define CC_SKIP_MONS   0x08 /* skip locations occupied by monsters */
#define CC_SKIP_INACCS 0x10 /* skip !ZAP_POS: reject rock and wall locations
                             * but allow pools, unlike !ACCESSIBLE */

/* flags for decide_to_shift() */
#define SHIFT_SEENMSG 0x01 /* put out a message if in sight */
#define SHIFT_MSG 0x02     /* always put out a message */

/* m_poisongas_ok() return values */
#define M_POISONGAS_BAD   0 /* poison gas is bad */
#define M_POISONGAS_MINOR 1 /* poison gas is ok, maybe causes coughing */
#define M_POISONGAS_OK    2 /* ignores poison gas completely */

/* flags for deliver_obj_to_mon */
#define DF_NONE     0x00
#define DF_RANDOM   0x01
#define DF_ALL      0x04

/* special mhpmax value when loading bones monster to flag as extinct or
 * genocided */
#define DEFUNCT_MONSTER (-100)

/* macro form of adjustments of physical damage based on Half_physical_damage.
 * Can be used on-the-fly with the 1st parameter to losehp() if you don't
 * need to retain the dmg value beyond that call scope.
 * Take care to ensure it doesn't get used more than once in other instances.
 */
#define Maybe_Half_Phys(dmg) \
    ((Half_physical_damage) ? (((dmg) + 1) / 2) : (dmg))

/* object-selection menus: what to offer, by category, and in what order
   [sortloot flags gathered here too; see nh_objsel.h] */
#include "nh_objsel.h"

/* where an artifact came from, recorded when it is named */
#include "nh_oname.h"

/* Flags to control find_mid() and whereis_mon() */
#define FM_FMON 0x01    /* search the fmon chain */
#define FM_MIGRATE 0x02 /* search the migrating monster chain */
#define FM_MYDOGS 0x04  /* search gm.mydogs */
#define FM_YOU 0x08     /* check for gy.youmonst */
#define FM_EVERYWHERE (FM_YOU | FM_FMON | FM_MIGRATE | FM_MYDOGS)

/* Flags to control pick_[race,role,gend,align] routines in role.c */
#define PICK_RANDOM 0
#define PICK_RIGID 1

/* circumstances under which a trap is being triggered */
#include "nh_trapflags.h"

/* [test_move flags and m_move return values moved to nh_move.h] */

/* [yes/no query wrappers moved to nh_msg.h] */

/* Macros for scatter */
#define VIS_EFFECTS 0x01 /* display visual effects */
#define MAY_HITMON 0x02  /* objects may hit monsters */
#define MAY_HITYOU 0x04  /* objects may hit you */
#define MAY_HIT (MAY_HITMON | MAY_HITYOU)
#define MAY_DESTROY 0x08  /* objects may be destroyed at random */
#define MAY_FRACTURE 0x10 /* boulders & statues may fracture */

/* Macros for launching objects */
#define ROLL 0x01          /* the object is rolling */
#define FLING 0x02         /* the object is flying through the air */
#define LAUNCH_UNSEEN 0x40 /* hero neither caused nor saw it */
#define LAUNCH_KNOWN 0x80  /* the hero caused this by explicit action */

/* enlightenment control flags */
#define BASICENLIGHTENMENT 1 /* show mundane stuff */
#define MAGICENLIGHTENMENT 2 /* show intrinsics and such */
#define ENL_GAMEINPROGRESS 0
#define ENL_GAMEOVERALIVE  1 /* ascension, escape, quit, trickery */
#define ENL_GAMEOVERDEAD   2

/* [sortloot() control flags moved to nh_objsel.h] */

/* flags for xkilled() [note: meaning of first bit used to be reversed,
   1 to give message and 0 to suppress] */
#define XKILL_GIVEMSG   0
#define XKILL_NOMSG     1
#define XKILL_NOCORPSE  2
#define XKILL_NOCONDUCT 4

/* [custompline() pline_flags moved to nh_msg.h] */

/* get_count flags */
#define GC_NOFLAGS   0
#define GC_SAVEHIST  1 /* save "Count: 123" in message history */
#define GC_CONDHIST  2 /* save "Count: N" in message history unless the
                        * first digit is passed in and N matches it */
#define GC_ECHOFIRST 4 /* echo "Count: 1" even when there's only one digit */

/* rloc() flags */
#define RLOC_NONE    0x00
#define RLOC_ERR     0x01 /* allow impossible() if no rloc */
#define RLOC_MSG     0x02 /* show vanish/appear msg */
#define RLOC_NOMSG   0x04 /* prevent appear msg, even for STRAT_APPEARMSG */

/* indices for some special tin types */
#define ROTTEN_TIN 0
#define HOMEMADE_TIN 1
#define SPINACH_TIN (-1)
#define RANDOM_TIN (-2)
#define HEALTHY_TIN (-3)

/* Corpse aging */
#define TAINT_AGE (50L)        /* age when corpses go bad */
#define TROLL_REVIVE_CHANCE 37 /* 1/37 chance for 50 turns ~ 75% chance */
#define ROT_AGE (250L)         /* age when corpses rot away */

/* Some misc definitions */
#define POTION_OCCUPANT_CHANCE(n) (13 + 2 * (n))
#define WAND_BACKFIRE_CHANCE 100
#define WAND_WREST_CHANCE 121
#define BALL_IN_MON (u.uswallow && uball && uball->where == OBJ_FREE)
#define CHAIN_IN_MON (u.uswallow && uchain && uchain->where == OBJ_FREE)
#define NODIAG(monnum) ((monnum) == PM_GRID_BUG)

/* Flags to control menus */
#define MENUTYPELEN sizeof("traditional ")
#define MENU_TRADITIONAL 0
#define MENU_COMBINATION 1
#define MENU_FULL 2
#define MENU_PARTIAL 3

/* flags to control teleds() */
#define TELEDS_NO_FLAGS   0
#define TELEDS_ALLOW_DRAG 1
#define TELEDS_TELEPORT   2

/* flags for mktrap() */
#define MKTRAP_NOFLAGS       0x0U
#define MKTRAP_SEEN          0x1U /* trap is seen */
#define MKTRAP_MAZEFLAG      0x2U /* choose random coords instead of room */
#define MKTRAP_NOSPIDERONWEB 0x4U /* web will not generate a spider */
#define MKTRAP_NOVICTIM      0x8U /* no victim corpse or items on it */

#define MON_POLE_DIST 5 /* How far monsters can use pole-weapons */
#define PET_MISSILE_RANGE2 36 /* Square of distance within which pets shoot */

/* flags passed to getobj() to control how it responds to player input */
#define GETOBJ_NOFLAGS  0x0
#define GETOBJ_ALLOWCNT 0x1 /* is a count allowed with this command? */
#define GETOBJ_PROMPT   0x2 /* should it force a prompt for input? (prevents
                             * it exiting early with "You don't have anything
                             * to foo" if nothing in inventory is valid) */

/* flags for hero_breaks() and hits_bars(); BRK_KNOWN* let callers who have
   already called breaktest() prevent it from being called again since it
   has a random factor which makes it be non-deterministic */
#define BRK_BY_HERO        0x01
#define BRK_FROM_INV       0x02
#define BRK_KNOWN2BREAK    0x04
#define BRK_KNOWN2NOTBREAK 0x08
#define BRK_KNOWN_OUTCOME  (BRK_KNOWN2BREAK | BRK_KNOWN2NOTBREAK)
#define BRK_MELEE          0x10

/* extended command return values */
#define ECMD_OK     0x00 /* cmd done successfully */
#define ECMD_TIME   0x01 /* cmd took time, uses up a turn */
#define ECMD_CANCEL 0x02 /* cmd canceled by user */
#define ECMD_FAIL   0x04 /* cmd failed to finish, maybe with a yafm */

/* flags for newcham() */
#define NO_NC_FLAGS          0U
#define NC_SHOW_MSG          0x01U
#define NC_VIA_WAND_OR_SPELL 0x02U

/* Constant passed to explode() for gas spores because gas spores are weird.
 * Specifically, this is an exception to whole "explode() uses dobuzz types"
 * system (the range -1 to -9 isn't used by it, for some reason), where this
 * is effectively an extra dobuzz type, and some zap.c code needs to be aware
 * of it. */
#define PHYS_EXPL_TYPE -1

/* macros for dobuzz() type */
#define BZ_VALID_ADTYP(adtyp) ((adtyp) >= AD_MAGM && (adtyp) <= AD_SPC2)

#define BZ_OFS_AD(adtyp) (abs((adtyp) - AD_MAGM) % 10)
#define BZ_OFS_WAN(otyp) (abs((otyp) - WAN_MAGIC_MISSILE) % 10)
#define BZ_OFS_SPE(otyp) (abs((otyp) - SPE_MAGIC_MISSILE) % 10)
/* hero shooting a wand */
#define BZ_U_WAND(bztyp) (0 + (bztyp))     /*  0..9  */
/* hero casting a spell */
#define BZ_U_SPELL(bztyp) (10 + (bztyp))   /* 10..19 */
/* hero breathing as a monster */
#define BZ_U_BREATH(bztyp) (20 + (bztyp))  /* 20..29 */
/* monster casting a spell */
#define BZ_M_SPELL(bztyp) (-10 - (bztyp))  /* -19..-10 */
/* monster breathing */
#define BZ_M_BREATH(bztyp) (-20 - (bztyp)) /* -29..-20 */
/* monster shooting a wand; note: not -9 to -0 because -0 is ambiguous  */
#define BZ_M_WAND(bztyp) (-30 - (bztyp))   /* -39..-30 */

/* pick a random entry from array */
#define ROLL_FROM(array) array[rn2(SIZE(array))]
/* array with terminator variation */
/* #define ROLL_FROMT(array) array[rn2(SIZE(array) - 1)] */

/* validate index of array */
#define IndexOk(idx, array) \
    ((idx) >= 0 && (idx) < SIZE(array))
/* array with terminator variation */
#define IndexOkT(idx, array) \
    ((idx) >= 0 && (idx) < (SIZE(array) - 1))

#define FEATURE_NOTICE_VER(major, minor, patch)                    \
    (((unsigned long) major << 24) | ((unsigned long) minor << 16) \
     | ((unsigned long) patch << 8) | ((unsigned long) 0))

#define FEATURE_NOTICE_VER_MAJ (flags.suppress_alert >> 24)
#define FEATURE_NOTICE_VER_MIN \
    (((unsigned long) (0x0000000000FF0000L & flags.suppress_alert)) >> 16)
#define FEATURE_NOTICE_VER_PATCH \
    (((unsigned long) (0x000000000000FF00L & flags.suppress_alert)) >> 8)

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#define plur(x) (((x) == 1) ? "" : "s")

/* Cast to int, but limit value to range. */
#define LIMIT_TO_RANGE_INT(lo, hi, var) \
    ((int) ((var) < (lo) ? (lo) : (var) > (hi) ? (hi) : (var)))

#define ARM_BONUS(obj) \
    (objects[(obj)->otyp].a_ac + (obj)->spe                             \
     - min((int) greatest_erosion(obj), objects[(obj)->otyp].a_ac))

#define makeknown(x) discover_object((x), TRUE, TRUE, TRUE)
#define distu(xx, yy) dist2((coordxy) (xx), (coordxy) (yy), u.ux, u.uy)
#define mdistu(mon) distu((mon)->mx, (mon)->my)
#define onlineu(xx, yy) online2((coordxy)(xx), (coordxy)(yy), u.ux, u.uy)

#define rn1(x, y) (rn2(x) + (y))

/* negative armor class is randomly weakened to prevent invulnerability */
#define AC_VALUE(AC) ((AC) >= 0 ? (AC) : -rnd(-(AC)))

#if defined(MICRO) && !defined(__DJGPP__)
#define getuid() 1
#define getlogin() ((char *) 0)
#endif /* MICRO */

/* These may have been defined to platform-specific values in *conf.h
 * or on the compiler command line from a hints file or Makefile */

#ifndef QSORTCALLBACK
#define QSORTCALLBACK
#endif

#ifndef SIG_RET_TYPE
#define SIG_RET_TYPE void (*)(int)
#endif

#define DEVTEAM_EMAIL "devteam@nethack.org"
#define DEVTEAM_URL "https://www.nethack.org/"

#if !defined(CROSSCOMPILE) || defined(CROSSCOMPILE_TARGET)
#include "nhlua.h"
#endif

#if !defined(RECOVER_C)

#include "extern.h"
#include "savefile.h"
#include "decl.h"

#endif  /* RECOVER_C */

#endif /* HACK_H */

