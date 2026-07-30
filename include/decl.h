/* NetHack 5.0  decl.h  $NHDT-Date: 1781973078 2026/06/20 16:31:18 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.408 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2007. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file decl.h
 * @brief Every global the game has, gathered and grouped.
 *
 * NetHack accumulated a great many globals over its history, scattered across the files that happened to need them. They have since been
 * collected here, and the collection has a shape worth understanding before reading it.
 *
 * The globals live in structures grouped by the first letter of their name. So a global called @c bases sits in the @c b structure and is
 * written @c gb.bases. That looks arbitrary, and it is -- the grouping carries no meaning at all. Its purpose is mechanical: a global is
 * findable by its own name, every reference to one is visibly a reference to a global, and adding one does not mean choosing which of
 * twenty-six structures it thematically belongs to.
 *
 * The important division is not the letters but the two families. The @c g structures hold engine state that is rebuilt each time the game
 * runs, and the @c sv structures hold state that is saved with the game. Which family a global is in decides whether it survives saving,
 * and that is the one thing about this file that has consequences.
 *
 * Above the structures sit the constants and tables that are not state at all: the names of colours, the strings the game says often enough
 * to be worth naming, the default symbol tables. Many are reached through short aliases so that a message can read as prose.
 *
 * @note Alongside the two families there is a small structure of pre-zeroed values. Clearing a structure by copying from it is safer than
 *       clearing it field by field, since the structures gain fields.
 * @note The comments inside the structures name the file each global came from. Some of the comments that travelled with them, as the
 *       existing note admits, no longer make sense away from their original surroundings.
 * @warning Putting a global in the wrong family is not a compile error. It is a value that silently fails to persist, or one that persists
 *          when it should have been rebuilt.
 */

/**
 * @file decl.h
 * @brief 게임이 가진 모든 전역 변수를 모아 묶은 것.
 *
 * NetHack 은 그 역사를 거치며 아주 많은 전역 변수를 쌓았고, 그것들은 마침 필요했던 파일들에 흩어져 있었다. 그 뒤로 여기에 모였으며, 그 모음에는 읽기 전에 이해해 둘 만한 형태가 있다.
 *
 * 전역 변수들은 이름의 첫 글자로 묶인 구조체 안에 산다. 그래서 @c bases 라는 전역 변수는 @c b 구조체에 있고 @c gb.bases 로 적힌다. 그것은 임의적으로 보이고, 실제로 임의적이다. 그 묶음에는 아무 의미도 없다. 목적은
 * 기계적이다. 전역 변수를 자기 이름으로 찾을 수 있고, 그것에 대한 모든 참조가 눈에 보이게 전역 참조이며, 하나를 더하는 일이 스물여섯 구조체 중 어디에 주제적으로 속하는지 고르는 일이 되지 않는다.
 *
 * 중요한 구분은 글자가 아니라 두 계열이다. @c g 구조체들은 게임이 실행될 때마다 다시 만들어지는 엔진 상태를 담고, @c sv 구조체들은 게임과 함께 저장되는 상태를 담는다. 어느 계열에 있는지가 그 전역 변수가 저장을 견디는지를
 * 정하며, 이 파일에서 결과를 낳는 것은 그 하나다.
 *
 * 구조체들 위에는 상태가 전혀 아닌 상수와 표들이 놓인다. 색의 이름, 게임이 이름 붙일 만큼 자주 말하는 문장, 기본 심볼 표. 많은 것이 짧은 별칭으로 접근되어 메시지가 산문처럼 읽히게 한다.
 *
 * @note 두 계열과 나란히 미리 0으로 채워진 값들의 작은 구조체가 있다. 그것에서 복사해 구조체를 비우는 것이 필드 하나하나를 비우는 것보다 안전하다. 구조체는 필드가 늘어나기 때문이다.
 * @note 구조체 안의 주석들은 각 전역 변수가 어느 파일에서 왔는지 밝힌다. 기존 주석이 인정하듯, 그것들과 함께 따라온 주석 중 일부는 원래의 주변에서 떨어져 나오면 더는 뜻이 통하지 않는다.
 * @warning 전역 변수를 잘못된 계열에 두는 것은 컴파일 오류가 아니다. 조용히 보존되지 못하는 값이거나, 다시 만들어져야 했는데 보존되는 값이다.
 */

#ifndef DECL_H
#define DECL_H

/**
 * @var c_obj_colors
 * @brief The colour words used to describe an unidentified object.
 * @note These are the words the player sees -- "ruby", "amber" -- not display colours. A potion's appearance is one of these, and which one
 *       is shuffled each game.
 */
/**
 * @var c_obj_colors
 * @brief 미확인 물건을 기술하는 데 쓰이는 색 낱말들.
 * @note 이들은 표시 색이 아니라 플레이어가 보는 낱말이다. "루비", "호박". 물약의 외형이 이 중 하나이며, 어느 것인지는 매 게임 섞인다.
 */
/* The names of the colors used for gems, etc. */
extern const char *c_obj_colors[];

/**
 * @name Script callbacks
 * @brief The names of the events a level script may hook, and how many hooks each has.
 * @note Names and counts are separate arrays indexed alike, so a script registering a hook increments a count without the name table being
 *       writable.
 * @{
 */
/**
 * @name 스크립트 콜백
 * @brief 레벨 스크립트가 걸 수 있는 사건들의 이름과, 각각에 걸린 개수.
 * @note 이름과 개수가 같은 방식으로 색인되는 별개의 배열이다. 그래서 스크립트가 훅을 등록할 때 이름 표가 쓰기 가능해지지 않고도 개수가 늘어난다.
 * @{
 */
/* lua callback queue names */
extern const char * const nhcb_name[];
extern int nhcb_counts[];
/** @} */

/**
 * @name Colour names
 * @brief The words for colours, held in one place and reached by alias.
 * @note Named rather than written as literals so that a colour word appears once in the binary and so that the same word is used everywhere --
 *       a message saying "amber" and another saying "orange-brown" for the same colour would be a bug the compiler cannot see.
 * @{
 */
/**
 * @name 색 이름
 * @brief 색을 가리키는 낱말들. 한곳에 보관되고 별칭으로 접근된다.
 * @note 리터럴로 적지 않고 이름을 붙인 것은, 색 낱말이 실행 파일에 한 번만 나오게 하고 어디서나 같은 낱말이 쓰이게 하기 위함이다. 같은 색에 대해 한 메시지는 "호박"이라 하고 다른 메시지는 "주황갈색"이라 하는 것은 컴파일러가
 *       볼 수 없는 버그다.
 * @{
 */
extern NEARDATA const struct c_color_names c_color_names;
#define NH_BLACK c_color_names.c_black
#define NH_AMBER c_color_names.c_amber
#define NH_GOLDEN c_color_names.c_golden
#define NH_LIGHT_BLUE c_color_names.c_light_blue
#define NH_RED c_color_names.c_red
#define NH_GREEN c_color_names.c_green
#define NH_SILVER c_color_names.c_silver
#define NH_BLUE c_color_names.c_blue
#define NH_PURPLE c_color_names.c_purple
#define NH_WHITE c_color_names.c_white
#define NH_ORANGE c_color_names.c_orange
/** @} */

