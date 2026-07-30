/* NetHack 5.0  wintype.h       $NHDT-Date: 1781973092 2026/06/20 16:31:32 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.60 $ */
/* Copyright (c) David Cohrs, 1991                                */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file wintype.h
 * @brief The vocabulary the game and its display share.
 *
 * NetHack's core knows nothing about how it is displayed. It says "put this string here", "offer these choices", "draw this
 * thing at that square", and something else decides whether that means a terminal, a window with tiles, or an X11 display. This
 * file is the vocabulary those two halves speak in -- and nothing more than that, deliberately, so a display can be written
 * against it without including the game.
 *
 * The central idea is the generic value. A menu entry has to be able to stand for an object, a monster, a command, or a number,
 * and the core cannot tell the display which -- so the value is a union of everything it might be, small enough to be passed
 * around cheaply, with a separate list of names for what is in it.
 *
 * A window is an integer rather than a pointer for the same reason: the core must be able to name a window without knowing what
 * a window is made of.
 *
 * @note A few things here would naturally belong with the map or the symbols, and the existing comment says why they are here
 *       instead: X11 does not include most of the game's headers, and needs them.
 * @warning The generic value carries no record of which member is live. What is in it is agreed between whoever filled it and
 *          whoever reads it, so reading the wrong member yields a plausible value silently.
 */

/**
 * @file wintype.h
 * @brief 게임과 그 표시부가 공유하는 어휘.
 *
 * NetHack 의 코어는 자신이 어떻게 표시되는지 전혀 모른다. "이 문자열을 여기에 놓아라", "이 선택지를 제시하라", "저 칸에 이것을 그려라"라고 말하고, 그것이 터미널을 뜻하는지 타일이 있는 창을
 * 뜻하는지 X11 화면을 뜻하는지는 다른 것이 정한다. 이 파일은 그 두 절반이 말하는 어휘이며, 의도적으로 그 이상은 아니다. 그래서 표시부를 게임을 포함하지 않고 이것에 맞춰 작성할 수 있다.
 *
 * 핵심 발상은 범용 값이다. 메뉴 항목은 물건, 몬스터, 명령, 또는 숫자를 나타낼 수 있어야 하고, 코어는 표시부에게 그 중 무엇인지 알려 줄 수 없다. 그래서 그 값은 그것이 될 수 있는 모든 것의 공용체이며,
 * 값싸게 전달될 만큼 작고, 그 안에 무엇이 있는지에 대한 별도의 이름 목록이 딸린다.
 *
 * 창이 포인터가 아니라 정수인 것도 같은 이유다. 코어는 창이 무엇으로 이루어져 있는지 모른 채로 창을 지칭할 수 있어야 한다.
 *
 * @note 여기 있는 몇 가지는 본래 지도나 심볼 쪽에 속한다. 기존 주석이 왜 대신 여기에 있는지 밝힌다. X11 은 게임 헤더의 대부분을 포함하지 않으면서 그것들을 필요로 한다.
 * @warning 범용 값은 어느 멤버가 유효한지에 대한 기록을 지니지 않는다. 그 안에 무엇이 있는지는 채운 쪽과 읽는 쪽 사이의 합의이므로, 잘못된 멤버를 읽으면 조용히 그럴듯한 값이 나온다.
 */

#ifndef WINTYPE_H
#define WINTYPE_H

/**
 * @brief A handle naming one window.
 * @note An integer rather than a pointer so that the core can name a window without knowing anything about what a window is. The
 *       display keeps whatever it needs on its own side and looks it up by this.
 */
/**
 * @brief 창 하나를 지칭하는 핸들.
 * @note 포인터가 아니라 정수여서, 코어가 창이 무엇인지 전혀 모른 채로 창을 지칭할 수 있다. 표시부는 필요한 것을 자기 쪽에 보관하고 이 값으로 찾는다.
 */
typedef int winid; /* a window identifier */

/**
 * @brief A value that can be anything -- an object, a monster, a number, a command.
 *
 * The core hands the display values it must carry back untouched: this menu line stands for that object, this one for that
 * command. The display never interprets them, so one type serves for all of it.
 *
 * @note Kept no larger than a pointer, as the existing comment requires, so it can be passed by value everywhere rather than
 *       allocated and freed.
 * @warning Nothing here says which member holds the value. The names in @c any_types exist for code that needs to record that
 *          separately, and a union member read without that agreement gives a believable answer.
 */
