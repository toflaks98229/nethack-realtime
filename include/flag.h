/* NetHack 5.0	flag.h	$NHDT-Date: 1781973080 2026/06/20 16:31:20 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.265 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you change the flag structure make sure you increment EDITLEVEL in   */
/* patchlevel.h if needed.  Changing the instance_flags structure does     */
/* not require incrementing EDITLEVEL.                                     */

/**
 * @file flag.h
 * @brief The player's settings, and the game's own switches.
 *
 * Three structures live here, and the difference between them is the whole organising idea.
 *
 * The first holds the player's choices that belong to the character: whether to pick things up automatically,
 * whether to confirm before attacking a pet, which role and race were chosen. These are saved with the game, so a
 * restored game plays the way it was being played -- and their layout is therefore part of the save format.
 *
 * The second holds settings that belong to the session rather than the character: how the map is drawn, what the
 * window port can do, which font to use. These are established each time the game starts and never saved, which is
 * why changing them does not invalidate a save.
 *
 * Mixed in with the second are a good number of fields that are not settings at all. They are the game's internal
 * switches -- am I currently running a script, is a command being parsed, is this the dump log -- and the comment
 * above that structure concedes they belong somewhere else. They are here because they had nowhere better to be.
 *
 * The third is the accessibility settings, kept separate because they change what the game says rather than what it
 * does.
 *
 * @note Many options are read as a character or a small number rather than as a boolean, because they have more than
 *       two states. The named values for each sit next to the field, and the plain letters are what the player types.
 * @note The window capability fields, all named alike, are requests rather than facts: the core sets what it would
 *       like and the port grants what it can.
 * @warning Changing the first structure means bumping the save format version, as the standing comment at the top of
 *          the file instructs. Changing the second does not.
 */

/**
 * @file flag.h
 * @brief 플레이어의 설정과, 게임 자신의 스위치들.
 *
 * 여기에는 세 구조체가 살고 있고, 그 차이가 이 파일을 조직하는 발상 전체다.
 *
 * 첫 번째는 캐릭터에 속하는 플레이어의 선택을 담는다. 물건을 자동으로 집을지, 애완동물을 공격하기 전에 확인할지, 어떤 직업과 종족을 골랐는지. 이들은 게임과 함께 저장되므로, 복원된
 * 게임은 플레이되던 방식대로 플레이된다. 따라서 그 배치가 저장 형식의 일부다.
 *
 * 두 번째는 캐릭터가 아니라 세션에 속하는 설정을 담는다. 지도를 어떻게 그리는지, 창 포트가 무엇을 할 수 있는지, 어떤 글꼴을 쓸지. 이들은 게임이 시작될 때마다 세워지고 결코 저장되지
 * 않는다. 그것을 바꾸는 것이 저장을 무효로 만들지 않는 이유다.
 *
 * 두 번째에 뒤섞여 설정이 전혀 아닌 필드도 꽤 많이 있다. 그것들은 게임의 내부 스위치다. 지금 스크립트를 실행 중인가, 명령이 파싱되고 있는가, 이것이 덤프 로그인가. 그 구조체 위의
 * 주석은 그것들이 다른 곳에 속한다고 인정하고 있다. 더 나은 자리가 없어서 여기에 있는 것이다.
 *
 * 세 번째는 접근성 설정이며, 게임이 무엇을 하는지가 아니라 무엇을 말하는지를 바꾸므로 따로 있다.
 *
 * @note 많은 옵션이 논리값이 아니라 문자나 작은 수로 읽힌다. 상태가 둘보다 많기 때문이다. 각각의 이름 붙은 값이 그 필드 곁에 있고, 평범한 글자들은 플레이어가 입력하는 것이다.
 * @note 다 비슷한 이름을 가진 창 능력 필드들은 사실이 아니라 요청이다. 코어가 원하는 것을 적어 두고 포트가 할 수 있는 것을 승인한다.
 * @warning 첫 번째 구조체를 바꾸는 것은 파일 맨 위의 상시 주석이 지시하듯 저장 형식 버전을 올린다는 뜻이다. 두 번째를 바꾸는 것은 그렇지 않다.
 */

#ifndef FLAG_H
#define FLAG_H

/**
 * @brief The player's settings that belong to this character.
 *
 * Saved with the game so that a restored character is played the way it was being played. That is why the character's
 * role, race and gender live here too -- they are choices, and they must survive restoring alongside the choices
 * about how to play.
 *
 * @note @c beginner is not a setting the player makes but a state the game observes; it makes early feedback more
 *       explanatory and stops doing so later.
 * @note @c moonphase and @c friday13 are recorded rather than computed on demand, because they must not change under
 *       the hero mid-game if a session spans midnight.
 * @note The reserved fields at the end exist so that a new setting can be added without changing the layout, and so
 *       without invalidating existing saves.
 * @warning @c wizard and @c discover are alternative names for two of these, not separate fields. Debugging mode and
 *          exploration mode are ordinary flags here.
 */
/**
 * @brief 이 캐릭터에 속하는 플레이어의 설정.
 *
 * 게임과 함께 저장되어, 복원된 캐릭터가 플레이되던 방식대로 플레이되게 한다. 캐릭터의 직업, 종족, 성별도 여기에 사는 이유가 그것이다. 그것들도 선택이며, 어떻게 플레이할지에 관한 선택과
 * 나란히 복원을 견뎌야 한다.
 *
 * @note @c beginner 는 플레이어가 정하는 설정이 아니라 게임이 관찰하는 상태다. 초반의 안내를 더 설명적으로 만들고 나중에는 그만둔다.
 * @note @c moonphase 와 @c friday13 은 필요할 때 계산되지 않고 기록된다. 한 세션이 자정을 넘길 때 영웅 아래에서 그것이 바뀌어서는 안 되기 때문이다.
 * @note 끝의 예약된 필드들은 배치를 바꾸지 않고, 따라서 기존 저장을 무효로 만들지 않고 새 설정을 더할 수 있도록 있다.
 * @warning @c wizard 와 @c discover 는 이 중 두 필드의 다른 이름이며 별개의 필드가 아니다. 디버깅 모드와 탐색 모드는 여기의 평범한 플래그다.
 */