/**
 * @name Common phrases
 * @brief Sentences the game says often enough to be worth naming.
 *
 * Held once and aliased so that "nothing happens" is the same sentence everywhere it is said. That matters more than it sounds: a player learns
 * to read these as signals, and two wordings of the same outcome would read as two different outcomes.
 *
 * @note @c fakename is not a phrase but a placeholder name, and the existing comment gives the reason it exists: a monster whose given name ends
 *       in "s" would otherwise fool the code that chooses between singular and plural verb forms.
 * @{
 */
/**
 * @name 자주 쓰는 문구
 * @brief 게임이 이름을 붙일 만큼 자주 말하는 문장들.
 *
 * 한 번만 보관되고 별칭으로 쓰여, "아무 일도 일어나지 않는다"가 말해지는 모든 곳에서 같은 문장이 되게 한다. 그것은 들리는 것보다 중요하다. 플레이어는 이 문장들을 신호로 읽는 것을 익히며, 같은 결과에 대한 두 가지 표현은 두 가지
 * 다른 결과로 읽힌다.
 *
 * @note @c fakename 은 문구가 아니라 자리 표시 이름이며, 기존 주석이 그것이 존재하는 이유를 밝힌다. 이름이 "s"로 끝나는 몬스터는 그러지 않으면 단수와 복수 동사형 중에서 고르는 코드를 속인다.
 * @{
 */
/* common_strings */
extern const struct c_common_strings c_common_strings;
#define nothing_happens c_common_strings.c_nothing_happens
#define nothing_seems_to_happen c_common_strings.c_nothing_seems_to_happen
#define thats_enough_tries c_common_strings.c_thats_enough_tries
#define silly_thing_to c_common_strings.c_silly_thing_to
#define shudder_for_moment c_common_strings.c_shudder_for_moment
#define something c_common_strings.c_something
#define Something c_common_strings.c_Something
#define You_can_move_again c_common_strings.c_You_can_move_again
#define Never_mind c_common_strings.c_Never_mind
#define vision_clears c_common_strings.c_vision_clears
#define the_your c_common_strings.c_the_your
/* fakename[] used occasionally so vtense() won't be fooled by an assigned
   name ending in 's' */
#define fakename c_common_strings.c_fakename
/** @} */

/**
 * @name Default symbols
 * @brief The built-in appearance of each object class and monster class.
 * @note Constant, and not the tables drawn from. The writable versions below are copied from these, so a player's symbol changes can always be
 *       undone.
 * @{
 */
/**
 * @name 기본 심볼
 * @brief 각 물건 계열과 몬스터 계열의 내장 외형.
 * @note 상수이며, 그릴 때 쓰는 표가 아니다. 아래의 쓰기 가능한 판본이 이것에서 복사되므로, 플레이어의 심볼 변경은 언제든 되돌릴 수 있다.
 * @{
 */
/* default object class symbols */
extern const struct class_sym def_oc_syms[MAXOCLASSES];

/* default mon class symbols */
extern const struct class_sym def_monsyms[MAXMCLASSES];
/** @} */

/**
 * @var disclosure_options
 * @brief The letters naming each kind of end-of-game disclosure.
 * @note The order matches the disclosure settings array in flag.h, so a letter and a setting correspond by position.
 */
/**
 * @var disclosure_options
 * @brief 게임 종료 시 공개의 각 종류를 지칭하는 글자들.
 * @note 순서가 flag.h 의 공개 설정 배열과 맞으므로, 글자와 설정이 위치로 대응된다.
 */
extern const char disclosure_options[];

/**
 * @var emptystr
 * @brief An empty string that is writable.
 * @note Exists because a function taking a writable buffer cannot be handed a literal. The existing comment records this; passing a literal would
 *       compile and then be written to.
 * @warning Writable and shared. Anything that writes into it affects every other user.
 */
/**
 * @var emptystr
 * @brief 쓰기 가능한 빈 문자열.
 * @note 쓰기 가능한 버퍼를 받는 함수에 리터럴을 건넬 수 없기 때문에 존재한다. 기존 주석이 이것을 기록한다. 리터럴을 넘기면 컴파일된 뒤 그것에 쓰게 된다.
 * @warning 쓰기 가능하고 공유된다. 여기에 쓰는 것은 다른 모든 사용자에게 영향을 준다.
 */
extern char emptystr[];

#ifdef WIN32
extern boolean fqn_prefix_locked[PREFIX_COUNT];
#endif
#ifdef PREFIXES_IN_USE
extern const char *fqn_prefix_names[PREFIX_COUNT];
#endif

extern NEARDATA boolean has_strong_rngseed;
extern struct engr *head_engr;

/**
 * @var hexdd
 * @brief The hexadecimal digits, in both cases.
 * @note Held once because four separate files need it, as the existing comment lists. Its length covers both cases plus a terminator.
 */
/**
 * @var hexdd
 * @brief 십육진 숫자들. 대소문자 둘 다.
 * @note 기존 주석이 나열하듯 네 개의 서로 다른 파일이 그것을 필요로 하므로 한 번만 보관된다. 그 길이는 두 대소문자와 종결자를 덮는다.
 */
/* used by coloratt.c, options.c, utf8map.c, windows.c */
extern const char hexdd[33];

/**
 * @var materialnm
 * @brief The word for each material an object can be made of.
 * @note Indexed by material, so the order matches the material enumeration in objclass.h and cannot be sorted.
 */
/**
 * @var materialnm
 * @brief 물건이 만들어질 수 있는 각 재질을 가리키는 낱말.
 * @note 재질로 색인되므로 순서가 objclass.h 의 재질 열거와 맞아야 하고 정렬할 수 없다.
 */
/* material strings */
extern const char *materialnm[];

/**
 * @name Symbols in use
 * @brief The symbol tables actually drawn from, after any changes the player made.
 * @note Writable, unlike the default tables above. These are what a symbol set or a customisation modifies, and the defaults are what they are
 *       restored from.
 * @{
 */
/**
 * @name 사용 중인 심볼
 * @brief 플레이어가 한 변경이 반영된 뒤 실제로 그릴 때 쓰이는 심볼 표.
 * @note 위의 기본 표와 달리 쓰기 가능하다. 심볼 세트나 사용자 지정이 바꾸는 것이 이것이고, 그것을 되돌릴 때 쓰는 것이 기본 표다.
 * @{
 */
/* current mon class symbols */
extern uchar monsyms[MAXMCLASSES];

/* current object class symbols */
extern uchar oc_syms[MAXOCLASSES];
/** @} */

/**
 * @var quitchars
 * @brief The keys that mean "never mind" at a prompt.
 * @note Several rather than one, since a player may reach for escape, a space or a return depending on what they think the prompt is. Accepting
 *       all of them is what makes cancelling reliable.
 */
/**
 * @var quitchars
 * @brief 프롬프트에서 "그만두겠다"를 뜻하는 키들.
 * @note 하나가 아니라 여럿인 것은, 플레이어가 그 프롬프트를 무엇이라 여기는지에 따라 escape 나 공백이나 return 으로 손을 뻗기 때문이다. 그 전부를 받아들이는 것이 취소를 믿을 수 있게 만드는 것이다.
 */