/**
 * @brief 무엇이든 될 수 있는 값. 물건, 몬스터, 숫자, 명령.
 *
 * 코어는 표시부에게 손대지 않고 되돌려받아야 하는 값을 건넨다. 이 메뉴 줄은 저 물건을, 이 줄은 저 명령을 나타낸다. 표시부는 그것을 결코 해석하지 않으므로, 하나의 타입이 그 전부를 맡는다.
 *
 * @note 기존 주석이 요구하듯 포인터보다 크지 않게 유지된다. 그래서 할당하고 해제하는 대신 어디서나 값으로 전달될 수 있다.
 * @warning 어느 멤버가 값을 담고 있는지 말하는 것은 여기에 없다. @c any_types 의 이름들은 그것을 따로 기록해야 하는 코드를 위해 존재하며, 그 합의 없이 읽은 공용체 멤버는 그럴듯한 답을 준다.
 */
/* generic parameter - must not be any larger than a pointer */
typedef union any {
    genericptr_t a_void;
    struct obj *a_obj;
    struct monst *a_monst;
    int a_int;
    int a_xint16;
    int a_xint8;
    char a_char;
    schar a_schar;
    uchar a_uchar;
    unsigned int a_uint;
    long a_long;
    unsigned long a_ulong;
    coordxy a_coordxy;
    int *a_iptr;
    xint16 *a_xint16ptr;
    xint8 *a_xint8ptr;
    long *a_lptr;
    coordxy *a_coordxyptr;
    unsigned long *a_ulptr;
    unsigned *a_uptr;
    const char *a_string;
    int (*a_nfunc)(void);
    unsigned long a_mask32; /* used by status highlighting */
    int64 a_int64;
    uint64 a_uint64;
    /* add types as needed */
} anything;
/**
 * @def ANY_P
 * @brief The generic value written out longhand, for use in prototypes.
 * @note Exists because one old compiler mishandled the typedef in a prototype, as the existing comment records. It is used
 *       throughout for consistency rather than only where it was needed.
 */
/**
 * @def ANY_P
 * @brief 범용 값을 길게 적어 낸 것. 프로토타입에서 쓰기 위한 것.
 * @note 기존 주석이 기록하듯 어떤 예전 컴파일러가 프로토타입 안의 typedef 를 잘못 다뤘기 때문에 존재한다. 필요했던 곳에만 쓰지 않고 일관성을 위해 전반적으로 쓴다.
 */
#define ANY_P union any /* avoid typedef in prototypes
                         * (buggy old Ultrix compiler) */

/**
 * @brief Names for what a generic value is currently holding.
 * @note The union cannot say what is in it, so where that has to be recorded, one of these is stored alongside. Not every caller
 *       needs it -- most agree in advance -- which is why it is separate rather than part of the union.
 * @warning @c ANY_INVALID must stay last, since it is used as the count and the bound.
 */
/**
 * @brief 범용 값이 지금 무엇을 담고 있는지에 대한 이름들.
 * @note 공용체는 자기 안에 무엇이 있는지 말할 수 없으므로, 그것을 기록해야 하는 곳에서는 이 중 하나를 곁에 저장한다. 모든 호출자가 그것을 필요로 하지는 않는다. 대부분은 미리 합의한다. 그것이 공용체의
 *       일부가 아니라 따로 있는 이유다.
 * @warning @c ANY_INVALID 는 마지막에 있어야 한다. 개수와 상한으로 쓰이기 때문이다.
 */
/* symbolic names for the data types housed in anything */
enum any_types {
    ANY_VOID = 1,
    ANY_OBJ,         /* struct obj */
    ANY_MONST,       /* struct monst (not used) */
    ANY_INT,         /* int */
    ANY_CHAR,        /* char */
    ANY_UCHAR,       /* unsigned char */
    ANY_SCHAR,       /* signed char */
    ANY_UINT,        /* unsigned int */
    ANY_LONG,        /* long */
    ANY_ULONG,       /* unsigned long */
    ANY_IPTR,        /* pointer to int */
    ANY_UPTR,        /* pointer to unsigned int */
    ANY_LPTR,        /* pointer to long */
    ANY_ULPTR,       /* pointer to unsigned long */
    ANY_STR,         /* pointer to null-terminated char string */
    ANY_NFUNC,       /* pointer to function taking no args, returning int */
    ANY_MASK32,      /* 32-bit mask (stored as unsigned long) */
    ANY_INT16,      /* xint16 / coordxy (16-bit signed) */

    ANY_INVALID      /* leave this last */
};