struct flag {
    boolean acoustics;       /* allow dungeon sound messages */
    boolean armorstatus;     /* show armor info on status lines */
    boolean autodig;         /* MRKR: Automatically dig */
    boolean autoquiver;      /* Automatically fill quiver */
    boolean autoopen;        /* open doors by walking into them */
    boolean beginner;        /* True early in each game; affects feedback */
    boolean biff;            /* enable checking for mail */
    boolean bones;           /* allow saving/loading bones */
    boolean confirm;         /* confirm before hitting tame monsters */
    boolean dark_room;       /* show shadows in lit rooms */
    boolean debug;           /* in debugging mode (aka wizard mode) */
#define wizard flags.debug
    boolean end_own;         /* list all own scores */
    boolean explore;         /* in exploration mode (aka discover mode) */
#define discover flags.explore
    boolean female;
    boolean friday13;        /* it's Friday the 13th */
    boolean goldX;           /* for BUCX filtering, whether gold is X or U */
    boolean help;            /* look in data file for info about stuff */
    boolean tips;            /* show helpful hints? */
    boolean tutorial;        /* ask if player wants tutorial level? */
    boolean ignintr;         /* ignore interrupts */
    boolean implicit_uncursed; /* maybe omit "uncursed" status in inventory */
    boolean ins_chkpt;       /* checkpoint as appropriate; INSURANCE */
    boolean invlet_constant; /* let objects keep their inventory symbol */
    boolean legacy;          /* print game entry "story" */
    boolean lit_corridor;    /* show a dark corr as lit if it is in sight */
    boolean mention_decor;   /* give feedback for unobscured furniture */
    boolean mention_walls;   /* give feedback when bumping walls */
    boolean nap;             /* `timed_delay' option for display effects */
    boolean nopick_dropped;  /* items you dropped may be autopicked */
    boolean null;            /* OK to send nulls to the terminal */
    boolean pickup;          /* whether you pickup or move and look */
    boolean pickup_stolen;   /* auto-pickup items stolen by a monster */
    boolean pickup_thrown;   /* auto-pickup items you threw */
    boolean pushweapon; /* When wielding, push old weapon into second slot */
    boolean quick_farsight;  /* True disables map browsing during random
                              * clairvoyance */
    boolean rest_on_space;   /* space means rest */
    boolean safe_dog;        /* give complete protection to the dog */
    boolean safe_wait;       /* prevent wait or search next to hostile */
    boolean showexp;         /* show experience points */
    boolean showscore;       /* show score */
    boolean showvers;        /* show version on status lines */
    boolean silent;          /* whether the bell rings or not */
    boolean sortpack;        /* sorted inventory */
    boolean sparkle;         /* show "resisting" special FX (Scott Bigham) */
    boolean standout;        /* use standout for --More-- */
    boolean terrainstatus;   /* show terrain info on status lines */
    boolean time;            /* display elapsed 'time' */
    boolean tombstone;       /* print tombstone */
    boolean verbose;         /* max battle info */
    boolean weaponstatus;    /* show weapon info on status lines */
    int end_top, end_around; /* describe desired score list */
    /**
     * @brief Which measures the hero may take automatically against a locked thing.
     * @note Bits, and the player may permit several: try the key, kick it, force it with a weapon. Permission is
     *       given in advance because being asked at each locked chest is tedious, but kicking a chest can break what
     *       is inside -- so the choice is the player's and is remembered.
     */
    /**
     * @brief 잠긴 것에 대해 영웅이 자동으로 취해도 되는 수단.
     * @note 비트이며 플레이어가 여러 개를 허용할 수 있다. 열쇠를 써 보기, 차기, 무기로 억지로 열기. 허락을 미리 받는 것은 잠긴 상자마다 묻는 것이 번거롭기 때문이다. 그런데 상자를
     *       차면 안의 것이 부서질 수 있다. 그래서 그 선택은 플레이어의 것이고 기억된다.
     */
    unsigned autounlock;     /* locked door/chest action */
#define AUTOUNLOCK_UNTRAP    1
#define AUTOUNLOCK_APPLY_KEY 2
#define AUTOUNLOCK_KICK      4
#define AUTOUNLOCK_FORCE     8
    /**
     * @brief The phase of the moon when the game began.
     * @note Affects luck and a few monsters' behaviour, so it is real game state and not decoration.
     * @warning The two named values are the extremes of the scale, not the whole of it -- intermediate phases are the
     *          numbers between, so this must be compared rather than tested.
     */
    /**
     * @brief 게임이 시작될 때의 달의 위상.
     * @note 운과 몇몇 몬스터의 행동에 영향을 준다. 그래서 장식이 아니라 실제 게임 상태다.
     * @warning 이름 붙은 두 값은 그 척도의 양 극단이며 전부가 아니다. 중간 위상은 그 사이의 숫자이므로, 이것은 검사가 아니라 비교되어야 한다.
     */
    unsigned moonphase;
    /**
     * @brief Which one-off notices the player has asked not to see again.
     * @note Kept with the character rather than the session, so dismissing a notice is permanent for that game.
     */
    /**
     * @brief 플레이어가 다시 보지 않겠다고 한 일회성 알림이 무엇인지.
     * @note 세션이 아니라 캐릭터와 함께 보관된다. 그래서 알림을 물리는 것은 그 게임 동안 영구하다.
     */
    unsigned long suppress_alert;
#define NEW_MOON 0
#define FULL_MOON 4
    /**
     * @brief Which actions the player wants to be asked about more insistently.
     *
     * Every one of these guards an action that is easy to take by accident and impossible to take back: quitting,
     * attacking a peaceful monster, breaking a wand, praying at the wrong moment. Turning a bit on does not add a
     * question -- it makes an existing question demand a full "yes" rather than accepting a single keystroke.
     *
     * @note Which ones are on is a matter of taste, which is why this is a set of bits rather than one setting. A
     *       player who has lost a game to a stray keystroke turns on the one that would have saved them.
     */
    /**
     * @brief 플레이어가 더 끈질기게 확인받고 싶어 하는 행동이 무엇인지.
     *
     * 이들 각각은 실수로 하기 쉽고 되돌릴 수 없는 행동을 지킨다. 그만두기, 평화로운 몬스터를 공격하기, 지팡이를 부러뜨리기, 잘못된 순간에 기도하기. 비트를 켜는 것은 질문을 더하는 것이
     * 아니다. 이미 있는 질문이 키 하나를 받아들이는 대신 온전한 "yes"를 요구하게 만든다.
     *
     * @note 어느 것을 켜는지는 취향의 문제이며, 그래서 이것이 하나의 설정이 아니라 비트 묶음이다. 잘못 눌린 키 하나로 게임을 잃어 본 플레이어는 자신을 구했을 그 비트를 켠다.
     */
    unsigned paranoia_bits; /* alternate confirmation prompting */
#define PARANOID_CONFIRM    0x0001
#define PARANOID_QUIT       0x0002
#define PARANOID_DIE        0x0004
#define PARANOID_BONES      0x0008
#define PARANOID_HIT        0x0010
#define PARANOID_PRAY       0x0020
#define PARANOID_REMOVE     0x0040
#define PARANOID_BREAKWAND  0x0080
#define PARANOID_WERECHANGE 0x0100
#define PARANOID_EATING     0x0200
#define PARANOID_SWIM       0x0400
#define PARANOID_TRAP       0x0800
#define PARANOID_AUTOALL    0x1000
    /**
     * @brief Which parts of the version to show on the status line.
     * @warning The bit order is not the display order. The existing comment states the display order is name, branch,
     *          number, so these values cannot be walked in sequence to build the string.
     */
    /**
     * @brief 상태줄에 버전의 어느 부분을 보일지.
     * @warning 비트 순서가 표시 순서가 아니다. 기존 주석은 표시 순서가 이름, 브랜치, 번호라고 밝히고 있다. 그래서 이 값들을 차례로 밟아 문자열을 만들 수는 없다.
     */
    unsigned versinfo; /* flag mask for 'showvers' option */
    /* mask bits for 'versinfo'; numeric order does not match display order
       which is "name branch number" */
#define VI_NUMBER 1 /* x.y.z */
#define VI_NAME   2 /* game's name (ie, "nethack") */
#define VI_BRANCH 4 /* development branch (from git, via Makefile -CFLAGS) */
    int pickup_burden; /* maximum burden before prompt */
    int pile_limit;    /* controls feedback when walking over objects */
    char discosort;    /* order of dodiscovery/doclassdisco output: o,s,c,a */
    char sortloot; /* 'n'=none, 'l'=loot (pickup), 'f'=full ('l'+invent) */
    uchar vanq_sortmode; /* [uint_8] order of vanquished monsters: 0..7 */
    /**
     * @brief The order the player wants their pack listed in, as a sequence of class symbols.
     * @note A list rather than a comparison rule, so the player can put whatever they consult most at the top. Its
     *       length is the number of classes because every class must appear exactly once.
     */
    /**
     * @brief 플레이어가 가방이 나열되기를 원하는 순서. 계열 심볼의 나열로.
     * @note 비교 규칙이 아니라 목록이다. 그래서 플레이어가 가장 자주 보는 것을 맨 위에 둘 수 있다. 길이가 계열의 수인 것은 모든 계열이 정확히 한 번 나와야 하기 때문이다.
     */
    char inv_order[MAXOCLASSES];
    /**
     * @brief Which classes of object are picked up automatically.
     * @note Also a list of class symbols, and an empty one means nothing is picked up automatically -- which is
     *       different from automatic pickup being switched off, since the switch is a separate setting.
     */
    /**
     * @brief 어떤 계열의 물건이 자동으로 집히는지.
     * @note 이것도 계열 심볼의 목록이며, 비어 있으면 아무것도 자동으로 집히지 않는다. 그것은 자동 집기가 꺼진 것과 다르다. 그 스위치는 별개의 설정이기 때문이다.
     */
    char pickup_types[MAXOCLASSES];
/**
 * @name End-of-game disclosure
 * @brief Whether to reveal each kind of information when the game ends, and whether to ask.
 *
 * Six kinds of information can be disclosed, and for each the player may want it always, never, or to be asked. So each
 * is a character rather than a flag, and the character encodes both the answer and whether to prompt: a letter means
 * ask with that default, and a sign means do it without asking.
 *
 * @note @c '?' and @c '#' are the special case for the vanquished-monsters listing, which has a third possible answer
 *       beyond yes and no.
 * @{
 */
/**
 * @name 게임 종료 시 공개
 * @brief 게임이 끝날 때 각 종류의 정보를 드러낼지, 그리고 물을지.
 *
 * 공개될 수 있는 정보가 여섯 종류이고, 각각에 대해 플레이어는 항상 원하거나, 결코 원하지 않거나, 묻기를 원할 수 있다. 그래서 각각이 플래그가 아니라 문자이며, 그 문자가 답과 물을지
 * 여부를 함께 담는다. 글자는 그 값을 기본으로 하여 묻는다는 뜻이고, 기호는 묻지 않고 그렇게 한다는 뜻이다.
 *
 * @note @c '?' 와 @c '#' 는 물리친 몬스터 목록을 위한 특별한 경우다. 그것은 예와 아니오 말고 세 번째 답이 있다.
 * @{
 */
#define NUM_DISCLOSURE_OPTIONS 6 /* i,a,v,g,c,o (decl.c) */
#define DISCLOSE_PROMPT_DEFAULT_YES 'y'
#define DISCLOSE_PROMPT_DEFAULT_NO 'n'
#define DISCLOSE_PROMPT_DEFAULT_SPECIAL '?' /* v, default a */
#define DISCLOSE_YES_WITHOUT_PROMPT '+'
#define DISCLOSE_NO_WITHOUT_PROMPT '-'
#define DISCLOSE_SPECIAL_WITHOUT_PROMPT '#' /* v, use a */
    char end_disclose[NUM_DISCLOSURE_OPTIONS + 1]; /* disclose various
                                                      info upon exit */
/** @} */
    char menu_style;    /* User interface style setting */
    boolean made_fruit; /* don't easily let user overflow fruit limit */