extern const char quitchars[];
extern NEARDATA char tune[6];
extern const schar xdir[], ydir[], zdir[], dirs_ord[];
extern const char vowels[];
extern const char ynchars[];
extern const char ynqchars[];
extern const char ynaqchars[];
extern const char ynNaqchars[];
extern const char rightleftchars[];
extern const char hidespinchars[];
extern NEARDATA long yn_number;
extern struct restore_info restoreinfo;
extern NEARDATA struct savefile_info sfcap, sfrestinfo, sfsaveinfo;
extern const int shield_static[];

extern NEARDATA struct obj *uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
    *uarmu, /* under-wear, so to speak */
    *uskin, *uamul, *uleft, *uright, *ublindf, *uwep, *uswapwep, *uquiver;
extern NEARDATA struct obj *uchain; /* defined only when punished */
extern NEARDATA struct obj *uball;
extern NEARDATA struct you u;
extern NEARDATA time_t ubirthday;
extern NEARDATA struct u_realtime urealtime;

/* Window system stuff */
extern NEARDATA winid WIN_MESSAGE;
extern NEARDATA winid WIN_STATUS;
extern NEARDATA winid WIN_MAP, WIN_INVEN;

/**
 * @brief The few terminal facts the core itself needs.
 *
 * Declared here as well as in the terminal header, as the existing comment records, so that code which does not include the terminal machinery can
 * still ask how many lines there are. The duplicate declaration is guarded so that including both does not conflict.
 *
 * @note The two string members are the sequences that switch a terminal into and out of its line-drawing font. They are held rather than emitted
 *       directly because a terminal that has neither must be able to supply nothing.
 */
/**
 * @brief 코어 자신이 필요로 하는 몇 가지 터미널 사실.
 *
 * 기존 주석이 기록하듯 터미널 헤더와 함께 여기에도 선언된다. 그래서 터미널 기제를 포함하지 않는 코드도 줄이 몇 개인지 물을 수 있다. 그 중복 선언은 둘 다 포함해도 충돌하지 않도록 보호되어 있다.
 *
 * @note 두 문자열 멤버는 터미널을 선 그리기 글꼴로 넣고 빼는 열이다. 직접 내보내는 대신 보관되는 것은, 둘 다 없는 터미널이 아무것도 제공하지 않을 수 있어야 하기 때문이다.
 */
#ifndef TCAP_H
extern struct tc_gbl_data {   /* also declared in tcap.h */
    char *tc_AS, *tc_AE; /* graphics start and end (tty font swapping) */
    int tc_LI, tc_CO;    /* lines and columns */
} tc_gbl_data;
#define AS gt.tc_gbl_data.tc_AS
#define AE gt.tc_gbl_data.tc_AE
#define LI gt.tc_gbl_data.tc_LI
#define CO gt.tc_gbl_data.tc_CO
#endif

#ifdef PANICTRACE
extern const char *ARGV0;
#endif

/**
 * @brief Requests that the status line needs redrawing, and how much of it.
 *
 * Three levels rather than one flag, because the status line is redrawn far more often than anything on it changes. Asking for the least that will
 * do is what keeps the display quiet -- which matters especially to a player using a screen reader, for whom a redraw is something spoken.
 *
 * @note The narrowest of the three is for the turn counter alone, since that changes every move and nothing else usually does.
 */
/**
 * @brief 상태줄을 다시 그려야 한다는 요청과, 그 중 얼마만큼인지.
 *
 * 플래그 하나가 아니라 세 단계인 것은, 상태줄이 그 위의 무엇이 바뀌는 것보다 훨씬 자주 다시 그려지기 때문이다. 충분한 만큼 중 가장 적은 것을 요청하는 것이 표시부를 조용하게 유지하는 것이며, 그것은 특히 화면 읽기 프로그램을 쓰는
 * 플레이어에게 중요하다. 그에게 다시 그리기는 말해지는 것이다.
 *
 * @note 셋 중 가장 좁은 것은 턴 계수기만을 위한 것이다. 그것은 매 걸음 바뀌고 다른 것은 보통 그렇지 않다.
 */
struct display_hints {
    boolean botl;            /* partially redo status line */
    boolean botlx;           /* print an entirely new bottom line */
    boolean time_botl;       /* context.botl for 'time' (moves) only */
};
extern struct display_hints disp;

/*
 * 'gX' -- instance_globals holds engine state that does not need to be
 * persisted upon game exit.  The initialization state is well defined
 * and set in decl.c during early early engine initialization.
 *
 * Unlike instance_flags, values in the structure can be of any type.
 *
 * Pulled from other files to be grouped in one place.  Some comments
 * which came with them don't make much sense out of their original context.
 */

/**
 * @name Engine globals
 * @brief Globals that are rebuilt each time the game runs, grouped by first letter.
 *
 * Twenty-six structures, one per initial letter, and the grouping means nothing. What it buys is that a global is found by its own name, that every
 * use of one is visibly a global, and that adding one requires no decision.
 *
 * These are not saved. Their starting values are set during early initialisation, as the existing comment records, and that is the whole of their
 * lifecycle -- so anything here that ought to survive a save is in the wrong family.
 *
 * @note Unlike the option flags, these may be of any type. That is the point of the arrangement: it collects globals without forcing them into a
 *       common shape.
 * @{
 */
/**
 * @name 엔진 전역 변수
 * @brief 게임이 실행될 때마다 다시 만들어지는 전역 변수들. 첫 글자로 묶여 있다.
 *
 * 첫 글자마다 하나씩 스물여섯 구조체이며, 그 묶음에는 아무 의미가 없다. 그것이 사 주는 것은, 전역 변수를 자기 이름으로 찾을 수 있다는 것, 그것을 쓰는 모든 곳이 눈에 보이게 전역이라는 것, 그리고 하나를 더하는 데 아무 결정도 필요하지
 * 않다는 것이다.
 *
 * 이들은 저장되지 않는다. 기존 주석이 기록하듯 초기 초기화 중에 시작값이 정해지며, 그것이 그 수명의 전부다. 그래서 여기 있는 것 중 저장을 견뎌야 하는 것은 잘못된 계열에 있는 것이다.
 *
 * @note 옵션 플래그와 달리 이들은 어떤 타입이든 될 수 있다. 그것이 이 배치의 요점이다. 전역 변수들을 공통된 모양으로 억지로 맞추지 않고 모은다.
 * @{
 */
struct instance_globals_a {
    /* decl.c */
    int (*afternmv)(void);

    /* detect.c */
    int already_found_flag; /* used to augment first "already found a monster"
                             * message if 'cmdassist' is Off */
    /* do.c */
    boolean at_ladder;

    /* dog.c */
    struct autopickup_exception *apelist;

    /* end.c */
    struct valuable_data amulets[LAST_AMULET + 1 - FIRST_AMULET];

    /* mon.c */
    short *animal_list; /* list of PM values for animal monsters */
    int animal_list_count;

#ifdef CHANGE_COLOR
    /* options.c */
    uint32 altpalette[CLR_MAX];
#endif

    /* pickup.c */
    int A_first_hint; /* menustyle:Full plus 'A' response + !paranoid:A */
    int A_second_hint; /* menustyle:Full plus 'A' response + paranoid:A */
    boolean abort_looting;

    /* shk.c */
    boolean auto_credit;

    /* sounds.c */
    enum soundlib_ids active_soundlib;