/**
 * @brief One thing the player chose from a menu.
 * @note The count is separate from the identifier because a player may pick a quantity as well as an item -- "drop seven of
 *       these" is one selection with two parts.
 */
/**
 * @brief 플레이어가 메뉴에서 고른 것 하나.
 * @note 개수가 식별자와 따로 있는 것은, 플레이어가 항목과 함께 수량도 고를 수 있기 때문이다. "이것 중 일곱 개를 버려라"는 두 부분을 가진 하나의 선택이다.
 */
/* menu return list */
typedef struct mi {
    /**
     * @brief What was chosen, in whatever form the core supplied.
     * @note Passed back untouched, so the core can recognise its own value rather than matching on text.
     */
    /**
     * @brief 무엇이 골라졌는지. 코어가 제공한 형태 그대로.
     * @note 손대지 않고 되돌려지므로, 코어가 글을 맞춰 보는 대신 자기 값을 알아볼 수 있다.
     */
    anything item;     /* identifier */
    /**
     * @brief How many the player asked for.
     * @note Not always meaningful; a menu that does not accept counts leaves this at the value meaning "all of it".
     */
    /**
     * @brief 플레이어가 몇 개를 요청했는지.
     * @note 항상 의미가 있는 것은 아니다. 개수를 받지 않는 메뉴는 이것을 "전부"를 뜻하는 값으로 남긴다.
     */
    long count;        /* count */
    /**
     * @brief What the display noted about this line -- selected, excluded from inverting, and so on.
     */
    /**
     * @brief 표시부가 이 줄에 대해 기록한 것. 선택됨, 반전에서 제외됨 등.
     */
    unsigned itemflags; /* item flags */
} menu_item;
/**
 * @def MENU_ITEM_P
 * @brief The menu selection type written out longhand, for the same reason as @c ANY_P.
 */
/**
 * @def MENU_ITEM_P
 * @brief 메뉴 선택 타입을 길게 적어 낸 것. @c ANY_P 와 같은 이유다.
 */
#define MENU_ITEM_P struct mi

/* These would be in sym.h and display.h if they weren't needed to
   define the windowproc interface for X11 which doesn't include
   most of the main NetHack header files */

/**
 * @brief The plain way to draw something: one character and one colour.
 * @note Held as an index into the symbol table rather than the character itself, so a player's symbol changes take effect without
 *       anything re-deriving the drawing.
 */
/**
 * @brief 무언가를 그리는 평범한 방식. 문자 하나와 색 하나.
 * @note 문자 자체가 아니라 심볼 표에 대한 색인으로 보관된다. 그래서 플레이어의 심볼 변경이 그리기를 다시 유도하는 것 없이 적용된다.
 */
struct classic_representation {
    int color;
    int symidx;
};

/**
 * @brief The multi-byte way to draw something, held in both the forms a display might want.
 * @note The single code point and the encoded bytes are both kept because terminals differ in which they can take, and converting
 *       at draw time for every square would be wasteful.
 */
/**
 * @brief 무언가를 그리는 다중 바이트 방식. 표시부가 원할 수 있는 두 형태 모두로 보관된다.
 * @note 코드 포인트 하나와 인코딩된 바이트열을 둘 다 보관하는 것은, 터미널이 받아들일 수 있는 것이 서로 다르고 매 칸마다 그릴 때 변환하는 것이 낭비이기 때문이다.
 */
struct unicode_representation {
    uint32 utf32ch;
    uint8 *utf8str;
};

/**
 * @brief Every way of drawing one thing, so a display can take whichever it supports.
 *
 * The core does not know what the display can do, so rather than asking, it supplies all of them: a character and colour, a tile
 * number, a 256-colour index, a true colour, and where available a multi-byte sequence. Each display reads the one it wants and
 * ignores the rest.
 *
 * @note That is why this looks redundant. It is not alternatives to choose between but the same thing expressed several ways.
 */
/**
 * @brief 한 사물을 그리는 모든 방식. 표시부가 자신이 지원하는 것을 골라 쓸 수 있도록.
 *
 * 코어는 표시부가 무엇을 할 수 있는지 모르므로, 묻는 대신 전부를 제공한다. 문자와 색, 타일 번호, 256색 색인, 트루컬러, 그리고 가능한 곳에서는 다중 바이트 열. 각 표시부는 자신이 원하는 것을 읽고 나머지는
 * 무시한다.
 *
 * @note 이것이 잉여처럼 보이는 이유다. 그것은 골라야 할 대안이 아니라 같은 것을 여러 방식으로 표현한 것이다.
 */
