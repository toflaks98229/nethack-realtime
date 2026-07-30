/* NetHack 5.0	coloratt.c	$NHDT-Date: 1781973043 2026/06/20 16:30:43 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.20 $ */
/* Copyright (c) Pasi Kallinen, 2024 */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file coloratt.c
 * @brief Colors and attributes: naming them, parsing them, and fitting them to
 *        what the terminal can actually show.
 *
 * The player names colors in configuration -- for menu coloring, status
 * highlighting, and custom symbols -- so those names have to be recognized and
 * turned into something the interface can use.
 *
 * What that is depends on the terminal. A build may have sixteen colors, two
 * hundred and fifty-six, or the full range, and a color asked for is mapped to
 * the nearest one available rather than refused.
 *
 * @note Attributes (bold, inverse, blink, and so on) travel alongside color and
 *       are parsed by the same code, since configuration mixes them freely.
 */

/**
 * @file coloratt.c
 * @brief 색과 속성. 이름 붙이기, 해석하기, 그리고 터미널이 실제로 보여 줄 수 있는
 *        것에 맞추기.
 *
 * 플레이어는 설정에서 색을 이름으로 지정한다. 메뉴 색칠, 상태 강조, 사용자 지정
 * 기호 등에 쓰이므로, 그 이름들을 알아보고 인터페이스가 쓸 수 있는 것으로 바꿔야
 * 한다.
 *
 * 그것이 무엇인지는 터미널에 달려 있다. 빌드에 따라 16색일 수도, 256색일 수도,
 * 전체 범위일 수도 있으며, 요청된 색은 거부되는 대신 사용 가능한 가장 가까운
 * 색으로 대응된다.
 *
 * @note 속성(굵게, 반전, 깜빡임 등)은 색과 함께 다니며 같은 코드가 해석한다.
 *       설정에서 둘을 자유롭게 섞어 쓰기 때문이다.
 */

#include "hack.h"

/** A colour's name paired with the colour it means. */
/** 색의 이름과 그것이 뜻하는 색을 짝지은 것. */
struct color_names {
    const char *name;
    int color;
};

/**
 * @var colornames
 * @brief The colour names a player may write in configuration.
 *
 * Split into two halves by an entry with no name. Everything before it is a colour's proper name; everything after is an alias for one of those, as the existing comment records.
 *
 * The division matters because the two halves are used differently. Matching a name the player typed searches the whole table, so aliases work; listing the colours for the player to choose from stops at the separator, so the list does not offer the same colour under
 * four names.
 *
 * @note The aliases are there because players spell colours differently -- grey and gray, purple and magenta -- and because some names describe the same colour more naturally than the internal one does.
 * @note "Transparent" is an alias for having no colour, which is the same thing from the point of view of drawing but a different thing to think about: one means unstated, the other means see through.
 * @warning The separator entry's colour value is not meaningful and must not be used. Code walking the table has to stop at the missing name rather than at a colour it recognises.
 */
/**
 * @var colornames
 * @brief 플레이어가 설정에 쓸 수 있는 색 이름들.
 *
 * 이름 없는 항목으로 두 반쪽으로 나뉜다. 기존 주석이 기록하듯 그 앞의 모든 것은 색의 제 이름이고, 그 뒤의 모든 것은 그중 하나에 대한 딴이름이다.
 *
 * 그 나눔이 중요한 것은 두 반쪽이 다르게 쓰이기 때문이다. 플레이어가 입력한 이름을 맞추는 것은 표 전체를 찾으므로 딴이름이 통한다. 플레이어가 고를 색을 나열하는 것은 그 구분자에서 멈추므로, 그 목록이 같은 색을 네 이름으로 내놓지 않는다.
 *
 * @note 딴이름이 있는 것은 플레이어가 색을 다르게 적기 때문이며, grey와 gray, purple과 magenta처럼. 그리고 어떤 이름이 내부의 이름보다 같은 색을 더 자연스럽게 서술하기 때문이다.
 * @note "투명"은 색이 없음에 대한 딴이름이며, 그것은 그리기의 관점에서는 같은 것이지만 생각하기에는 다른 것이다. 하나는 밝히지 않음을 뜻하고 다른 하나는 꿰뚫어 보임을 뜻한다.
 * @warning 구분자 항목의 색 값은 뜻이 없으며 쓰여서는 안 된다. 그 표를 걸어가는 코드는 자기가 알아보는 색이 아니라 없는 이름에서 멈춰야 한다.
 */
static const struct color_names colornames[] = {
    { "black", CLR_BLACK },
    { "red", CLR_RED },
    { "green", CLR_GREEN },
    { "brown", CLR_BROWN },
    { "blue", CLR_BLUE },
    { "magenta", CLR_MAGENTA },
    { "cyan", CLR_CYAN },
    { "gray", CLR_GRAY },
    { "orange", CLR_ORANGE },
    { "light green", CLR_BRIGHT_GREEN },
    { "yellow", CLR_YELLOW },
    { "light blue", CLR_BRIGHT_BLUE },
    { "light magenta", CLR_BRIGHT_MAGENTA },
    { "light cyan", CLR_BRIGHT_CYAN },
    { "white", CLR_WHITE },
    { "no color", NO_COLOR },
    { (const char *) 0, CLR_BLACK }, /* everything after this is an alias */
    { "transparent", NO_COLOR },
    { "purple", CLR_MAGENTA },
    { "light purple", CLR_BRIGHT_MAGENTA },
    { "bright purple", CLR_BRIGHT_MAGENTA },
    { "grey", CLR_GRAY },
    { "bright red", CLR_ORANGE },
    { "bright green", CLR_BRIGHT_GREEN },
    { "bright blue", CLR_BRIGHT_BLUE },
    { "bright magenta", CLR_BRIGHT_MAGENTA },
    { "bright cyan", CLR_BRIGHT_CYAN }
};

/** An attribute's name paired with the attribute it means. */
/** 속성의 이름과 그것이 뜻하는 속성을 짝지은 것. */
struct attr_names {
    const char *name;
    int attr;
};

/**
 * @var attrnames
 * @brief The attribute names a player may write in configuration.
 * @note Divided by a nameless entry into proper names and aliases, the same convention the colour table uses -- so anything that walks one table can walk the other.
 * @note "Normal" is an alias for having no attribute, because a player who wants plain text is likelier to write that than to write "none".
 */
/**
 * @var attrnames
 * @brief 플레이어가 설정에 쓸 수 있는 속성 이름들.
 * @note 색 표가 쓰는 것과 같은 관례로 이름 없는 항목에 의해 제 이름과 딴이름으로 나뉘므로, 한 표를 걸어가는 무엇이든 다른 표를 걸어갈 수 있다.
 * @note "normal"은 속성이 없음에 대한 딴이름이다. 맨 글을 원하는 플레이어가 "none"이라고 쓰기보다 그렇게 쓸 가능성이 크기 때문이다.
 */
static const struct attr_names attrnames[] = {
    { "none", ATR_NONE },
    { "bold", ATR_BOLD },
    { "dim", ATR_DIM },
    { "italic", ATR_ITALIC },
    { "underline", ATR_ULINE },
    { "blink", ATR_BLINK },
    { "inverse", ATR_INVERSE },
    { (const char *) 0, ATR_NONE }, /* everything after this is an alias */
    { "normal", ATR_NONE },
    { "uline", ATR_ULINE },
    { "reverse", ATR_INVERSE },
};

/**
 * @name Colour kind abbreviations
 * @brief Short names for the three colour kinds, so the table below stays readable.
 * @note Abbreviated only because the table has three hundred entries and the full names would push each line past its width. They are undefined after the table.
 * @{
 */
/**
 * @name 색 종류 줄임말
 * @brief 세 색 종류를 위한 짧은 이름. 아래 표가 읽을 수 있는 채로 머물도록.
 * @note 표가 삼백 항목을 가지고 온전한 이름이 각 줄을 그 너비를 넘어 밀 것이기 때문에만 줄여졌다. 그것들은 표 뒤에 정의 해제된다.
 * @{
 */

/* { colortyp, tableindex, rgbindx, name, r, g, b }, */

#define NHC nh_color
#define NOC no_color
#define RGBC rgb_color

/** @} */

/**
 * @var colortable
 * @brief Every colour the game knows, with its name and its actual red, green and blue values.
 *
 * The bridge between a colour named in configuration and a colour a terminal can produce. Because what a terminal can produce varies -- sixteen colours, two hundred and fifty-six, or the full range -- a colour asked for is matched to the nearest available one rather
 * than refused, and matching needs the actual values rather than the names.
 *
 * Two distinct groups. The first sixteen are the basic colours the game has always had, which every interface can show. The rest are extended colours, available only where the terminal supports them.
 *
 * @note Each entry carries two index numbers, and they count differently. One is the entry's position in this whole table; the other restarts at zero for the extended colours, because those are addressed separately by the interfaces that support them. Reading one where
 *       the other is meant is off by sixteen and yields a plausible colour.
 * @note One entry is neither basic nor extended but the absence of colour, sitting among the basic ones at the position the interfaces reserve for it.
 * @warning A dozen names appear twice -- most of the basic ones, including black, red, green, brown, blue, magenta, cyan, gray, orange, yellow and white -- once as a basic colour and once as an extended one, and several of the pairs hold different values. The basic
 *          entry's values are what that colour has always looked like; the extended entry's are the standard web colour of the same name, which for green and brown and blue is visibly not the same shade. So a configuration naming one of these gets a different colour
 *          depending on which half is searched, and the pairs are genuinely distinct rather than duplicates to be merged.
 * @note The basic entries' values are approximations chosen to look right, not definitions. A terminal showing sixteen colours produces its own shades and these values are used only for finding the nearest match.
 * @note Every entry's values are repeated as a comment in hexadecimal, which is how the extended colours are normally written down -- so an entry can be checked against a colour chart without converting anything.
 */
/**
 * @var colortable
 * @brief 게임이 아는 모든 색, 그 이름과 실제 빨강, 초록, 파랑 값과 함께.
 *
 * 설정에서 이름 지어진 색과 터미널이 낼 수 있는 색 사이의 다리다. 터미널이 낼 수 있는 것이 열여섯 색, 이백오십육 색, 또는 전체 범위로 달라지기 때문에, 요청된 색은 거부되는 대신 쓸 수 있는 가장 가까운 것에 맞춰지며, 맞추기는 이름이 아니라 실제 값을 필요로 한다.
 *
 * 두 별개의 묶음이다. 첫 열여섯은 게임이 언제나 가져 온 기본 색이며, 모든 인터페이스가 보일 수 있다. 나머지는 확장 색이며, 터미널이 그것을 뒷받침하는 곳에서만 쓸 수 있다.
 *
 * @note 각 항목이 두 색인 번호를 지니며, 그것들이 다르게 센다. 하나는 이 표 전체에서 그 항목의 자리다. 다른 하나는 확장 색에 대해 영에서 다시 시작하는데, 그것들이 그것을 뒷받침하는 인터페이스에 의해 따로 주소 지어지기 때문이다. 다른 것이 뜻해진 곳에서 하나를 읽는 것은 열여섯만큼 어긋나며
 *       그럴듯한 색을 낸다.
 * @note 한 항목은 기본도 확장도 아니라 색의 없음이며, 인터페이스가 그것을 위해 남겨 둔 자리에서 기본 것들 사이에 앉아 있다.
 * @warning 열두 이름이 두 번 나타난다. black, red, green, brown, blue, magenta, cyan, gray, orange, yellow, white를 포함해 기본 이름 대부분이다. 한 번은 기본 색으로, 한 번은 확장 색으로. 그리고 그 짝 가운데 여럿이 다른 값을 담는다. 기본 항목의 값은 그 색이 언제나 어떻게 보여 왔는지다.
 *          확장 항목의 값은 같은 이름의 표준 웹 색이며, green과 brown과 blue에 대해 그것은 눈에 보이게 같은 빛깔이 아니다. 그래서 이 가운데 하나를 이름 짓는 설정은 어느 반쪽이 찾아지는지에 따라 다른 색을 얻으며, 그 짝들은 합쳐질 중복이 아니라 진짜로 별개다.
 * @note 기본 항목의 값은 정의가 아니라 옳게 보이도록 골라진 근사다. 열여섯 색을 보이는 터미널은 자기 빛깔을 내며, 이 값은 가장 가까운 맞음을 찾는 데만 쓰인다.
 * @note 모든 항목의 값이 십육진수로 주석에 되풀이되며, 그것이 확장 색이 보통 적히는 방식이다. 그래서 항목이 무엇도 변환하지 않고 색 표와 견주어 확인될 수 있다.
 */