    /* trap.c */
    /* context for water_damage(), managed by water_damage_chain();
        when more than one stack of potions of acid explode while processing
        a chain of objects, use alternate phrasing after the first message */
    struct h2o_ctx acid_ctx;

    boolean havestate;
};

struct instance_globals_b {

    /* botl.c */
    struct istat_s blstats[2][MAXBLSTATS];
    boolean blinit;
#ifdef STATUS_HILITES
    long bl_hilite_moves;
#endif

    /* decl.c */
    coord bhitpos; /* place where throw or zap hits or stops */
    struct obj *billobjs; /* objects not yet paid for */

    /* files.c */
    char bones[BONESSIZE];

    /* hack.c */
    unsigned bldrpush_oid; /* id of last boulder pushed */
    long bldrpushtime;     /* turn that a message was given for pushing
                            * a boulder; used in lieu of Norep() */

    /* mkmaze.c */
    lev_region bughack; /* for preserving the insect legs when wallifying
                         * baalz level */

    /* pickup.c */
    boolean bucx_filter;

    /* zap.c */
    struct monst *buzzer; /* zapper/caster/breather who initiates buzz() */

    /* new */
    boolean bot_disabled;

    boolean havestate;
};

struct instance_globals_c {

    struct _cmd_queue *command_queue[NUM_CQS];

    /* botl.c */
    unsigned long cond_hilites[BL_ATTCLR_MAX];
    int condmenu_sortorder;

    /* cmd.c */
    struct cmd Cmd; /* flag.h */
    /* Provide a means to redo the last command.  The flag `in_doagain'
       (decl.c below) is set to true while redoing the command.  This flag
       is tested in commands that require additional input (like `throw'
       which requires a thing and a direction), and the input prompt is
       not shown.  Also, while in_doagain is TRUE, no keystrokes can be
       saved into the saveq. */
    coord clicklook_cc;
    /* decl.c */
    char chosen_windowtype[WINTYPELEN];
    int cmd_key; /* parse() / rhack() */
    struct Cmd_bind *cmd_bind;
    cmdcount_nht command_count;
    /* some objects need special handling during destruction or placement */
    struct obj *current_wand;  /* wand currently zapped/applied */
#ifdef DEF_PAGER
    const char *catmore; /* external pager; from getenv() or DEF_PAGER */
#endif

    /* dog.c */
    char catname[PL_PSIZ];

    /* end.c */
    char *crash_email;  // email for crash reports
    char *crash_name;   // human name for crash reports
    int crash_urlmax;   // maximum length for the url of a crash report

    /* symbols.c */
    int currentgraphics;

    /* files.c, cfgfiles.c */
    char *cmdline_rcfile;  /* set in unixmain.c, used in options.c */
    char *config_section_chosen;
    char *config_section_current;
    boolean chosen_symset_start;
    boolean chosen_symset_end;

    /* invent.c */
    /* for perm_invent when operating on a partial inventory display, so that
       persistent one doesn't get shrunk during filtering for item selection
       then regrown to full inventory, possibly being resized in process */
    winid cached_pickinv_win;
    int core_invent_state;

    /* options.c */
    char *cmdline_windowsys; /* set in unixmain.c */
    struct menucoloring *color_colorings; /* alternate set of menu colors */

    /* pickup.c */
    /* current_container is set in use_container(), to be used by the
       callback routines in_container() and out_container() from askchain()
       and use_container().  Also used by menu_loot() and container_gone(). */
    struct obj *current_container;
    boolean class_filter;

    /* questpgr.c */
    char cvt_buf[CVT_BUF_SIZE];

    /* sounds.c */
    enum soundlib_ids chosen_soundlib;

    /* sp_lev.c */
    struct sp_coder *coder;

    /* uhitm.c */
    short corpsenm_digested; /* monster type being digested, set by gulpum */

    /* zap.c */
    /* new */
    boolean converted_savefile_loaded;

    boolean havestate;
};

struct instance_globals_d {

    /* decl.c */
    int doorindex;
    long done_money;
    long domove_attempting;
    long domove_succeeded;
#define DOMOVE_WALK         0x00000001
#define DOMOVE_RUSH         0x00000002
    boolean defer_see_monsters;

    /* dig.c */
    boolean did_dig_msg;

    /* do.c */
    char *dfr_pre_msg;  /* pline() before level change */
    char *dfr_post_msg; /* pline() after level change */
    int did_nothing_flag; /* to augment the no-rest-next-to-monster message */

    /* dog.c */
    char dogname[PL_PSIZ];

    /* end.c */
    long done_seq; /* for counting deaths occurring on same hero_seq */

    /* mon.c */
    boolean disintegested;

    /* objname.c */
    /* distantname used by distant_name() to pass extra information to
       xname_flags(); it would be much cleaner if this were a parameter,
       but that would require all xname() and doname() calls to be modified */
    int distantname;

    /* pickup.c */
    boolean decor_fumble_override;
    boolean decor_levitate_override;

    /* new */
    boolean deferred_showpaths;
    char *deferred_showpaths_dir;
    boolean disable_glyphname_hash_indices_prefill;

    boolean havestate;
};

struct instance_globals_e {

    /* cmd.c */
    winid en_win;
    boolean en_via_menu;
    struct ext_func_tab *ext_tlist; /* info for rhack() from doextcmd() */

    /* eat.c */
    char *eatmbuf; /* set by cpostfx() */

    /* mkmaze.c */
    struct bubble *ebubbles;

    /* new stuff */
    int early_raw_messages;   /* if raw_prints occurred early prior
                                 to gb.beyond_savefile_load */

    boolean havestate;
};

struct instance_globals_f {

    /* decl.c */
    struct trap *ftrap;
    char *fqn_prefix[PREFIX_COUNT];
    struct fruit *ffruit;

    /* eat.c */
    boolean force_save_hs;

    /* mhitm.c */
    boolean far_noise;

    /* rumors.c */
    long false_rumor_size;
    unsigned long false_rumor_start;
    long false_rumor_end;

    /* shk.c */
    long int followmsg; /* last time of follow message */

    boolean havestate;
};

struct instance_globals_g {

    /* display.c */
    gbuf_entry gbuf[ROWNO][COLNO];
    coordxy gbuf_start[ROWNO];
    coordxy gbuf_stop[ROWNO];

    /* do_name.c */
    coordxy getposx, getposy; /* cursor position in case of async resize */
    struct selectionvar *gloc_filter_map;
    int gloc_filter_floodfill_match_glyph;

    /* dog.c */
    xint16 gtyp;  /* type of dog's current goal */
    coordxy gx; /* x position of dog's current goal */
    coordxy gy; /* y position of dog's current goal */

    /* dokick.c */
    const char *gate_str;

    /* end.c */
    /* 1st +1: subtracting first from last, 2nd +1: one slot for all glass */
    struct valuable_data gems[LAST_REAL_GEM + 1 - FIRST_REAL_GEM + 1];

    /* invent.c */
    long glyph_reset_timestamp;

    /* nhlua.c */
    boolean gmst_stored;
    long gmst_moves;
    struct obj *gmst_invent;
    genericptr_t *gmst_ubak, *gmst_disco, *gmst_mvitals;
    struct spell gmst_spl_book[MAXSPELL + 1];

    /* pline.c */
    struct gamelog_line *gamelog;