    /* KMH, role patch -- Variables used during startup.
     *
     * If the user wishes to select a role, race, gender, and/or alignment
     * during startup, the choices should be recorded here.  This
     * might be specified through command-line options, environmental
     * variables, a popup dialog box, menus, etc.
     *
     * These values are each an index into an array.  They are not
     * characters or letters, because that limits us to 26 roles.
     * They are not booleans, because someday someone may need a neuter
     * gender.  Negative values are used to indicate that the user
     * hasn't yet specified that particular value.  If you determine
     * that the user wants a random choice, then you should set an
     * appropriate random value; if you just left the negative value,
     * the user would be asked again!
     *
     * These variables are stored here because the u structure is
     * cleared during character initialization, and because the
     * flags structure is restored for saved games.  Thus, we can
     * use the same parameters to build the role entry for both
     * new and restored games.
     *
     * These variables should not be referred to after the character
     * is initialized or restored (specifically, after role_init()
     * is called).
     */
/**
 * @name Character creation choices
 * @brief What the player has chosen so far, while they are still choosing.
 *
 * Indices rather than letters, deliberately -- letters would cap the number of roles at twenty-six -- and integers
 * rather than booleans, so that a third gender remains possible. A negative value means the choice has not been made
 * yet.
 *
 * They live in the saved settings rather than with the hero because the hero structure is wiped during character
 * creation and this must survive that. Being saved also means the same code can build the role entry for a new game
 * and a restored one.
 *
 * @warning Meaningless once the character exists. The existing comment is explicit that these must not be consulted
 *          after initialisation.
 * @warning A choice the player asked to be random must be resolved to an actual value, not left negative. Leaving it
 *          negative asks the player again, as the existing comment warns.
 * @{
 */
/**
 * @name 캐릭터 생성 선택
 * @brief 플레이어가 아직 고르는 중일 때, 지금까지 무엇을 골랐는지.
 *
 * 의도적으로 글자가 아니라 색인이다. 글자로는 직업의 수가 스물여섯으로 묶인다. 그리고 논리값이 아니라 정수여서 세 번째 성별이 여전히 가능하다. 음수는 그 선택이 아직 이뤄지지 않았음을
 * 뜻한다.
 *
 * 이들이 영웅이 아니라 저장되는 설정 안에 사는 것은, 영웅 구조체가 캐릭터 생성 중에 지워지고 이것은 그것을 견뎌야 하기 때문이다. 저장된다는 것은 또한 같은 코드가 새 게임과 복원된 게임의
 * 직업 항목을 만들 수 있다는 뜻이다.
 *
 * @warning 캐릭터가 존재하면 무의미하다. 기존 주석은 초기화 이후 이것을 참조해서는 안 된다고 명시하고 있다.
 * @warning 플레이어가 무작위로 해 달라고 한 선택은 실제 값으로 결정되어야 하고 음수로 남겨져서는 안 된다. 기존 주석이 경고하듯 음수로 남기면 플레이어에게 다시 묻게 된다.
 * @{
 */
    int initrole;  /* starting role      (index into roles[])   */
    int initrace;  /* starting race      (index into races[])   */
    int initgend;  /* starting gender    (index into genders[]) */
    int initalign; /* starting alignment (index into aligns[])  */
    int randomall; /* randomly assign everything not specified */
    int pantheon;  /* deity selection for priest character */
/** @} */
    /* Items which were in iflags in 3.4.x to preserve savefile compatibility
     */
    boolean lootabc;   /* use "a/b/c" rather than "o/i/b" when looting */
    boolean showrace;  /* show hero glyph by race rather than by role */
    boolean travelcmd; /* allow travel command */
    int runmode;       /* update screen display during run moves */

/**
 * @name Reserved space
 * @brief Unused fields kept so a new setting can be added without changing the layout.
 * @note Since this structure is saved, adding a field would otherwise invalidate every existing save. Consuming one of
 *       these does not.
 * @{
 */
/**
 * @name 예약된 자리
 * @brief 배치를 바꾸지 않고 새 설정을 더할 수 있도록 남겨 둔 미사용 필드.
 * @note 이 구조체가 저장되므로, 그러지 않으면 필드를 더하는 것이 기존 모든 저장을 무효로 만든다. 이 중 하나를 쓰는 것은 그렇지 않다.
 * @{
 */
    int reserved1;
    int reserved2;
    int reserved3;
    int reserved4;
    int reserved5;
    int reserved6;
    int reserved7;
    int reserved8;
/** @} */
};