const struct nethack_color colortable[] = {
    { NHC,    0,   0, "black", 0x00, 0x00, 0x00 },      /*        CLR_BLACK */
    { NHC,    1,   0, "red", 0xFF, 0x00, 0x00 },        /*          CLR_RED */
    { NHC,    2,   0, "green", 0x22, 0x8B, 0x22 },      /*        CLR_GREEN */
    { NHC,    3,   0, "brown", 0xA5, 0x2A, 0x2A },      /*        CLR_BROWN */
    { NHC,    4,   0, "blue", 0x00, 0x00, 0xFF },       /*         CLR_BLUE */
    { NHC,    5,   0, "magenta", 0xFF, 0x00, 0xFF },    /*      CLR_MAGENTA */
    { NHC,    6,   0, "cyan", 0x00, 0xFF, 0xFF },       /*         CLR_CYAN */
    { NHC,    7,   0, "gray", 0x80, 0x80, 0x80 },       /*         CLR_GRAY */
    { NOC,    8,   0, "nocolor", 0x00, 0x00, 0x00 },    /*         NO_COLOR */
    { NHC,    9,   0, "orange", 0xFF, 0xA5, 0x00 },     /*       CLR_ORANGE */
    { NHC,   10,   0, "bright-green",
                      0x00, 0x80, 0x00 },               /* CLR_BRIGHT_GREEN */
    { NHC,   11,   0, "yellow", 0xFF, 0xFF, 0x00 },     /*       CLR_YELLOW */
    { NHC,   12,   0, "bright-blue", 0xAD, 0xD8, 0xE6 }, /* CLR_BRIGHT_BLUE */
    { NHC,   13,   0, "bright-magenta",
                      0x93, 0x70, 0xDB },             /* CLR_BRIGHT_MAGENTA */
    { NHC,   14,   0, "light-cyan", 0xE0, 0xFF, 0xFF }, /*  CLR_BRIGHT_CYAN */
    { NHC,   15,   0, "white", 0xFF, 0xFF, 0xFF },      /*        CLR_WHITE */
    { RGBC,  16,   0, "maroon", 0x80, 0x00, 0x00 },              /* #800000 */
    { RGBC,  17,   1, "dark-red", 0x8B, 0x00, 0x00 },            /* #8B0000 */
    { RGBC,  18,   2, "brown", 0xA5, 0x2A, 0x2A },               /* #A52A2A */
    { RGBC,  19,   3, "firebrick", 0xB2, 0x22, 0x22 },           /* #B22222 */
    { RGBC,  20,   4, "crimson", 0xDC, 0x14, 0x3C },             /* #DC143C */
    { RGBC,  21,   5, "red", 0xFF, 0x00, 0x00 },                 /* #FF0000 */
    { RGBC,  22,   6, "tomato", 0xFF, 0x63, 0x47 },              /* #FF6347 */
    { RGBC,  23,   7, "coral", 0xFF, 0x7F, 0x50 },               /* #FF7F50 */
    { RGBC,  24,   8, "indian-red", 0xCD, 0x5C, 0x5C },          /* #CD5C5C */
    { RGBC,  25,   9, "light-coral", 0xF0, 0x80, 0x80 },         /* #F08080 */
    { RGBC,  26,  10, "dark-salmon", 0xE9, 0x96, 0x7A },         /* #E9967A */
    { RGBC,  27,  11, "salmon", 0xFA, 0x80, 0x72 },              /* #FA8072 */
    { RGBC,  28,  12, "light-salmon", 0xFF, 0xA0, 0x7A },        /* #FFA07A */
    { RGBC,  29,  13, "orange-red", 0xFF, 0x45, 0x00 },          /* #FF4500 */
    { RGBC,  30,  14, "dark-orange", 0xFF, 0x8C, 0x00 },         /* #FF8C00 */
    { RGBC,  31,  15, "orange", 0xFF, 0xA5, 0x00 },              /* #FFA500 */
    { RGBC,  32,  16, "gold", 0xFF, 0xD7, 0x00 },                /* #FFD700 */
    { RGBC,  33,  17, "dark-golden-rod", 0xB8, 0x86, 0x0B },     /* #B8860B */
    { RGBC,  34,  18, "golden-rod", 0xDA, 0xA5, 0x20 },          /* #DAA520 */
    { RGBC,  35,  19, "pale-golden-rod", 0xEE, 0xE8, 0xAA },     /* #EEE8AA */
    { RGBC,  36,  20, "dark-khaki", 0xBD, 0xB7, 0x6B },          /* #BDB76B */
    { RGBC,  37,  21, "khaki", 0xF0, 0xE6, 0x8C },               /* #F0E68C */
    { RGBC,  38,  22, "olive", 0x80, 0x80, 0x00 },               /* #808000 */
    { RGBC,  39,  23, "yellow", 0xFF, 0xFF, 0x00 },              /* #FFFF00 */
    { RGBC,  40,  24, "yellow-green", 0x9A, 0xCD, 0x32 },        /* #9ACD32 */
    { RGBC,  41,  25, "dark-olive-green", 0x55, 0x6B, 0x2F },    /* #556B2F */
    { RGBC,  42,  26, "olive-drab", 0x6B, 0x8E, 0x23 },          /* #6B8E23 */
    { RGBC,  43,  27, "lawn-green", 0x7C, 0xFC, 0x00 },          /* #7CFC00 */
    { RGBC,  44,  28, "chart-reuse", 0x7F, 0xFF, 0x00 },         /* #7FFF00 */
    { RGBC,  45,  29, "green-yellow", 0xAD, 0xFF, 0x2F },        /* #ADFF2F */
    { RGBC,  46,  30, "dark-green", 0x00, 0x64, 0x00 },          /* #006400 */
    { RGBC,  47,  31, "green", 0x00, 0x80, 0x00 },               /* #008000 */
    { RGBC,  48,  32, "forest-green", 0x22, 0x8B, 0x22 },        /* #228B22 */
    { RGBC,  49,  33, "lime", 0x00, 0xFF, 0x00 },                /* #00FF00 */
    { RGBC,  50,  34, "lime-green", 0x32, 0xCD, 0x32 },          /* #32CD32 */
    { RGBC,  51,  35, "light-green", 0x90, 0xEE, 0x90 },         /* #90EE90 */
    { RGBC,  52,  36, "pale-green", 0x98, 0xFB, 0x98 },          /* #98FB98 */
    { RGBC,  53,  37, "dark-sea-green", 0x8F, 0xBC, 0x8F },      /* #8FBC8F */
    { RGBC,  54,  38, "medium-spring-green", 0x00, 0xFA, 0x9A }, /* #00FA9A */
    { RGBC,  55,  39, "spring-green", 0x00, 0xFF, 0x7F },        /* #00FF7F */
    { RGBC,  56,  40, "sea-green", 0x2E, 0x8B, 0x57 },           /* #2E8B57 */
    { RGBC,  57,  41, "medium-aqua-marine", 0x66, 0xCD, 0xAA },  /* #66CDAA */
    { RGBC,  58,  42, "medium-sea-green", 0x3C, 0xB3, 0x71 },    /* #3CB371 */
    { RGBC,  59,  43, "light-sea-green", 0x20, 0xB2, 0xAA },     /* #20B2AA */
    { RGBC,  60,  44, "dark-slate-gray", 0x2F, 0x4F, 0x4F },     /* #2F4F4F */
    { RGBC,  61,  45, "teal", 0x00, 0x80, 0x80 },                /* #008080 */
    { RGBC,  62,  46, "dark-cyan", 0x00, 0x8B, 0x8B },           /* #008B8B */
    { RGBC,  63,  47, "aqua", 0x00, 0xFF, 0xFF },                /* #00FFFF */
    { RGBC,  64,  48, "cyan", 0x00, 0xFF, 0xFF },                /* #00FFFF */
    { RGBC,  65,  49, "light-cyan", 0xE0, 0xFF, 0xFF },          /* #E0FFFF */
    { RGBC,  66,  50, "dark-turquoise", 0x00, 0xCE, 0xD1 },      /* #00CED1 */
    { RGBC,  67,  51, "turquoise", 0x40, 0xE0, 0xD0 },           /* #40E0D0 */
    { RGBC,  68,  52, "medium-turquoise", 0x48, 0xD1, 0xCC },    /* #48D1CC */
    { RGBC,  69,  53, "pale-turquoise", 0xAF, 0xEE, 0xEE },      /* #AFEEEE */
    { RGBC,  70,  54, "aqua-marine", 0x7F, 0xFF, 0xD4 },         /* #7FFFD4 */
    { RGBC,  71,  55, "powder-blue", 0xB0, 0xE0, 0xE6 },         /* #B0E0E6 */
    { RGBC,  72,  56, "cadet-blue", 0x5F, 0x9E, 0xA0 },          /* #5F9EA0 */
    { RGBC,  73,  57, "steel-blue", 0x46, 0x82, 0xB4 },          /* #4682B4 */
    { RGBC,  74,  58, "corn-flower-blue", 0x64, 0x95, 0xED },    /* #6495ED */
    { RGBC,  75,  59, "deep-sky-blue", 0x00, 0xBF, 0xFF },       /* #00BFFF */
    { RGBC,  76,  60, "dodger-blue", 0x1E, 0x90, 0xFF },         /* #1E90FF */
    { RGBC,  77,  61, "light-blue", 0xAD, 0xD8, 0xE6 },          /* #ADD8E6 */
    { RGBC,  78,  62, "sky-blue", 0x87, 0xCE, 0xEB },            /* #87CEEB */
    { RGBC,  79,  63, "light-sky-blue", 0x87, 0xCE, 0xFA },      /* #87CEFA */
    { RGBC,  80,  64, "midnight-blue", 0x19, 0x19, 0x70 },       /* #191970 */
    { RGBC,  81,  65, "navy", 0x00, 0x00, 0x80 },                /* #000080 */
    { RGBC,  82,  66, "dark-blue", 0x00, 0x00, 0x8B },           /* #00008B */
    { RGBC,  83,  67, "medium-blue", 0x00, 0x00, 0xCD },         /* #0000CD */
    { RGBC,  84,  68, "blue", 0x00, 0x00, 0xFF },                /* #0000FF */
    { RGBC,  85,  69, "royal-blue", 0x41, 0x69, 0xE1 },          /* #4169E1 */
    { RGBC,  86,  70, "blue-violet", 0x8A, 0x2B, 0xE2 },         /* #8A2BE2 */
    { RGBC,  87,  71, "indigo", 0x4B, 0x00, 0x82 },              /* #4B0082 */
    { RGBC,  88,  72, "dark-slate-blue", 0x48, 0x3D, 0x8B },     /* #483D8B */
    { RGBC,  89,  73, "slate-blue", 0x6A, 0x5A, 0xCD },          /* #6A5ACD */
    { RGBC,  90,  74, "medium-slate-blue", 0x7B, 0x68, 0xEE },   /* #7B68EE */
    { RGBC,  91,  75, "medium-purple", 0x93, 0x70, 0xDB },       /* #9370DB */
    { RGBC,  92,  76, "dark-magenta", 0x8B, 0x00, 0x8B },        /* #8B008B */
    { RGBC,  93,  77, "dark-violet", 0x94, 0x00, 0xD3 },         /* #9400D3 */
    { RGBC,  94,  78, "dark-orchid", 0x99, 0x32, 0xCC },         /* #9932CC */
    { RGBC,  95,  79, "medium-orchid", 0xBA, 0x55, 0xD3 },       /* #BA55D3 */
    { RGBC,  96,  80, "purple", 0x80, 0x00, 0x80 },              /* #800080 */
    { RGBC,  97,  81, "thistle", 0xD8, 0xBF, 0xD8 },             /* #D8BFD8 */
    { RGBC,  98,  82, "plum", 0xDD, 0xA0, 0xDD },                /* #DDA0DD */
    { RGBC,  99,  83, "violet", 0xEE, 0x82, 0xEE },              /* #EE82EE */
    { RGBC, 100,  84, "magenta", 0xFF, 0x00, 0xFF },             /* #FF00FF */
    { RGBC, 101,  85, "orchid", 0xDA, 0x70, 0xD6 },              /* #DA70D6 */
    { RGBC, 102,  86, "medium-violet-red", 0xC7, 0x15, 0x85 },   /* #C71585 */
    { RGBC, 103,  87, "pale-violet-red", 0xDB, 0x70, 0x93 },     /* #DB7093 */
    { RGBC, 104,  88, "deep-pink", 0xFF, 0x14, 0x93 },           /* #FF1493 */
    { RGBC, 105,  89, "hot-pink", 0xFF, 0x69, 0xB4 },            /* #FF69B4 */
    { RGBC, 106,  90, "light-pink", 0xFF, 0xB6, 0xC1 },          /* #FFB6C1 */
    { RGBC, 107,  91, "pink", 0xFF, 0xC0, 0xCB },                /* #FFC0CB */
    { RGBC, 108,  92, "antique-white", 0xFA, 0xEB, 0xD7 },       /* #FAEBD7 */
    { RGBC, 109,  93, "beige", 0xF5, 0xF5, 0xDC },               /* #F5F5DC */
    { RGBC, 110,  94, "bisque", 0xFF, 0xE4, 0xC4 },              /* #FFE4C4 */
    { RGBC, 111,  95, "blanched-almond", 0xFF, 0xEB, 0xCD },     /* #FFEBCD */
    { RGBC, 112,  96, "wheat", 0xF5, 0xDE, 0xB3 },               /* #F5DEB3 */
    { RGBC, 113,  97, "corn-silk", 0xFF, 0xF8, 0xDC },           /* #FFF8DC */
    { RGBC, 114,  98, "lemon-chiffon", 0xFF, 0xFA, 0xCD },       /* #FFFACD */
    { RGBC, 115,  99, "light-golden-rod-yellow",
                      0xFA, 0xFA, 0xD2 },                        /* #FAFAD2 */
    { RGBC, 116, 100, "light-yellow", 0xFF, 0xFF, 0xE0 },        /* #FFFFE0 */
    { RGBC, 117, 101, "saddle-brown", 0x8B, 0x45, 0x13 },        /* #8B4513 */
    { RGBC, 118, 102, "sienna", 0xA0, 0x52, 0x2D },              /* #A0522D */
    { RGBC, 119, 103, "chocolate", 0xD2, 0x69, 0x1E },           /* #D2691E */
    { RGBC, 120, 104, "peru", 0xCD, 0x85, 0x3F },                /* #CD853F */
    { RGBC, 121, 105, "sandy-brown", 0xF4, 0xA4, 0x60 },         /* #F4A460 */
    { RGBC, 122, 106, "burly-wood", 0xDE, 0xB8, 0x87 },          /* #DEB887 */
    { RGBC, 123, 107, "tan", 0xD2, 0xB4, 0x8C },                 /* #D2B48C */
    { RGBC, 124, 108, "rosy-brown", 0xBC, 0x8F, 0x8F },          /* #BC8F8F */
    { RGBC, 125, 109, "moccasin", 0xFF, 0xE4, 0xB5 },            /* #FFE4B5 */
    { RGBC, 126, 110, "navajo-white", 0xFF, 0xDE, 0xAD },        /* #FFDEAD */
    { RGBC, 127, 111, "peach-puff", 0xFF, 0xDA, 0xB9 },          /* #FFDAB9 */
    { RGBC, 128, 112, "misty-rose", 0xFF, 0xE4, 0xE1 },          /* #FFE4E1 */
    { RGBC, 129, 113, "lavender-blush", 0xFF, 0xF0, 0xF5 },      /* #FFF0F5 */
    { RGBC, 130, 114, "linen", 0xFA, 0xF0, 0xE6 },               /* #FAF0E6 */
    { RGBC, 131, 115, "old-lace", 0xFD, 0xF5, 0xE6 },            /* #FDF5E6 */
    { RGBC, 132, 116, "papaya-whip", 0xFF, 0xEF, 0xD5 },         /* #FFEFD5 */
    { RGBC, 133, 117, "sea-shell", 0xFF, 0xF5, 0xEE },           /* #FFF5EE */
    { RGBC, 134, 118, "mint-cream", 0xF5, 0xFF, 0xFA },          /* #F5FFFA */
    { RGBC, 135, 119, "slate-gray", 0x70, 0x80, 0x90 },          /* #708090 */
    { RGBC, 136, 120, "light-slate-gray", 0x77, 0x88, 0x99 },    /* #778899 */
    { RGBC, 137, 121, "light-steel-blue", 0xB0, 0xC4, 0xDE },    /* #B0C4DE */
    { RGBC, 138, 122, "lavender", 0xE6, 0xE6, 0xFA },            /* #E6E6FA */
    { RGBC, 139, 123, "floral-white", 0xFF, 0xFA, 0xF0 },        /* #FFFAF0 */
    { RGBC, 140, 124, "alice-blue", 0xF0, 0xF8, 0xFF },          /* #F0F8FF */
    { RGBC, 141, 125, "ghost-white", 0xF8, 0xF8, 0xFF },         /* #F8F8FF */
    { RGBC, 142, 126, "honeydew", 0xF0, 0xFF, 0xF0 },            /* #F0FFF0 */
    { RGBC, 143, 127, "ivory", 0xFF, 0xFF, 0xF0 },               /* #FFFFF0 */
    { RGBC, 144, 128, "azure", 0xF0, 0xFF, 0xFF },               /* #F0FFFF */
    { RGBC, 145, 129, "snow", 0xFF, 0xFA, 0xFA },                /* #FFFAFA */
    { RGBC, 146, 130, "black", 0x00, 0x00, 0x00 },               /* #000000 */
    { RGBC, 147, 131, "dim-gray", 0x69, 0x69, 0x69 },            /* #696969 */
    { RGBC, 148, 132, "gray", 0x80, 0x80, 0x80 },                /* #808080 */
    { RGBC, 149, 133, "dark-gray", 0xA9, 0xA9, 0xA9 },           /* #A9A9A9 */
    { RGBC, 150, 134, "silver", 0xC0, 0xC0, 0xC0 },              /* #C0C0C0 */
    { RGBC, 151, 135, "light-gray", 0xD3, 0xD3, 0xD3 },          /* #D3D3D3 */
    { RGBC, 152, 136, "gainsboro", 0xDC, 0xDC, 0xDC },           /* #DCDCDC */
    { RGBC, 153, 137, "white-smoke", 0xF5, 0xF5, 0xF5 },         /* #F5F5F5 */
    { RGBC, 154, 138, "white", 0xFF, 0xFF, 0xFF },               /* #FFFFFF */
};

