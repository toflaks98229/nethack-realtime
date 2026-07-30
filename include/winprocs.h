/* NetHack 5.0	winprocs.h	$NHDT-Date: 1781973091 2026/06/20 16:31:31 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.95 $ */
/* Copyright (c) David Cohrs, 1992                                */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINPROCS_H
#define WINPROCS_H

#include "botl.h"
#ifndef CLR_MAX
#include "color.h"
#endif

/**
 * @file winprocs.h
 * @brief The interface between the game and whatever displays it.
 *
 * This is the whole boundary. Everything the core does to communicate with the player -- ask a question, offer a menu, draw a
 * square, read a keystroke -- goes through one table of function pointers, and installing a different table is what makes the same
 * binary run as a terminal game, a tiled window, or an X11 client.
 *
 * The table is reached through macros named as if they were functions. That is deliberate: the core calls @c putstr and does not
 * appear to know it is calling through a pointer, so the indirection costs nothing in readability.
 *
 * A handful of those names have been taken back into the core over time, and the commented-out lines record which. Where the core
 * needs to do something common before or after -- validate a buffer, apply the player's menu colours -- it now provides a real
 * function of that name which calls the display's, so a call site did not have to change.
 *
 * Because displays differ in what they can do, the table also declares what this one supports. The core states a preference and
 * consults the capability bits before relying on it, which is how one core serves a display that has fonts and one that has
 * twenty-four lines of text.
 *
 * @note The two capability words are full or nearly so, and the running commentary in them records how many bits remain. That is
 *       the practical limit on adding a capability.
 * @warning A second table of the same shape exists for chaining -- inserting a layer between the core and the real display, for
 *          tracing. The existing comment insists the two must match, and nothing checks it: a function added to one and not the
 *          other builds fine and calls the wrong pointer.
 */

/**
 * @file winprocs.h
 * @brief 게임과 그것을 표시하는 것 사이의 인터페이스.
 *
 * 이것이 그 경계 전체다. 코어가 플레이어와 소통하기 위해 하는 모든 일 -- 질문하기, 메뉴 제시하기, 칸 그리기, 키 입력 읽기 -- 이 하나의 함수 포인터 표를 통해 이뤄지고, 다른 표를 설치하는 것이 같은
 * 실행 파일을 터미널 게임으로, 타일 창으로, X11 클라이언트로 돌아가게 만드는 것이다.
 *
 * 그 표는 함수인 것처럼 이름 붙은 매크로로 접근된다. 의도적이다. 코어는 @c putstr 을 호출하며 자신이 포인터를 통해 호출하고 있다는 것을 알아 보이지 않는다. 그래서 그 간접 참조가 가독성에서 아무 비용도
 * 들지 않는다.
 *
 * 그 이름 중 몇 개는 시간이 흐르며 코어로 되돌려졌고, 주석 처리된 줄들이 어느 것인지 기록한다. 코어가 앞이나 뒤에 공통된 일을 해야 하는 곳 -- 버퍼 길이를 검증하고, 플레이어의 메뉴 색을 적용하고 -- 에서는
 * 이제 코어가 그 이름의 실제 함수를 제공하고 그것이 표시부의 것을 호출한다. 그래서 호출 지점은 바뀔 필요가 없었다.
 *
 * 표시부가 할 수 있는 것이 서로 다르므로, 이 표는 자신이 무엇을 지원하는지도 선언한다. 코어는 선호를 밝히고 그것에 의존하기 전에 능력 비트를 확인한다. 하나의 코어가 글꼴을 가진 표시부와 스물네 줄의
 * 텍스트를 가진 표시부를 함께 맡는 방식이 그것이다.
 *
 * @note 두 능력 워드는 가득 찼거나 거의 찼으며, 그 안의 진행 주석이 몇 비트가 남았는지 기록한다. 그것이 능력을 더하는 데 대한 실질적인 한계다.
 * @warning 같은 모양의 두 번째 표가 사슬 연결을 위해 존재한다. 추적을 위해 코어와 실제 표시부 사이에 한 층을 끼워 넣는 것이다. 기존 주석은 둘이 일치해야 한다고 강조하며, 그것을 검사하는 것은 없다.
 *          한쪽에만 더한 함수는 문제없이 빌드되고 잘못된 포인터를 호출한다.
 */

/**
 * @brief Which display this is.
 * @note An identifier rather than a name comparison. The commented-out older version of @c WINDOWPORT below compared strings, and
 *       this exists so that asking "am I the terminal port" is a comparison of two integers.
 * @note Includes the chaining layers and the stub displays, since they occupy the same slot as a real one.
 */
/**
 * @brief 이것이 어느 표시부인지.
 * @note 이름 비교가 아니라 식별자다. 아래 @c WINDOWPORT 의 주석 처리된 예전 판본은 문자열을 비교했고, 이것은 "내가 터미널 포트인가"를 묻는 일이 두 정수의 비교가 되도록 존재한다.
 * @note 사슬 연결 층과 껍데기 표시부도 포함한다. 그것들이 실제 표시부와 같은 자리를 차지하기 때문이다.
 */
enum wp_ids { wp_tty = 1, wp_X11, wp_Qt, wp_mswin, wp_curses,
              wp_chainin, wp_chainout, wp_shim,
              wp_hup, wp_guistubs, wp_ttystubs,
#if defined(AMIGA)
              wp_amii, wp_amiv,
#endif
#ifdef OUTDATED_STUFF
              wp_mac, wp_Gem, wp_Gnome,
#endif
              wp_trace  // XXX do we need this?  should chainin/out get an id? TBD
};