/*
 * Flags that are set each time the game is started.
 * These are not saved with the game.
 *
 */

/* values for iflags.getpos_coords */
#define GPCOORDS_NONE    'n'
#define GPCOORDS_MAP     'm'
#define GPCOORDS_COMPASS 'c'
#define GPCOORDS_COMFULL 'f'
#define GPCOORDS_SCREEN  's'

enum getloc_filters {
    GFILTER_NONE = 0,
    GFILTER_VIEW,
    GFILTER_AREA,

    NUM_GFILTER
};

#ifdef WIN32
enum windows_key_handling {
    no_keyhandling,
    default_keyhandling,
    ray_keyhandling,
    nh340_keyhandling
};
#endif

struct debug_flags {
    boolean test;
#ifdef TTY_GRAPHICS
    boolean ttystatus;
#endif
#ifdef WIN32
    boolean immediateflips;
#endif
};

enum windowcolors_windows {
    wcolor_menu, wcolor_message, wcolor_status, wcolor_text,
    WC_COUNT
};

struct windowcolors_struct {
    char *fg;
    char *bg;
};

/**
 * @brief Settings that make the game describe in words what it normally shows on the map.
 *
 * A player using a screen reader cannot glance at the map, so things that are ordinarily conveyed by a glyph changing --
 * a monster appearing, a monster moving, the terrain altering -- have to be said aloud instead. These switches ask for
 * that.
 *
 * @note @c mon_notices_blocked is a count rather than a flag, so that noticing can be suspended and resumed in nested
 *       fashion. A count means an inner suspension does not undo an outer one when it ends.
 */
/**
 * @brief 게임이 보통 지도에 보이는 것을 말로 기술하게 만드는 설정.
 *
 * 화면 읽기 프로그램을 쓰는 플레이어는 지도를 훑어볼 수 없다. 그래서 보통 글리프의 변화로 전달되는 것들 -- 몬스터가 나타남, 몬스터가 움직임, 지형이 바뀜 -- 을 대신 소리로 말해야 한다.
 * 이 스위치들이 그것을 요청한다.
 *
 * @note @c mon_notices_blocked 는 플래그가 아니라 개수다. 그래서 알아채기를 중첩된 방식으로 멈추고 되살릴 수 있다. 개수라는 것은 안쪽의 멈춤이 끝날 때 바깥쪽 멈춤을 되돌리지 않는다는
 *       뜻이다.
 */
struct accessibility_data {
    boolean accessiblemsg; /* use msg_loc for plined messages */
    coord msg_loc;         /* accessiblemsg: location */
    boolean mon_notices;   /* msg when hero notices a monster */
    int mon_notices_blocked; /* temp disable mon_notices */
    boolean mon_movement;  /* msg when hero sees monster move */
    boolean glyph_updates; /* msg when map glyphs change */
};

/**
 * @name Suspending monster notices
 * @brief Temporarily stop announcing monsters, then resume.
 *
 * Used where the game has something else to say and interleaving "you see a jackal" would break the message up. The pair
 * must be balanced, and being a count rather than a flag means nesting works.
 *
 * @note Suspending does not lose the notices -- catching up afterwards is a separate call, so the player is still told
 *       what appeared while announcements were off.
 * @warning Turning noticing back on more times than it was turned off is a programming error, and @c notice_mon_on
 *          reports it rather than letting the count go negative.
 * @{
 */
/**
 * @name 몬스터 알림 잠시 멈추기
 * @brief 몬스터 알림을 잠시 멈추고 다시 시작한다.
 *
 * 게임이 다른 할 말이 있고 "재칼이 보인다"가 끼어들면 그 메시지가 끊길 곳에서 쓰인다. 이 짝은 균형이 맞아야 하며, 플래그가 아니라 개수라는 것이 중첩이 작동한다는 뜻이다.
 *
 * @note 멈추는 것이 알림을 잃는 것은 아니다. 나중에 따라잡는 것은 별도의 호출이므로, 알림이 꺼져 있던 동안 무엇이 나타났는지도 플레이어에게 여전히 알려진다.
 * @warning 알림을 끈 횟수보다 많이 켜는 것은 프로그래밍 오류이며, @c notice_mon_on 은 개수가 음수가 되게 두는 대신 그것을 보고한다.
 * @{
 */
/* Use notice_mon_off() / notice_mon_on() to temporarily disable
   noticing the monsters in the vision code - perhaps the game
   needs to output some other messages in between.
   Call notice_all_mons() afterwards to catch up. */
#define notice_mon_off() do { a11y.mon_notices_blocked++; } while(0)
#define notice_mon_on()  do { if (--a11y.mon_notices_blocked < 0) { \
            impossible("mon_notices_blocked<0");                    \
            a11y.mon_notices_blocked = 0;                           \
        } } while(0)
/** @} */

enum debug_fuzzer_states {
    fuzzer_off,
    fuzzer_impossible_panic,
    fuzzer_impossible_continue
};

/*
 * Stuff that really isn't option or platform related and does not
 * get saved and restored.  They are set and cleared during the game
 * to control the internal behavior of various NetHack functions
 * and probably warrant a structure of their own elsewhere some day.
 */