typedef struct glyph_map_entry {
    unsigned glyphflags;
    struct classic_representation sym;
    uint32 customcolor;
    uint16 color256idx;
    short int tileidx;
#ifdef ENHANCED_SYMBOLS
    struct unicode_representation *u;
#endif
} glyph_map;

/* glyph plus additional info
   if you add fields or change the ordering, fix up the following:
        g_info initialization in display.c
        nul_glyphinfo initialization in display.c
 */
/**
 * @brief What to draw at one square, resolved as far as the core can take it.
 *
 * The glyph is the core's own answer to "what is here" -- this monster, this object, this terrain, seen or remembered. Everything
 * else in the structure is that answer worked out into drawing terms, so a display never has to interpret a glyph itself.
 *
 * @warning Fields are added at the end and the order matters. The existing comment lists the two initialisers in display.c that
 *          must be fixed up alongside any change here, and neither failure would be a compile error.
 */
/**
 * @brief 한 칸에 무엇을 그릴지. 코어가 할 수 있는 만큼 해결된 상태로.
 *
 * 글리프는 "여기 무엇이 있는가"에 대한 코어 자신의 답이다. 이 몬스터, 이 물건, 이 지형, 보이는 것 또는 기억된 것. 구조체의 나머지 전부는 그 답을 그리기의 용어로 풀어 낸 것이다. 그래서 표시부가 글리프를 직접
 * 해석할 필요가 결코 없다.
 *
 * @warning 필드는 끝에 더해지고 순서가 중요하다. 기존 주석은 여기의 어떤 변경과 함께 손봐야 하는 display.c 의 두 초기화를 나열하고 있으며, 어느 쪽을 빠뜨려도 컴파일 오류가 되지 않는다.
 */
typedef struct glyphinfo {
    int glyph;            /* the display entity */
    /**
     * @brief The character a terminal would show, already worked out.
     */
    /**
     * @brief 터미널이 보여 줄 문자. 이미 계산된 것.
     */
    int ttychar;
    /**
     * @brief The colour of the frame around this square, where a display draws one.
     */
    /**
     * @brief 이 칸을 둘러싼 테두리의 색. 표시부가 그것을 그리는 경우.
     */
    uint32 framecolor;
    /**
     * @brief The same thing expressed every other way the display might prefer.
     */
    /**
     * @brief 같은 것을 표시부가 선호할 수 있는 다른 모든 방식으로 표현한 것.
     */
    glyph_map gm;
} glyph_info;
/*#define GLYPH_INFO_P struct glyphinfo //not used*/

/**
 * @name How many a menu may pick
 * @brief What kind of choice a menu is offering.
 * @note Three cases rather than two, because a menu that picks nothing is still a menu -- it is how a list is shown to be read
 *       rather than chosen from.
 * @warning These values are assumed to be exactly zero, one and two elsewhere, as the existing comment records, so they cannot be
 *          renumbered.
 * @{
 */
/**
 * @name 메뉴가 고를 수 있는 개수
 * @brief 메뉴가 어떤 종류의 선택을 제시하는지.
 * @note 둘이 아니라 세 경우다. 아무것도 고르지 않는 메뉴도 여전히 메뉴다. 목록을 고르는 대상이 아니라 읽는 대상으로 보이는 방식이다.
 * @warning 기존 주석이 기록하듯 이 값들이 다른 곳에서 정확히 0, 1, 2 로 가정되므로 번호를 다시 매길 수 없다.
 * @{
 */
/* select_menu() "how" argument types */
/* [MINV_PICKMASK in monst.h assumes these have values of 0, 1, 2] */
#define PICK_NONE 0 /* user picks nothing (display only) */
#define PICK_ONE 1  /* only pick one */
#define PICK_ANY 2  /* can pick any amount */
/** @} */

/**
 * @name Window kinds
 * @brief What a window is for, which is all the core says about it.
 * @note The core asks for a kind and the display decides what that looks like -- a message area may be a line at the top or a
 *       pane at the side, and the core neither knows nor cares.
 * @note A port may define further kinds of its own, as the existing comment invites, which is why the last standard one is named.
 * @{
 */
/**
 * @name 창의 종류
 * @brief 창이 무엇을 위한 것인지. 코어가 그것에 대해 말하는 것은 그것뿐이다.
 * @note 코어는 종류를 요청하고 표시부가 그것이 어떻게 보일지 정한다. 메시지 영역은 위쪽의 한 줄일 수도 옆의 창일 수도 있고, 코어는 알지도 신경 쓰지도 않는다.
 * @note 기존 주석이 권하듯 포트가 자기 종류를 더 정의할 수 있다. 마지막 표준 종류에 이름이 붙어 있는 이유가 그것이다.
 * @{
 */