/**
 * @brief One complete display: what it is called, what it can do, and every routine it provides.
 *
 * Installing one of these is the whole act of choosing a display. Every routine is required -- a null entry is not a display that
 * omits a feature, it is a crash the first time the core needs that feature.
 *
 * @warning Must match the chaining table below exactly, as the existing comment insists. The two are separate declarations of the
 *          same interface and nothing verifies they agree.
 */
/**
 * @brief 하나의 완전한 표시부. 무엇이라 불리는지, 무엇을 할 수 있는지, 그리고 제공하는 모든 루틴.
 *
 * 이것 하나를 설치하는 것이 표시부를 고르는 행위 전체다. 모든 루틴이 필수다. 빈 항목은 어떤 기능을 생략한 표시부가 아니라, 코어가 그 기능을 처음 필요로 할 때의 충돌이다.
 *
 * @warning 기존 주석이 강조하듯 아래의 사슬 연결 표와 정확히 일치해야 한다. 둘은 같은 인터페이스의 별개 선언이며, 그것들이 일치하는지 검증하는 것은 없다.
 */
/* NB: this MUST match chain_procs below */
struct window_procs {
    /**
     * @brief The name a player writes to ask for this display.
     * @warning The leading character is significant, as the existing comment sets out: a leading dash is forbidden and a leading plus
     *          is reserved for the chaining layers, so a real display's name must begin with a letter.
     */
    /**
     * @brief 플레이어가 이 표시부를 요청하기 위해 적는 이름.
     * @warning 기존 주석이 밝히듯 첫 글자가 의미를 지닌다. 앞의 붙임표는 금지되어 있고 앞의 더하기는 사슬 연결 층을 위해 예약되어 있으므로, 실제 표시부의 이름은 글자로 시작해야 한다.
     */
    const char *name;     /* Names should start with [a-z].  Names must
                           * not start with '-'.  Names starting with
                           * '+' are reserved for processors. */
    /**
     * @brief Which display this is, for code that must ask.
     */
    /**
     * @brief 이것이 어느 표시부인지. 물어야 하는 코드를 위해.
     */
    enum wp_ids wp_id;
    /**
     * @brief What this display supports, as capability bits.
     * @note The core consults this before relying on a preference the player expressed, so an unsupported setting is quietly ignored
     *       rather than failing.
     */
    /**
     * @brief 이 표시부가 무엇을 지원하는지. 능력 비트로.
     * @note 코어는 플레이어가 밝힌 선호에 의존하기 전에 이것을 확인한다. 그래서 지원되지 않는 설정은 실패하는 대신 조용히 무시된다.
     */
    unsigned long wincap; /* window port capability options supported */
    /**
     * @brief The rest of the capability bits.
     * @note A second word exists only because the first filled up; the split carries no meaning.
     */
    /**
     * @brief 나머지 능력 비트들.
     * @note 두 번째 워드가 있는 것은 첫 번째가 가득 찼기 때문일 뿐이다. 그 분할에는 의미가 없다.
     */
    unsigned long wincap2; /* additional window port capability options */
    /**
     * @brief Which of the game's colours this display can actually show.
     * @note Per colour rather than a single flag, because a display may have some and not others -- and the core would rather choose a
     *       different colour than be told colour is unavailable.
     */
    /**
     * @brief 게임의 색 중 이 표시부가 실제로 보일 수 있는 것이 무엇인지.
     * @note 단일 플래그가 아니라 색마다 하나인 것은, 표시부가 일부는 가지고 일부는 갖지 않을 수 있기 때문이다. 그리고 코어는 색을 쓸 수 없다는 말을 듣기보다 다른 색을 고르는 편을 택한다.
     */
    boolean has_color[CLR_MAX];
    void (*win_init_nhwindows)(int *, char **);
    void (*win_player_selection)(void);
    void (*win_askname)(void);
    void (*win_get_nh_event)(void);
    void (*win_exit_nhwindows)(const char *);
    void (*win_suspend_nhwindows)(const char *);
    void (*win_resume_nhwindows)(void);
    winid (*win_create_nhwindow)(int);
    void (*win_clear_nhwindow)(winid);
    void (*win_display_nhwindow)(winid, boolean);
    void (*win_destroy_nhwindow)(winid);
    void (*win_curs)(winid, int, int);
    void (*win_putstr)(winid, int, const char *);
    void (*win_putmixed)(winid, int, const char *);
    void (*win_display_file)(const char *, boolean);
    void (*win_start_menu)(winid, unsigned long);
    void (*win_add_menu)(winid, const glyph_info *, const ANY_P *,
                         char, char, int, int,
                         const char *, unsigned int);
    void (*win_end_menu)(winid, const char *);
    int (*win_select_menu)(winid, int, MENU_ITEM_P **);
    char (*win_message_menu)(char, int, const char *);
    void (*win_mark_synch)(void);
    void (*win_wait_synch)(void);
#ifdef CLIPPING
    void (*win_cliparound)(int, int);
#endif
#ifdef POSITIONBAR
    void (*win_update_positionbar)(char *);
#endif
    void (*win_print_glyph)(winid, coordxy, coordxy,
                            const glyph_info *, const glyph_info *);
    void (*win_raw_print)(const char *);
    void (*win_raw_print_bold)(const char *);
    int (*win_nhgetch)(void);
    int (*win_nh_poskey)(coordxy *, coordxy *, int *);
    void (*win_nhbell)(void);
    int (*win_doprev_message)(void);
    char (*win_yn_function)(const char *, const char *, char);
    void (*win_getlin)(const char *, char *);
    int (*win_get_ext_cmd)(void);
    void (*win_number_pad)(int);
    void (*win_delay_output)(void);
#ifdef CHANGE_COLOR
    void (*win_change_color)(int, long, int);
#ifdef MAC68K
    void (*win_change_background)(int);
    short (*win_set_font_name)(winid, char *);
#endif
    char *(*win_get_color_string)(void);
#endif