/**
 * @brief Session settings and the game's internal switches, mixed together.
 *
 * Two unrelated things share this structure, and the existing comment above admits as much. The first is everything about
 * how this session presents itself: which window features are wanted, what fonts and colours, how the map is drawn. None
 * of it is saved, because it describes the terminal and not the character -- which is why changing any of it costs
 * nothing.
 *
 * The second is the game's own bookkeeping: am I inside a script, is a command being parsed, is the terminal gone, is
 * this the dump log. These are not options at all. They exist here because they need to be reachable from everywhere and
 * had nowhere better to live.
 *
 * @note The window capability fields are requests. The core states what it would like and each port grants what it can,
 *       so a set field is not a promise the feature is in effect.
 * @note The saved copies of three of the hero's own flags are here rather than local so that a hangup save can restore
 *       them. Detection temporarily clears the originals, and a crash in between must not leave the hero permanently
 *       swallowed.
 * @note @c returning_missile is deliberately an untyped pointer to avoid a dependency; it holds an object.
 * @note @c obsolete exists to be pointed at. Retired options aim their handling at it so that reading an old
 *       configuration file does not fail, and nothing ever reads the value.
 * @warning Not saved. Anything here that genuinely needs to survive a save belongs in the persistent structure instead.
 */
/**
 * @brief 세션 설정과 게임의 내부 스위치가 뒤섞여 있는 구조체.
 *
 * 서로 무관한 두 가지가 이 구조체를 공유하며, 위의 기존 주석도 그것을 인정한다. 첫째는 이 세션이 자신을 어떻게 제시하는지에 관한 모든 것이다. 어떤 창 기능을 원하는지, 어떤 글꼴과 색인지,
 * 지도를 어떻게 그리는지. 어느 것도 저장되지 않는다. 캐릭터가 아니라 터미널을 기술하기 때문이다. 그것을 바꾸는 데 아무 비용이 들지 않는 이유가 그것이다.
 *
 * 둘째는 게임 자신의 기록이다. 스크립트 안에 있는가, 명령이 파싱되고 있는가, 터미널이 사라졌는가, 이것이 덤프 로그인가. 이것들은 옵션이 전혀 아니다. 어디서나 닿을 수 있어야 하고 더 나은
 * 자리가 없어서 여기에 있다.
 *
 * @note 창 능력 필드들은 요청이다. 코어가 원하는 것을 밝히고 각 포트가 할 수 있는 것을 승인하므로, 켜진 필드가 그 기능이 유효하다는 약속은 아니다.
 * @note 영웅 자신의 플래그 세 개의 사본이 지역 변수가 아니라 여기에 있는 것은, 접속 끊김 저장이 그것을 복원할 수 있도록 하기 위함이다. 탐지가 원본을 잠시 지우며, 그 사이의 충돌이 영웅을
 *       영구히 삼켜진 상태로 남겨서는 안 된다.
 * @note @c returning_missile 은 의존성을 피하기 위해 의도적으로 타입 없는 포인터다. 물건 하나를 담는다.
 * @note @c obsolete 는 가리켜지기 위해 존재한다. 폐기된 옵션들이 그 처리를 여기로 향하게 해서 예전 설정 파일을 읽는 것이 실패하지 않게 하며, 그 값을 읽는 것은 아무것도 없다.
 * @warning 저장되지 않는다. 여기 있는 것 중 정말로 저장을 견뎌야 하는 것은 대신 지속되는 구조체에 속한다.
 */
struct instance_flags {
    boolean defer_plname;  /* X11 hack: askname() might not set svp.plname */
    boolean fuzzerpending; /* fuzzing requested on command line but not active
                            * yet (to allow interactive initialization prior
                            * to input becoming taken over);
                            * True => enable fuzzer when entering moveloop */
    boolean herecmd_menu;  /* use menu when mouseclick on yourself */
    boolean idlecheckpoint;  /* platform should perform a checkpoint update
                              * if waiting for input longer than 10 seconds */
    boolean invis_goldsym; /* gold symbol is ' '? */
    boolean in_lua;        /* executing a lua script */
    boolean lua_testing;   /* doing lua tests */
    boolean term_gone;     /* terminal is gone, abort abort abort */
    boolean nofollowers;   /* level change ignores pets (for tutorial) */
    boolean partly_eaten_hack; /* extra flag for xname() used when it's called
                                * indirectly so we can't use xname_flags() */
    boolean query_menu;    /* use a menu for yes/no queries */
    boolean remember_getpos; /* save getpos() positioning in do-again queue */
    boolean sad_feeling;   /* unseen pet is dying */
    boolean showdamage;    /* extra message reporting damage hero has taken */
    boolean pending_customizations; /* at least one custom. was specified */
    xint8 debug_fuzzer;    /* fuzz testing */
    int at_midnight;       /* only valid during end of game disclosure */
    int at_night;          /* also only valid during end of game disclosure */
    int failing_untrap;    /* move_into_trap() -> spoteffects() -> dotrap() */
    int getdir_click;      /* as input to getdir(): non-zero, accept simulated
                            * click that's not adjacent to or on hero;
                            * as output from getdir(): simulated button used
                            * 0 (none) or CLICK_1 (left) or CLICK_2 (right) */
    int getloc_filter;     /* GFILTER_foo */
    int in_lava_effects;   /* hack for Boots_off() */
    int last_msg;          /* indicator of last message player saw */
    int menuobjsyms;       /* value of 'menu_objsyms' option;
                            * ought to be in flags rather than iflags */
    int override_ID;       /* true to force full identification of objects */
    int parse_config_file_src;  /* hack for parse_config_line() */
    int purge_monsters;    /* # of dead monsters still on fmon list */
    int raw_printed;       /* count of messages issued before window_inited */
    int suppress_price;    /* controls doname() for unpaid objects */
    unsigned  terrainmode; /* for getpos()'s autodescribe during #terrain */
#define TER_MAP    0x01U
#define TER_TRP    0x02U
#define TER_OBJ    0x04U
#define TER_MON    0x08U
#define TER_FULL   0x10U   /* explore|wizard mode view full map */
#define TER_DETECT 0x20U   /* detect_foo magic rather than #terrain */
    boolean bgcolors;      /* display background colors on a map position */
    boolean getloc_moveskip;
    boolean getloc_travelmode;
    boolean getloc_usemenu;
    coord travelcc;        /* coordinates for travel_cache */
    boolean trav_debug;    /* display travel path (#if DEBUG only) */
    boolean window_inited; /* true if init_nhwindows() completed */
    boolean vision_inited; /* true if vision is ready */
    boolean sanity_check;  /* run sanity checks */
    boolean sanity_no_check; /* skip next sanity check */
    boolean debug_overwrite_stairs; /* debug: allow overwriting stairs */
    boolean debug_mongen;  /* debug: prevent monster generation */
    boolean debug_hunger;  /* debug: prevent hunger */
    boolean debug_prevent_pline;  /* debug: prevent pline going to UI */
    boolean mon_polycontrol; /* debug: control monster polymorphs */
    boolean mon_telecontrol; /* debug: control monster teleports */
    boolean in_dumplog;    /* doing the dumplog right now? */
    boolean in_parse;      /* is a command being parsed? */
     /* suppress terminate during options parsing, for --showpaths */
    boolean initoptions_noterminate;