    /* region.c */
    boolean gas_cloud_diss_within;
    int gas_cloud_diss_seen;

    /* new stuff */
    /* per-level glyph mapping flags */
    long glyphmap_perlevel_flags;

    boolean havestate;
};

struct instance_globals_h {

    /* decl.c */
    const char *hname; /* name of the game (argv[0] of main) */
#if defined(MICRO) || defined(WIN32)
    char hackdir[PATHLEN]; /* where rumors, help, record are */
#endif /* MICRO || WIN32 */
    long hero_seq; /* 'moves*8 + n' where n is updated each hero move during
                    * the current turn */

    /* dog.c */
    char horsename[PL_PSIZ];

    /* mhitu.c */
    unsigned hitmsg_mid;
    struct attack *hitmsg_prev;

    boolean havestate;
};

struct instance_globals_i {

    /* decl.c */
    int in_doagain;
    boolean in_mklev;
    boolean in_steed_dismounting;
    struct obj *invent;

    /* do_wear.c */
    /* starting equipment gets auto-worn at beginning of new game,
       and we don't want stealth or displacement feedback then */
    boolean initial_don; /* manipulated in set_wear() */

    /* invent.c */
    char *invbuf;
    unsigned invbufsiz;
    boolean item_action_in_progress;
    int in_sync_perminvent;

    /* mon.c */
    struct monst **itermonarr; /* temporary array of all N monsters
                                * on the current level */

    /* restore.c */
    struct bucket *id_map;

    /* sp_lev.c */
    boolean in_mk_themerooms;

    /* new */

    boolean havestate;
};

struct instance_globals_j {

    /* apply.c */
    int jumping_is_magic; /* current jump result of magic */

    boolean havestate;
};

struct instance_globals_k {

    coord kickedloc; /* location hero just kicked */

    /* decl.c */
    struct obj *kickedobj;     /* object in flight due to kicking */

    /* read.c */
    boolean known;

    boolean havestate;
};

struct instance_globals_l {

    /* cmd.c */
    cmdcount_nht last_command_count;

    /* decl.c (before being incorporated into instance_globals_*) */
#if defined(UNIX) || defined(VMS)
    int locknum; /* max num of simultaneous users */
#endif
#ifdef MICRO
    char levels[PATHLEN]; /* where levels are */
#endif /* MICRO */

    /* files.c */
    int lockptr;
    char lock[LOCKNAMESIZE];

    /* invent.c */
    int lastinvnr;  /* 0 ... 51 (never saved&restored) */

    /* light.c */
    light_source *light_base;

    /* mklev.c */
    genericptr_t luathemes[MAXDUNGEON];

    /* mon.c */
    unsigned last_hider; /* m_id of hides-under mon seen going into hiding */

    /* nhlan.c */
#ifdef MAX_LAN_USERNAME
    char lusername[MAX_LAN_USERNAME];
    int lusername_size;
#endif

    /* nhlua.c */
    genericptr_t luacore; /* lua_State * */
    char lua_warnbuf[BUFSZ];
    int loglua;
    int lua_sid;

    /* options.c */
    boolean loot_reset_justpicked;

    /* save.c */
    struct obj *looseball;  /* track uball during save and... */
    struct obj *loosechain; /* track uchain since saving might free it */

    /* sp_lev.c */
    char *lev_message;
    lev_region *lregions;

    /* trap.c */
    struct launchplace launchplace;

    /* windows.c */
    struct win_choices *last_winchoice;

    /* new stuff */
    char lua_ver[LUA_VER_BUFSIZ];
    char lua_copyright[LUA_COPYRIGHT_BUFSIZ];

    boolean havestate;
};

struct instance_globals_m {

    /* apply.c */
    int mkot_trap_warn_count;

    /* botl.c */
    int mrank_sz; /* loaded by max_rank_sz */

    /* decl.c */
    cmdcount_nht multi;
    const char *multi_reason;
    char multireasonbuf[QBUFSZ]; /* note: smaller than usual [BUFSZ] */
    /* for xname handling of multiple shot missile volleys:
       number of shots, index of current one, validity check, shoot vs throw */
    struct multishot m_shot;
    boolean mrg_to_wielded; /* weapon picked is merged with wielded one */
    struct menucoloring *menu_colorings;
    struct obj *migrating_objs; /* objects moving to another dungeon level */

    /* dog.c */
    struct monst *mydogs; /* monsters that went down/up together with @ */
    struct monst *migrating_mons; /* monsters moving to another level */

    /* dokick.c */
    struct rm *maploc;

    /* mhitm.c */
    struct monst *mswallower; /* for gas spore explosion when it's swallowed*/

    /* mhitu.c */
    int mhitu_dieroll;

    /* mklev.c */
    boolean made_branch; /* used only during level creation */

    /* mkmap.c */
    coordxy min_rx; /* rectangle bounds for regions */
    coordxy max_rx;
    coordxy min_ry;
    coordxy max_ry;

    /* mkobj.c */
    boolean mkcorpstat_norevive; /* for trolls */

    /* mthrowu.c */
    int mesg_given; /* for m_throw()/thitu() 'miss' message */
    struct monst *mtarget;  /* monster being shot by another monster */
    struct monst *marcher; /* monster that is shooting */

    /* muse.c */
    boolean m_using; /* kludge to use mondied instead of killed */
    struct musable m;

    /* options.c */
    /* Allow the user to map incoming characters to various menu commands. */
    char mapped_menu_cmds[MAX_MENU_MAPPED_CMDS + 1]; /* exported */
    char mapped_menu_op[MAX_MENU_MAPPED_CMDS + 1];

    /* region.c */
    int max_regions;

    /* trap.c */
    boolean mentioned_water; /* set to True by water_damage() if it issues
                              * a message about water; dodip() should make
                              * POT_WATER should become discovered */

    boolean havestate;
};

struct instance_globals_n {

    /* botl.c */
    int now_or_before_idx;   /* 0..1 for array[2][] first index */

    /* decl.c */
    const char *nomovemsg;
    int nsubroom;

    /* dokick.c */
    struct rm nowhere;

    /* files.c */
    int nesting;
    int no_sound_notified; /* run-time option processing: warn once if built
                            * without USER_SOUNDS and config file contains
                            * SOUND=foo or SOUNDDIR=bar */

    /* mhitm.c */
    long noisetime;

    /* mkmap.c */
    char *new_locations;
    int n_loc_filled;

    /* options.c */
    short n_menu_mapped;

    /* potion.c */
    boolean notonhead; /* for long worms */

    /* questpgr.c */
    char nambuf[CVT_BUF_SIZE];

    /* restore.c */
    int n_ids_mapped;

    /* sp_lev.c */
    int num_lregions;

    /* u_init.c */
    short nocreate;
    short nocreate2;
    short nocreate3;
    short nocreate4;

    boolean havestate;
};

struct instance_globals_o {

    struct obj *objs_deleted;

    /* dbridge.c */
    struct entity occupants[ENTITIES];

    /* decl.c */
    int (*occupation)(void);
    int occtime;
    int otg_temp; /* used by object_to_glyph() [otg] */
    struct obj *otg_otmp; /* used by obj_is_piletop() */
    const char *occtxt; /* defined when occupation != NULL */