/* window types */
/* any additional port specific types should be defined in win*.h */
#define NHW_MESSAGE 1
#define NHW_STATUS 2
#define NHW_MAP 3
#define NHW_MENU 4
#define NHW_TEXT 5
#define NHW_PERMINVENT 6
#define NHW_LAST_TYPE NHW_PERMINVENT
/** @} */

/**
 * @name Text attributes
 * @brief How a line of text should be emphasised.
 *
 * The values match the terminal standard, which is a convenience rather than a requirement: a terminal port can pass them through
 * untranslated, and a graphical port maps them to whatever it does instead.
 *
 * @warning The last two are not appearances at all. One marks a message as needing to be seen, the other asks that it not be kept
 *          in the message history, and each may be combined with one real attribute -- so an attribute argument must be masked
 *          rather than compared.
 * @{
 */
/**
 * @name 텍스트 속성
 * @brief 텍스트 한 줄을 어떻게 강조할지.
 *
 * 값들이 터미널 표준과 일치하는데, 그것은 요구가 아니라 편의다. 터미널 포트는 그것을 번역 없이 그대로 넘길 수 있고, 그래픽 포트는 자신이 대신 하는 것으로 대응시킨다.
 *
 * @warning 마지막 둘은 외형이 전혀 아니다. 하나는 메시지가 반드시 보여져야 함을 표시하고, 다른 하나는 그것을 메시지 이력에 남기지 말아 달라고 요청하며, 각각 하나의 실제 속성과 결합될 수 있다. 그래서 속성
 *          인자는 비교되는 것이 아니라 마스킹되어야 한다.
 * @{
 */
/* attribute types for putstr; the same as the ANSI value, for convenience */
#define ATR_NONE       0
#define ATR_BOLD       1
#define ATR_DIM        2
#define ATR_ITALIC     3
#define ATR_ULINE      4
#define ATR_BLINK      5
#define ATR_INVERSE    7
/* not a display attribute but passed to putstr() as an attribute;
   can be masked with one regular display attribute */
#define ATR_URGENT    16
#define ATR_NOHISTORY 32
/** @} */

/**
 * @name Mouse buttons
 * @brief Which button was pressed, when a position came from a click.
 * @note Only two, and named by number rather than by side, since which side is which is a matter of the player's own setup.
 * @{
 */
/**
 * @name 마우스 버튼
 * @brief 위치가 클릭에서 왔을 때 어느 버튼이 눌렸는지.
 * @note 둘뿐이며, 좌우가 아니라 번호로 이름 붙어 있다. 어느 쪽이 어느 쪽인지는 플레이어 자신의 설정 문제이기 때문이다.
 * @{
 */
/* nh_poskey() modifier types */
#define CLICK_1 1
#define CLICK_2 2
#define NUM_MOUSE_BUTTONS 2
/** @} */

/**
 * @def WIN_ERR
 * @brief The handle value meaning "no window".
 * @note Negative so it cannot collide with any real handle, whatever numbering a display chooses.
 */
/**
 * @def WIN_ERR
 * @brief "창이 없음"을 뜻하는 핸들 값.
 * @note 음수여서, 표시부가 어떤 번호 체계를 고르든 실제 핸들과 충돌할 수 없다.
 */
/* invalid winid */
#define WIN_ERR ((winid) -1)

/**
 * @name Menu keys
 * @brief The keystrokes that work inside a menu regardless of what the menu is about.
 *
 * Defined centrally so every display offers the same keys, and given as characters because that is what a player types. A player
 * may remap them, so these are defaults rather than fixed bindings.
 *
 * @note The page and whole-menu forms are deliberately paired -- select this page, select everything -- since a long menu makes the
 *       distinction matter.
 * @note The two shifting keys are not for navigating the menu itself; the existing comment records that they exist for the
 *       persistent inventory window.
 * @{
 */