    /* stuff that is related to options and/or user or platform preferences
     */
    unsigned msg_history; /* hint: # of top lines to save */
    int getpos_coords;    /* show coordinates when getting cursor position */
    int menuinvertmode;   /* 0 = invert toggles every item;
                           * 1 = invert skips 'all items' item */
    int terrain_typ;      /* index into terrain_descr[] for botl */
    color_attr menu_headings;    /* CLR_ and ATR_ for menu headings */
    uint32_t colorcount;    /* store how many colors terminal is capable of */
    boolean use_truecolor;  /* force use of truecolor */
#ifdef ALTMETA
    boolean altmeta;        /* Alt+c sends ESC c rather than M-c */
#endif
    boolean autodescribe;     /* autodescribe mode in getpos() */
    boolean cbreak;           /* in cbreak mode, rogue format */
    boolean deferred_X;       /* deferred entry into explore mode */
    boolean defer_decor;      /* terrain change message vs slipping on ice */
    boolean echo;             /* 1 to echo characters */
    boolean force_invmenu;    /* always menu when handling inventory */
    boolean hilite_pile;      /* mark piles of objects with a hilite */
    boolean menu_head_objsym; /* Show obj symbol in menu headings; controlled
                               * by 'menuobjsyms' */
    boolean menu_overlay;     /* Draw menus over the map */
    boolean menu_requested;   /* Flag for overloaded use of 'm' prefix
                               * on some non-move commands */
    boolean menu_tab_sep;     /* Use tabs to separate option menu fields */
    boolean news;             /* print news */
    boolean num_pad;          /* use numbers for movement commands */
    boolean perm_invent;      /* display persistent inventory window */
    boolean perm_invent_pending;  /* need to try again */
    boolean pricequotes;      /* display price quotes on unIDd objects */
    boolean renameallowed;    /* can change hero name during role selection */
    boolean renameinprogress; /* we are changing hero name */
    boolean sounds;           /* master on/off switch for using soundlib */
    boolean status_updates;   /* allow updates to bottom status lines;
                               * disable to avoid excessive noise when using
                               * a screen reader (use ^X to review status) */
    boolean toptenwin;        /* ending list in window instead of stdout */
    boolean tux_penalty;      /* True iff hero is a monk and wearing a suit */
    boolean use_background_glyph; /* use background glyph when appropriate */
    boolean use_menu_color;   /* use color in menus; only if wc_color */
    boolean use_menu_glyphs;  /* use object glyphs in menus, if the port
                               * supports it; controlled by 'menuobjsyms' */
#ifdef STATUS_HILITES
    long hilite_delta;        /* number of moves to leave a temp hilite lit */
    long unhilite_deadline; /* time when oldest temp hilite should be unlit */
#endif
    boolean voices;           /* enable text-to-speech or other talking */
    boolean zerocomp;         /* write zero-compressed save files */
    boolean rlecomp;          /* alternative to zerocomp; run-length encoding
                               * compression of levels when writing savefile */
    schar ice_rating;         /* ice_descr()'s classification of ice terrain */
    schar prev_decor;         /* 'mention_decor' just mentioned this */
    uchar num_pad_mode;       /* for num_pad==True, controls how 5 behaves
                               * and/or 789456123 vs phone-style 123456789;
                               * for False, qwertY vs qwertZ */
    uchar perminv_mode;       /* what to display in persistent invent window
                               * 0: nothing, 1: all inventory except gold,
                               * 2: full including gold, 8: in-use items only,
                               * 5|6: 1|2 with invent letters shown in empty
                               * slots (TTY only: 'sparse' modes) */
    uchar bouldersym;         /* symbol for boulder display */
    char prevmsg_window;      /* type of old message window to use */
    boolean extmenu;          /* extended commands use menu interface */
#ifdef MICRO
    boolean BIOS; /* use IBM or ST BIOS calls when appropriate */
#endif
#if defined(MICRO) || defined(WIN32)
    boolean rawio; /* whether can use rawio (IOCTL call) */
#endif
#if defined(MSDOS) || defined(WIN32)
    boolean tile_view;
    boolean over_view;
    boolean traditional_view;
#endif
#ifdef MSDOS
    boolean hasvga; /* has a vga adapter */
    boolean usevga; /* use the vga adapter */
    boolean hasvesa; /* has a VESA-capable VGA adapter */
    boolean usevesa; /* use the VESA-capable VGA adapter */
    boolean grmode; /* currently in graphics mode */
#endif
#ifdef LAN_FEATURES
    boolean lan_mail;         /* mail is initialized */
    boolean lan_mail_fetched; /* mail is awaiting display */
#endif
#ifdef TTY_TILES_ESCCODES
    boolean vt_tiledata;     /* output console codes for tile support in TTY */
#endif
#ifdef TTY_SOUND_ESCCODES
    boolean vt_sounddata;    /* output console codes for sound support in TTY*/
#endif
    boolean cmdassist;       /* provide detailed assistance for some comnds */
    boolean fireassist;      /* autowield launcher when using fire-command */
    boolean wizweight;       /* display weight of everything in wizard mode */
    boolean wizmgender;      /* test gender info from core in window port */
    boolean customcolors;    /* support customcolors defined in glyphmap */
    boolean customsymbols;   /* support customsymbols defined in glyphmap */
    /*
     * Window capability support.
     */
    boolean wc_color;         /* use color graphics                  */
    boolean wc_hilite_pet;    /* hilight pets                        */
    boolean wc_ascii_map;     /* show map using traditional ascii    */
    boolean wc_tiled_map;     /* show map using tiles                */
    boolean wc_preload_tiles; /* preload tiles into memory           */
    int wc_tile_width;        /* tile width                          */
    int wc_tile_height;       /* tile height                         */
    char *wc_tile_file;       /* name of tile file;overrides default */
    boolean wc_inverse;       /* use inverse video for some things   */
    int wc_align_status;      /*  status win at top|bot|right|left   */
    int wc_align_message;     /* message win at top|bot|right|left   */
    int wc_vary_msgcount;     /* show more old messages at a time    */
#if 0
    char *wc_foregrnd_menu; /* points to foregrnd color name for menu win   */
    char *wc_backgrnd_menu; /* points to backgrnd color name for menu win   */
    char *wc_foregrnd_message; /* points to foregrnd color name for msg win */
    char *wc_backgrnd_message; /* points to backgrnd color name for msg win */
    char *wc_foregrnd_status; /* points to foregrnd color name for status   */
    char *wc_backgrnd_status; /* points to backgrnd color name for status   */
    char *wc_foregrnd_text; /* points to foregrnd color name for text win   */
    char *wc_backgrnd_text; /* points to backgrnd color name for text win   */
#else
    struct windowcolors_struct wcolors[WC_COUNT];
#endif
    char *wc_font_map;      /* points to font name for the map win */
    char *wc_font_message;  /* points to font name for message win */
    char *wc_font_status;   /* points to font name for status win  */
    char *wc_font_menu;     /* points to font name for menu win    */
    char *wc_font_text;     /* points to font name for text win    */
    int wc_fontsiz_map;     /* font size for the map win           */
    int wc_fontsiz_message; /* font size for the message window    */
    int wc_fontsiz_status;  /* font size for the status window     */
    int wc_fontsiz_menu;    /* font size for the menu window       */
    int wc_fontsiz_text;    /* font size for text windows          */
    int wc_scroll_amount;   /* scroll this amount at scroll_margin */
    int wc_scroll_margin;   /* scroll map when this far from the edge */
    int wc_map_mode;        /* specify map viewing options, mostly
                             * for backward compatibility */
    int wc_player_selection;    /* method of choosing character */
#if defined(MSDOS)
    unsigned wc_video_width;    /* X resolution of screen */
    unsigned wc_video_height;   /* Y resolution of screen */
#endif
    boolean wc_splash_screen;   /* display an opening splash screen or not */
    boolean wc_popup_dialog;    /* put queries in pop up dialogs instead of
                                 * in the message window */
    boolean wc_eight_bit_input; /* allow eight bit input               */
    boolean wc2_fullscreen;     /* run fullscreen */
    boolean wc2_softkeyboard;   /* use software keyboard */
    boolean wc2_wraptext;       /* wrap text */
    boolean wc2_selectsaved;    /* display a menu of user's saved games */
    boolean wc2_darkgray;    /* try to use dark-gray color for black glyphs */
    boolean wc2_hitpointbar;  /* show graphical bar representing hit points */
    boolean wc2_guicolor;       /* allow colours in gui (outside map) */
    int wc_mouse_support;       /* allow mouse support */
    int wc2_term_cols;          /* terminal width, in characters */
    int wc2_term_rows;          /* terminal height, in characters */
    int wc2_statuslines;        /* default = 2, curses can handle 3 */
    int wc2_windowborders;      /* display borders on NetHack windows */
    int wc2_petattr;            /* text attributes for pet */
#ifdef WIN32
#define MAX_ALTKEYHANDLING 25
    char altkeyhandling[MAX_ALTKEYHANDLING];
    enum windows_key_handling key_handling;
#endif
    /* copies of values in struct u, used during detection when the
       originals are temporarily cleared; kept here rather than
       locally so that they can be restored during a hangup save */
    Bitfield(save_uswallow, 1);
    Bitfield(save_uinwater, 1);
    Bitfield(save_uburied, 1);
    struct debug_flags debug;
    boolean windowtype_locked;   /* windowtype can't change from configfile */
    boolean windowtype_deferred; /* pick a windowport and store it in
                                    chosen_windowport[], but do not switch to
                                    it in the midst of options processing */
    genericptr_t returning_missile; /* 'struct obj *'; Mjollnir or aklys */
    boolean wiz_error_flag;     /* flag for tracking failed wizmode auth */
    boolean explore_error_flag; /* ditto for explore mode */
    boolean obsolete;  /* obsolete options can point at this, it isn't used */
};