#undef NHC
#undef NOC
#undef RGBC

#ifdef CHANGE_COLOR
staticfn int32 alt_color_spec(const char *cp);
#endif

/**
 * @brief Reduce a colour table entry to the single value the interfaces use.
 *
 * The three colour kinds are encoded differently in that one value, which is why this is not simply reading a field. An extended colour becomes its three components packed together; a basic colour becomes its index with a marker bit set; the absence of colour becomes a
 * particular value with the same marker.
 *
 * @param cte the table entry
 * @return the packed colour
 * @note The marker bit is what lets a receiver tell a basic colour's small index from an extended colour's packed components, which would otherwise be indistinguishable -- an index of 4 and a nearly-black colour are the same number.
 * @note An unrecognised colour kind yields the absence of colour rather than being reported. That is the safe answer: something is drawn, uncoloured.
 */
/**
 * @brief 색 표 항목을 인터페이스가 쓰는 하나의 값으로 줄인다.
 *
 * 세 색 종류가 그 하나의 값에 다르게 부호화되며, 그것이 이것이 단순히 항목을 읽는 것이 아닌 이유다. 확장 색은 자기 세 성분이 함께 압축된 것이 된다. 기본 색은 표시 비트가 설정된 자기 색인이 된다. 색의 없음은 같은 표시를 가진 특정한 값이 된다.
 *
 * @param cte 그 표 항목
 * @return 압축된 색
 * @note 그 표시 비트가 받는 쪽이 기본 색의 작은 색인을 확장 색의 압축된 성분과 구별할 수 있게 하는 것이며, 그러지 않으면 그것들이 구별될 수 없다. 색인 4와 거의 검은 색은 같은 숫자다.
 * @note 인식되지 않는 색 종류는 보고되는 대신 색의 없음을 낸다. 그것이 안전한 답이다. 무언가가 색 없이 그려진다.
 */
int32
colortable_to_int32(const struct nethack_color *cte)
{
    int32 clr = NO_COLOR | NH_BASIC_COLOR;

    if (cte->colortyp == rgb_color)
        clr = (cte->r << 16) | (cte->g << 8) | cte->b;
    else if (cte->colortyp == nh_color)
        clr = cte->tableindex | NH_BASIC_COLOR;
    return clr;
}

/**
 * @brief Render a colour and attribute back into the form a configuration file uses.
 *
 * The inverse of parsing, needed because the game shows a player their current settings in a form they could type back. A setting displayed differently from how it is written would be unusable as an example.
 *
 * @param ca the colour and attribute
 * @return the text, in a shared buffer
 * @warning Both parts come from routines that can return nothing when the value has no name, and neither is checked. A colour or attribute with no name in the tables is formatted as a null string, which the formatting machinery renders as a placeholder rather than
 *          crashing -- but the result is not something a player could type back.
 * @warning Shared buffer; the next call overwrites it.
 */
/**
 * @brief 색과 속성을 설정 파일이 쓰는 형태로 되돌려 그린다.
 *
 * 해석의 역이며, 게임이 플레이어에게 자기 현재 설정을 그가 되입력할 수 있는 형태로 보이기 때문에 필요하다. 쓰이는 방식과 다르게 보이는 설정은 예시로서 쓸모없을 것이다.
 *
 * @param ca 그 색과 속성
 * @return 그 글. 공유된 버퍼에
 * @warning 두 부분 모두 값이 이름을 갖지 않을 때 아무것도 돌려주지 않을 수 있는 함수에서 오며, 어느 것도 검사되지 않는다. 표에 이름이 없는 색이나 속성은 널 문자열로 형식화되고, 형식화 기계가 그것을 부서지는 대신 자리표로 그린다. 그러나 그 결과는 플레이어가 되입력할 수 있는 것이 아니다.
 * @warning 공유된 버퍼다. 다음 호출이 그것을 덮어쓴다.
 */
char *
color_attr_to_str(color_attr *ca)
{
    static char buf[BUFSZ];

    Sprintf(buf, "%s&%s",
            clr2colorname(ca->color),
            attr2attrname(ca->attr));
    return buf;
}

/**
 * @brief Read a colour and attribute from configuration text.
 *
 * Accepts either both parts joined by an ampersand or one part alone. A single part is ambiguous -- it could be either -- so it is tried as an attribute first and as a colour only if that fails.
 *
 * @param ca receives the colour and attribute
 * @param str the text
 * @return whether it was understood
 * @note When both parts are present but neither matches in the order written, the two are tried the other way round. So "bold&red" and "red&bold" both work, which spares a player from having to remember an order that has no reason behind it.
 * @note A part not given defaults to no colour or no attribute rather than being left as it was, so parsing always yields a complete setting.
 * @warning The reordering attempt cannot work as intended, and the existing note says so: the matching routines report their own errors as they go, so by the time the first order has failed the player has already been told it was wrong. The retry then succeeds and the
 *          spurious error stands.
 * @note The text is copied before being parsed, because the parsing splits it at the ampersand by overwriting that character.
 */
/**
 * @brief 설정 글에서 색과 속성을 읽는다.
 *
 * 앤드 기호로 이어진 두 부분이나 홀로 있는 한 부분을 받아들인다. 한 부분은 애매하다. 그것은 어느 쪽이든일 수 있다. 그래서 먼저 속성으로 시도되고, 그것이 실패할 때만 색으로 시도된다.
 *
 * @param ca 색과 속성을 받는다
 * @param str 그 글
 * @return 그것이 이해되었는지
 * @note 두 부분이 다 있지만 쓰인 순서로는 어느 것도 맞지 않을 때, 그 둘이 반대로 시도된다. 그래서 "bold&red"와 "red&bold"가 모두 통하며, 그것이 플레이어를 뒤에 아무 이유도 없는 순서를 기억해야 하는 일에서 덜어 준다.
 * @note 주어지지 않은 부분은 그대로 남겨지는 대신 색 없음이나 속성 없음으로 기본값이 되므로, 해석이 언제나 완전한 설정을 낸다.
 * @warning 그 순서 바꾸기 시도는 의도대로 동작할 수 없으며, 기존 적바림이 그렇다고 말한다. 맞추기 함수들이 진행하면서 자기 오류를 보고하므로, 첫 순서가 실패한 시점에 플레이어는 이미 그것이 틀렸다고 들었다. 그다음 재시도가 성공하고 그 헛된 오류가 남는다.
 * @note 글이 해석되기 전에 복사되는데, 그 해석이 앤드 기호를 덮어씀으로써 그것을 거기서 쪼개기 때문이다.
 */
boolean
color_attr_parse_str(color_attr *ca, char *str)
{
    char buf[BUFSZ];
    char *amp = NULL;
    int tmp, c = NO_COLOR, a = ATR_NONE;

    (void) strncpy(buf, str, sizeof buf - 1);
    buf[sizeof buf - 1] = '\0';

    if ((amp = strchr(buf, '&')) != 0)
        *amp = '\0';

    if (amp) {
        amp++;
        c = match_str2clr(buf, FALSE);
        a = match_str2attr(amp, TRUE);
        /* FIXME: match_str2clr & match_str2attr give config_error_add(),
           so this is useless */
        if (c >= CLR_MAX && a == -1) {
            /* try other way around */
            c = match_str2clr(amp, FALSE);
            a = match_str2attr(buf, TRUE);
        }
        if (c >= CLR_MAX || a == -1)
            return FALSE;
    } else {
        /* one param only */
        tmp = match_str2attr(buf, FALSE);
        if (tmp == -1) {
            tmp = match_str2clr(buf, FALSE);
            if (tmp >= CLR_MAX)
                return FALSE;
            c = tmp;
        } else {
            a = tmp;
        }
    }
    ca->attr = a;
    ca->color = c;
    return TRUE;
}

/**
 * @brief Ask the player to choose a colour and then an attribute.
 * @param ca the current setting, replaced if the player chooses
 * @param prompt what to ask
 * @return whether the player chose rather than cancelling
 * @note Neither part is written back unless both were chosen, so cancelling the second question leaves the setting entirely unchanged rather than half changed.
 * @note Each question offers the existing value as the default, so a player changing one part need not restate the other.
 */
/**
 * @brief 플레이어에게 색을 고르고 그다음 속성을 고르라고 청한다.
 * @param ca 현재 설정. 플레이어가 고르면 갈아치워진다
 * @param prompt 무엇을 물을지
 * @return 플레이어가 취소하는 대신 골랐는지
 * @note 둘 다 골라지지 않으면 어느 부분도 되쓰이지 않으므로, 두 번째 질문을 취소하는 것이 설정을 반만 바뀐 채가 아니라 온전히 바뀌지 않은 채로 남긴다.
 * @note 각 질문이 기존 값을 기본값으로 내놓으므로, 한 부분을 바꾸는 플레이어가 다른 부분을 다시 말해야 하지 않는다.
 */
boolean
query_color_attr(color_attr *ca, const char *prompt)
{
    int c, a;

    c = query_color(prompt, ca->color);
    if (c == -1)
        return FALSE;
    a = query_attr(prompt, ca->attr);
    if (a == -1)
        return FALSE;
    ca->color = c;
    ca->attr = a;
    return TRUE;
}

/**
 * @brief The name of an attribute.
 * @param attr the attribute
 * @return its name, or null if it has none
 * @note Searches the whole table including the aliases, so it can return an alias if one precedes the proper name. In practice the proper names come first, so it returns those.
 * @warning Does not skip the nameless separator entry, unlike its colour counterpart. That entry's attribute is the same as "none", so asking for the name of no attribute matches the separator's value before reaching a name -- but the separator's name is null, and this
 *          returns it. The real "none" entry comes first in the table, so the answer happens to be right; it would not be if the table were reordered.
 */
/**
 * @brief 속성의 이름.
 * @param attr 그 속성
 * @return 그 이름, 또는 없으면 널
 * @note 딴이름을 포함해 표 전체를 찾으므로, 딴이름이 제 이름보다 앞선다면 딴이름을 돌려줄 수 있다. 실제로는 제 이름이 먼저 오므로 그것을 돌려준다.
 * @warning 자기 색 짝과 달리 이름 없는 구분자 항목을 건너뛰지 않는다. 그 항목의 속성이 "none"과 같으므로, 속성 없음의 이름을 묻는 것이 이름에 이르기 전에 구분자의 값과 맞는다. 그런데 구분자의 이름은 널이고, 이것은 그것을 돌려준다. 진짜 "none" 항목이 표에서 먼저 오므로 그 답이 마침 옳다.
 *          표가 다시 배치되면 그러지 않을 것이다.
 */
const char *
attr2attrname(int attr)
{
    int i;

    for (i = 0; i < SIZE(attrnames); i++)
        if (attrnames[i].attr == attr)
            return attrnames[i].name;
    return (char *) 0;
}

/*
 * Color support functions and data for "color"
 *
 * Used by: optfn_()
 *
 */

/**
 * @brief The name of a colour.
 * @param clr the colour
 * @return its name, or null if it has none
 * @note Skips entries with no name, so the separator between proper names and aliases does not stop the search. Since proper names come first, this returns the proper name rather than an alias.
 * @note Searches the small name table rather than the large colour table, so it names only the basic colours. An extended colour has no name here.
 */
/**
 * @brief 색의 이름.
 * @param clr 그 색
 * @return 그 이름, 또는 없으면 널
 * @note 이름 없는 항목을 건너뛰므로, 제 이름과 딴이름 사이의 구분자가 찾기를 멈추지 않는다. 제 이름이 먼저 오므로, 이것은 딴이름이 아니라 제 이름을 돌려준다.
 * @note 큰 색 표가 아니라 작은 이름 표를 찾으므로 기본 색만 이름 짓는다. 확장 색은 여기에 이름이 없다.
 */
const char *
clr2colorname(int clr)
{
    int i;

    for (i = 0; i < SIZE(colornames); i++)
        if (colornames[i].name && colornames[i].color == clr)
            return colornames[i].name;
    return (char *) 0;
}

/**
 * @brief Recognise a colour by name, forgivingly.
 *
 * Matching disregards spaces, hyphens and underscores, so a player writing "lightblue", "light blue" or "light-blue" all get the same colour. As the existing comment concedes, that also accepts nonsense with punctuation scattered through it, which was judged not worth
 * preventing.
 *
 * @param str the name, which may have trailing space; the caller has removed any leading space
 * @param suppress_msg whether to stay silent rather than reporting an unknown name
 * @return the colour, or a value past the last colour meaning none matched
 * @note A bare number is accepted as a colour index if no name matched. That lets a player name a colour the tables do not, but it also means a typo consisting of digits becomes a colour rather than an error.
 * @note The failure value is one past the last real colour rather than a negative, so a caller comparing against the colour count catches it. A caller treating the result as an index would read past the colour tables.
 * @note Silence is optional because this is called speculatively when parsing a setting that might be an attribute instead, and a failed guess is not an error yet.
 */
/**
 * @brief 색을 이름으로 너그럽게 알아본다.
 *
 * 맞추기가 공백, 붙임표, 밑줄을 무시하므로, "lightblue", "light blue", "light-blue"를 쓰는 플레이어가 모두 같은 색을 얻는다. 기존 주석이 인정하듯 그것은 기호가 흩어져 있는 뜻 없는 것도 받아들이는데, 그것은 막을 만하지 않다고 판단되었다.
 *
 * @param str 그 이름. 뒤따르는 공백이 있을 수 있다. 호출자가 앞선 공백을 없앴다
 * @param suppress_msg 알 수 없는 이름을 보고하는 대신 잠잠히 있을지
 * @return 그 색, 또는 아무것도 맞지 않았음을 뜻하는 마지막 색 다음의 값
 * @note 어떤 이름도 맞지 않았으면 맨 숫자가 색 색인으로 받아들여진다. 그것이 플레이어가 표가 이름 짓지 않는 색을 이름 짓게 하지만, 숫자로 이루어진 오타가 오류가 아니라 색이 된다는 뜻이기도 하다.
 * @note 실패 값이 음수가 아니라 마지막 진짜 색 하나 다음이므로, 색 개수와 견주어 비교하는 호출자가 그것을 잡는다. 결과를 색인으로 다루는 호출자는 색 표를 넘어 읽을 것이다.
 * @note 잠잠함이 선택적인 것은, 이것이 대신 속성일 수도 있는 설정을 해석할 때 추측으로 호출되고 실패한 짐작이 아직 오류가 아니기 때문이다.
 */
