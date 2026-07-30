/* NetHack 5.0	wintty.h	$NHDT-Date: 1781973092 2026/06/20 16:31:32 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.84 $ */
/* Copyright (c) David Cohrs, 1991,1992                           */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file wintty.h
 * @brief The terminal display's own declarations.
 *
 * One of a family, one per display: each implements the interface the game defines and keeps whatever it needs to do so behind this header. Nothing in the game includes these --
 * only the display itself does -- which is what keeps the boundary a boundary.
 *
 * What a terminal display needs is a model of the screen. A terminal has no windows, so the display makes them: it keeps for each what it contains and where on the screen it sits,
 * and draws them by moving the cursor. That is why the structures here describe rectangles of characters rather than anything windowlike.
 *
 * The persistent inventory support is the most involved part, and its shape follows from the same limitation. A terminal cannot have a second window beside the map, so the inventory
 * occupies part of the same screen, held as a grid of cells with each cell remembering whether it needs redrawing. Redrawing only what changed is not an optimisation here -- a
 * terminal is slow enough that redrawing everything would be visible.
 *
 * @note A cell holds either a character or a glyph, and the union is arranged with the pointer first so that a zeroed cell is a valid empty one, as the accompanying comment records.
 * @note The minimum dimensions declared for the inventory are why it can be refused: a terminal too small to hold it is told so rather than having it drawn over the map.
 */

/**
 * @file wintty.h
 * @brief 터미널 표시부 자신의 선언들.
 *
 * 표시부마다 하나씩 있는 가족의 하나다. 각각은 게임이 정의한 인터페이스를 구현하고, 그렇게 하기 위해 필요한 것을 이 헤더 뒤에 보관한다. 게임의 어느 것도 이 헤더들을 포함하지 않으며 -- 표시부 자신만이 포함한다 -- 그것이 그 경계를 경계로 유지하는 것이다.
 *
 * 터미널 표시부가 필요로 하는 것은 화면의 모형이다. 터미널에는 창이 없으므로 표시부가 그것을 만든다. 각각에 대해 무엇을 담고 있고 화면의 어디에 놓이는지를 보관하고, 커서를 옮기며 그것들을 그린다. 여기의 구조체들이 창 같은 무엇이 아니라 문자의 사각형을 기술하는 이유가 그것이다.
 *
 * 지속 소지품 지원이 가장 복잡한 부분이며, 그 모양이 같은 제약에서 따라 나온다. 터미널은 지도 옆에 두 번째 창을 가질 수 없으므로, 소지품이 같은 화면의 일부를 차지하고 칸의 격자로 보관되며 각 칸이 자신이 다시 그려져야 하는지를 기억한다. 바뀐 것만 다시 그리는 것은 여기서
 * 최적화가 아니다. 터미널은 전부를 다시 그리는 것이 눈에 보일 만큼 느리다.
 *
 * @note 칸은 문자나 글리프 중 하나를 담으며, 딸린 주석이 기록하듯 그 공용체는 포인터가 먼저 오도록 짜여 있다. 그래서 0으로 채워진 칸이 유효한 빈 칸이 된다.
 * @note 소지품에 대해 선언된 최소 크기가 그것이 거절될 수 있는 이유다. 그것을 담기에 너무 작은 터미널은 그것이 지도 위에 그려지는 대신 그렇다고 통보받는다.
 */

#ifndef WINTTY_H
#define WINTTY_H

#ifndef WINDOW_STRUCTS
#define WINDOW_STRUCTS

#ifdef TTY_PERM_INVENT

enum { tty_perminv_minrow = 28, tty_perminv_mincol = 79 };
/* for static init of zerottycell, put pointer first */
union ttycellcontent {
    glyph_info *gi;
    char ttychar;
};
struct tty_perminvent_cell {
    Bitfield(refresh, 1);
    Bitfield(text, 1);
    Bitfield(glyph, 1);
    union ttycellcontent content;
    uint32 color;       /* adjusted color 0 = ignore
                         * 1-16             = NetHack color + 1
                         * 17..16,777,233   = 24-bit color  + 17
                         */
};
#endif