    void (*win_outrip)(winid, int, time_t);
    void (*win_preference_update)(const char *);
    char *(*win_getmsghistory)(boolean);
    void (*win_putmsghistory)(const char *, boolean);
    void (*win_status_init)(void);
    void (*win_status_finish)(void);
    void (*win_status_enablefield)(int, const char *, const char *,
                                   boolean);
    void (*win_status_update)(int, genericptr_t, int, int, int,
                              unsigned long *);
    boolean (*win_can_suspend)(void);
    void (*win_update_inventory)(int);
    win_request_info *(*win_ctrl_nhwindow)(winid, int, win_request_info *);
};

/**
 * @var windowprocs
 * @brief The display currently installed. Every call below goes through this.
 * @note Declared volatile where hangup handling is built in, because the handler may replace it while the game is in the middle of a
 *       call -- a display whose terminal has gone must stop being used immediately.
 */
/**
 * @var windowprocs
 * @brief 지금 설치된 표시부. 아래의 모든 호출이 이것을 통해 이뤄진다.
 * @note 접속 끊김 처리가 빌드에 포함된 곳에서는 volatile 로 선언된다. 게임이 호출 도중에 있는 동안 처리기가 이것을 교체할 수 있기 때문이다. 터미널이 사라진 표시부는 즉시 쓰이기를 멈춰야 한다.
 */
extern
#ifdef HANGUPHANDLING
    volatile
#endif
    NEARDATA struct window_procs windowprocs;

/*
 * If you wish to only support one window system and not use procedure
 * pointers, add the appropriate #ifdef below.
 * XXX which is what?
 */

/**
 * @name Calling the display
 * @brief Names that look like functions but reach through the installed table.
 *
 * Written this way so the core reads as if it were calling ordinary functions. The indirection is invisible at every call site, which
 * is what makes replacing the whole display a matter of assigning one structure.
 *
 * @note Several names are absent here and commented out instead. Those have real functions of the same name in the core now, which do
 *       shared work and then call the display's routine -- so the call sites did not change when that happened. The comments record
 *       which and where.
 * @warning Not functions: taking the address of one of these, or passing one where a function pointer is expected, does not work.
 * @{
 */
/**
 * @name 표시부 호출하기
 * @brief 함수처럼 보이지만 설치된 표를 통해 닿는 이름들.
 *
 * 코어가 평범한 함수를 호출하는 것처럼 읽히도록 이렇게 쓰였다. 그 간접 참조가 모든 호출 지점에서 보이지 않으며, 그것이 표시부 전체를 교체하는 일을 구조체 하나를 대입하는 일로 만드는 것이다.
 *
 * @note 몇몇 이름이 여기에 없고 대신 주석 처리되어 있다. 그것들은 이제 코어에 같은 이름의 실제 함수가 있으며, 공통된 일을 하고 나서 표시부의 루틴을 호출한다. 그래서 그렇게 되었을 때 호출 지점은 바뀌지
 *       않았다. 주석들이 어느 것이 어디에 있는지 기록한다.
 * @warning 함수가 아니다. 이 중 하나의 주소를 취하거나 함수 포인터를 기대하는 곳에 넘기는 것은 통하지 않는다.
 * @{
 */
#define init_nhwindows (*windowprocs.win_init_nhwindows)
#define player_selection (*windowprocs.win_player_selection)
#define askname (*windowprocs.win_askname)
#define get_nh_event (*windowprocs.win_get_nh_event)
#define exit_nhwindows (*windowprocs.win_exit_nhwindows)
#define suspend_nhwindows (*windowprocs.win_suspend_nhwindows)
#define resume_nhwindows (*windowprocs.win_resume_nhwindows)
#define create_nhwindow (*windowprocs.win_create_nhwindow)
#define clear_nhwindow (*windowprocs.win_clear_nhwindow)
#define display_nhwindow (*windowprocs.win_display_nhwindow)
#define destroy_nhwindow (*windowprocs.win_destroy_nhwindow)
#define curs (*windowprocs.win_curs)
#define putstr (*windowprocs.win_putstr)
#define putmixed (*windowprocs.win_putmixed)
#define display_file (*windowprocs.win_display_file)
#define start_menu (*windowprocs.win_start_menu)
#define end_menu (*windowprocs.win_end_menu)
/* 5.0: There are real add_menu() and select_menu in the core now.
 *      add_menu does some common activities, such as menu_colors.
 *      select_menu does some before and after activities.
 *      add_menu() and select_menu() are in windows.c
 */
/* #define add_menu (*windowprocs.win_add_menu) */
/* #define select_menu (*windowprocs.win_select_menu) */
#define message_menu (*windowprocs.win_message_menu)

#define mark_synch (*windowprocs.win_mark_synch)
#define wait_synch (*windowprocs.win_wait_synch)
#ifdef CLIPPING
#define cliparound (*windowprocs.win_cliparound)
#endif
#ifdef POSITIONBAR
#define update_positionbar (*windowprocs.win_update_positionbar)
#endif
#define print_glyph (*windowprocs.win_print_glyph)
#define raw_print (*windowprocs.win_raw_print)
#define raw_print_bold (*windowprocs.win_raw_print_bold)
#define nhgetch (*windowprocs.win_nhgetch)
#define nh_poskey (*windowprocs.win_nh_poskey)
#define nhbell (*windowprocs.win_nhbell)
#define nh_doprev_message (*windowprocs.win_doprev_message)
/* 5.0: There is a real getlin() in the core now, which does
 *      some before and after activities.
 *      [alternative fix for menu search via ':'.]
 *      getlin() is in windows.c
 */