int
match_str2clr(char *str, boolean suppress_msg)
{
    int i, c = CLR_MAX;

    /* allow "lightblue", "light blue", and "light-blue" to match "light blue"
       (also junk like "_l i-gh_t---b l u e" but we won't worry about that);
       also copes with trailing space; caller has removed any leading space */
    for (i = 0; i < SIZE(colornames); i++)
        if (colornames[i].name
            && fuzzymatch(str, colornames[i].name, " -_", TRUE)) {
            c = colornames[i].color;
            break;
        }
    if (i == SIZE(colornames) && digit(*str))
        c = atoi(str);

    if (c < 0 || c >= CLR_MAX) {
        if (!suppress_msg)
            config_error_add("Unknown color '%.60s'", str);
        c = CLR_MAX; /* "none of the above" */
    }
    return c;
}

/**
 * @brief Recognise an attribute by name, forgivingly.
 * @param str the name
 * @param complain whether to report an unknown name
 * @return the attribute, or -1 if none matched
 * @note Matches the same way colours do, disregarding spaces and punctuation, so the two halves of a setting are read by the same rules.
 * @note The failure value is negative here while the colour routine's is positive. The two are not interchangeable, and a caller that tested them the same way would misread one of them.
 * @note Does not accept a bare number, unlike the colour routine. There is no reason to name an attribute by its bit value, and accepting one would turn a mistyped colour into an attribute.
 */
/**
 * @brief 속성을 이름으로 너그럽게 알아본다.
 * @param str 그 이름
 * @param complain 알 수 없는 이름을 보고할지
 * @return 그 속성, 또는 아무것도 맞지 않았으면 -1
 * @note 색이 하는 것과 같은 방식으로, 공백과 기호를 무시하며 맞춘다. 그래서 설정의 두 반쪽이 같은 규칙으로 읽힌다.
 * @note 실패 값이 여기서는 음수인 반면 색 함수의 것은 양수다. 그 둘은 서로 바꿔 쓸 수 없으며, 그것을 같은 방식으로 검사한 호출자는 그중 하나를 잘못 읽을 것이다.
 * @note 색 함수와 달리 맨 숫자를 받아들이지 않는다. 속성을 그 비트 값으로 이름 지을 이유가 없고, 그것을 받아들이는 것은 잘못 입력된 색을 속성으로 만들 것이다.
 */
int
match_str2attr(const char *str, boolean complain)
{
    int i, a = -1;

    for (i = 0; i < SIZE(attrnames); i++)
        if (attrnames[i].name
            && fuzzymatch(str, attrnames[i].name, " -_", TRUE)) {
            a = attrnames[i].attr;
            break;
        }

    if (a == -1 && complain)
        config_error_add("Unknown text attribute '%.50s'", str);

    return a;
}

/**
 * @brief Offer the player a menu of attributes to choose from.
 *
 * Serves two settings with different rules, as the existing comment records: menu headers and menu colouring take one attribute, while status highlighting takes several combined. The comment also records the author's view that allowing several was overkill and that
 * restricting it to one would have been simpler -- which explains why most of this routine's complexity exists.
 *
 * @param prompt what to ask
 * @param dflt_attr the current attribute, offered preselected
 * @return the chosen attribute or combination, or -1 if cancelled
 * @warning Which of the two behaviours applies is decided by whether the prompt begins with a particular word. So the caller controls it by wording, not by an argument, and rewording a prompt silently changes whether a player can pick more than one attribute.
 * @note The combining case returns a different set of values from the single case -- highlighting flags rather than attributes -- so the two returns are not the same kind of thing despite the one return type. A caller must know which it asked for.
 * @note "None" is excluded when anything else was picked, since no attribute combined with bold means bold. It is honoured only when it was the sole choice.
 * @note A preselected entry makes the counting awkward, and the code says so at each point: choosing one thing can report two picks because the preselection counts, and choosing nothing can report zero either because the player accepted the default or because they
 *       deselected it. The three returns are distinguished carefully for that reason.
 */
/**
 * @brief 플레이어에게 고를 속성 메뉴를 내놓는다.
 *
 * 기존 주석이 기록하듯 다른 규칙을 가진 두 설정을 섬긴다. 메뉴 제목과 메뉴 색칠은 하나의 속성을 취하고, 상태 강조는 합쳐진 여럿을 취한다. 그 주석은 또한 여럿을 허용한 것이 과했고 하나로 제한하는 것이 더 단순했을 것이라는 지은이의 견해를 기록하며, 그것이 이 함수의 복잡함 대부분이 존재하는 이유를 설명한다.
 *
 * @param prompt 무엇을 물을지
 * @param dflt_attr 현재 속성. 미리 골라진 채로 내놓아진다
 * @return 골라진 속성이나 조합, 또는 취소되면 -1
 * @warning 두 행동 중 어느 것이 적용되는지가 그 물음이 특정한 낱말로 시작하는지로 정해진다. 그래서 호출자가 인자가 아니라 표현으로 그것을 다스리며, 물음의 표현을 바꾸는 것이 플레이어가 하나보다 많은 속성을 고를 수 있는지를 조용히 바꾼다.
 * @note 합치는 경우가 하나인 경우와 다른 값 묶음을 돌려준다. 속성이 아니라 강조 표시. 그래서 하나의 반환 형에도 그 두 반환은 같은 종류의 것이 아니다. 호출자가 자기가 어느 것을 요청했는지 알아야 한다.
 * @note 다른 무엇이 골라졌을 때 "none"은 배제된다. 속성 없음이 굵게와 합쳐지는 것은 굵게를 뜻하기 때문이다. 그것은 그것이 유일한 선택이었을 때만 존중된다.
 * @note 미리 골라진 항목이 세기를 까다롭게 만들며, 코드가 각 지점에서 그렇다고 말한다. 하나를 고르는 것이 미리 고르기가 셈되기 때문에 두 번의 고르기를 알릴 수 있고, 아무것도 고르지 않는 것이 플레이어가 기본값을 받아들였기 때문일 수도 그것을 골라 뺐기 때문일 수도 있어서 영을 알릴 수 있다. 그 이유로 세 반환이
 *       조심스럽게 구별된다.
 */
int
query_attr(const char *prompt, int dflt_attr)
{
    winid tmpwin;
    anything any;
    int i, pick_cnt;
    menu_item *picks = (menu_item *) 0;
    boolean allow_many = (prompt && !strncmpi(prompt, "Choose", 6));
    int clr = NO_COLOR;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany;
    for (i = 0; i < SIZE(attrnames); i++) {
        if (!attrnames[i].name)
            break;
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, 0, 0,
                 attrnames[i].attr, clr, attrnames[i].name,
                 (attrnames[i].attr == dflt_attr) ? MENU_ITEMFLAGS_SELECTED
                                                  : MENU_ITEMFLAGS_NONE);
    }
    end_menu(tmpwin, (prompt && *prompt) ? prompt : "Pick an attribute");
    pick_cnt = select_menu(tmpwin, allow_many ? PICK_ANY : PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);
    if (pick_cnt > 0) {
        int j, k = 0;

        if (allow_many) {
            /* PICK_ANY, with one preselected entry (ATR_NONE) which
               should be excluded if any other choices were picked */
            for (i = 0; i < pick_cnt; ++i) {
                j = picks[i].item.a_int - 1;
                if (attrnames[j].attr != ATR_NONE || pick_cnt == 1) {
                    switch (attrnames[j].attr) {
                    case ATR_NONE:
                        k = HL_NONE;
                        break;
                    case ATR_BOLD:
                        k |= HL_BOLD;
                        break;
                    case ATR_DIM:
                        k |= HL_DIM;
                        break;
                    case ATR_ITALIC:
                        k |= HL_ITALIC;
                        break;
                    case ATR_ULINE:
                        k |= HL_ULINE;
                        break;
                    case ATR_BLINK:
                        k |= HL_BLINK;
                        break;
                    case ATR_INVERSE:
                        k |= HL_INVERSE;
                        break;
                    }
                }
            }
        } else {
            /* PICK_ONE, but might get 0 or 2 due to preselected entry */
            j = picks[0].item.a_int - 1;
            /* pick_cnt==2: explicitly picked something other than the
               preselected entry */
            if (pick_cnt == 2 && attrnames[j].attr == dflt_attr)
                j = picks[1].item.a_int - 1;
            k = attrnames[j].attr;
        }
        free((genericptr_t) picks);
        return k;
    } else if (pick_cnt == 0 && !allow_many) {
        /* PICK_ONE, preselected entry explicitly chosen */
        return dflt_attr;
    }
    /* either ESC to explicitly cancel (pick_cnt==-1) or
       PICK_ANY with preselected entry toggled off and nothing chosen */
    return -1;
}

/**
 * @brief Offer the player a menu of colours to choose from.
 *
 * Each colour's name is shown in that colour, which requires temporarily replacing the player's own menu-colouring rules with rules matching the colour names, and forcing menu colouring on whether or not the player uses it. Both are undone afterwards.
 *
 * That substitution is the whole reason this is not a plain menu: a list of colour names in the default colour would make choosing one guesswork.
 *
 * @param prompt what to ask
 * @param dflt_color the current colour, offered preselected
 * @return the chosen colour, or -1 if cancelled
 * @note The listing stops at the nameless separator, so the aliases are not offered. A player picking from the menu sees each colour once.
 * @note Only ever offers one colour, unlike the attribute menu, so it has none of that routine's combining. The preselected-entry counting is still needed for the same reason.
 * @warning The player's menu-colouring rules and the setting that enables them are altered for the duration and restored on the way out. A path that left without restoring them would leave the player's own colouring rules replaced for the rest of the game.
 */
/**
 * @brief 플레이어에게 고를 색 메뉴를 내놓는다.
 *
 * 각 색의 이름이 그 색으로 보이며, 그것은 플레이어 자신의 메뉴 색칠 규칙을 색 이름과 맞는 규칙으로 잠시 갈아치우고, 플레이어가 그것을 쓰는지와 무관하게 메뉴 색칠을 켜기를 요구한다. 둘 다 뒤에 되돌려진다.
 *
 * 그 갈아치우기가 이것이 맨 메뉴가 아닌 이유 전부다. 기본 색으로 된 색 이름 목록은 하나를 고르는 것을 짐작으로 만들 것이다.
 *
 * @param prompt 무엇을 물을지
 * @param dflt_color 현재 색. 미리 골라진 채로 내놓아진다
 * @return 골라진 색, 또는 취소되면 -1
 * @note 나열이 이름 없는 구분자에서 멈추므로 딴이름은 내놓아지지 않는다. 메뉴에서 고르는 플레이어는 각 색을 한 번 본다.
 * @note 속성 메뉴와 달리 언제나 하나의 색만 내놓으므로, 그 함수의 합치기가 하나도 없다. 미리 골라진 항목 세기는 같은 이유로 여전히 필요하다.
 * @warning 플레이어의 메뉴 색칠 규칙과 그것을 켜는 설정이 그동안 바뀌고 나가는 길에 되돌려진다. 그것을 되돌리지 않고 떠나는 경로는 플레이어 자신의 색칠 규칙을 남은 게임 동안 갈아치워진 채로 남길 것이다.
 */
int
query_color(const char *prompt, int dflt_color)
{
    winid tmpwin;
    anything any;
    int i, pick_cnt;
    menu_item *picks = (menu_item *) 0;

    /* replace user patterns with color name ones and force 'menucolors' On */
    basic_menu_colors(TRUE);

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany;
    for (i = 0; i < SIZE(colornames); i++) {
        if (!colornames[i].name)
            break;
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, 0, 0,
                 ATR_NONE, NO_COLOR, colornames[i].name,
                 (colornames[i].color == dflt_color) ? MENU_ITEMFLAGS_SELECTED
                                                     : MENU_ITEMFLAGS_NONE);
    }
    end_menu(tmpwin, (prompt && *prompt) ? prompt : "Pick a color");
    pick_cnt = select_menu(tmpwin, PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);

    /* remove temporary color name patterns and restore user-specified ones;
       reset 'menucolors' option to its previous value */
    basic_menu_colors(FALSE);

    if (pick_cnt > 0) {
        i = colornames[picks[0].item.a_int - 1].color;
        /* pick_cnt==2: explicitly picked something other than the
           preselected entry */
        if (pick_cnt == 2 && i == NO_COLOR)
            i = colornames[picks[1].item.a_int - 1].color;
        free((genericptr_t) picks);
        return i;
    } else if (pick_cnt == 0) {
        /* pick_cnt==0: explicitly picking preselected entry toggled it off */
        return dflt_color;
    }
    return -1;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Which pattern-matching library this build uses.
 *
 * Shared from wherever the build's matching code lives rather than tested for, because the two libraries differ in whether a pattern matches anywhere in a string or only from its beginning -- so a pattern that works with one must be written differently for the other.
 */
/**
 * @brief 이 빌드가 어느 무늬 맞추기 라이브러리를 쓰는지.
 *
 * 검사되는 대신 빌드의 맞추기 코드가 있는 곳에서 공유된다. 두 라이브러리가 무늬가 문자열의 어디서든 맞는지 그 시작에서만 맞는지에서 다르기 때문이다. 그래서 한쪽에서 통하는 무늬는 다른 쪽을 위해 다르게 쓰여야 한다.
 */
extern const char regex_id[]; /* from sys/share/<various>regex.{c,cpp} */

/**
 * @brief Substitute a set of menu-colouring rules that paint each colour's name in that colour, or put the player's own rules back.
 *
 * The mechanism behind the colour-picking menu. There is no way to colour a menu entry directly, so the entries are coloured by adding rules that match their own names -- a rule saying "blue" is blue.
 *
 * @param load_colors true to install the substitute rules, false to restore the player's own
 * @note Black and white are deliberately left uncoloured, and the existing comment gives the reason: one of the two will match the background on any terminal, so colouring those entries would make one of them invisible.
 * @note The substitute rules are built once and kept, as the existing comment records, so a player who picks colours repeatedly pays for building them only the first time.
 * @note The pattern form depends on which matching library the build uses, because one of them anchors patterns at the start of the string and needs a leading wildcard to match anywhere.
 * @note The rules are built in the order that puts the longer names first, and the existing comment explains why this matters: adding rules prepends them, so walking the names forward leaves the specific ones ahead of the general ones. Without that, the rule for "blue"
 *       would match the entry named "light blue" and colour it wrongly.
 * @warning The player's own rules are not copied but set aside by pointer, and restoring puts the pointer back. So the substitution must be undone before anything else touches the rules, and it cannot be nested -- a second substitution would set aside the substitute set
 *          as though it were the player's.
 */