/**
 * @name 메뉴 키
 * @brief 메뉴가 무엇에 관한 것이든 그 안에서 작동하는 키들.
 *
 * 모든 표시부가 같은 키를 제공하도록 한곳에 정의되며, 플레이어가 입력하는 것이 그것이므로 문자로 주어진다. 플레이어가 다시 대응시킬 수 있으므로, 이들은 고정된 대응이 아니라 기본값이다.
 *
 * @note 페이지 단위와 전체 단위 형태가 의도적으로 짝을 이룬다. 이 페이지를 선택, 전부를 선택. 긴 메뉴에서는 그 구별이 중요해지기 때문이다.
 * @note 두 개의 이동 키는 메뉴 자체를 돌아다니기 위한 것이 아니다. 기존 주석은 그것들이 지속 소지품 창을 위해 존재한다고 기록하고 있다.
 * @{
 */
/* menu window keyboard commands (may be mapped); menu_shift_right and
   menu_shift_left are for interacting with persistent inventory window */
/* clang-format off */
#define MENU_FIRST_PAGE         '^'
#define MENU_LAST_PAGE          '|'
#define MENU_NEXT_PAGE          '>'
#define MENU_PREVIOUS_PAGE      '<'
#define MENU_SHIFT_RIGHT        '}'
#define MENU_SHIFT_LEFT         '{'
#define MENU_SELECT_ALL         '.'
#define MENU_UNSELECT_ALL       '-'
#define MENU_INVERT_ALL         '@'
#define MENU_SELECT_PAGE        ','
#define MENU_UNSELECT_PAGE      '\\'
#define MENU_INVERT_PAGE        '~'
#define MENU_SEARCH             ':'
/** @} */

/**
 * @name Menu line flags
 * @brief What the display should know about one line of a menu.
 * @note @c SKIPINVERT and @c SKIPMENUCOLORS both exclude a line from something applied to the menu as a whole -- a heading should
 *       not be selected by "invert all", and a line already coloured for its own reason should not be recoloured.
 * @{
 */
/**
 * @name 메뉴 줄 플래그
 * @brief 표시부가 메뉴의 한 줄에 대해 알아야 하는 것.
 * @note @c SKIPINVERT 와 @c SKIPMENUCOLORS 는 둘 다 메뉴 전체에 적용되는 것에서 한 줄을 제외한다. 제목은 "전부 반전"으로 선택되어서는 안 되고, 자기 이유로 이미 색이 입혀진 줄은 다시 색이 입혀져서는
 *       안 된다.
 * @{
 */
#define MENU_ITEMFLAGS_NONE           0x0000000U
#define MENU_ITEMFLAGS_SELECTED       0x0000001U
#define MENU_ITEMFLAGS_SKIPINVERT     0x0000002U
#define MENU_ITEMFLAGS_SKIPMENUCOLORS 0x0000004U
/** @} */

/**
 * @name Menu behaviour requests
 * @brief How a menu should behave, as opposed to what is in it.
 * @note Requests rather than instructions. The existing comment is explicit that not every port will honour a new flag at first, so
 *       the standard behaviour has to remain a sensible fallback.
 * @{
 */
/**
 * @name 메뉴 동작 요청
 * @brief 메뉴에 무엇이 들어 있는지가 아니라 메뉴가 어떻게 동작해야 하는지.
 * @note 지시가 아니라 요청이다. 기존 주석은 모든 포트가 새 플래그를 처음부터 받아들이지는 않을 것이라고 명시하고 있다. 그래서 표준 동작이 합당한 대비책으로 남아 있어야 한다.
 * @{
 */
/* 5.0+ enhanced menu flags that not all window ports are likely to
 * support initially.
 *
 * As behavior and appearance modification flags are added, the various
 * individual window ports will likely have to be updated to respond
 * to the flags in an appropriate way.
 */

#define MENU_BEHAVE_STANDARD      0x0000000U
#define MENU_BEHAVE_PERMINV       0x0000001U
/** @} */

/**
 * @brief Whether the persistent inventory window is being turned on, turned off, or neither.
 * @note Three states rather than a boolean because the interesting moment is the transition: code that reacts to the window
 *       appearing or disappearing needs to know which, and most of the time neither is happening.
 */
/**
 * @brief 지속 소지품 창이 켜지는 중인지, 꺼지는 중인지, 아니면 둘 다 아닌지.
 * @note 논리값이 아니라 세 상태인 것은 흥미로운 순간이 그 전환이기 때문이다. 창이 나타나거나 사라지는 것에 반응하는 코드는 어느 쪽인지 알아야 하고, 대부분의 시간에는 어느 쪽도 일어나지 않는다.
 */
enum perm_invent_toggles {
    toggling_off = -1,
    toggling_not =  0,
    toggling_on  =  1
};