/* #define getlin (*windowprocs.win_getlin) */
#define get_ext_cmd (*windowprocs.win_get_ext_cmd)
#define number_pad (*windowprocs.win_number_pad)
#define nh_delay_output (*windowprocs.win_delay_output)
#ifdef CHANGE_COLOR
#define change_color (*windowprocs.win_change_color)
#ifdef MAC68K
#define change_background (*windowprocs.win_change_background)
#define set_font_name (*windowprocs.win_set_font_name)
#endif
#define get_color_string (*windowprocs.win_get_color_string)
#endif

/* 3.4.2: There is a real yn_function() in the core now, which does
 *        some buffer length validation on the parameters prior to
 *        invoking the window port routine. yn_function() is in cmd.c
 */
/* #define yn_function (*windowprocs.win_yn_function) */

#define outrip (*windowprocs.win_outrip)
#define preference_update (*windowprocs.win_preference_update)
#define getmsghistory (*windowprocs.win_getmsghistory)
#define putmsghistory (*windowprocs.win_putmsghistory)
/* there is a status_initialize() in botl.c,
 * which calls win_status_init() directly; same with status_finish.
 */
#define status_enablefield (*windowprocs.win_status_enablefield)
#define status_update (*windowprocs.win_status_update)
#define ctrl_nhwindow (*windowprocs.win_ctrl_nhwindow)
/** @} */

/**
 * @def WPID
 * @brief Fill in a display's name and identifier from one token.
 * @param name the display's short name, as both text and identifier suffix
 * @note Expands to two initialisers, not one value. It exists so the name and the identifier cannot disagree -- writing them
 *       separately is how a display ends up answering to the wrong name.
 */
/**
 * @def WPID
 * @brief 표시부의 이름과 식별자를 하나의 토큰에서 채운다.
 * @param name 표시부의 짧은 이름. 글과 식별자 접미사 둘 다로 쓰인다
 * @note 하나의 값이 아니라 두 초기화자로 펼쳐진다. 이름과 식별자가 서로 어긋날 수 없도록 존재한다. 그것들을 따로 적는 것이 표시부가 잘못된 이름에 응답하게 되는 경로다.
 */
/*
 *
 */
#define WPID(name) #name, wp_##name
/**
 * @def WPIDMINUS
 * @brief The same, but with the name marked as one a player may not ask for.
 * @param name the display's short name
 * @note The leading dash is what the name rule at the top of the structure forbids for a real display, so this is how an entry is made
 *       unselectable by name while still occupying its slot.
 */
/**
 * @def WPIDMINUS
 * @brief 같은 일을 하되, 이름을 플레이어가 요청할 수 없는 것으로 표시한다.
 * @param name 표시부의 짧은 이름
 * @note 앞의 붙임표는 구조체 맨 위의 이름 규칙이 실제 표시부에 대해 금지하는 것이다. 그래서 이것이 어떤 항목을 자기 자리를 차지한 채로 이름으로 고를 수 없게 만드는 방법이다.
 */
#define WPIDMINUS(name) "-" #name, wp_##name

/**
 * @name Display capabilities
 * @brief What a display can do, so the core can tell what a setting will actually accomplish.
 *
 * These are not settings but statements of ability. A player may ask for a particular map font; whether that means anything depends on
 * the display, and the core checks here before acting on the request rather than passing it on and hoping.
 *
 * Read them alongside the preference fields in flag.h: for most of these bits there is a corresponding field there, and the pair is a
 * request and the ability to honour it.
 *
 * @warning This word is full -- the running commentary ends with "no free bits" -- so a new capability must go in the second word.
 * @{
 */
/**
 * @name 표시부 능력
 * @brief 표시부가 무엇을 할 수 있는지. 코어가 어떤 설정이 실제로 무엇을 이룰지 알 수 있도록.
 *
 * 이들은 설정이 아니라 능력의 선언이다. 플레이어는 특정한 지도 글꼴을 요청할 수 있고, 그것이 무엇을 뜻하는지는 표시부에 달려 있다. 코어는 그 요청을 그대로 넘기고 바라는 대신 여기를 먼저 확인한다.
 *
 * flag.h 의 선호 필드들과 나란히 읽을 것. 이 비트 대부분에는 그곳에 대응하는 필드가 있고, 그 짝이 요청과 그것을 받아들일 능력이다.
 *
 * @warning 이 워드는 가득 찼다. 진행 주석이 "남은 비트 없음"으로 끝난다. 그래서 새 능력은 두 번째 워드로 가야 한다.
 * @{
 */
/*
 * WINCAP
 * Window port preference capability bits.
 * Some day this might be better in its own wincap.h file.
 */