/**
 * @brief 각 색의 이름을 그 색으로 칠하는 메뉴 색칠 규칙 묶음을 대신 놓거나, 플레이어 자신의 규칙을 되돌린다.
 *
 * 색 고르기 메뉴 뒤의 방식이다. 메뉴 항목을 곧바로 칠할 방법이 없으므로, 그 항목들이 자기 이름과 맞는 규칙을 더함으로써 칠해진다. "blue"라고 말하는 규칙이 파랑이다.
 *
 * @param load_colors 대신할 규칙을 설치하려면 참, 플레이어 자신의 것을 되돌리려면 거짓
 * @note 검정과 흰색이 의도적으로 칠해지지 않은 채로 남겨지며, 기존 주석이 이유를 준다. 그 둘 중 하나가 어느 터미널에서든 바탕과 맞을 것이므로, 그 항목을 칠하는 것은 그중 하나를 보이지 않게 만들 것이다.
 * @note 기존 주석이 기록하듯 대신할 규칙이 한 번 만들어져 지녀지므로, 되풀이해 색을 고르는 플레이어가 그것을 만드는 값을 첫 번에만 치른다.
 * @note 무늬 형태가 빌드가 어느 맞추기 라이브러리를 쓰는지에 달려 있다. 그중 하나가 무늬를 문자열 시작에 매고 어디서든 맞추려면 앞선 아무거나 문자를 필요로 하기 때문이다.
 * @note 규칙이 더 긴 이름을 먼저 두는 순서로 만들어지며, 기존 주석이 이것이 왜 중요한지 설명한다. 규칙을 더하는 것이 그것을 앞에 붙이므로, 이름을 앞으로 걸어가는 것이 구체적인 것을 일반적인 것 앞에 남긴다. 그것 없이는 "blue"를 위한 규칙이 "light blue"라는 이름의 항목과 맞아 그것을 틀리게
 *       칠할 것이다.
 * @warning 플레이어 자신의 규칙이 복사되는 것이 아니라 포인터로 치워지며, 되돌리기가 그 포인터를 되돌려 놓는다. 그래서 그 갈아치우기는 다른 무엇이 그 규칙을 건드리기 전에 되돌려져야 하며, 겹칠 수 없다. 두 번째 갈아치우기는 대신할 묶음을 플레이어의 것인 양 치울 것이다.
 */