/* menu structure */
typedef struct tty_mi {
    struct tty_mi *next;
    anything identifier; /* user identifier */
    long count;          /* user count */
    char *str;           /* description string (including accelerator) */
    glyph_info glyphinfo;    /* glyph */
    int attr;            /* string attribute */
    int color;           /* string color */
    boolean selected;    /* TRUE if selected by user */
    unsigned itemflags;  /* item flags */
    char selector;       /* keyboard accelerator */
    char gselector;      /* group accelerator */
} tty_menu_item;

/* descriptor for tty-based windows */
struct WinDesc {
    int flags;             /* window flags */
    xint16 type;           /* type of window */
    boolean active;        /* true if window is active */
    boolean blanked;       /* for erase_tty_screen(); not used [yet?] */
    short offx, offy;      /* offset from topleft of display */
    long rows, cols;       /* dimensions */
    long curx, cury;       /* current cursor position */
    long maxrow, maxcol;   /* the maximum size used -- for MENU wins;
                            * maxcol is also used by WIN_MESSAGE for
                            * tracking the ^P command */
    unsigned long mbehavior; /* menu behavior flags (MENU) */
    short *datlen;         /* allocation size for *data */
    char **data;           /* window data [row][column] */
    char *morestr;         /* string to display instead of default */
    tty_menu_item *mlist;  /* menu information (MENU) */
    tty_menu_item **plist; /* menu page pointers (MENU) */
    long plist_size;       /* size of allocated plist (MENU) */
    long npages;           /* number of pages in menu (MENU) */
    long nitems;           /* total number of items (MENU) */
    short how;             /* menu mode - pick 1 or N (MENU) */
    char menu_ch;          /* menu char (MENU) */
#ifdef TTY_PERM_INVENT
    struct tty_perminvent_cell **cells;
#endif
};

/* window flags */
#define WIN_CANCELLED 1
#define WIN_STOP 1        /* for NHW_MESSAGE; stops output; sticks until
                           * next input request or reversed by WIN_NOSTOP */