/* clang-format off */
#define WC_COLOR         0x00000001L /* 01 Port can display things in color  */
#define WC_HILITE_PET    0x00000002L /* 02 supports hilite pet               */
#define WC_ASCII_MAP     0x00000004L /* 03 supports an ascii map             */
#define WC_TILED_MAP     0x00000008L /* 04 supports a tiled map              */
#define WC_PRELOAD_TILES 0x00000010L /* 05 supports pre-loading tiles        */
#define WC_TILE_WIDTH    0x00000020L /* 06 prefer this width of tile         */
#define WC_TILE_HEIGHT   0x00000040L /* 07 prefer this height of tile        */
#define WC_TILE_FILE     0x00000080L /* 08 alternative tile file name        */
#define WC_INVERSE       0x00000100L /* 09 Port supports inverse video       */
#define WC_ALIGN_MESSAGE 0x00000200L /* 10 supports mesg alignment top|b|l|r */
#define WC_ALIGN_STATUS  0x00000400L /* 11 supports status alignmt top|b|l|r */
#define WC_VARY_MSGCOUNT 0x00000800L /* 12 supports varying message window   */
#define WC_FONT_MAP      0x00001000L /* 13 supports spec of map window font  */
#define WC_FONT_MESSAGE  0x00002000L /* 14 supports spec of message font     */
#define WC_FONT_STATUS   0x00004000L /* 15 supports spec of status font      */
#define WC_FONT_MENU     0x00008000L /* 16 supports spec of menu font        */
#define WC_FONT_TEXT     0x00010000L /* 17 supports spec of text window font */
#define WC_FONTSIZ_MAP   0x00020000L /* 18 supports spec of map font size    */
#define WC_FONTSIZ_MESSAGE 0x040000L /* 19 supports spec of mesg font size   */
#define WC_FONTSIZ_STATUS 0x0080000L /* 20 supports spec of status font size */
#define WC_FONTSIZ_MENU  0x00100000L /* 21 supports spec of menu font size   */
#define WC_FONTSIZ_TEXT  0x00200000L /* 22 supports spec of text font size   */
#define WC_SCROLL_MARGIN 0x00400000L /* 23 supports setting map scroll marg  */
#define WC_SPLASH_SCREEN 0x00800000L /* 24 supports display of splash screen */
#define WC_POPUP_DIALOG  0x01000000L /* 25 supports queries in popup dialogs */
#define WC_SCROLL_AMOUNT 0x02000000L /* 26 scroll this amount at scroll marg */
#define WC_EIGHT_BIT_IN  0x04000000L /* 27 8-bit character input             */
#define WC_PERM_INVENT   0x08000000L /* 28 supports persistent inventory win */
#define WC_MAP_MODE      0x10000000L /* 29 map_mode option                   */
#define WC_WINDOWCOLORS  0x20000000L /* 30 background color for mesg window  */
#define WC_PLAYER_SELECTION 0x40000000L /* 31 supports player selection      */
#ifdef NHSTDC
#define WC_MOUSE_SUPPORT 0x80000000UL /* 32 mouse support                    */
#else
#define WC_MOUSE_SUPPORT 0x80000000L /* 32 mouse support                     */
#endif
                                     /* no free bits */
/** @} */

/**
 * @name Further display capabilities
 * @brief The continuation of the capability bits, in a second word.
 *
 * A second word purely because the first is full. There is no thematic division, so a capability's being here rather than there says
 * nothing except that it was added later.
 *
 * @note Three of these say the display can be @e told something rather than that it can @e do something: that it accepts a flush
 *       request after status updates, a reset request meaning redraw everything, and the two non-display attribute flags on messages.
 *       They exist because a core that sent those to a display that ignored them would produce visible glitches rather than nothing.
 * @note The remaining-bit counts in the comments are the practical budget for future capabilities.
 * @{
 */
/**
 * @name 추가 표시부 능력
 * @brief 능력 비트의 이어짐. 두 번째 워드로.
 *
 * 순전히 첫 워드가 가득 찼기 때문의 두 번째 워드다. 주제별 구분이 없으므로, 어떤 능력이 저기가 아니라 여기 있다는 것은 그것이 나중에 더해졌다는 것 말고는 아무것도 말하지 않는다.
 *
 * @note 이 중 셋은 표시부가 무언가를 @e 할 수 있다는 것이 아니라 무언가를 @e 들을 수 있다는 것을 말한다. 상태 갱신 뒤의 비우기 요청을 받아들인다는 것, 전부 다시 그리라는 뜻의 초기화 요청을 받아들인다는 것,
 *       그리고 메시지에 붙는 두 개의 비표시 속성 플래그. 그것들이 있는 것은, 그것을 무시하는 표시부에게 코어가 그것을 보내면 아무 일도 없는 것이 아니라 눈에 보이는 결함이 생기기 때문이다.
 * @note 주석에 적힌 남은 비트 수가 앞으로의 능력을 위한 실질적인 예산이다.
 * @{
 */
#define WC2_FULLSCREEN    0x0001L /* 01 display full screen             */
#define WC2_SOFTKEYBOARD  0x0002L /* 02 software keyboard               */
#define WC2_WRAPTEXT      0x0004L /* 03 wrap long lines of text         */
#define WC2_HILITE_STATUS 0x0008L /* 04 hilite fields in status         */
#define WC2_SELECTSAVED   0x0010L /* 05 saved game selection menu       */
#define WC2_DARKGRAY      0x0020L /* 06 use bold black for black glyphs */
#define WC2_HITPOINTBAR   0x0040L /* 07 show bar representing hit points */
#define WC2_FLUSH_STATUS  0x0080L /* 08 call status_update(BL_FLUSH)
                                   *    after updating status window fields */
#define WC2_RESET_STATUS  0x0100L /* 09 call status_update(BL_RESET) to
                                   *    indicate 'draw everything'      */
#define WC2_TERM_SIZE     0x0200L /* 10 support setting terminal size   */
#define WC2_STATUSLINES   0x0400L /* 11 switch between 2|3 lines of status */
#define WC2_WINDOWBORDERS 0x0800L /* 12 display borders on nh windows   */
#define WC2_PETATTR       0x1000L /* 13 attributes for hilite_pet       */
#define WC2_GUICOLOR      0x2000L /* 14 display colours outside map win */
/* pline() can overload the display attributes argument passed to putstr()
   with one or more flags and at most one of bold/blink/inverse/&c */