/**
 * @brief The pieces a persistent inventory setting is made of.
 * @warning @c InvSparse is not a setting on its own. The existing comment states it must be combined with one of the others, so a
 *          value of it alone is not valid.
 */
/**
 * @brief 지속 소지품 설정이 이루어지는 조각들.
 * @warning @c InvSparse 는 그 자체로 하나의 설정이 아니다. 기존 주석은 그것이 다른 것 중 하나와 결합되어야 한다고 밝히고 있으므로, 그것 하나만인 값은 유효하지 않다.
 */
/* perm_invent modes */
enum inv_mode_bits {
    InvNormal   = 1,
    InvShowGold = 2,
    InvSparse   = 4, /* must be ORed with Normal or ShowGold to be valid */
    InvInUse    = 8
};
/**
 * @brief The valid combinations, named as the settings a player can actually choose.
 * @note Named rather than left to be assembled, so that the invalid combinations simply have no name -- which is how the
 *       constraint on the sparse bit is expressed without needing to be checked.
 * @note The "sparse" name is admitted to be confusing by the existing comment: it shows every inventory letter including the empty
 *       slots, and only means anything to the terminal display.
 */
/**
 * @brief 유효한 조합들. 플레이어가 실제로 고를 수 있는 설정으로 이름 붙인 것.
 * @note 조립되도록 남기지 않고 이름을 붙인 것은, 유효하지 않은 조합에는 그냥 이름이 없게 하기 위함이다. 희소 비트에 대한 제약이 검사될 필요 없이 표현되는 방식이 그것이다.
 * @note 기존 주석은 "희소"라는 이름이 혼란스럽다고 인정한다. 그것은 빈 칸까지 포함해 모든 소지품 글자를 보이며, 터미널 표시부에게만 의미가 있다.
 */
enum inv_modes { /* 'perminv_mode' option settings */
    InvOptNone       = 0,           /* no perm_invent */
    InvOptOn         = InvNormal,   /* 1 */
    InvOptFull       = InvShowGold, /* 2 */
#if 1 /*#ifdef TTY_PERM_INVENT*/
    /* confusingly-named "sparse mode" shows all inventory letters, even when
       their slots are empty; only meaningful for tty's perm_invent */
    InvOptOn_grid    = InvNormal | InvSparse,   /* 5 */
    InvOptFull_grid  = InvShowGold | InvSparse, /* 6 */
#endif
    InvOptInUse      = InvInUse,    /* 8 */
};

/**
 * @brief A display's answer about why it cannot do what was asked.
 *
 * The core cannot know whether a display has room for a persistent inventory window, or whether it has finished starting up. So it
 * asks, and these are the possible replies -- and they are distinct because the core reacts differently: too small is worth telling
 * the player about, too early is worth trying again later.
 */
/**
 * @brief 표시부가 요청받은 것을 왜 할 수 없는지에 대한 답.
 *
 * 코어는 표시부에 지속 소지품 창을 놓을 자리가 있는지, 또는 그것이 시작을 마쳤는지 알 수 없다. 그래서 묻고, 이들이 가능한 답이다. 그리고 코어가 서로 다르게 반응하므로 이들이 구별된다. 너무 작다는 것은 플레이어에게
 * 알릴 만하고, 너무 이르다는 것은 나중에 다시 시도할 만하다.
 */
enum to_core_flags {
    active           = 0x001,
    too_small        = 0x002,
    prohibited       = 0x004,
    no_init_done     = 0x008,
    too_early        = 0x010,
};

/**
 * @brief What the core is asking the display to do or tell it.
 * @note One request per call, so a single exchange serves for setting a mode, asking what the display can manage, and adjusting the
 *       appearance of a prompt.
 */
/**
 * @brief 코어가 표시부에게 무엇을 하라고, 또는 무엇을 알려 달라고 요청하는지.
 * @note 호출마다 요청 하나다. 그래서 한 번의 주고받음이 모드를 정하는 일, 표시부가 무엇을 감당할 수 있는지 묻는 일, 프롬프트의 외형을 조정하는 일을 함께 맡는다.
 */
enum from_core_requests {
    invalid_core_request = 0,
    set_mode             = 1,
    request_settings     = 2,
    set_menu_promptstyle = 3,
};

/**
 * @brief What the display tells the core about itself.
 * @note Both what it needs and what it has, so the core can see the shortfall rather than being told only that something did not
 *       fit.
 * @note @c use_update_inventory asks the core to fall back to the older, coarser way of refreshing the window, for a display that
 *       cannot manage the per-slot interface.
 */