/**
 * @name Deprecated names
 * @brief Older names for fields that have since been renamed.
 * @note Kept as aliases rather than removed so that port code written against the old names still compiles. They name the
 *       same fields, not copies.
 * @{
 */
/**
 * @name 폐기된 이름
 * @brief 그 뒤로 이름이 바뀐 필드들의 예전 이름.
 * @note 없애지 않고 별칭으로 남겨 두어, 예전 이름으로 작성된 포트 코드가 여전히 컴파일된다. 사본이 아니라 같은 필드를 가리킨다.
 * @{
 */
/*
 * Old, deprecated names
 */
#ifdef TTY_GRAPHICS
#define eight_bit_tty wc_eight_bit_input
#endif
#define use_color wc_color
#define hilite_pet wc_hilite_pet
#define use_inverse wc_inverse
#ifdef MAC_GRAPHICS_ENV
#define large_font obsolete
#endif
#ifdef MAC68K
#define popup_dialog wc_popup_dialog
#endif
#define preload_tiles wc_preload_tiles
/** @} */

extern NEARDATA struct flag flags;
extern NEARDATA struct instance_flags iflags;
extern NEARDATA struct accessibility_data a11y;

/* last_msg values
 * Usage:
 *  pline("some message");
 *    pline: vsprintf + putstr + iflags.last_msg = PLNMSG_UNKNOWN;
 *  iflags.last_msg = PLNMSG_some_message;
 * and subsequent code can adjust the next message if it is affected
 * by some_message.  The next message will clear iflags.last_msg.
 */
/**
 * @brief What the last message said, so the next one can take account of it.
 *
 * Messages sometimes have to read as a pair. "You see a wand here" followed by a separate sentence naming it would be
 * clumsy, so the second message needs to know what the first was. Rather than passing that along, the message routine
 * records it and the next piece of code adjusts.
 *
 * @note Producing a message sets this to unknown, and the code that wanted it remembered sets it afterwards. So the value
 *       only survives until the next message, which is exactly the lifetime it needs.
 * @warning A stale value would make an unrelated message read as a continuation, which is why the default is set on every
 *          message rather than cleared by whoever consumed it.
 */
/**
 * @brief 지난 메시지가 무엇이었는지. 다음 메시지가 그것을 고려할 수 있도록.
 *
 * 메시지는 때때로 한 쌍으로 읽혀야 한다. "여기 지팡이가 보인다" 뒤에 그것의 이름을 말하는 별개의 문장이 오면 어색하므로, 두 번째 메시지가 첫 번째가 무엇이었는지 알아야 한다. 그것을 전달하는
 * 대신 메시지 루틴이 그것을 기록하고 다음 코드가 맞춰 조정한다.
 *
 * @note 메시지를 내는 것이 이 값을 미지로 설정하고, 그것이 기억되기를 원한 코드가 그 뒤에 설정한다. 그래서 값은 다음 메시지까지만 살아남으며, 그것이 정확히 필요한 수명이다.
 * @warning 낡은 값은 무관한 메시지를 이어지는 말로 읽히게 만든다. 그래서 기본값이 소비한 쪽에 의해 지워지는 것이 아니라 매 메시지마다 설정된다.
 */
enum plnmsg_types {
    PLNMSG_UNKNOWN = 0,         /* arbitrary */
    PLNMSG_ONE_ITEM_HERE,       /* "you see <single item> here" */
    PLNMSG_TOWER_OF_FLAME,      /* scroll of fire */
    PLNMSG_CAUGHT_IN_EXPLOSION, /* explode() feedback */
    PLNMSG_ENVELOPED_IN_GAS,    /* create_gas_cloud() feedback */
    PLNMSG_OBJ_GLOWS,           /* "the <obj> glows <color>" */
    PLNMSG_OBJNAM_ONLY,         /* xname/doname only, for #tip */
    PLNMSG_OK_DONT_DIE,         /* overriding death in explore/wizard mode */
    PLNMSG_BACK_ON_GROUND,      /* leaving water */
    PLNMSG_GROWL,               /* growl() gave some message */
    PLNMSG_HIDE_UNDER,          /* hero saw a monster hide under something */
    PLNMSG_MON_TAKES_OFF_ITEM,  /* thief (nymph, monkey) taking worn item */
    PLNMSG_enum /* 'none of the above' */
};