#define WC2_URGENT_MESG   0x4000L  /* 15 putstr(WIN_MESSAGE) supports urgency
                                    *    via non-display attribute flag  */
#define WC2_SUPPRESS_HIST 0x8000L  /* 16 putstr(WIN_MESSAGE) supports history
                                    *    suppression via non-disp attr   */
#define WC2_MENU_SHIFT   0x010000L /* 17 horizontal menu scrolling */
#define WC2_U_UTF8STR    0x020000L /* 18 utf8str support */
#define WC2_EXTRACOLORS  0x040000L /* 19 color support beyond NH_BASIC_COLOR */
                                   /* 13 free bits */
#define WC2_EXTRASTATUS  0x080000L /* 20 optional weaponstatus, armorstatus,
                                    *    terrainstatus */
                                   /* 12 free bits */
/** @} */

/**
 * @name Window placement
 * @brief Which edge a window is put against.
 * @note Only an edge, not a position -- the display decides how large the window is and how the rest is arranged around it.
 * @{
 */
/**
 * @name 창 배치
 * @brief 창을 어느 변에 붙일지.
 * @note 위치가 아니라 변만이다. 창의 크기와 그 둘레의 나머지를 어떻게 배치할지는 표시부가 정한다.
 * @{
 */
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
#define ALIGN_TOP    3
#define ALIGN_BOTTOM 4
/** @} */

/**
 * @name How a character is chosen
 * @brief Whether the display presents character creation as a dialog or as a series of questions.
 * @note A display with no way to show a dialog must be able to say so, since the choice cannot simply be skipped.
 * @{
 */
/**
 * @name 캐릭터를 고르는 방식
 * @brief 표시부가 캐릭터 생성을 대화 상자로 제시하는지 연속된 질문으로 제시하는지.
 * @note 대화 상자를 보일 방법이 없는 표시부는 그것을 말할 수 있어야 한다. 그 선택을 그냥 건너뛸 수는 없기 때문이다.
 * @{
 */
/* player_selection */
#define VIA_DIALOG  0
#define VIA_PROMPTS 1
/** @} */

/**
 * @name Map mode settings
 * @brief Older combined settings naming a font size and whether tiles are used.
 * @warning Deprecated, as the existing comment says. The separate tile and font settings replaced them; these remain so that an old
 *          configuration file still loads.
 * @{
 */
/**
 * @name 지도 모드 설정
 * @brief 글꼴 크기와 타일 사용 여부를 함께 지칭하는 예전의 통합 설정.
 * @warning 기존 주석이 말하듯 폐기되었다. 별개의 타일 설정과 글꼴 설정이 그것을 대체했다. 예전 설정 파일이 여전히 읽히도록 남아 있다.
 * @{
 */
/* map_mode settings - deprecated */
#define MAP_MODE_TILES      0
#define MAP_MODE_ASCII4x6   1
#define MAP_MODE_ASCII6x8   2
#define MAP_MODE_ASCII8x8   3
#define MAP_MODE_ASCII16x8  4
#define MAP_MODE_ASCII7x12  5
#define MAP_MODE_ASCII8x12  6
#define MAP_MODE_ASCII16x12 7
#define MAP_MODE_ASCII12x16 8
#define MAP_MODE_ASCII10x18 9
#define MAP_MODE_ASCII_FIT_TO_SCREEN 10
#define MAP_MODE_TILES_FIT_TO_SCREEN 11
/** @} */

/**
 * @brief The name of one capability paired with its bit.
 * @note Exists so that a capability can be named in a configuration file and in messages. Without it the bits would have no textual
 *       form, and a player could not be told which of their settings the display cannot honour.
 */
/**
 * @brief 능력 하나의 이름과 그 비트의 짝.
 * @note 능력이 설정 파일과 메시지에서 지칭될 수 있도록 존재한다. 그것이 없으면 비트들에는 글로 된 형태가 없고, 플레이어에게 자기 설정 중 어느 것을 표시부가 받아들일 수 없는지 알려 줄 수 없다.
 */
struct wc_Opt {
    const char *wc_name;
    unsigned long wc_bit;
};

/* Macro for the currently active Window Port whose function
   pointers have been loaded */