/**
 * @brief 표시부가 자신에 대해 코어에게 알려 주는 것.
 * @note 필요한 것과 가진 것을 함께 알려 주므로, 코어가 무언가 맞지 않았다는 말만 듣는 대신 그 부족분을 볼 수 있다.
 * @note @c use_update_inventory 는 칸 단위 인터페이스를 감당할 수 없는 표시부를 위해, 코어에게 더 예전의 거친 갱신 방식으로 돌아가 달라고 요청한다.
 */
struct to_core {
    long tocore_flags;
    boolean active;
    boolean use_update_inventory;    /* disable the newer slot interface */
    int maxslot;
    int needrows, needcols;
    int haverows, havecols;
};

/**
 * @brief What the core tells the display.
 * @note Carries the request itself plus whatever that request needs, rather than a separate structure per request -- there are few
 *       enough that one shape serves.
 */
/**
 * @brief 코어가 표시부에게 알려 주는 것.
 * @note 요청마다 별도의 구조체를 두는 대신, 요청 자체와 그 요청이 필요로 하는 것을 함께 지닌다. 요청이 적어서 하나의 모양으로 충분하다.
 */
struct from_core {
    enum from_core_requests core_request;
    enum inv_modes invmode;
    color_attr menu_promptstyle;
};

/**
 * @brief One exchange between core and display, holding both directions.
 * @note A single structure passed in and out rather than a return value, so the display can answer in the same call it was asked --
 *       which keeps the interface to one function however many requests are added.
 */
/**
 * @brief 코어와 표시부 사이의 한 번의 주고받음. 양방향을 함께 담는다.
 * @note 반환값이 아니라 하나의 구조체가 들고 나간다. 그래서 표시부가 요청받은 그 호출에서 답할 수 있고, 요청이 몇 개가 더해지든 인터페이스가 함수 하나로 유지된다.
 */
struct win_request_info_t {
    struct to_core tocore;
    struct from_core fromcore;
};

typedef struct win_request_info_t win_request_info;
/**
 * @var zerowri
 * @brief An all-zero exchange, to copy from before filling one in.
 * @note Exists so a caller starts from a known state rather than clearing fields it does not know about -- which matters because a
 *       leftover field would be read as a request.
 */
/**
 * @var zerowri
 * @brief 전부 0인 주고받음. 하나를 채우기 전에 복사해 오기 위한 것.
 * @note 호출자가 자신이 모르는 필드를 지우는 대신 알려진 상태에서 시작하도록 존재한다. 남아 있는 필드가 요청으로 읽히기 때문에 중요하다.
 */
extern win_request_info zerowri;    /* windows.c */

/* #define CORE_INVENT */

/* In a binary with multiple window interfaces linked in, this is
 * a structure to track certain interface capabilities that cannot be
 * statically done at compile time. Some of them can be toggled and
 * the core needs to know if they are active or not at the time.
 */

/**
 * @brief Whether the map is currently being drawn as characters or as tiles.
 * @note A display may be able to do both and switch between them, which is why this is a current state rather than a build-time
 *       decision.
 */
/**
 * @brief 지도가 지금 문자로 그려지고 있는지 타일로 그려지고 있는지.
 * @note 표시부가 둘 다 할 수 있고 그 사이를 전환할 수도 있다. 그래서 이것이 빌드 시점의 결정이 아니라 현재 상태다.
 */
enum win_display_modes {
    wdmode_traditional = 0,
    wdmode_tiled
};

/**
 * @brief Display capabilities the core cannot work out at build time.
 * @note The existing comment above explains the need: a binary may have several displays linked in, and some of what they can do can
 *       be switched while running -- so the core has to be able to ask rather than assume.
 */
/**
 * @brief 코어가 빌드 시점에 알아낼 수 없는 표시부의 능력.
 * @note 위의 기존 주석이 그 필요를 설명한다. 하나의 실행 파일에 여러 표시부가 함께 링크될 수 있고, 그것들이 할 수 있는 것 중 일부는 실행 중에 전환될 수 있다. 그래서 코어가 가정하는 대신 물을 수 있어야 한다.
 */
struct win_settings {
    enum win_display_modes wdmode;
    /**
     * @brief The colour of the border drawn around the map, where a display draws one.
     */
    /**
     * @brief 지도를 둘러싼 테두리의 색. 표시부가 그것을 그리는 경우.
     */
    uint32 map_frame_color;
};

/* clang-format on */

#endif /* WINTYPE_H */