#define WIN_LOCKHISTORY 2 /* for NHW_MESSAGE; suppress history updates */
#define WIN_NOSTOP 4      /* current message has been designated as urgent;
                           * prevents WIN_STOP from becoming set if current
                           * message triggers --More-- and user types ESC
                           * (current message won't have been seen yet) */

/* topline states */
#define TOPLINE_EMPTY          0 /* empty */
#define TOPLINE_NEED_MORE      1 /* non-empty, need --More-- */
#define TOPLINE_NON_EMPTY      2 /* non-empty, no --More-- required */
#define TOPLINE_SPECIAL_PROMPT 3 /* special prompt state */

/* descriptor for tty-based displays -- all the per-display data */
struct DisplayDesc {
    short rows, cols;   /* width and height of tty display */
    short curx, cury;   /* current cursor position on the screen */
    uint32 color;        /* current color */
    uint32 colorflags;   /* NH_BASIC_COLOR or 24-bit color */
    uint32 framecolor;   /* current background color */
    int attrs;          /* attributes in effect */
    int toplin;         /* flag for topl stuff */
    int rawprint;       /* number of raw_printed lines since synch */
    int inmore;         /* non-zero if more() is active */
    int inread;         /* non-zero if reading a character */
    int intr;           /* non-zero if inread was interrupted */
    winid lastwin;      /* last window used for I/O */
    char dismiss_more;  /* extra character accepted at --More-- */
    int topl_utf8;      /* non-zero if utf8 in str */
    int mixed;          /* we are processing mixed output */
};

#endif /* WINDOW_STRUCTS */

#ifdef STATUS_HILITES
struct tty_status_fields {
    int idx;
    int color;
    int attr;
    int x, y;
    size_t lth;
    boolean valid;
    boolean dirty;
    boolean redraw;
    boolean sanitycheck; /* was 'last_in_row' */
};
#endif

#define MAXWIN 20 /* maximum number of windows, cop-out */

/* tty dependent window types */
#ifdef NHW_BASE
#undef NHW_BASE
#endif
#define NHW_BASE (NHW_LAST_TYPE + 1)

/* external declarations */

extern struct window_procs tty_procs;

/* port specific variable declarations */
extern winid BASE_WINDOW;

extern struct WinDesc *wins[MAXWIN];

extern struct DisplayDesc *ttyDisplay; /* the tty display descriptor */

extern char morc;         /* last character typed to xwaitforspace */
extern char defmorestr[]; /* default --more-- prompt */

/* port specific external function references */

/* ### getline.c ### */

extern void xwaitforspace(const char *);

/* ### termcap.c, video.c ### */

/*
 * TERM or NO_TERMS
 *
 * The tty windowport interface relies on lower-level support routines
 * to actually manipulate the terminal/display. Those are the right place
 * for doing strange and arcane things such as outputting escape sequences
 * to select a color or whatever.  wintty.c should concern itself with WHERE
 * to put stuff in a window.

 * The TERM routines are found in:
 *
 * !NO_TERMS:          termcap.c
 *
 * NO_TERMS:
 *            WINCON   sys/windows/consoletty.c
 *            MSDOS    sys/msdos/video.c
 *
 */
extern void backsp(void);
extern void cl_end(void);
extern void cl_eos(void);
extern void graph_on(void);
extern void graph_off(void);
extern void home(void);
extern void standoutbeg(void);
extern void standoutend(void);
extern int term_attr_fixup(int);
extern void term_clear_screen(void);
extern void term_curs_set(int);
extern void term_end_attr(int attr);
extern void term_end_color(void);
extern void term_end_extracolor(void);
extern void term_end_raw_bold(void);
extern void term_end_screen(void);
extern void term_start_attr(int attr);
extern void term_start_bgcolor(int color);
extern void term_start_color(int color);
extern void term_start_extracolor(uint32, uint16);
extern void term_start_raw_bold(void);
extern void term_start_screen(void);
extern void term_startup(int *, int *);
extern void term_shutdown(void);

extern int xputc(int);
extern void xputs(const char *);
#if 0
extern void revbeg(void);
extern void boldbeg(void);
extern void blinkbeg(void);
extern void dimbeg(void);
extern void m_end(void);
#endif
#if defined(SCREEN_VGA) || defined(SCREEN_8514) || defined(SCREEN_VESA)
extern void xputg(const glyph_info *, const glyph_info *);
#endif

/* ### topl.c ### */

extern void show_topl(const char *);
extern void remember_topl(void);
extern void addtopl(const char *);
extern void more(void);
extern void update_topl(const char *);
extern void putsyms(const char *);

/* ### wintty.c ### */

#ifdef CLIPPING
extern void setclipped(void);
#endif
extern void docorner(int, int, int);
extern void end_glyphout(void);
extern void g_putch(int);
#ifdef ENHANCED_SYMBOLS
#if defined(WIN32) || defined(UNIX) || defined(MSDOS)
extern void g_pututf8(uint8 *);
#endif
#endif /* ENHANCED_SYMBOLS */
extern void erase_tty_screen(void);
extern void win_tty_init(int);

/* tty interface */
extern void tty_init_nhwindows(int *, char **);
extern void tty_preference_update(const char *);
extern void tty_player_selection(void);
extern void tty_askname(void);
extern void tty_get_nh_event(void);
extern void tty_exit_nhwindows(const char *);
extern void tty_suspend_nhwindows(const char *);
extern void tty_resume_nhwindows(void);
extern winid tty_create_nhwindow(int);
extern void tty_clear_nhwindow(winid);
extern void tty_display_nhwindow(winid, boolean);
extern void tty_dismiss_nhwindow(winid);
extern void tty_destroy_nhwindow(winid);
extern void tty_curs(winid, int, int);
extern void tty_putstr(winid, int, const char *);
extern void tty_putmixed(winid window, int attr, const char *str);
extern void tty_display_file(const char *, boolean);
extern void tty_start_menu(winid, unsigned long);
extern void tty_add_menu(winid, const glyph_info *, const ANY_P *, char,
                         char, int, int, const char *, unsigned int);
extern void tty_end_menu(winid, const char *);
extern int tty_select_menu(winid, int, MENU_ITEM_P **);
extern char tty_message_menu(char, int, const char *);
extern void tty_mark_synch(void);
extern void tty_wait_synch(void);
#ifdef CLIPPING
extern void tty_cliparound(int, int);
#endif
#ifdef POSITIONBAR
extern void tty_update_positionbar(char *);
#endif
extern void tty_print_glyph(winid, coordxy, coordxy, const glyph_info *,
                            const glyph_info *);
extern void tty_raw_print(const char *);
extern void tty_raw_print_bold(const char *);
extern int tty_nhgetch(void);
extern int tty_nh_poskey(coordxy *, coordxy *, int *);
extern void tty_nhbell(void);
extern int tty_doprev_message(void);
extern char tty_yn_function(const char *, const char *, char);
extern void tty_getlin(const char *, char *);
extern int tty_get_ext_cmd(void);
extern void tty_number_pad(int);
extern void tty_delay_output(void);
#ifdef CHANGE_COLOR
extern void tty_change_color(int color, long rgb, int reverse);
#ifdef MAC68K
extern void tty_change_background(int white_or_black);
extern short set_tty_font_name(winid, char *);
#endif
extern char *tty_get_color_string(void);
#endif
extern void tty_status_enablefield(int, const char *, const char *, boolean);
extern void tty_status_init(void);
extern void tty_status_update(int, genericptr_t, int, int,
                              int, unsigned long *);

extern void genl_outrip(winid, int, time_t);

extern char *tty_getmsghistory(boolean);
extern void tty_putmsghistory(const char *, boolean);
extern void tty_update_inventory(int);
extern win_request_info *tty_ctrl_nhwindow(winid, int, win_request_info *);

#ifdef TTY_PERM_INVENT
extern void tty_refresh_inventory(int start, int stop, int y);
#endif

/* termcap is implied if NO_TERMS is not defined */
#ifndef NO_TERMS
#ifndef NO_TERMCAP_HEADERS
#include <curses.h>
#ifdef clear_screen /* avoid a conflict */
#undef clear_screen
#endif
#include <term.h>
#ifdef bell
#undef bell
#endif
#ifdef color_names
#undef color_names
#endif
#ifdef tone
#undef tone
#endif
#ifdef hangup
#undef hangup
#endif
#else
extern int tgetent(char *, const char *);
extern void tputs(const char *, int, int (*)(int));
extern int tgetnum(const char *);
extern int tgetflag(const char *);
extern char *tgetstr(const char *, char **);
extern char *tgoto(const char *, int, int);
#endif /* NO_TERMCAP_HEADERS */
#else  /* ?NO_TERMS */
#ifdef MAC68K
#ifdef putchar
#undef putchar
#undef putc
#endif
#define putchar term_putc
#define fflush term_flush
#define puts term_puts
extern int term_putc(int c);
extern int term_flush(void *desc);
extern int term_puts(const char *str);
#endif /* MAC68K */
#if defined(MSDOS) || defined(WIN32)
#if defined(SCREEN_BIOS) || defined(SCREEN_DJGPPFAST) || defined(WIN32)
#undef putchar
#undef putc
#undef puts
#define putchar(x) xputc(x) /* these are in video.c, nttty.c */
#define putc(x) xputc(x)
#define puts(x) xputs(x)
#endif /*SCREEN_BIOS || SCREEN_DJGPPFAST || WIN32 */
#ifdef POSITIONBAR
extern void video_update_positionbar(char *);
#endif
#endif /*MSDOS*/
#endif /* NO_TERMS */

#endif /* WINTTY_H */