    /* symbols.c */
    nhsym ov_primary_syms[SYM_MAX];   /* loaded primary symbols          */
    nhsym ov_rogue_syms[SYM_MAX];   /* loaded rogue symbols           */

    /* invent.c */
    /* query objlist callback: return TRUE if obj is at given location */
    coord only;

    /* o_init.c */
    short oclass_prob_totals[MAXOCLASSES];

    /* options.c */

    /* builtin_opt, syscf_, rc_file_, environ_, play_opt */
    enum option_phases opt_phase;
    boolean opt_initial;
    boolean opt_from_file;
    boolean opt_need_redraw; /* for doset() */
    boolean opt_need_glyph_reset;
    boolean opt_need_promptstyle;
    boolean opt_reset_customcolors;
    boolean opt_reset_customsymbols;
    boolean opt_update_basic_palette;
    boolean opt_symset_changed;

    /* pickup.c */
    int oldcap; /* last encumbrance */

    /* restore.c */
    struct fruit *oldfruit;

    /* rumors.c */
    int oracle_flg; /* -1=>don't use, 0=>need init, 1=>init done */

    /* uhitm.c */
    boolean override_confirmation; /* Used to flag attacks caused by
                                    * Stormbringer's maliciousness. */
    /* zap.c */
    boolean obj_zapped;

    boolean havestate;
};

struct instance_globals_p {

    /* apply.c */
    int polearm_range_min;
    int polearm_range_max;

    /* decl.c */
    int plnamelen; /* length of plname[] if that came from getlogin() */
    char pl_race; /* character's race */
    struct plinemsg_type *plinemsg_types;

    /* dog.c */
    int petname_used; /* user preferred pet name has been used */
    char preferred_pet; /* '\0', 'c', 'd', 'n' (none) */

    /* symbols.c */
    nhsym primary_syms[SYM_MAX];   /* loaded primary symbols          */

    /* invent.c */
    int perm_invent_toggling_direction;

    /* pickup.c */
    boolean picked_filter;
    int pickup_encumbrance; /* when picking up multiple items in a single
                             * operation, encumbrance after previous item */

    /* pline.c */
    unsigned pline_flags;
    char prevmsg[BUFSZ];

    /* potion.c */
    int potion_nothing;
    int potion_unkn;

    /* pray.c */
    /* values calculated when prayer starts, and used when completed */
    aligntyp p_aligntyp;
    int p_trouble;
    int p_type; /* (-1)-3: (-1)=really naughty, 3=really good */

    /* weapon.c */
    struct obj *propellor;

    /* zap.c */
    int  poly_zapped;

    /* new stuff */
    int puzzling_criteria;
    char puzzling_ilets[invlet_basic + 1];

    boolean havestate;
};

struct instance_globals_q {

    boolean havestate;
};

struct instance_globals_r {

    /* symbols.c */
    nhsym rogue_syms[SYM_MAX];   /* loaded rogue symbols           */

    /* extralev.c */
    struct rogueroom r[3][3];

    /* mkmaze.c */
    boolean ransacked;

    /* region.c */
    NhRegion **regions;

    /* rip.c */
    char **rip;

    /* role.c */
    char role_pa[NUM_BP];
    char role_post_attribs;
    struct role_filter rfilter;

    /* shk.c */
    struct repo repo;

    boolean havestate;
};

struct instance_globals_s {

    /* artifact.c */
    int spec_dbon_applies; /* coordinate effects from spec_dbon() with
                              messages in artifact_hit() */

    /* decl.c */
    stairway *stairs;
    int smeq[MAXNROFROOMS + 1];
    boolean stoned; /* done to monsters hit by 'c' */
    struct mkroom *subrooms;

    /* do.c */
    d_level save_dlevel; /* ? [even back in 3.4.3, only used in bones.c] */

    /* symbols.c */
    struct symsetentry symset[NUM_GRAPHICS];
    /* adds UNICODESET */
    struct symset_customization
        sym_customizations[NUM_GRAPHICS + 1][custom_count];
    nhsym showsyms[SYM_MAX]; /* symbols to be displayed */

    /* files.c */
    int symset_count;             /* for pick-list building only */
    int symset_which_set;
    /* SAVESIZE, BONESSIZE, LOCKNAMESIZE are defined in "fnamesiz.h" */
    char SAVEF[SAVESIZE]; /* relative path of save file from playground */
#ifdef MICRO
    char SAVEP[SAVESIZE]; /* holds path of directory for save file */
#endif

    /* invent.c */
    unsigned sortlootmode; /* set by sortloot() for use by sortloot_cmp();
                            * reset by sortloot when done */
    /* mhitm.c */
    boolean skipdrin; /* mind flayer against headless target */

    /* mon.c */
    boolean somebody_can_move;

    /* options.c */
    struct symsetentry *symset_list; /* files.c will populate this with
                                      * list of available sets */
    boolean save_menucolors; /* copy of iflags.use_menu_colors */
    struct menucoloring *save_colorings; /* copy of gm.menu_colorings */
    boolean simple_options_help;

    /* pickup.c */
    boolean sellobj_first; /* True => need sellobj_state(); False => don't */
    boolean shop_filter;

    /* pline.c */
#ifdef DUMPLOG_CORE
    unsigned saved_pline_index;  /* slot in saved_plines[] to use next */
    char *saved_plines[DUMPLOG_MSG_COUNT];
#endif

    /* polyself.c */
    int sex_change_ok; /* controls whether taking on new form or becoming new
                          man can also change sex (ought to be an arg to
                          polymon() and newman() instead) */

    /* shk.c */
    /* auto-response flag for/from "sell foo?" 'a' => 'y', 'q' => 'n' */
    char sell_response;
    int sell_how;

    /* spells.c */
    int spl_sortmode;   /* index into spl_sortchoices[] */
    int *spl_orderindx; /* array of svs.spl_book[] indices */

    /* steal.c */
    unsigned int stealoid; /* object to be stolen */
    unsigned int stealmid; /* monster doing the stealing */

    /* vision.c */
    int seethru; /* 'bubble' debugging: clouds and water don't block light */

    boolean havestate;
};

struct instance_globals_t {

    /* apply.c */
    struct trapinfo trapinfo;

    /* decl.c */
    schar tbx;  /* mthrowu: target x */
    schar tby;  /* mthrowu: target y */
    char toplines[TBUFSZ];
    struct obj *thrownobj;     /* object in flight due to throwing */
    /* Windowing stuff that's really tty oriented, but present for all ports */
    struct tc_gbl_data tc_gbl_data; /* AS,AE, LI,CO */

    /* hack.c */
    anything tmp_anything;
    struct selectionvar *travelmap;

    /* invent.c */
    /* query objlist callback: return TRUE if obj type matches "this_type" */
    int this_type;
    const char *this_title; /* title for inventory list of specific type */

    /* muse.c */
    coordxy trapx;
    coordxy trapy;

    /* rumors.c */
    long true_rumor_size; /* rumor size variables are signed so that value -1
                           * can be used as a flag */
    unsigned long true_rumor_start; /* rumor start offsets are unsigned due
                                     * to use of %lx format */
    long true_rumor_end; /* rumor end offsets are signed because they're
                          * compared with [dlb_]ftell() */

    /* sp_lev.c */
    boolean themeroom_failed;