void
basic_menu_colors(
    boolean load_colors) /* True: temporarily replace menu color entries with
                          * a fake set of menu colors which match their names;
                          * False: restore user-specified colorings */
{
    if (load_colors) {
        /* replace normal menu colors with a set specifically for colors */
        gs.save_menucolors = iflags.use_menu_color;
        gs.save_colorings = gm.menu_colorings;

        iflags.use_menu_color = TRUE;
        if (gc.color_colorings) {
            /* use the alternate colorings which were set up previously */
            gm.menu_colorings = gc.color_colorings;
        } else {
            /* create the alternate colorings once */
            char cnm[QBUFSZ];
            int i, c;
            boolean pmatchregex = !strcmpi(regex_id, "pmatchregex");
            const char *patternfmt = pmatchregex ? "*%s" : "%s";

            /* menu_colorings pointer has been saved; clear it in order
               to add the alternate entries as if from scratch */
            gm.menu_colorings = (struct menucoloring *) 0;

            /* this orders the patterns last-in/first-out; that means
               that the "light <foo>" variations come before the basic
               "<foo>" ones, which is exactly what we want (so that the
               shorter basic names won't get false matches as substrings
               of the longer ones) */
            for (i = 0; i < SIZE(colornames); ++i) {
                if (!colornames[i].name) /* first alias entry has no name */
                    break;
                c = colornames[i].color;
                if (c == CLR_BLACK || c == CLR_WHITE || c == NO_COLOR)
                    continue; /* skip these */
                Sprintf(cnm, patternfmt, colornames[i].name);
                add_menu_coloring_parsed(cnm, c, ATR_NONE);
            }

            /* right now, menu_colorings contains the alternate color list;
               remember that list for future pick-a-color instances and
               also keep it as is for this instance */
            gc.color_colorings = gm.menu_colorings;
        }
    } else {
        /* restore normal user-specified menu colors */
        iflags.use_menu_color = gs.save_menucolors;
        gm.menu_colorings = gs.save_colorings;
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Add one menu-colouring rule from an already-validated pattern.
 * @param str the pattern
 * @param c the colour to apply
 * @param a the attribute to apply
 * @return whether the rule was added
 * @note Compiles the pattern again although it has already been checked, and the existing comment gives the only reason it can fail here: compiling allocates, and that can run out of memory even for a pattern known to be valid.
 * @note Frees the compiled pattern before the rule, and the existing comment explains the order: the failure may itself have been an allocation failure, so releasing the larger thing first gives the error reporting room to work.
 * @note New rules are prepended, so the most recently added is matched first. That is what makes the ordering in the colour menu's substitute rules work, and it also means a player's later rule overrides an earlier one.
 * @note Turns menu colouring on as a side effect. A player who adds a rule means for it to take effect, so requiring them to enable the feature separately would be a trap.
 */
/**
 * @brief 이미 검증된 무늬에서 메뉴 색칠 규칙 하나를 더한다.
 * @param str 그 무늬
 * @param c 적용할 색
 * @param a 적용할 속성
 * @return 그 규칙이 더해졌는지
 * @note 이미 검사되었음에도 무늬를 다시 컴파일하며, 기존 주석이 그것이 여기서 실패할 수 있는 유일한 이유를 준다. 컴파일하기가 메모리를 할당하며, 그것은 유효한 것으로 알려진 무늬에 대해서도 메모리가 다할 수 있다.
 * @note 컴파일된 무늬를 규칙보다 먼저 해제하며, 기존 주석이 그 순서를 설명한다. 그 실패가 그 자체로 할당 실패였을 수 있으므로, 더 큰 것을 먼저 놓아주는 것이 오류 보고에 일할 자리를 준다.
 * @note 새 규칙이 앞에 붙으므로, 가장 최근에 더해진 것이 먼저 맞춰진다. 그것이 색 메뉴의 대신할 규칙에서 그 순서가 통하게 하는 것이며, 플레이어의 나중 규칙이 앞선 것을 덮어쓴다는 뜻이기도 하다.
 * @note 딸린 결과로 메뉴 색칠을 켠다. 규칙을 더하는 플레이어는 그것이 효력을 내기를 뜻하므로, 그에게 그 기능을 따로 켜라고 요구하는 것은 함정일 것이다.
 */
boolean
add_menu_coloring_parsed(const char *str, int c, int a)
{
    static const char re_error[] = "Menucolor regex error";
    struct menucoloring *tmp;

    if (!str)
        return FALSE;
    tmp = (struct menucoloring *) alloc(sizeof *tmp);
    tmp->match = regex_init();
    /* test_regex_pattern() has already validated this regexp but parsing
       it again could conceivably run out of memory */
    if (!regex_compile(str, tmp->match)) {
        char errbuf[BUFSZ];
        char *re_error_desc = regex_error_desc(tmp->match, errbuf);

        /* free first in case reason for regcomp failure was out-of-memory */
        regex_free(tmp->match);
        free((genericptr_t) tmp);
        config_error_add("%s: %s", re_error, re_error_desc);
        return FALSE;
    }
    tmp->next = gm.menu_colorings;
    tmp->origstr = dupstr(str);
    tmp->color = c;
    tmp->attr = a;
    gm.menu_colorings = tmp;
    iflags.use_menu_color = TRUE;
    return TRUE;
}

/**
 * @brief Read a menu-colouring rule from configuration text and add it.
 *
 * The form is a pattern, an equals sign, and a colour optionally followed by an attribute.
 *
 * @param tmpstr the text; never null but possibly empty, as the existing comment records
 * @return whether the rule was understood and added
 * @note The pattern is deliberately not tidied of spaces while the colour part is, and the existing comment marks the distinction: spacing inside a pattern is part of what the pattern matches, so collapsing it would change which items the rule applies to.
 * @note Surrounding quotes are stripped from the pattern if present and matched, so a player may quote a pattern containing spaces without the quotes becoming part of it. Mismatched quotes are left alone rather than reported, so a pattern beginning with a quote and
 *       ending without one keeps both characters.
 * @note Unlike the general colour-and-attribute parsing, the order here is fixed: colour then attribute. The equals sign already establishes which side is which, so there is nothing to guess.
 * @note The text is copied before parsing, since parsing splits it in place at the equals and the ampersand.
 */
/**
 * @brief 설정 글에서 메뉴 색칠 규칙을 읽어 더한다.
 *
 * 그 형태는 무늬, 등호, 그리고 선택적으로 속성이 뒤따르는 색이다.
 *
 * @param tmpstr 그 글. 기존 주석이 기록하듯 결코 널이 아니지만 비어 있을 수 있다
 * @return 그 규칙이 이해되고 더해졌는지
 * @note 색 부분은 공백이 정돈되는 반면 무늬는 의도적으로 그러지 않으며, 기존 주석이 그 구별을 표시한다. 무늬 안의 띄어쓰기는 그 무늬가 맞추는 것의 일부이므로, 그것을 줄이는 것은 그 규칙이 어느 항목에 적용되는지를 바꿀 것이다.
 * @note 둘러싼 인용 부호가 있고 짝이 맞으면 무늬에서 떼어지므로, 플레이어가 공백을 담은 무늬를 인용 부호가 그것의 일부가 되지 않게 하면서 인용할 수 있다. 짝이 맞지 않는 인용 부호는 보고되는 대신 그대로 남겨지므로, 인용 부호로 시작하고 그것 없이 끝나는 무늬는 두 문자를 다 지닌다.
 * @note 일반적인 색과 속성 해석과 달리 여기서 순서는 고정되어 있다. 색 그다음 속성. 등호가 이미 어느 쪽이 어느 쪽인지 세우므로, 짐작할 것이 없다.
 * @note 해석이 등호와 앤드 기호에서 제자리에서 글을 쪼개므로, 글이 해석 전에 복사된다.
 */
boolean
add_menu_coloring(char *tmpstr) /* never Null but could be empty */
{
    int c = NO_COLOR, a = ATR_NONE;
    char *tmps, *cs, *amp;
    char str[BUFSZ];

    (void) strncpy(str, tmpstr, sizeof str - 1);
    str[sizeof str - 1] = '\0';

    if ((cs = strchr(str, '=')) == 0) {
        config_error_add("Malformed MENUCOLOR");
        return FALSE;
    }

    tmps = cs + 1; /* advance past '=' */
    mungspaces(tmps);
    if ((amp = strchr(tmps, '&')) != 0)
        *amp = '\0';

    c = match_str2clr(tmps, FALSE);
    if (c >= CLR_MAX)
        return FALSE;

    if (amp) {
        tmps = amp + 1; /* advance past '&' */
        a = match_str2attr(tmps, TRUE);
        if (a == -1)
            return FALSE;
    }

    /* the regexp portion here has not been condensed by mungspaces() */
    *cs = '\0';
    tmps = str;
    if (*tmps == '"' || *tmps == '\'') {
        cs--;
        while (isspace((uchar) *cs))
            cs--;
        if (*cs == *tmps) {
            *cs = '\0';
            tmps++;
        }
    }
    return add_menu_coloring_parsed(tmps, c, a);
}

/**
 * @brief Release every menu-colouring rule, the player's and the substitute set alike.
 *
 * Two lists must be freed and either may already be empty. Rather than writing the freeing twice, the second list is moved into the first's place and the loop runs again -- so the loop body exists once and runs at most twice, as the existing comment records.
 *
 * @note Frees the compiled pattern and the original text as well as the rule, since a rule owns all three.
 * @note Clears the substitute list's pointer as it goes, so a later colour menu builds the substitutes afresh rather than reusing freed memory.
 */
/**
 * @brief 플레이어의 것과 대신할 묶음을 똑같이, 모든 메뉴 색칠 규칙을 놓아준다.
 *
 * 두 목록이 해제되어야 하고 어느 것이든 이미 비어 있을 수 있다. 그 해제하기를 두 번 쓰는 대신, 두 번째 목록이 첫 번째의 자리로 옮겨지고 되돌기가 다시 돈다. 그래서 기존 주석이 기록하듯 되돌기 본문이 한 번 존재하고 최대 두 번 돈다.
 *
 * @note 규칙뿐 아니라 컴파일된 무늬와 원래 글도 해제한다. 규칙이 그 셋 모두를 소유하기 때문이다.
 * @note 진행하면서 대신할 목록의 포인터를 지우므로, 나중의 색 메뉴가 해제된 메모리를 다시 쓰는 대신 대신할 것을 새로 만든다.
 */
void
free_menu_coloring(void)
{
    /* either menu_colorings or color_colorings or both might need to
       be freed or already be Null; do-loop will iterate at most twice */
    do {
        struct menucoloring *tmp, *tmp2;

        for (tmp = gm.menu_colorings; tmp; tmp = tmp2) {
            tmp2 = tmp->next;
            regex_free(tmp->match);
            free((genericptr_t) tmp->origstr);
            free((genericptr_t) tmp);
        }
        gm.menu_colorings = gc.color_colorings;
        gc.color_colorings = (struct menucoloring *) 0;
    } while (gm.menu_colorings);
}

/**
 * @brief Release one menu-colouring rule, chosen by position.
 * @param idx which rule, counting from zero
 * @note Positions count from the front of the list, which is the most recently added rule. That is the order the rules are shown in, so a player removing the third rule they see removes the third entry here.
 * @note Applies only to the player's own rules, never to the substitute set, as the existing comment records. The substitutes are not a player's to edit.
 * @note An index past the end does nothing rather than being reported. The index comes from a menu the player picked from, so it cannot ordinarily be wrong.
 */
/**
 * @brief 자리로 골라진 메뉴 색칠 규칙 하나를 놓아준다.
 * @param idx 어느 규칙인지. 영에서 세어서
 * @note 자리는 목록의 앞에서 세어지며, 그것이 가장 최근에 더해진 규칙이다. 그것이 규칙이 보이는 순서이므로, 자기가 보는 세 번째 규칙을 없애는 플레이어가 여기의 세 번째 항목을 없앤다.
 * @note 기존 주석이 기록하듯 플레이어 자신의 규칙에만 적용되며 대신할 묶음에는 결코 적용되지 않는다. 대신할 것은 플레이어가 편집할 것이 아니다.
 * @note 끝을 넘어선 색인은 보고되는 대신 아무것도 하지 않는다. 그 색인이 플레이어가 고른 메뉴에서 오므로, 보통은 틀릴 수 없다.
 */
void
free_one_menu_coloring(int idx) /* 0 .. */
{
    struct menucoloring *tmp = gm.menu_colorings;
    struct menucoloring *prev = NULL;

    while (tmp) {
        if (idx == 0) {
            struct menucoloring *next = tmp->next;

            regex_free(tmp->match);
            free((genericptr_t) tmp->origstr);
            free((genericptr_t) tmp);
            if (prev)
                prev->next = next;
            else
                gm.menu_colorings = next;
            return;
        }
        idx--;
        prev = tmp;
        tmp = tmp->next;
    }
}

/**
 * @brief How many menu-colouring rules are in force.
 * @return the count
 * @note Counted by walking rather than kept, so it reflects whichever list is currently installed -- which during a colour menu is the substitute set rather than the player's.
 */
/**
 * @brief 몇 개의 메뉴 색칠 규칙이 효력을 내고 있는지.
 * @return 그 개수
 * @note 지녀지는 대신 걸어가서 세어지므로, 지금 설치된 목록이 무엇이든 그것을 비춘다. 색 메뉴 동안 그것은 플레이어의 것이 아니라 대신할 묶음이다.
 */
int
count_menucolors(void)
{
    struct menucoloring *tmp;
    int count = 0;

    for (tmp = gm.menu_colorings; tmp; tmp = tmp->next)
        count++;
    return count;
}

/**
 * @brief Recognise a colour written in any of the three accepted forms.
 *
 * A basic colour's name, a hexadecimal value, or one of the extended colours' names. Tried in that order, because the basic names are the ones a player is likeliest to write and because a name shared between the two halves should give the basic colour.
 *
 * @param buf the text, which must not be null
 * @return the packed colour, or -1 if nothing matched
 * @note A hexadecimal value is accepted only if nothing follows it. The trailing character is captured specifically to detect that, since the conversion would otherwise stop at the junk and accept it.
 * @note Extended names are matched disregarding spaces, hyphens and underscores, so a player may write "dark-red", "dark red" or "darkred". The names in the table use hyphens; the forgiving match is what makes the other spellings work.
 * @note A name containing "grey" is also tried with that spelt "gray", since the table uses one spelling and players use both. The substitution is done by copying the four characters directly rather than through the general text-replacing routine, and the comment records
 *       why: that routine distinguishes case and this must not.
 * @note The extended search walks the whole colour table including the basic entries, so a basic name reaches it only if the earlier attempt already failed -- which it will not, so the duplicate names resolve to the basic colours.
 */
/**
 * @brief 받아들여지는 세 형태 중 어느 것으로든 쓰인 색을 알아본다.
 *
 * 기본 색의 이름, 십육진수 값, 또는 확장 색의 이름 중 하나. 그 순서로 시도되는데, 기본 이름이 플레이어가 쓸 가능성이 가장 큰 것이고 두 반쪽 사이에 공유되는 이름이 기본 색을 주어야 하기 때문이다.
 *
 * @param buf 그 글. 널이어서는 안 된다
 * @return 압축된 색, 또는 아무것도 맞지 않았으면 -1
 * @note 십육진수 값은 그 뒤에 아무것도 따르지 않을 때만 받아들여진다. 뒤따르는 문자가 그것을 알아내려고 특별히 붙잡히는데, 그러지 않으면 그 변환이 그 쓰레기에서 멈추고 그것을 받아들일 것이기 때문이다.
 * @note 확장 이름이 공백, 붙임표, 밑줄을 무시하며 맞춰지므로, 플레이어가 "dark-red", "dark red", "darkred"라고 쓸 수 있다. 표의 이름은 붙임표를 쓰고, 너그러운 맞추기가 다른 표기가 통하게 하는 것이다.
 * @note "grey"를 담은 이름은 그것이 "gray"로 적힌 채로도 시도되는데, 표가 한 표기를 쓰고 플레이어가 둘 다 쓰기 때문이다. 그 갈아치우기가 일반 글 갈아치우기 함수를 통하는 대신 네 문자를 곧바로 복사함으로써 이루어지며, 주석이 이유를 적는다. 그 함수는 대소문자를 가리고 이것은 그래서는 안 된다.
 * @note 확장 찾기가 기본 항목을 포함해 색 표 전체를 걸어가므로, 기본 이름은 앞선 시도가 이미 실패했을 때만 그것에 이른다. 그것은 실패하지 않을 것이므로, 중복된 이름은 기본 색으로 풀린다.
 */
int32
check_enhanced_colors(char *buf)
{
    char xtra = '\0'; /* used to catch trailing junk after "#rrggbb" */
    unsigned r, g, b;
    int32 retcolor = -1, color;

    if ((color = match_str2clr(buf, TRUE)) != CLR_MAX)  {
        retcolor = color | NH_BASIC_COLOR;
    } else if (sscanf(buf, "#%02x%02x%02x%c", &r, &g, &b, &xtra) >= 3) {
        retcolor = !xtra ? (int32) ((r << 16) | (g << 8) | b) : -1;
    } else {
        /* altbuf: allow user's "grey" to match colortable[]'s "gray";
         * fuzzymatch(): ignore spaces, hyphens, and underscores so that
         * space or underscore in user-supplied name will match hyphen
         * [note: caller splits text at spaces so we won't see any here]
         */
        char *altbuf = NULL, *grey = strstri(buf, "grey");
        ptrdiff_t greyoffset = grey ? (grey - buf) : -1;

        if (greyoffset >= 0) {
            altbuf = dupstr(buf);
            /* use direct copy because strsubst() is case-sensitive */
            /*(void) strncpy(&altbuf[greyoffset], "gray", 4);*/
            (void) memcpy(altbuf + greyoffset, "gray", 4);
        }
        for (color = 0; color < SIZE(colortable); ++color) {
            if (fuzzymatch(buf, colortable[color].name, " -_", TRUE)
                || (altbuf && fuzzymatch(altbuf, colortable[color].name,
                                         " -_", TRUE))) {
                retcolor = colortable_to_int32(&colortable[color]);
                break;
            }
        }
        if (altbuf)
            free(altbuf);
    }
    return retcolor;
}

/**
 * @brief Name a packed colour, preferring a name over a hexadecimal value.
 *
 * The inverse of recognising one, used to show a player what colour a setting currently holds. A name is more useful than six hexadecimal digits, so a colour that has one is named.
 *
 * @param colorindx the packed colour
 * @return the name, or a hexadecimal value in a shared buffer if it has no name
 * @note A basic colour is told from an extended one by whether the marker bit is set, and the existing comment sets out how that is tested: clearing the bit and comparing against the original. They differ only if the bit was set.
 * @note The hexadecimal form is built first and then replaced if a name is found, so a colour with no name still has something to show. That costs a formatting call that is often wasted, and buys not having two paths.
 * @note The name search starts past the basic colours, so an extended colour is never given a basic colour's name even when their values coincide. That is the deliberate opposite of what the recognising routine does, which prefers the basic name -- so recognising a name
 *       and then naming the result can give a different name back.
 * @note A negative colour is named as having no colour, so an unset setting reports something a player can understand rather than a number.
 * @warning The hexadecimal result is in a shared buffer, overwritten by the next call that needs it. A name is a pointer into the table and is safe.
 */
/**
 * @brief 압축된 색을 이름 짓되, 십육진수 값보다 이름을 앞세운다.
 *
 * 그것을 알아보는 것의 역이며, 어떤 설정이 지금 어떤 색을 담는지 플레이어에게 보이는 데 쓰인다. 이름이 여섯 개의 십육진수 숫자보다 쓸모 있으므로, 이름을 가진 색은 이름 지어진다.
 *
 * @param colorindx 압축된 색
 * @return 그 이름, 또는 이름이 없으면 공유된 버퍼의 십육진수 값
 * @note 기본 색은 표시 비트가 설정되었는지로 확장 색과 구별되며, 기존 주석이 그것이 어떻게 검사되는지 밝힌다. 그 비트를 지우고 원래 것과 비교하기. 그 비트가 설정되었을 때만 그것들이 다르다.
 * @note 십육진수 형태가 먼저 만들어지고 그다음 이름이 발견되면 갈아치워지므로, 이름 없는 색도 보일 무엇을 가진다. 그것은 흔히 헛되는 형식화 호출을 들이고, 두 경로를 갖지 않음을 산다.
 * @note 이름 찾기가 기본 색을 지나 시작하므로, 확장 색이 값이 겹칠 때조차 기본 색의 이름을 받는 일이 결코 없다. 그것은 기본 이름을 앞세우는 알아보기 함수가 하는 것의 의도된 반대다. 그래서 이름을 알아본 뒤 그 결과를 이름 짓는 것이 다른 이름을 되줄 수 있다.
 * @note 음수 색은 색이 없는 것으로 이름 지어지므로, 설정되지 않은 설정이 숫자가 아니라 플레이어가 이해할 수 있는 무엇을 알린다.
 * @warning 십육진수 결과는 공유된 버퍼에 있으며, 그것을 필요로 하는 다음 호출이 덮어쓴다. 이름은 표를 가리키는 포인터이며 안전하다.
 */
const char *
wc_color_name(int32 colorindx)
{
    static char hexcolor[sizeof "#rrggbb"]; /* includes room for '\0' */
    const char *result = "no-color";

    if (colorindx >= 0) {
        int32 basicindx = colorindx & ~NH_BASIC_COLOR;

        /* if colorindx has NH_BASIC_COLOR bit set, basicindx won't,
           so differing implies a basic color */
        if (basicindx != colorindx) {
            assert(basicindx < 16);
            result = colortable[basicindx].name;
        } else {
            int indx;
            long r = (colorindx >> 16) & 0x0000ff, /* shift rrXXXX to rr */
                 g = (colorindx >> 8) & 0x0000ff,  /* shift XXggXX to gg */
                 b = colorindx & 0x0000ff;         /* mask  XXXXbb to bb */

            Snprintf(hexcolor, sizeof hexcolor, "#%02x%02x%02x",
                     (uint8) r, (uint8) g, (uint8) b);
            result = hexcolor;
            /* override hex value if this is a named color */
            for (indx = 16; indx < SIZE(colortable); ++indx)
                if (colortable[indx].r == r
                    && colortable[indx].g == g
                    && colortable[indx].b == b) {
                    result = colortable[indx].name;
                    break;
                }
        }
    }
    return result;
}

/**
 * @brief Is this text nothing but hexadecimal digits and hyphens?
 * @param buf the text
 * @return whether it is
 * @note Hyphens are accepted along with the digits, because the text this examines is a run of values separated by hyphens rather than a single number.
 * @note The set of digits is shared from elsewhere rather than written here, as the existing comment records, so that what counts as a hexadecimal digit is decided in one place.
 * @note Empty text passes. The callers use this to reject text that is not a colour specification, and empty text is rejected for other reasons.
 */
/**
 * @brief 이 글이 십육진수 숫자와 붙임표 말고는 아무것도 아닌가?
 * @param buf 그 글
 * @return 그런지
 * @note 붙임표가 숫자와 함께 받아들여지는데, 이것이 살펴보는 글이 하나의 수가 아니라 붙임표로 나뉜 값의 이어짐이기 때문이다.
 * @note 기존 주석이 기록하듯 그 숫자 묶음이 여기 쓰이는 대신 다른 곳에서 공유되므로, 무엇이 십육진수 숫자로 셈되는지가 한 곳에서 정해진다.
 * @note 빈 글이 통과한다. 호출자들이 색 명세가 아닌 글을 거부하려고 이것을 쓰며, 빈 글은 다른 이유로 거부된다.
 */
boolean
onlyhexdigits(const char *buf)
{
    const char *dp = buf;

    for (dp = buf; *dp; ++dp) {
        if (!(strchr(hexdd, *dp) || *dp == '-'))
            return FALSE;
    }
    return TRUE;
}

/**
 * @brief Read a colour written as three hyphen-separated components, or as a name.
 *
 * The form is three numbers joined by hyphens. If the text is not that, it is tried as a colour name instead -- so one setting accepts either, and a player need not know which form a particular option wanted.
 *
 * @param rgbstr the text, which may be null
 * @return the packed colour, or -1 if it was not understood
 * @warning The two checks applied to the text disagree about what a digit is, and the stricter one wins. The text must first pass a test that accepts hexadecimal digits, but the parsing loop then rejects anything that is not a decimal digit, and the components are read as
 *          decimal. So a value written with letters -- which the first test invites -- is refused by the second, and the routine's name promises a form it does not accept.
 * @note Each component must be one to three characters, which bounds them at a plausible length without checking that the value fits in a byte. A component of "999" is accepted and its excess bits run into the next component when packed.
 * @note Splits the text in place by overwriting the hyphens, which is why it works on a copy.
 * @note A null or empty argument yields no match rather than being an error, so a caller may pass an unset setting directly.
 */
/**
 * @brief 붙임표로 나뉜 세 성분으로 쓰인 색, 또는 이름으로 쓰인 색을 읽는다.
 *
 * 그 형태는 붙임표로 이어진 세 숫자다. 글이 그것이 아니면 대신 색 이름으로 시도된다. 그래서 하나의 설정이 어느 쪽이든 받아들이며, 플레이어가 특정 선택지가 어느 형태를 원했는지 알아야 하지 않는다.
 *
 * @param rgbstr 그 글. 널일 수 있다
 * @return 압축된 색, 또는 이해되지 않았으면 -1
 * @warning 글에 적용되는 두 검사가 무엇이 숫자인지에 대해 어긋나며, 더 엄한 쪽이 이긴다. 글이 먼저 십육진수 숫자를 받아들이는 시험을 통과해야 하지만, 그다음 해석 되돌기가 십진수 숫자가 아닌 무엇이든 거부하고 성분이 십진수로 읽힌다. 그래서 첫 시험이 권하는, 글자로 쓰인 값이 두 번째에 의해
 *          거부되며, 이 함수의 이름은 자기가 받아들이지 않는 형태를 약속한다.
 * @note 각 성분이 한 문자에서 세 문자여야 하며, 그것이 그 값이 한 바이트에 들어가는지 검사하지 않으면서 그럴듯한 길이로 제한한다. "999"인 성분이 받아들여지고 압축될 때 그 남는 비트가 다음 성분으로 흘러 들어간다.
 * @note 붙임표를 덮어씀으로써 제자리에서 글을 쪼개며, 그것이 사본에서 일하는 이유다.
 * @note 널이거나 빈 인자는 오류가 되는 대신 맞지 않음을 내므로, 호출자가 설정되지 않은 설정을 곧바로 넘길 수 있다.
 */
int32_t
rgbstr_to_int32(const char *rgbstr)
{
    int r, g, b, milestone = 0;
    char *cp, *c_r, *c_g, *c_b;
    int32_t rgb = 0;
    char buf[BUFSZ];
    boolean dash = FALSE;


    Snprintf(buf, sizeof buf, "%s",
             rgbstr ? rgbstr : "");

    if (*buf && onlyhexdigits(buf)) {
        c_g = c_b = (char *) 0;
        c_r = cp = buf;
        while (*cp) {
            if (digit(*cp) || *cp == '-') {
                if (*cp == '-') {
                    *cp = '\0';
                    milestone++;
                    dash = TRUE;
                }
                cp++;
                if (dash) {
                    if (milestone < 2)
                        c_g = cp;
                    else
                        c_b = cp;
                    dash = FALSE;
                }
            } else {
                return -1L;
            }
        }
        /* sanity checks */
        if (c_r && c_g && c_b
            && (strlen(c_r) > 0 && strlen(c_r) < 4)
            && (strlen(c_g) > 0 && strlen(c_g) < 4)
            && (strlen(c_b) > 0 && strlen(c_b) < 4)) {
            r = atoi(c_r);
            g = atoi(c_g);
            b = atoi(c_b);
            rgb = (r << 16) | (g << 8) | (b << 0);
            return rgb;
        }
    } else if (*buf) {
        /* perhaps an enhanced color name was used instead of rgb value? */
        if ((rgb = check_enhanced_colors(buf)) != -1) {
            return rgb;
        }
    }
    return -1;
}

/**
 * @brief Give a map symbol a custom colour, recording both the exact colour and the nearest available one.
 *
 * Two values are stored rather than one because interfaces differ in what they can use. One that supports the full range takes the exact colour; one limited to a fixed palette needs an index into it, and computing that index at drawing time would repeat the work for every
 * frame.
 *
 * @param gmap the symbol
 * @param nhcolor the colour asked for
 * @return whether anything was set
 * @note The nearest index falls back to zero when no near colour could be found, so the field always holds something an interface can index with.
 * @note Accepts a null symbol and does nothing, so a caller need not check first.
 */
/**
 * @brief 맵 기호에 사용자 지정 색을 주며, 정확한 색과 쓸 수 있는 가장 가까운 색을 모두 기록한다.
 *
 * 인터페이스가 쓸 수 있는 것에서 다르기 때문에 하나가 아니라 두 값이 저장된다. 전체 범위를 뒷받침하는 것은 정확한 색을 취한다. 고정된 색판에 갇힌 것은 그 안의 색인을 필요로 하며, 그리는 때에 그 색인을 계산하는 것은 그 일을 매 화면마다 되풀이할 것이다.
 *
 * @param gmap 그 기호
 * @param nhcolor 요청된 색
 * @return 무엇이 설정되었는지
 * @note 가까운 색을 찾을 수 없었을 때 가장 가까운 색인이 영으로 물러나므로, 그 항목이 언제나 인터페이스가 색인할 수 있는 무엇을 담는다.
 * @note 널 기호를 받아들이고 아무것도 하지 않으므로, 호출자가 먼저 검사해야 하지 않는다.
 */
int
set_map_customcolor(glyph_map *gmap, uint32 nhcolor)
{
    glyph_map *tmpgm = gmap;
    uint32 closecolor = 0;
    uint16 clridx = 0;

    if (!tmpgm)
        return 0;

    gmap->customcolor = nhcolor;
    if (closest_color(nhcolor, &closecolor, &clridx))
        gmap->color256idx = clridx;
    else
        gmap->color256idx = 0;
    return 1;
}

/**
 * @var color_256_definitions
 * @brief The actual colours behind the two-hundred-and-fifty-six-colour palette, so an arbitrary colour can be matched to the nearest one.
 *
 * Needed because a terminal offering this palette accepts an index, not a colour. Turning a colour the player asked for into an index requires knowing what each index looks like, and only the terminal knows that -- so the values are recorded here.
 *
 * @note Taken from a particular terminal's palette, as the existing comment records, and by way of another variant of the game. So these are one terminal's choices rather than a standard, and a terminal using different shades will produce visibly different results from the
 *       same index.
 * @note Each entry carries its index explicitly rather than relying on position, which is what lets the table start at sixteen -- the first sixteen indices are the basic colours and are not described here.
 */
/**
 * @var color_256_definitions
 * @brief 이백오십육 색 색판 뒤의 실제 색들. 임의의 색이 가장 가까운 것에 맞춰질 수 있도록.
 *
 * 이 색판을 내놓는 터미널이 색이 아니라 색인을 받아들이기 때문에 필요하다. 플레이어가 요청한 색을 색인으로 바꾸는 것은 각 색인이 어떻게 보이는지 아는 것을 요구하며, 터미널만이 그것을 안다. 그래서 그 값이 여기 적혀 있다.
 *
 * @note 기존 주석이 기록하듯 특정 터미널의 색판에서, 그리고 게임의 다른 변종을 거쳐 가져와졌다. 그래서 이것들은 표준이 아니라 한 터미널의 선택이며, 다른 빛깔을 쓰는 터미널은 같은 색인에서 눈에 보이게 다른 결과를 낼 것이다.
 * @note 각 항목이 자리에 의존하는 대신 자기 색인을 명시적으로 지니며, 그것이 이 표가 열여섯에서 시작할 수 있게 하는 것이다. 첫 열여섯 색인은 기본 색이며 여기 서술되지 않는다.
 */
static struct {
    int index;
    uint32 value;
} color_256_definitions[] = {
    /* from unnethack - these are the colors used by xterm
       when $TERM is set to xterm-256color */
    {  16, 0x000000 }, {  17, 0x00005f }, {  18, 0x000087 },
    {  19, 0x0000af }, {  20, 0x0000d7 }, {  21, 0x0000ff },
    {  22, 0x005f00 }, {  23, 0x005f5f }, {  24, 0x005f87 },
    {  25, 0x005faf }, {  26, 0x005fd7 }, {  27, 0x005fff },
    {  28, 0x008700 }, {  29, 0x00875f }, {  30, 0x008787 },
    {  31, 0x0087af }, {  32, 0x0087d7 }, {  33, 0x0087ff },
    {  34, 0x00af00 }, {  35, 0x00af5f }, {  36, 0x00af87 },
    {  37, 0x00afaf }, {  38, 0x00afd7 }, {  39, 0x00afff },
    {  40, 0x00d700 }, {  41, 0x00d75f }, {  42, 0x00d787 },
    {  43, 0x00d7af }, {  44, 0x00d7d7 }, {  45, 0x00d7ff },
    {  46, 0x00ff00 }, {  47, 0x00ff5f }, {  48, 0x00ff87 },
    {  49, 0x00ffaf }, {  50, 0x00ffd7 }, {  51, 0x00ffff },
    {  52, 0x5f0000 }, {  53, 0x5f005f }, {  54, 0x5f0087 },
    {  55, 0x5f00af }, {  56, 0x5f00d7 }, {  57, 0x5f00ff },
    {  58, 0x5f5f00 }, {  59, 0x5f5f5f }, {  60, 0x5f5f87 },
    {  61, 0x5f5faf }, {  62, 0x5f5fd7 }, {  63, 0x5f5fff },
    {  64, 0x5f8700 }, {  65, 0x5f875f }, {  66, 0x5f8787 },
    {  67, 0x5f87af }, {  68, 0x5f87d7 }, {  69, 0x5f87ff },
    {  70, 0x5faf00 }, {  71, 0x5faf5f }, {  72, 0x5faf87 },
    {  73, 0x5fafaf }, {  74, 0x5fafd7 }, {  75, 0x5fafff },
    {  76, 0x5fd700 }, {  77, 0x5fd75f }, {  78, 0x5fd787 },
    {  79, 0x5fd7af }, {  80, 0x5fd7d7 }, {  81, 0x5fd7ff },
    {  82, 0x5fff00 }, {  83, 0x5fff5f }, {  84, 0x5fff87 },
    {  85, 0x5fffaf }, {  86, 0x5fffd7 }, {  87, 0x5fffff },
    {  88, 0x870000 }, {  89, 0x87005f }, {  90, 0x870087 },
    {  91, 0x8700af }, {  92, 0x8700d7 }, {  93, 0x8700ff },
    {  94, 0x875f00 }, {  95, 0x875f5f }, {  96, 0x875f87 },
    {  97, 0x875faf }, {  98, 0x875fd7 }, {  99, 0x875fff },
    { 100, 0x878700 }, { 101, 0x87875f }, { 102, 0x878787 },
    { 103, 0x8787af }, { 104, 0x8787d7 }, { 105, 0x8787ff },
    { 106, 0x87af00 }, { 107, 0x87af5f }, { 108, 0x87af87 },
    { 109, 0x87afaf }, { 110, 0x87afd7 }, { 111, 0x87afff },
    { 112, 0x87d700 }, { 113, 0x87d75f }, { 114, 0x87d787 },
    { 115, 0x87d7af }, { 116, 0x87d7d7 }, { 117, 0x87d7ff },
    { 118, 0x87ff00 }, { 119, 0x87ff5f }, { 120, 0x87ff87 },
    { 121, 0x87ffaf }, { 122, 0x87ffd7 }, { 123, 0x87ffff },
    { 124, 0xaf0000 }, { 125, 0xaf005f }, { 126, 0xaf0087 },
    { 127, 0xaf00af }, { 128, 0xaf00d7 }, { 129, 0xaf00ff },
    { 130, 0xaf5f00 }, { 131, 0xaf5f5f }, { 132, 0xaf5f87 },
    { 133, 0xaf5faf }, { 134, 0xaf5fd7 }, { 135, 0xaf5fff },
    { 136, 0xaf8700 }, { 137, 0xaf875f }, { 138, 0xaf8787 },
    { 139, 0xaf87af }, { 140, 0xaf87d7 }, { 141, 0xaf87ff },
    { 142, 0xafaf00 }, { 143, 0xafaf5f }, { 144, 0xafaf87 },
    { 145, 0xafafaf }, { 146, 0xafafd7 }, { 147, 0xafafff },
    { 148, 0xafd700 }, { 149, 0xafd75f }, { 150, 0xafd787 },
    { 151, 0xafd7af }, { 152, 0xafd7d7 }, { 153, 0xafd7ff },
    { 154, 0xafff00 }, { 155, 0xafff5f }, { 156, 0xafff87 },
    { 157, 0xafffaf }, { 158, 0xafffd7 }, { 159, 0xafffff },
    { 160, 0xd70000 }, { 161, 0xd7005f }, { 162, 0xd70087 },
    { 163, 0xd700af }, { 164, 0xd700d7 }, { 165, 0xd700ff },
    { 166, 0xd75f00 }, { 167, 0xd75f5f }, { 168, 0xd75f87 },
    { 169, 0xd75faf }, { 170, 0xd75fd7 }, { 171, 0xd75fff },
    { 172, 0xd78700 }, { 173, 0xd7875f }, { 174, 0xd78787 },
    { 175, 0xd787af }, { 176, 0xd787d7 }, { 177, 0xd787ff },
    { 178, 0xd7af00 }, { 179, 0xd7af5f }, { 180, 0xd7af87 },
    { 181, 0xd7afaf }, { 182, 0xd7afd7 }, { 183, 0xd7afff },
    { 184, 0xd7d700 }, { 185, 0xd7d75f }, { 186, 0xd7d787 },
    { 187, 0xd7d7af }, { 188, 0xd7d7d7 }, { 189, 0xd7d7ff },
    { 190, 0xd7ff00 }, { 191, 0xd7ff5f }, { 192, 0xd7ff87 },
    { 193, 0xd7ffaf }, { 194, 0xd7ffd7 }, { 195, 0xd7ffff },
    { 196, 0xff0000 }, { 197, 0xff005f }, { 198, 0xff0087 },
    { 199, 0xff00af }, { 200, 0xff00d7 }, { 201, 0xff00ff },
    { 202, 0xff5f00 }, { 203, 0xff5f5f }, { 204, 0xff5f87 },
    { 205, 0xff5faf }, { 206, 0xff5fd7 }, { 207, 0xff5fff },
    { 208, 0xff8700 }, { 209, 0xff875f }, { 210, 0xff8787 },
    { 211, 0xff87af }, { 212, 0xff87d7 }, { 213, 0xff87ff },
    { 214, 0xffaf00 }, { 215, 0xffaf5f }, { 216, 0xffaf87 },
    { 217, 0xffafaf }, { 218, 0xffafd7 }, { 219, 0xffafff },
    { 220, 0xffd700 }, { 221, 0xffd75f }, { 222, 0xffd787 },
    { 223, 0xffd7af }, { 224, 0xffd7d7 }, { 225, 0xffd7ff },
    { 226, 0xffff00 }, { 227, 0xffff5f }, { 228, 0xffff87 },
    { 229, 0xffffaf }, { 230, 0xffffd7 }, { 231, 0xffffff },
    { 232, 0x080808 }, { 233, 0x121212 }, { 234, 0x1c1c1c },
    { 235, 0x262626 }, { 236, 0x303030 }, { 237, 0x3a3a3a },
    { 238, 0x444444 }, { 239, 0x4e4e4e }, { 240, 0x585858 },
    { 241, 0x626262 }, { 242, 0x6c6c6c }, { 243, 0x767676 },
    { 244, 0x808080 }, { 245, 0x8a8a8a }, { 246, 0x949494 },
    { 247, 0x9e9e9e }, { 248, 0xa8a8a8 }, { 249, 0xb2b2b2 },
    { 250, 0xbcbcbc }, { 251, 0xc6c6c6 }, { 252, 0xd0d0d0 },
    { 253, 0xdadada }, { 254, 0xe4e4e4 }, { 255, 0xeeeeee },
};

/**
 * @brief How different two colours look to a person.
 *
 * Not the plain distance between the two sets of components, which would treat every channel as equally noticeable. The eye is far more sensitive to green than to blue, and its sensitivity to red depends on how bright the red already is -- so the channels are weighted, and
 * the red weighting varies with the average of the two reds.
 *
 * @param rgb1 one colour
 * @param rgb2 the other
 * @return a number that is larger the more different they look
 * @note Taken from elsewhere rather than derived, as the existing comment records, and the source is named there. That matters because the constants have no local justification -- they cannot be reasoned about here, only cited.
 * @note The result is a squared measure, so it can be compared against another result but is not a distance in any unit. Only the ordering is meaningful.
 */
/**
 * @brief 두 색이 사람에게 얼마나 다르게 보이는지.
 *
 * 두 성분 묶음 사이의 맨 거리가 아니다. 그것은 모든 통로를 똑같이 눈에 띄는 것으로 다룰 것이다. 눈은 파랑보다 초록에 훨씬 민감하고, 빨강에 대한 민감함은 그 빨강이 이미 얼마나 밝은지에 달려 있다. 그래서 통로들이 가중되며, 빨강의 가중치가 두 빨강의 평균과 함께 달라진다.
 *
 * @param rgb1 한 색
 * @param rgb2 다른 색
 * @return 그것들이 다르게 보일수록 큰 수
 * @note 기존 주석이 기록하듯 끌어내어진 것이 아니라 다른 곳에서 가져와졌고, 그 출처가 거기 이름 지어져 있다. 그것이 중요한 것은 그 상수들이 국지적인 정당화를 갖지 않기 때문이다. 그것들은 여기서 따져질 수 없고 인용될 수만 있다.
 * @note 결과가 제곱된 잣대이므로, 다른 결과와 비교될 수 있지만 어떤 단위의 거리도 아니다. 순서만이 뜻이 있다.
 */
int
color_distance(uint32_t rgb1, uint32_t rgb2)
{
    int r1 = (rgb1 >> 16) & 0xFF;
    int g1 = (rgb1 >> 8) & 0xFF;
    int b1 = (rgb1) & 0xFF;
    int r2 = (rgb2 >> 16) & 0xFF;
    int g2 = (rgb2 >> 8) & 0xFF;
    int b2 = (rgb2) & 0xFF;

    int rmean = (r1 + r2) / 2;
    int r = r1 - r2;
    int g = g1 - g2;
    int b = b1 - b2;
    return ((((512 + rmean) * r * r) >> 8) + 4 * g * g
            + (((767 - rmean) * b * b) >> 8));
}

/**
 * @brief Find the palette entry that looks most like a given colour.
 * @param lcolor the colour wanted
 * @param closecolor receives the palette colour found
 * @param clridx receives its index
 * @return whether anything was found
 * @note Stops at an exact match rather than measuring the rest, which is both faster and avoids the possibility of the weighted measure preferring some other entry to an exact one.
 * @note Returns the palette entry's recorded index rather than its position in the table, which are different numbers -- the table begins at sixteen.
 * @note Reports failure if either output was not asked for, so a caller wanting only one of the two must still supply both.
 */
/**
 * @brief 주어진 색과 가장 비슷하게 보이는 색판 항목을 찾는다.
 * @param lcolor 바라진 색
 * @param closecolor 발견된 색판 색을 받는다
 * @param clridx 그 색인을 받는다
 * @return 무엇이 발견되었는지
 * @note 나머지를 재는 대신 정확한 맞음에서 멈추며, 그것은 더 빠르기도 하고 가중된 잣대가 정확한 것보다 다른 항목을 앞세울 가능성을 피하기도 한다.
 * @note 표에서의 자리가 아니라 색판 항목의 적힌 색인을 돌려주며, 그것들은 다른 숫자다. 그 표는 열여섯에서 시작한다.
 * @note 두 출력 중 하나가 요청되지 않았으면 실패를 알리므로, 그 둘 중 하나만 원하는 호출자도 여전히 둘 다를 내주어야 한다.
 */
boolean
closest_color(uint32 lcolor, uint32 *closecolor, uint16 *clridx)
{
    int i, color_index = -1, similar = INT_MAX, current;
    boolean retbool = FALSE;

    for (i = 0; i < SIZE(color_256_definitions); i++) {
        /* look for an exact match */
        if (lcolor == color_256_definitions[i].value) {
            color_index = i;
            break;
        }
        /* find a close color match */
        current = color_distance(lcolor, color_256_definitions[i].value);
        if (current < similar) {
            color_index = i;
            similar = current;
        }
    }
    if (closecolor && clridx && color_index >= 0) {
        *closecolor = color_256_definitions[color_index].value;
        *clridx = color_256_definitions[color_index].index;
        retbool = TRUE;
    }
    return retbool;
}

/**
 * @brief The colour a palette index stands for.
 * @param idx the index
 * @return the colour, or the absence of colour if the index is out of range
 * @warning Treats its argument as a position in the table rather than as a palette index, and those are not the same number -- the table's first entry records index sixteen. Since the only caller passes a value that closest_color() set from the recorded index, the colour
 *          returned is the one sixteen positions earlier than the one meant. The two agree only for an index of exactly sixteen.
 * @note Returns the absence of colour for an out-of-range index rather than reporting it, so a wrong index produces something drawable. Combined with the offset above, an index in the last sixteen of the palette falls off the end and comes back uncoloured.
 */
/**
 * @brief 색판 색인이 대신하는 색.
 * @param idx 그 색인
 * @return 그 색, 또는 색인이 범위를 벗어나면 색의 없음
 * @warning 자기 인자를 색판 색인이 아니라 표에서의 자리로 다루며, 그것들은 같은 숫자가 아니다. 표의 첫 항목이 색인 열여섯을 적는다. 유일한 호출자가 closest_color()가 적힌 색인에서 설정한 값을 넘기므로, 돌려주어지는 색은 뜻해진 것보다 열여섯 자리 앞선 것이다. 그 둘은 정확히 열여섯인 색인에
 *          대해서만 일치한다.
 * @note 범위를 벗어난 색인에 대해 그것을 알리는 대신 색의 없음을 돌려주므로, 틀린 색인이 그릴 수 있는 무엇을 낸다. 위의 어긋남과 합쳐져, 색판의 마지막 열여섯 안의 색인이 끝에서 떨어져 나가 색 없이 돌아온다.
 */
uint32
get_nhcolor_from_256_index(int idx)
{
    uint32 retcolor = NO_COLOR | NH_BASIC_COLOR;

    if (IndexOk(idx, color_256_definitions))
        retcolor = color_256_definitions[idx].value;
    return retcolor;
}

#ifdef CHANGE_COLOR

/**
 * @name Redefining what the basic colours look like
 *
 * On terminals that permit it, a player may say what shade each of the sixteen basic colours should actually be. That is a different facility from naming an extended colour: it changes what "red" looks like everywhere rather than choosing red for one thing.
 *
 * Present only where the build supports it, which is why the whole group is enclosed in a test.
 * @{
 */

/**
 * @brief How many of the basic colours the player has redefined.
 * @return the count
 * @note Counted by looking for entries that are not zero, so zero doubles as "not redefined". A player cannot therefore redefine a colour to be exactly black by value alone -- the marker bit the setting carries is what makes such an entry non-zero.
 */
/**
 * @brief 플레이어가 기본 색 가운데 몇 개를 다시 정의했는지.
 * @return 그 개수
 * @note 영이 아닌 항목을 찾아서 세어지므로, 영이 "다시 정의되지 않음"을 겸한다. 그러므로 플레이어가 값만으로 어떤 색을 정확히 검정으로 다시 정의할 수 없다. 그 설정이 지니는 표시 비트가 그런 항목을 영이 아니게 만드는 것이다.
 */
int
count_alt_palette(void)
{
    int clr, clrcount = 0;

    for (clr = 0; clr < CLR_MAX; ++clr) {
        if (ga.altpalette[clr] != 0U)
            clrcount++;
    }
    return clrcount;
}

/**
 * @brief Record one redefinition: which basic colour, and what it should look like.
 *
 * The form is a colour's name, a slash, and a value. The value is tried in two ways -- the hyphen-separated form first and then a more permissive form accepting several notations -- so a player may write the value however they are used to.
 *
 * @param op the text
 * @return whether the redefinition was recorded
 * @note Only the basic colours may be redefined, which is why the name is matched against the small name table. Redefining an extended colour would be meaningless: those are specified by value already.
 * @note The recorded entry carries a marker bit alongside the value, and the existing comment names the macro that strips it back off. That bit is what distinguishes a redefined colour from an unset one, so nothing may read the entry as a plain value.
 * @note Recording the redefinition does not apply it. A separate step tells the interface, so several redefinitions read from configuration take effect together rather than one at a time.
 * @warning The text is scanned for two separator characters but only one of them has any effect -- the colon is recognised and then ignored, so a setting written with a colon is silently treated as having no value at all. Only the slash separates.
 */
/**
 * @brief 다시 정의 하나를 적는다. 어느 기본 색이고, 그것이 어떻게 보여야 하는지.
 *
 * 그 형태는 색의 이름, 사선, 그리고 값이다. 그 값이 두 방식으로 시도된다. 붙임표로 나뉜 형태가 먼저, 그다음 여러 표기를 받아들이는 더 너그러운 형태가. 그래서 플레이어가 값을 자기가 익숙한 대로 쓸 수 있다.
 *
 * @param op 그 글
 * @return 그 다시 정의가 적혔는지
 * @note 기본 색만이 다시 정의될 수 있으며, 그것이 그 이름이 작은 이름 표와 견주어 맞춰지는 이유다. 확장 색을 다시 정의하는 것은 뜻이 없을 것이다. 그것들은 이미 값으로 명세된다.
 * @note 적힌 항목이 값과 나란히 표시 비트를 지니며, 기존 주석이 그것을 되벗겨내는 매크로를 이름 짓는다. 그 비트가 다시 정의된 색을 설정되지 않은 것과 구별하는 것이므로, 무엇도 그 항목을 맨 값으로 읽어서는 안 된다.
 * @note 다시 정의를 적는 것이 그것을 적용하지는 않는다. 따로 된 단계가 인터페이스에 알리므로, 설정에서 읽힌 여러 다시 정의가 하나씩이 아니라 함께 효력을 낸다.
 * @warning 글이 두 구분 문자를 찾아 훑어지지만 그중 하나만 어떤 효과를 갖는다. 쌍점은 알아보아지고 그다음 무시되므로, 쌍점으로 쓰인 설정이 조용히 값이 아예 없는 것으로 다뤄진다. 사선만이 나눈다.
 */
int
alternative_palette(char *op)
{
    char buf[BUFSZ], *c_colorid, *c_colorval, *cp;
    int reslt = 0, coloridx = CLR_MAX;
    long rgb = 0L;
    boolean slash = FALSE;

    if (!op)
        return 0;

    Snprintf(buf, sizeof buf, "%s", op);
    c_colorval = (char *) 0;
    c_colorid = cp = buf;
    while (*cp) {
        if (*cp == ':' || *cp == '/') {
            if (*cp == '/') {
                slash = TRUE;
                *cp = '\0';
            }
        }
        cp++;
        if (slash) {
            c_colorval = cp;
            slash = FALSE;
        }
    }
    /* some sanity checks */
    if (c_colorid && *c_colorid == ' ')
        c_colorid++;
    if (c_colorval && *c_colorval == ' ')
        c_colorval++;
    if (c_colorid)
        coloridx = match_str2clr(c_colorid, TRUE);

    if (c_colorval && coloridx >= 0 && coloridx < CLR_MAX) {
        rgb = rgbstr_to_int32(c_colorval);
        if (rgb == -1) {
            rgb = alt_color_spec(c_colorval);
        }
        if (rgb != -1) {
            ga.altpalette[coloridx] = (uint32) rgb | NH_ALTPALETTE;
            /* use COLORVAL(ga.altpalette[coloridx]) to get
               the actual rgb value out of ga.altpalette[] */
            reslt = 1;
        }
    }
    return reslt;
}

/**
 * @brief Tell the interface about every redefined colour.
 * @note Applies the redefinitions all at once rather than as each is read, so the display changes once instead of flickering through intermediate states while configuration is processed.
 * @note Strips the marker bit before passing each value on, since the interface expects a plain colour.
 * @note Skips entries that were never redefined, so an interface is told only about the colours the player actually changed.
 */
/**
 * @brief 다시 정의된 모든 색에 대해 인터페이스에 알린다.
 * @note 각각이 읽힐 때가 아니라 한꺼번에 다시 정의를 적용하므로, 설정이 처리되는 동안 중간 상태를 거쳐 깜빡이는 대신 화면이 한 번 바뀐다.
 * @note 각 값을 넘기기 전에 표시 비트를 벗겨낸다. 인터페이스가 맨 색을 기대하기 때문이다.
 * @note 결코 다시 정의되지 않은 항목을 건너뛰므로, 인터페이스가 플레이어가 실제로 바꾼 색에 대해서만 듣는다.
 */
void
change_palette(void)
{
    int clridx;

    for (clridx = 0; clridx < CLR_MAX; ++clridx) {
        if (ga.altpalette[clridx] != 0) {
            long rgb = (long) COLORVAL(ga.altpalette[clridx]);
            (*windowprocs.win_change_color)(clridx, rgb, 0);
        }
    }
}

/**
 * @brief Read a number written in any of several notations.
 *
 * Accepts decimal, hexadecimal after a hash or an escape, and octal after an escape. It exists because a player redefining a colour is likely to have the value written down in whichever notation their terminal's documentation used, and refusing three of the four would be
 * an obstacle with no purpose.
 *
 * @param str the text
 * @return the value, or -1 if it could not be read
 * @note The digit limit differs by notation, since the same value needs more digits in a smaller base. That is why the limit is set when the notation is recognised rather than being one number.
 * @note A single character is handled specially, since there is no notation prefix to recognise and a lone digit is simply itself.
 * @note The hexadecimal digits come from a shared table that lists each digit in both cases, which is why converting a digit to its value halves the position found -- the table has two entries per value.
 * @warning A character that is not a digit in the notation being read is silently skipped rather than ending the number. So "1x2" is read as twelve, and a mistyped value becomes a different value instead of an error. Only exceeding the digit limit fails.
 */
/**
 * @brief 여러 표기 중 어느 것으로든 쓰인 수를 읽는다.
 *
 * 십진수, 우물 기호나 벗어나기 뒤의 십육진수, 벗어나기 뒤의 팔진수를 받아들인다. 색을 다시 정의하는 플레이어가 자기 터미널의 설명서가 쓴 표기가 무엇이든 그것으로 값을 적어 두었을 가능성이 크고, 넷 중 셋을 거부하는 것은 목적 없는 방해가 될 것이기 때문에 존재한다.
 *
 * @param str 그 글
 * @return 그 값, 또는 읽을 수 없었으면 -1
 * @note 숫자 한계가 표기에 따라 다른데, 같은 값이 더 작은 밑수에서 더 많은 숫자를 필요로 하기 때문이다. 그것이 그 한계가 하나의 수인 대신 표기가 알아보아질 때 설정되는 이유다.
 * @note 한 문자는 특별히 다뤄지는데, 알아볼 표기 접두가 없고 홀로 있는 숫자가 그저 그 자신이기 때문이다.
 * @note 십육진수 숫자가 각 숫자를 두 대소문자로 나열하는 공유된 표에서 오며, 그것이 숫자를 그 값으로 바꾸는 것이 발견된 자리를 반으로 하는 이유다. 그 표가 값마다 두 항목을 가진다.
 * @warning 읽히고 있는 표기에서 숫자가 아닌 문자가 그 수를 끝내는 대신 조용히 건너뛰어진다. 그래서 "1x2"가 열둘로 읽히고, 잘못 입력된 값이 오류가 아니라 다른 값이 된다. 숫자 한계를 넘는 것만이 실패한다.
 */
staticfn int32
alt_color_spec(const char *str)
{
    static NEARDATA const char oct[] = "01234567", dec[] = "0123456789";
    /* hexdd[] is defined in decl.c */

    const char *dp, *cp = str;
    int32 cval = -1;
    int dcount, dlimit = 6;
    boolean hexescape = FALSE, octescape = FALSE;

    dcount = 0; /* for decimal, octal, hexadecimal cases */
    hexescape =
        (*cp == '\\' && cp[1] && (cp[1] == 'x' || cp[1] == 'X') && cp[2]);
    if (!hexescape) {
        octescape =
            (*cp == '\\' && cp[1] && (cp[1] == 'o' || cp[1] == 'O') && cp[2]);
    }

    if (hexescape || octescape) {
        cval = 0;
        cp += 2;
        if (octescape)
            dlimit = 8;
    } else if (*cp == '#' && cp[1]) {
        hexescape = TRUE;
        cval = 0;
        cp += 1;
    } else if (cp[1]) {
        cval = 0;
        dlimit = 8;
    } else if (!cp[1]) {
        if (strchr(dec, *cp) != 0) {
            /* simple val, or nothing left for \ to escape */
            cval = (*cp - '0');
        }
        dlimit = 1;
        cp++;
    }

    while (*cp) {
        if (!hexescape && !octescape && strchr(dec, *cp)) {
            cval = (cval * 10) + (*cp - '0');
        } else if (octescape && strchr(oct, *cp)) {
            cval = (cval * 8) + (*cp - '0');
        } else if (hexescape && (dp = strchr(hexdd, *cp)) != 0) {
            cval = (cval * 16) + ((int) (dp - hexdd) / 2);
        }
        ++cp;
        if (++dcount > dlimit) {
            cval = -1;
            break;
        }
    }
    return cval;
}
#endif /* CHANGE_COLOR */

/** @} */

/*coloratt.c*/