/**
 * @brief How often to redraw while the hero is running.
 *
 * Running many squares is one command, and redrawing every step of it is both slow and, on a fast machine, a blur. So the
 * player chooses a trade: not at all until it stops, every few steps, every step, or every step with a pause so the
 * movement can actually be watched.
 *
 * @note The last is slower than every step on purpose -- it exists to make running visible rather than fast.
 */
/**
 * @brief 영웅이 달리는 동안 얼마나 자주 다시 그릴지.
 *
 * 여러 칸을 달리는 것은 하나의 명령이고, 그 모든 걸음을 다시 그리는 것은 느리며 빠른 기계에서는 뭉개져 보인다. 그래서 플레이어가 절충을 고른다. 멈출 때까지 전혀 그리지 않기, 몇 걸음마다,
 * 매 걸음마다, 또는 움직임을 실제로 볼 수 있도록 멈춤을 두고 매 걸음마다.
 *
 * @note 마지막 것은 일부러 매 걸음보다 느리다. 달리기를 빠르게 하기 위해서가 아니라 보이게 하기 위해 존재한다.
 */
/* runmode options */
enum runmode_types {
    RUN_TPORT = 0, /* don't update display until movement stops */
    RUN_LEAP,      /* update display every 7 steps */
    RUN_STEP,      /* update display every single step */
    RUN_CRAWL      /* walk w/ extra delay after each update */
};

/**
 * @name Paranoia tests
 * @brief Ask whether a particular confirmation should be the insistent kind.
 *
 * One test per guarded action, so the code asking the question does not have to know which bit means what. Most turn a
 * single-keystroke question into one requiring a typed "yes", but a few do something else: one adds a prompt that would
 * not otherwise appear, and one makes a command prompt even when there is only one possible answer.
 *
 * @note @c ParanoidConfirm is the widest of them -- the existing comments record that where it applies, rejecting also
 *       requires an explicit answer rather than any other key.
 * @{
 */
/**
 * @name 편집증 검사
 * @brief 특정 확인이 끈질긴 종류여야 하는지 묻는다.
 *
 * 지켜지는 행동마다 검사가 하나씩이므로, 질문하는 코드가 어느 비트가 무엇을 뜻하는지 알 필요가 없다. 대부분은 키 하나로 되는 질문을 "yes"를 입력해야 하는 질문으로 바꾸지만, 몇 개는 다른 일을
 * 한다. 하나는 원래 나오지 않을 프롬프트를 더하고, 하나는 가능한 답이 하나뿐일 때에도 명령이 묻게 만든다.
 *
 * @note @c ParanoidConfirm 이 그 중 가장 넓다. 기존 주석들은 그것이 적용되는 곳에서는 거절도 다른 아무 키가 아니라 명시적인 답을 요구한다고 기록하고 있다.
 * @{
 */
/* paranoid confirmation prompting */
/* any yes confirmations also require explicit no (or ESC) to reject */
#define ParanoidConfirm ((flags.paranoia_bits & PARANOID_CONFIRM) != 0)
/* quit: yes vs y for "Really quit?" and "Enter explore mode?" */
#define ParanoidQuit ((flags.paranoia_bits & PARANOID_QUIT) != 0)
/* die: yes vs y for "Die?" (dying in explore mode or wizard mode) */
#define ParanoidDie ((flags.paranoia_bits & PARANOID_DIE) != 0)
/* hit: yes vs y for "Save bones?" in wizard mode */
#define ParanoidBones ((flags.paranoia_bits & PARANOID_BONES) != 0)
/* hit: yes vs y for "Really attack <the peaceful monster>?" */
#define ParanoidHit ((flags.paranoia_bits & PARANOID_HIT) != 0)
/* pray: ask "Really pray?" (accepts y answer, doesn't require yes),
   taking over for the old prayconfirm boolean option */
#define ParanoidPray ((flags.paranoia_bits & PARANOID_PRAY) != 0)
/* remove: remove ('R') and takeoff ('T') commands prompt for an inventory
   item even when only one accessory or piece of armor is currently worn */
#define ParanoidRemove ((flags.paranoia_bits & PARANOID_REMOVE) != 0)
/* breakwand: Applying a wand */
#define ParanoidBreakwand ((flags.paranoia_bits & PARANOID_BREAKWAND) != 0)
/* werechange: accepting randomly timed werecreature change to transform
   from human to creature or vice versa while having polymorph control */
#define ParanoidWerechange ((flags.paranoia_bits & PARANOID_WERECHANGE) != 0)
/* continue eating: prompt given _after_first_bite_ when eating something
   while satiated */
#define ParanoidEating ((flags.paranoia_bits & PARANOID_EATING) != 0)
/* Prevent going into lava or water without explicitly forcing it */
#define ParanoidSwim ((flags.paranoia_bits & PARANOID_SWIM) != 0)
/* Prevent going onto/into known trap unless it is harmless */
#define ParanoidTrap ((flags.paranoia_bits & PARANOID_TRAP) != 0)
/* Require confirmation for choosing 'A' in class menu for menustyle:Full */
#define ParanoidAutoAll ((flags.paranoia_bits & PARANOID_AUTOALL) != 0U)
/** @} */

/* command parsing, mainly dealing with number_pad handling;
   not saved and restored */

#ifdef NHSTDC
/* forward declaration sufficient to declare pointers */
struct ext_func_tab; /* from func_tab.h */
#endif

/**
 * @brief What kind of place the player is being asked to point at.
 *
 * When the game asks for a position it can cycle the cursor through the interesting candidates, and what counts as
 * interesting depends on why it is asking. Looking for a monster to name and looking for an unexplored corner want
 * different tours.
 *
 * @note @c GLOC_VALID is the widest -- any position at all that could be chosen -- so it is the fallback when the reason
 *       for asking implies no particular target.
 */
/**
 * @brief 플레이어가 어떤 종류의 곳을 가리키도록 요청받는지.
 *
 * 게임이 위치를 물을 때 커서를 흥미로운 후보들 사이로 돌릴 수 있고, 무엇이 흥미로운지는 무엇을 위해 묻는지에 달려 있다. 이름을 붙일 몬스터를 찾는 것과 탐사되지 않은 구석을 찾는 것은 서로 다른
 * 순회를 원한다.
 *
 * @note @c GLOC_VALID 가 가장 넓다. 고를 수 있는 아무 위치든. 그래서 묻는 이유가 특정 대상을 함축하지 않을 때의 기본값이다.
 */
enum gloctypes {
    GLOC_MONS = 0,
    GLOC_OBJS,
    GLOC_DOOR,
    GLOC_EXPLORE,
    GLOC_INTERESTING,
    GLOC_VALID,

    NUM_GLOCS
};

#endif /* FLAG_H */