    /* timeout.c */
    /* ordered timer list */
    struct fe *timer_base; /* "active" */

    /* topten.c */
    winid toptenwin;

    /* uhitm.c */
    int twohits; /* 0: single hit; 1: first of 2; 2: second of 2 */

    boolean havestate;
};

struct instance_globals_u {

    /* botl.c */
    boolean update_all;

    /* decl.c */
    boolean unweapon;

        /* revision.c */
    int uplift_needed_rev0_to_rev1;

    /* role.c */
    struct Role urole; /* player's role. May be munged in role_init() */
    struct Race urace; /* player's race. May be munged in role_init() */

    /* save.c */
    d_level uz_save;

    /* new stuff */
    boolean havestate;
};

struct instance_globals_v {

    /* botl.c */
    boolean valset[MAXBLSTATS];

    /* end.c */
    struct val_list valuables[3];

    /* mhitm.c */
    boolean vis;

    /* mklev.c */
    coordxy vault_x;
    coordxy vault_y;

    /* mon.c */
    boolean vamp_rise_msg;

    /* pickup.c */
    long val_for_n_or_more;
    /* list of menu classes for query_objlist() and allow_category callback
       (with room for all object classes, 'u'npaid, BUCX, and terminator) */
    char valid_menu_classes[MAXOCLASSES + 1 + 4 + 1];

    /* vision.c */
    seenV **viz_array;   /* used in cansee() and couldsee() macros */
    coordxy *viz_rmin;   /* min could see indices */
    coordxy *viz_rmax;   /* max could see indices */
    boolean vision_full_recalc;

    /* new stuff */
    struct sound_voice voice;

    boolean havestate;
};

struct instance_globals_w {

    /* decl.c */
    int warn_obj_cnt; /* count of monsters meeting criteria */
    long wailmsg;

    /* do_wear.c */
    uint8 wasinwater;

    /* symbols.c */
    nhsym warnsyms[WARNCOUNT]; /* the current warning display symbols */

    /* files.c */
    char wizkit[WIZKIT_MAX];

    /* hack.c */
    int wc; /* current weight_cap(); valid after call to inv_weight() */

    /* mkmaze.c */
    struct trap *wportal;

    /* new */
    struct win_settings wsettings;      /* wintype.h */
    long were_changes;                  /* were.c, allmain.c */

    boolean havestate;
};

struct instance_globals_x {

    /* decl.c */
    int x_maze_max;

    /* lock.c */
    struct xlock_s xlock;

    /* objnam.c */
    char *xnamep; /* obuf[] returned by xname(), for use in doname() for
                   * bounds checking; differs from xname() return value
                   * due to reserving PREFIX bytes at start and possibly
                   * skipping leading "the " after constructing result */

    /* sp_lev.c */
    coordxy xstart, xsize;

    boolean havestate;
};

struct instance_globals_y {

    /* decl.c */
    int y_maze_max;
    struct monst youmonst;

    /* pline.c */
    /* work buffer for You(), &c and verbalize() */
    char *you_buf;
    int you_buf_siz;

    /* sp_lev.c */
    coordxy ystart, ysize;

    boolean havestate;
};

struct instance_globals_z {

    /* mon.c */
    boolean zombify;

    /* muse.c */
    boolean zap_oseen; /* for wands which use mbhitm and are zapped at
                        * players.  We usually want an oseen local to
                        * the function, but this is impossible since the
                        * function mbhitm has to be compatible with the
                        * normal zap routines, and those routines don't
                        * remember who zapped the wand. */

    boolean havestate;
};

/** @} */

/**
 * @name Saved globals
 * @brief Globals that are written into the saved game, grouped the same way.
 *
 * The same alphabetical arrangement, and the same lack of meaning in it. The difference is the one that matters: everything in this family is part of
 * the saved game, so its type and layout are part of the save format.
 *
 * A global belongs here if losing it would lose something about the game rather than about this run -- the dungeon's shape, the state of every level,
 * the hero's spells and discoveries.
 *
 * @warning Adding a member changes the save format. Moving a global from the engine family to this one does too, and neither is a compile error.
 * @{
 */
/**
 * @name 저장되는 전역 변수
 * @brief 저장 게임에 기록되는 전역 변수들. 같은 방식으로 묶여 있다.
 *
 * 같은 알파벳 배치이고, 그 배치에 의미가 없는 것도 같다. 다른 점은 중요한 그 한 가지다. 이 계열의 모든 것이 저장 게임의 일부이므로, 그 타입과 배치가 저장 형식의 일부다.
 *
 * 어떤 전역 변수가 여기 속하는 것은, 그것을 잃는 것이 이번 실행에 관한 무엇이 아니라 그 게임에 관한 무엇을 잃는 경우다. 던전의 모양, 모든 레벨의 상태, 영웅의 주문과 감별.
 *
 * @warning 멤버를 더하는 것은 저장 형식을 바꾼다. 전역 변수를 엔진 계열에서 이 계열로 옮기는 것도 그렇고, 어느 쪽도 컴파일 오류가 아니다.
 * @{
 */
struct instance_globals_saved_b {
    /* dungeon.c */
    branch *branches; /* dungeon branch list */
    /* mkmaze.c */
    struct bubble *bbubbles;
    /* o_init.c */
    int bases[MAXOCLASSES + 2]; /* make bases[MAXOCLASSES+1] available */
};

struct instance_globals_saved_c {
    /* decl.c */
    struct context_info context;
};

struct instance_globals_saved_d {
    /* dungeon.c */
    dungeon dungeons[MAXDUNGEON]; /* ini'ed by init_dungeon() */
    struct dgn_topology dungeon_topology;
    /* decl.c */
    dest_area dndest;
    coord *doors; /* array of door locations */
    int doors_alloc; /* doors-array allocated size */
    /* o_init.c */
    short disco[NUM_OBJECTS];
};

struct instance_globals_saved_e {
    /* decl.c */
    struct exclusion_zone *exclusion_zones;
};

struct instance_globals_saved_h {
    /* decl.c */
    int hackpid; /* current process id */
};

struct instance_globals_saved_i {
    /* decl.c */
    coord inv_pos;
};

struct instance_globals_saved_k {
    /* decl.c */
    struct kinfo killer;
};

struct instance_globals_saved_l {
    /* decl.c */
    schar lastseentyp[COLNO][ROWNO]; /* last seen/touched dungeon typ */
    dlevel_t level; /* level map */
    struct linfo level_info[MAXLINFO];
};

struct instance_globals_saved_m {
    /* dungeon.c */
    mapseen *mapseenchn; /*DUNGEON_OVERVIEW*/
    /* decl.c */
    long moves; /* turn counter */
    struct mvitals mvitals[NUMMONS];
};

struct instance_globals_saved_n {
    /* dungeon.c */
    int n_dgns; /* number of dungeons (also used in mklev.c and do.c) */
    /* files.c */
    char nhuuid[NHUUIDSZ];
    /* mkroom.c */
    int nroom;
    /* region.c */
    int n_regions;
};

struct instance_globals_saved_o {
    /* rumors.c */
    unsigned oracle_cnt; /* oracles are handled differently from rumors... */
    unsigned long *oracle_loc;

    /* other */
    long omoves;  /* level timestamp */
};