#if 0
/* 5.0 The string comparison version isn't used anymore */
#define WINDOWPORT(wn) \
    (windowprocs.name && !strncmpi((#wn), windowprocs.name, strlen((#wn))))
#endif

/**
 * @def WINDOWPORT
 * @brief Ask whether a named display is the one currently installed.
 * @param wn the display's short name, as an identifier suffix
 * @note An integer comparison. The commented-out version above did the same by comparing name prefixes, which was both slower and
 *       ambiguous between names sharing a prefix.
 * @warning Its presence in the core is a compromise: code that must know which display it is running under is code the boundary was
 *          meant to make unnecessary.
 */
/**
 * @def WINDOWPORT
 * @brief 이름 붙은 표시부가 지금 설치된 것인지 묻는다.
 * @param wn 표시부의 짧은 이름. 식별자 접미사로
 * @note 정수 비교다. 위의 주석 처리된 판본은 이름 앞부분을 비교해 같은 일을 했는데, 그것은 더 느리고 접두어를 공유하는 이름들 사이에서 모호했다.
 * @warning 이것이 코어에 존재하는 것은 타협이다. 자신이 어떤 표시부 아래에서 돌아가고 있는지 알아야 하는 코드는 이 경계가 불필요하게 만들려 했던 코드다.
 */
#define WINDOWPORT(wn) (windowprocs.wp_id == wp_##wn)

/**
 * @name Character creation steps
 * @brief Which question the display is being asked to put to the player.
 *
 * A display that presents creation as a dialog needs to know which choice each part of it is for. So the steps are named, and the
 * companion macro turns a step into a value that can be passed where a chosen index is expected.
 *
 * @note @c RS_filter is not a question but the option to narrow what the other questions offer.
 * @warning The step values are converted into the negative range below the "unmade choice" markers, so a step and a real index cannot
 *          be confused -- but the conversion depends on those markers' values and cannot be read as an ordinary number.
 * @note The existing comment observes this ought to live in the core rather than in the display interface.
 * @{
 */
/**
 * @name 캐릭터 생성 단계
 * @brief 표시부가 플레이어에게 어느 질문을 하도록 요청받는지.
 *
 * 생성을 대화 상자로 제시하는 표시부는 그 각 부분이 어느 선택을 위한 것인지 알아야 한다. 그래서 단계들에 이름이 붙어 있고, 짝이 되는 매크로가 단계를 골라진 색인을 기대하는 곳에 넘길 수 있는 값으로 바꾼다.
 *
 * @note @c RS_filter 는 질문이 아니라 다른 질문들이 제시하는 것을 좁히는 선택지다.
 * @warning 단계 값들은 "정해지지 않은 선택" 표시들 아래의 음수 범위로 변환된다. 그래서 단계와 실제 색인이 혼동될 수 없다. 다만 그 변환은 그 표시들의 값에 의존하며 평범한 숫자로 읽을 수 없다.
 * @note 기존 주석은 이것이 표시부 인터페이스가 아니라 코어에 있어야 한다고 지적한다.
 * @{
 */
/* role selection by player_selection(); this ought to be in the core... */
#define RS_NAME    0
#define RS_ROLE    1
#define RS_RACE    2
#define RS_GENDER  3
#define RS_ALGNMNT 4
#define RS_filter  5
#define RS_menu_arg(x) (ROLE_RANDOM - ((x) + 1)) /* 0..5 -> -3..-8 */
/** @} */

/**
 * @name Display initialisation direction
 * @brief Whether a display is being installed or removed.
 * @note Choosing a display can happen more than once in a run, as the existing comment records, so the initialising code has to be told
 *       which way -- setting up and tearing down are not the same work in reverse.
 * @{
 */
/**
 * @name 표시부 초기화 방향
 * @brief 표시부가 설치되는 중인지 제거되는 중인지.
 * @note 기존 주석이 기록하듯 한 번의 실행에서 표시부를 고르는 일이 두 번 넘게 일어날 수 있다. 그래서 초기화 코드가 어느 방향인지 들어야 한다. 세우는 일과 허무는 일은 서로의 역순이 아니다.
 * @{
 */
/* Choose_windows() may be called multiple times; these constants tell the
 * init function whether the window system is coming or going. */
#define WININIT      0
#define WININIT_UNDO 1
/** @} */

#ifdef WINCHAIN
/* Setup phases for window chain elements.
        void *  rv = X_procs_chain(int, int, void *, void *, void *);
                Xprivate*          ALLOC n      0       0       0
                -                  INIT  n   self    next    nextdata
   where:
        Xprivate* is anything window chain entry type X wants back
        n is the link count (starting with 1)
        self is the Xprivate* returned earlier
        next is struct winprocs * or struct chainprocs * for the next link
        nextdata is the Xprivate* for the next link in the chain
*/
/**
 * @name Chain setup phases
 * @brief The two passes needed to build a chain of display layers.
 *
 * A chain cannot be built in one pass, because each link needs to know what comes after it and the links are created in order. So
 * allocation happens first for all of them, and only then is each told about its neighbour.
 *
 * @note The existing comment above gives the exact call shape for both phases, including what each argument means in each -- the
 *       arguments are unused in the first pass and meaningful in the second.
 * @{
 */
/**
 * @name 사슬 준비 단계
 * @brief 표시부 층의 사슬을 만드는 데 필요한 두 번의 통과.
 *
 * 사슬은 한 번의 통과로 만들 수 없다. 각 고리가 자기 뒤에 무엇이 오는지 알아야 하고 고리들은 순서대로 만들어지기 때문이다. 그래서 먼저 전부에 대해 할당이 일어나고, 그다음에야 각각에게 자기 이웃이 알려진다.
 *
 * @note 위의 기존 주석은 두 단계 모두의 정확한 호출 모양과 각 단계에서 각 인자가 무엇을 뜻하는지를 밝힌다. 인자들은 첫 통과에서는 쓰이지 않고 두 번째 통과에서 의미를 갖는다.
 * @{
 */
#define WINCHAIN_ALLOC 0
#define WINCHAIN_INIT  1
/** @} */

/* clang-format on */

/**
 * @def CARGS
 * @brief The extra first argument every chained routine takes: the link's own private data.
 * @note Untyped on purpose. Each link keeps whatever it likes behind this pointer, and the chaining machinery neither knows nor needs to
 *       know what -- which is what lets a link be written without changing anything else.
 */
/**
 * @def CARGS
 * @brief 사슬로 연결된 모든 루틴이 받는 추가 첫 인자. 그 고리 자신의 사적 데이터.
 * @note 의도적으로 타입이 없다. 각 고리는 이 포인터 뒤에 원하는 것을 보관하고, 사슬 기제는 그것이 무엇인지 알지도 알아야 하지도 않는다. 그것이 다른 아무것도 바꾸지 않고 고리를 작성할 수 있게 하는 것이다.
 */
#define CARGS void *

/**
 * @var wc_tracelogf
 * @brief The file a tracing layer writes to.
 * @note Exposed rather than kept private so that other debugging code can write into the same log and the entries interleave in order.
 */
/**
 * @var wc_tracelogf
 * @brief 추적 층이 기록하는 파일.
 * @note 사적으로 두지 않고 노출된 것은, 다른 디버깅 코드가 같은 로그에 기록해 항목들이 순서대로 섞이도록 하기 위함이다.
 */
extern FILE *wc_tracelogf; /* Expose log file for additional debugging. */

/**
 * @brief A layer that sits between the core and the real display.
 *
 * Identical to the display table in every routine, except that each takes the link's private data as an extra first argument. That is the
 * only difference, and it is what lets a layer forward a call to the next link after doing whatever it exists to do -- logging it,
 * usually.
 *
 * @warning Must match the display table exactly, routine for routine. Adding a routine to one and not the other compiles and then calls
 *          through a pointer that was never set.
 */
/**
 * @brief 코어와 실제 표시부 사이에 놓이는 층.
 *
 * 모든 루틴에서 표시부 표와 동일하다. 다만 각 루틴이 그 고리의 사적 데이터를 추가 첫 인자로 받는다. 그것이 유일한 차이이며, 층이 자신이 존재하는 이유의 일 -- 보통은 기록 -- 을 한 뒤 다음 고리로 호출을 넘길 수
 * 있게 하는 것이 그것이다.
 *
 * @warning 표시부 표와 루틴 하나하나까지 정확히 일치해야 한다. 한쪽에만 루틴을 더하면 컴파일된 뒤 설정된 적 없는 포인터를 통해 호출하게 된다.
 */
struct chain_procs {
    const char *name;     /* Names should start with [a-z].  Names must
                           * not start with '-'.  Names starting with
                           * '+' are reserved for processors. */
    enum wp_ids wp_id;
    unsigned long wincap; /* window port capability options supported */
    unsigned long wincap2; /* additional window port capability options */
    boolean has_color[CLR_MAX];
    void (*win_init_nhwindows)(CARGS, int *, char **);
    void (*win_player_selection)(CARGS);
    void (*win_askname)(CARGS);
    void (*win_get_nh_event)(CARGS);
    void (*win_exit_nhwindows)(CARGS, const char *);
    void (*win_suspend_nhwindows)(CARGS, const char *);
    void (*win_resume_nhwindows)(CARGS);
    winid (*win_create_nhwindow)(CARGS, int);
    void (*win_clear_nhwindow)(CARGS, winid);
    void (*win_display_nhwindow)(CARGS, winid, boolean);
    void (*win_destroy_nhwindow)(CARGS, winid);
    void (*win_curs)(CARGS, winid, int, int);
    void (*win_putstr)(CARGS, winid, int, const char *);
    void (*win_putmixed)(CARGS, winid, int, const char *);
    void (*win_display_file)(CARGS, const char *, boolean);
    void (*win_start_menu)(CARGS, winid, unsigned long);
    void (*win_add_menu)(CARGS, winid, const glyph_info *,
                         const ANY_P *, char, char, int,
                         int, const char *, unsigned int);
    void (*win_end_menu)(CARGS, winid, const char *);
    int (*win_select_menu)(CARGS, winid, int, MENU_ITEM_P **);
    char (*win_message_menu)(CARGS, char, int, const char *);
    void (*win_mark_synch)(CARGS);
    void (*win_wait_synch)(CARGS);
#ifdef CLIPPING
    void (*win_cliparound)(CARGS, int, int);
#endif
#ifdef POSITIONBAR
    void (*win_update_positionbar)(CARGS, char *);
#endif
    void (*win_print_glyph)(CARGS, winid, coordxy, coordxy,
                            const glyph_info *,
                            const glyph_info *);
    void (*win_raw_print)(CARGS, const char *);
    void (*win_raw_print_bold)(CARGS, const char *);
    int (*win_nhgetch)(CARGS);
    int (*win_nh_poskey)(CARGS, coordxy *, coordxy *, int *);
    void (*win_nhbell)(CARGS);
    int (*win_doprev_message)(CARGS);
    char (*win_yn_function)
               (CARGS, const char *, const char *, char);
    void (*win_getlin)(CARGS, const char *, char *);
    int (*win_get_ext_cmd)(CARGS);
    void (*win_number_pad)(CARGS, int);
    void (*win_delay_output)(CARGS);
#ifdef CHANGE_COLOR
    void (*win_change_color)(CARGS, int, long, int);
#ifdef MAC68K
    void (*win_change_background)(CARGS, int);
    short (*win_set_font_name)(CARGS, winid, char *);
#endif
    char *(*win_get_color_string)(CARGS);
#endif

    void (*win_outrip)(CARGS, winid, int, time_t);
    void (*win_preference_update)(CARGS, const char *);
    char *(*win_getmsghistory)(CARGS, boolean);
    void (*win_putmsghistory)(CARGS, const char *, boolean);
    void (*win_status_init)(CARGS);
    void (*win_status_finish)(CARGS);
    void (*win_status_enablefield)
               (CARGS, int, const char *, const char *, boolean);
    void (*win_status_update)(CARGS, int, genericptr_t, int, int, int,
                              unsigned long *);
    boolean (*win_can_suspend)(CARGS);
    void (*win_update_inventory)(CARGS, int);
    win_request_info *(*win_ctrl_nhwindow)(CARGS, winid, int,
                                           win_request_info *);
};
#endif /* WINCHAIN */

#endif /* WINPROCS_H */
