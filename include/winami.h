/* NetHack 5.0	winami.h	$NHDT-Date: 1781973091 2026/06/20 16:31:31 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.16 $ */
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1991. */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois, 1992, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file winami.h
 * @brief The Amiga display's own declarations.
 *
 * One of the display family. Its distinctive feature is that it is a graphical display of an older kind: real windows, but drawn with the system's own facilities and with the game
 * managing far more of the detail than a modern display would.
 *
 * The menu structures are the substantial part. A menu is held as a list of entries, each carrying not only what it stands for but the letter that selects it, the letter that selects
 * its group, and whether it is currently selected. The game supplies the first and the display assigns the rest -- so this is where the display's own bookkeeping about a menu lives,
 * separate from the menu the game asked for.
 *
 * @note Each entry can carry a glyph, so this display can show what an item looks like beside its name. That is why the entry holds a glyph as well as text.
 * @note A group selector is a letter that picks every entry of a kind at once. It is per entry rather than per group because which entries share a kind is decided as the menu is
 *       built.
 */

/**
 * @file winami.h
 * @brief Amiga 표시부 자신의 선언들.
 *
 * 표시부 가족의 하나. 그 특징은 그것이 더 예전 종류의 그래픽 표시부라는 것이다. 진짜 창이지만 시스템 자신의 기능으로 그려지고, 현대적인 표시부라면 하지 않을 만큼 게임이 훨씬 많은 세부를 관리한다.
 *
 * 메뉴 구조체가 그 중요한 부분이다. 메뉴는 항목의 목록으로 보관되며, 각각이 그것이 나타내는 것만이 아니라 그것을 고르는 글자, 그 묶음을 고르는 글자, 그리고 지금 선택되어 있는지를 지닌다. 게임이 첫 번째를 제공하고 표시부가 나머지를 배정한다. 그래서 이곳이 메뉴에 대한 표시부
 * 자신의 기록이 사는 곳이며, 게임이 요청한 메뉴와는 별개다.
 *
 * @note 각 항목이 글리프를 지닐 수 있으므로, 이 표시부는 항목의 이름 곁에 그것이 어떻게 보이는지를 보일 수 있다. 항목이 글이와 함께 글리프도 담는 이유가 그것이다.
 * @note 묶음 선택자는 한 종류의 모든 항목을 한꺼번에 고르는 글자다. 묶음마다가 아니라 항목마다인 것은, 어느 항목들이 종류를 공유하는지가 메뉴가 만들어지면서 정해지기 때문이다.
 */

#ifndef WINAMI_H
#define WINAMI_H

/**
 * @def MAXWINTAGS
 * @brief How many system window attributes this display can set at once.
 * @note A limit on one call rather than on windows. The system takes attributes as a list, and this is the size of the buffer that list is built in.
 */
/**
 * @def MAXWINTAGS
 * @brief 이 표시부가 한 번에 설정할 수 있는 시스템 창 속성의 개수.
 * @note 창에 대한 한계가 아니라 한 번의 호출에 대한 한계다. 시스템이 속성을 목록으로 받으며, 이것은 그 목록이 만들어지는 버퍼의 크기다.
 */
#define MAXWINTAGS 5

/*
 * Information specific to a menu window.  First a structure for each
 * menu entry, then the structure for each menu window.
 */
typedef struct amii_mi {
    struct amii_mi *next;
    anything identifier; /* Opaque type to identify this selection */
    long glyph;          /* Glyph for menu item */
    long count;          /* Object count */
    char selected;       /* Been selected? */
    char selector;       /* Char used to select this entry. */
    char gselector;      /* Group selector */
    char canselect;      /* Can user select this entry. */
    char attr;           /* Attribute for the line. */
    int color;           /* Color for the line (from menucolors). */
    char *str;           /* The text of the item. */
} amii_menu_item;

struct amii_menu {
    amii_menu_item *items; /* Starting pointer for item list. */
    amii_menu_item *last;  /* End pointer for item list. */
    const char *query;     /* Query string */
    int count;             /* Number of strings. */
    char chr;              /* Character to assign for accelerator */
    boolean has_glyphs;    /* Any item carries a real glyph (AMIV) */
};

/* descriptor for Amiga Intuition-based windows.  If we decide to cope with
 * tty-style windows also, then things will need to change. */
/* per-window data */
struct amii_WinDesc {
    xint16 type; /* type of window */
    struct amii_menu menu;
    boolean active; /* true if window is active */
    boolean wasup;  /* true if menu/text window was already open */
    short
        disprows; /* Rows displayed so far (used for paging in message win) */
    coordxy offx, offy;         /* offset from topleft of display */
    short vwx, vwy, vcx, vcy; /* View cursor location */
    short rows, cols;         /* dimensions */
    short curx, cury;         /* current cursor position */
    short maxrow, maxcol;     /* the maximum size used -- for INVEN wins */
    /* maxcol is also used by WIN_MESSAGE for */
    /* tracking the ^P command */
    char **data;        /* window data [row][column] */
    menu_item *mi;      /* Menu information */
    char *resp;         /* valid menu responses (for NHW_INVEN) */
    char *canresp;      /* cancel responses; 1st is the return value */
    char *morestr;      /* string to display instead of default */
                        /* amiga stuff */
    struct Window *win; /* Intuition window pointer */
    struct ExtNewWindow *newwin;        /* NewWindow alloc'd */
    struct TagItem wintags[MAXWINTAGS]; /* Tag items for this window */
    void *hook;         /* Hook structure pointer for tiles version */
#define FLMAP_INGLYPH 1 /* An NHW_MAP window is in glyph mode */
#define FLMAP_CURSUP 2  /* An NHW_MAP window has the cursor displayed */
#define FLMAP_SKIP 4
#define FLMSG_FIRST \
    1 /* First message in the NHW_MESSAGE window for this turn */
    long wflags;
    short cursx, cursy; /* Where the cursor is displayed at */
    short curs_apen,    /* Color cursor is displayed in */
        curs_bpen;
};

/* descriptor for intuition-based displays -- all the per-display data */
/* this is a generic thing - think of it as Screen level */

struct amii_DisplayDesc {
    /* we need this for Screen size (which will vary with display mode) */
    uchar rows, cols; /* width & height of display in text units */
    short xpix, ypix; /* width and height of display in pixels */
    int toplin;       /* flag for topl stuff */
    int rawprint;     /* number of raw_printed lines since synch */
    winid lastwin;    /* last window used for I/O */
};

typedef enum {
    WEUNK,
    WEKEY,
    WEMOUSE,
    WEMENU,
} WETYPE;

typedef struct WEVENT {
    WETYPE type;
    union {
        int key;
        struct {
            int x, y;
            int qual;
        } mouse;
        long menucode;
    } un;
} WEVENT;

#define MAXWIN 20 /* maximum number of windows, cop-out */

/* port specific variable declarations */
extern winid WIN_BASE;
extern winid WIN_OVER;
#define NHW_BASE (NHW_LAST_TYPE + 1)
#define NHW_OVER (NHW_LAST_TYPE + 2) /* overview window */

extern struct amii_WinDesc *amii_wins[MAXWIN + 1];

extern struct amii_DisplayDesc
    *amiIDisplay; /* the Amiga Intuition descriptor */

extern char morc;         /* last character typed to xwaitforspace */
extern char defmorestr[]; /* default --more-- prompt */

#endif /* WINAMI_H */