struct instance_globals_saved_p {
    /* decl.c */
    char plname[PL_NSIZ]; /* player name */
    char pl_character[PL_CSIZ];
    char pl_fruit[PL_FSIZ];
};

struct instance_globals_saved_q {
    /* quest.c */
    struct q_score quest_status;
};

struct instance_globals_saved_r {
    /* mkroom.c */
    struct mkroom rooms[(MAXNROFROOMS + 1) * 2];
};

struct instance_globals_saved_s {
    /* decl.c */
    struct spell spl_book[MAXSPELL + 1];
    s_level *sp_levchn;
};

struct instance_globals_saved_t {
    /* decl.c */
    char tune[6];
    /* timeout.c */
    unsigned long timer_id;
};

struct instance_globals_saved_u {
    /* decl.c */
    dest_area updest;
};

struct instance_globals_saved_w {
    /* reserved */
    long wreserve;
    int32_t wtreserved;
};

struct instance_globals_saved_x {
    /* mkmaze.c */
    int xmin, xmax; /* level boundaries x */
};

struct instance_globals_saved_y {
    /* mkmaze.c */
    int ymin, ymax; /* level boundaries y */
};

extern struct instance_globals_a ga;
extern struct instance_globals_b gb;
extern struct instance_globals_c gc;
extern struct instance_globals_d gd;
extern struct instance_globals_e ge;
extern struct instance_globals_f gf;
extern struct instance_globals_g gg;
extern struct instance_globals_h gh;
extern struct instance_globals_i gi;
extern struct instance_globals_j gj;
extern struct instance_globals_k gk;
extern struct instance_globals_l gl;
extern struct instance_globals_m gm;
extern struct instance_globals_n gn;
extern struct instance_globals_o go;
extern struct instance_globals_p gp;
extern struct instance_globals_q gq;
extern struct instance_globals_r gr;
extern struct instance_globals_s gs;
extern struct instance_globals_t gt;
extern struct instance_globals_u gu;
extern struct instance_globals_v gv;
extern struct instance_globals_w gw;
extern struct instance_globals_x gx;
extern struct instance_globals_y gy;
extern struct instance_globals_z gz;
extern struct instance_globals_saved_b svb;
extern struct instance_globals_saved_c svc;
extern struct instance_globals_saved_d svd;
extern struct instance_globals_saved_e sve;
extern struct instance_globals_saved_h svh;
extern struct instance_globals_saved_i svi;
extern struct instance_globals_saved_k svk;
extern struct instance_globals_saved_l svl;
extern struct instance_globals_saved_m svm;
extern struct instance_globals_saved_n svn;
extern struct instance_globals_saved_o svo;
extern struct instance_globals_saved_p svp;
extern struct instance_globals_saved_q svq;
extern struct instance_globals_saved_r svr;
extern struct instance_globals_saved_s svs;
extern struct instance_globals_saved_t svt;
extern struct instance_globals_saved_u svu;
extern struct instance_globals_saved_w svw;
extern struct instance_globals_saved_x svx;
extern struct instance_globals_saved_y svy;
/** @} */

/**
 * @var program_state
 * @brief What the game is currently doing, at the coarsest level.
 * @note Consulted by code that must behave differently while the game is starting up, saving, or ending -- a message routine called before the display
 *       exists cannot use it, and one called while panicking must not risk failing again.
 */
/**
 * @var program_state
 * @brief 게임이 지금 무엇을 하고 있는지. 가장 거친 수준에서.
 * @note 게임이 시작 중이거나 저장 중이거나 끝나는 중일 때 다르게 동작해야 하는 코드가 참조한다. 표시부가 존재하기 전에 호출된 메시지 루틴은 그것을 쓸 수 없고, 이상 종료 중에 호출된 것은 다시 실패할 위험을 감수해서는 안 된다.
 */
extern struct sinfo program_state; /* flags describing game's current state */
/* flags describing current level's loading/making/readiness status;
 * restlevelstate() already associated term 'levelstate' for a different
 * purpose, so attempt to avoid confusion
 */
/**
 * @var level_status
 * @brief How far along the current level is in being made or loaded.
 * @note Separate from the program state because a level can be half-built while the game is running normally, and code that runs during level creation
 *       must know not to expect a finished level.
 * @note Named as it is to avoid a collision the existing comment records: an unrelated routine had already taken the obvious name for a different
 *       purpose.
 */
/**
 * @var level_status
 * @brief 현재 레벨이 만들어지거나 적재되는 과정에서 어디까지 왔는지.
 * @note 프로그램 상태와 별개인 것은, 게임이 정상적으로 돌아가는 동안에도 레벨이 반쯤 지어진 상태일 수 있고, 레벨 생성 중에 돌아가는 코드가 완성된 레벨을 기대하지 않아야 하기 때문이다.
 * @note 기존 주석이 기록하는 충돌을 피하기 위해 이런 이름이 되었다. 무관한 어떤 루틴이 이미 다른 목적으로 그 뻔한 이름을 차지하고 있었다.
 */
extern struct levelstatus level_status;

/**
 * @brief Pre-zeroed values of the structures that are cleared most often.
 *
 * Clearing a structure by copying one of these is safer than assigning to its fields, because these structures gain fields over time and a
 * field-by-field clear silently stops being complete.
 *
 * @note Constant, so a clear cannot accidentally modify the template.
 * @note There is one per structure rather than a generic zeroing routine because the compiler can then copy them as whole values.
 */
/**
 * @brief 가장 자주 비워지는 구조체들의 미리 0으로 채워진 값.
 *
 * 이 중 하나를 복사해 구조체를 비우는 것이 그 필드들에 대입하는 것보다 안전하다. 이 구조체들은 시간이 흐르며 필드가 늘어나고, 필드 하나하나 비우기는 조용히 완전하지 않게 되기 때문이다.
 *
 * @note 상수이므로, 비우기가 실수로 그 원형을 바꿀 수 없다.
 * @note 일반적인 0 채우기 루틴 대신 구조체마다 하나씩 있는 것은, 그러면 컴파일러가 그것들을 값 전체로 복사할 수 있기 때문이다.
 */
struct const_globals {
    const struct obj zeroobj;      /* used to zero out a struct obj */
    const struct monst zeromonst;  /* used to zero out a struct monst */
    const anything zeroany;        /* used to zero out union any */
    const NhRect zeroNhRect;       /* used to zero out NhRect */
};

extern const struct const_globals cg;

/**
 * @var hands_obj
 * @brief A stand-in object representing the hero's bare hands.
 * @note Exists so that fighting unarmed can go through the same code as fighting with a weapon. Rules that take a weapon do not need a separate path
 *       for having none.
 * @warning Not a real object. It is not in any inventory and must not be treated as something that can be dropped, named or destroyed.
 */
/**
 * @var hands_obj
 * @brief 영웅의 맨손을 나타내는 대역 물건.
 * @note 맨손으로 싸우는 것이 무기로 싸우는 것과 같은 코드를 거칠 수 있도록 존재한다. 무기를 받는 규칙이 무기가 없는 경우를 위한 별도 경로를 필요로 하지 않는다.
 * @warning 실제 물건이 아니다. 어느 소지품에도 없으며, 버리거나 이름 붙이거나 파괴할 수 있는 것으로 취급되어서는 안 된다.
 */
extern struct obj hands_obj;

#endif /* DECL_H */
