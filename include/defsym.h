/* NetHack 5.0	defsym.h $NHDT-Date: 1781973078 2026/06/20 16:31:18 $ $NHDT-Branch: NetHack-5.0 $ $NHDT-Revision: 1.27 $ */
/*      Copyright (c) 2016 by Pasi Kallinen              */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file defsym.h
 * @brief The one place every drawable symbol is described.
 *
 * A symbol has to exist in a great many forms. It needs an enumerator so code can name it, a default character so it can be drawn, a colour, a name the player can type in a
 * configuration file, an entry in the tile mapping, and a place in the tables the game builds at startup. Writing those separately would let them drift, and the drift would be
 * invisible -- a symbol whose name no longer matched its value would simply do the wrong thing.
 *
 * So this file is the description, read many times over. Each includer defines a macro naming what it wants, and the same list produces a different form each time. The comment below
 * lists all of them and where each is used, and it is worth reading as a map: it says which files depend on this one and for what.
 *
 * @note So this is not a header in the usual sense. It has no include guard, and it defines nothing on its own -- what it produces depends entirely on which macro was defined before
 *       it was read.
 * @note One of the uses is dumping the enumerations for external tools. That is why the list carries a name as text as well as a value: something outside the game needs to be able to
 *       read them back.
 * @warning Adding a symbol adds it to every one of those forms at once, which is the point. Adding a form means adding a case here and in every one of the listed places -- omitting
 *          one leaves a symbol that exists in some tables and not others.
 */

/**
 * @file defsym.h
 * @brief 그릴 수 있는 모든 심볼이 기술되는 단 하나의 장소.
 *
 * 심볼은 아주 많은 형태로 존재해야 한다. 코드가 그것을 지칭할 열거자, 그것이 그려질 기본 문자, 색, 플레이어가 설정 파일에 입력할 이름, 타일 대응의 항목, 그리고 게임이 시작할 때 만드는 표들 안의 자리. 그것들을 따로 적으면 서로 어긋날 수 있고, 그 어긋남은 보이지 않는다.
 * 이름이 더는 자기 값과 맞지 않는 심볼은 그저 잘못된 일을 할 뿐이다.
 *
 * 그래서 이 파일이 그 기술이며, 여러 번 되풀이해 읽힌다. 각 포함하는 쪽이 자신이 원하는 것을 지칭하는 매크로를 정의하고, 같은 목록이 매번 다른 형태를 만들어 낸다. 아래의 주석이 그 전부와 각각이 어디에 쓰이는지를 나열하며, 지도로서 읽어 볼 가치가 있다. 어떤 파일이 이 파일에
 * 의존하고 무엇을 위해 그러는지 밝힌다.
 *
 * @note 그래서 이것은 통상적인 의미의 헤더가 아니다. 포함 보호가 없고 스스로는 아무것도 정의하지 않는다. 무엇을 만들어 내는지는 전적으로 읽히기 전에 어느 매크로가 정의되었는지에 달려 있다.
 * @note 그 용도 중 하나는 외부 도구를 위해 열거를 덤프하는 것이다. 목록이 값과 함께 글로서의 이름도 지니는 이유가 그것이다. 게임 밖의 무엇이 그것을 되읽을 수 있어야 한다.
 * @warning 심볼을 더하는 것은 그 모든 형태에 한꺼번에 더하는 일이며, 그것이 요점이다. 형태를 더하는 것은 여기와 나열된 모든 곳에 경우를 더하는 일이다. 하나를 빠뜨리면 어떤 표에는 있고 어떤 표에는 없는 심볼이 남는다.
 */

/*
    This header is included in multiple places to produce
    different code depending on its use. Its purpose is to
    ensure that there is only one definitive source for
    pchar, objclass and mon symbols.

    The morphing macro expansions are used in these places:
  - in include/sym.h for enums of some S_* symbol values
    (define PCHAR_S_ENUM, MONSYMS_S_ENUM prior to #include defsym.h)
  - in include/sym.h for enums of some DEF_* symbol values
    (define MONSYMS_DEFCHAR_ENUM prior to #include defsym.h)
  - in include/objclass.h for enums of some default character values
    (define OBJCLASS_DEFCHAR_ENUM prior to #include defsym.h)
  - in include/objclass.h for enums of some *_CLASS values
    (define OBJCLASS_CLASS_ENUM prior to #include defsym.h)
  - in include/objclass.h for enums of S_* symbol values
    (define OBJCLASS_S_ENUM prior to #include defsym.h)
  - in src/symbols.c for parsing S_ entries in config files
    (define PCHAR_PARSE, MONSYMS_PARSE, OBJCLASS_PARSE prior
    to #include defsym.h)
  - in src/drawing.c for initializing some data structures/arrays
    (define PCHAR_DRAWING, MONSYMS_DRAWING, OBJCLASS_DRAWING prior
    to #include defsym.h)
  - in win/share/tilemap.c for processing a tile file
    (define PCHAR_TILES prior to #include defsym.h).
  - in src/allmain.c for setting up the dumping of several enums
    (define DUMP_ENUMS_PCHAR, DUMP_ENUMS_MONSYS, DUMP_ENUMS_MONSYMS_DEFCHAR
     DUMP_ENUMS_OBJCLASS_DEFCHARS, DUMP_ENUMS_OBJCLASS_DEFCHARS
     DUMP_ENUMS_OBJCLASS_CLASSES, DUMP_ENUMS_OBJCLASS_SYMS)
*/

#if defined(PCHAR_S_ENUM)               \
    || defined(PCHAR_PARSE)             \
    || defined(PCHAR_DRAWING)           \
    || defined(PCHAR_TILES)             \
    || defined(DUMP_ENUMS_PCHAR)

/*
   PCHAR(idx, ch, sym, desc, clr)
       idx:     index used in enum
       ch:      character symbol
       sym:     symbol name for parsing purposes (also tile name)
       desc:    description
       clr:     color

   PCHAR2(idx, ch, sym, tilenm, desc, clr)
       idx:     index used in enum
       ch:      character symbol
       sym:     symbol name for parsing purposes
       tilenm:  if the name in the tile txt file differs from desc (below),
                the name in the tile txt file can be specified here.
       desc:    description
       clr:     color
*/

#if defined(PCHAR_S_ENUM)
/* sym.h */
#define PCHAR(idx, ch, sym, desc, clr) sym = idx,

#elif defined(PCHAR_PARSE)
/* symbols.c */
#define PCHAR(idx, ch, sym, desc, clr) { SYM_PCHAR, sym, #sym },

#elif defined(PCHAR_DRAWING)
/* drawing.c */
#define PCHAR(idx, ch, sym, desc, clr) { ch, desc, clr },

#elif defined(PCHAR_TILES)
/* win/share/tilemap.c */
#define PCHAR(idx, ch, sym, desc, clr) { sym, desc, desc },
#define PCHAR2(idx, ch, sym, tilenm, desc, clr) { sym, tilenm, desc },

#elif defined(DUMP_ENUMS_PCHAR)
/* allmain.c */
#define PCHAR(idx, ch, sym, desc, clr) { sym, #sym },
#ifndef PCHAR2
#define PCHAR2(idx, ch, sym, tilenm, desc, clr) { sym, #sym },
#endif
#endif

/* PCHAR with extra arg */
#ifndef PCHAR2
#define PCHAR2(idx, ch, sym, tilenm, desc, clr) PCHAR(idx, ch, sym, desc, clr)
#endif

    PCHAR2( 0, ' ',  S_stone,  "dark part of a room", "stone",  NO_COLOR)
    PCHAR2( 1, '|',  S_vwall,  "vertical wall", "wall",  CLR_GRAY)
    PCHAR2( 2, '-',  S_hwall,  "horizontal wall", "wall",  CLR_GRAY)
    PCHAR2( 3, '-',  S_tlcorn, "top left corner wall", "wall",  CLR_GRAY)
    PCHAR2( 4, '-',  S_trcorn, "top right corner wall", "wall",  CLR_GRAY)
    PCHAR2( 5, '-',  S_blcorn, "bottom left corner wall", "wall", CLR_GRAY)
    PCHAR2( 6, '-',  S_brcorn, "bottom right corner wall", "wall", CLR_GRAY)
    PCHAR2( 7, '-',  S_crwall, "cross wall", "wall", CLR_GRAY)
    PCHAR2( 8, '-',  S_tuwall, "tuwall", "wall", CLR_GRAY)
    PCHAR2( 9, '-',  S_tdwall, "tdwall", "wall", CLR_GRAY)
    PCHAR2(10, '|',  S_tlwall, "tlwall", "wall", CLR_GRAY)
    PCHAR2(11, '|',  S_trwall, "trwall", "wall", CLR_GRAY)
    /* start cmap A                                                      */
    PCHAR2(12, '.',  S_ndoor,  "no door", "doorway", CLR_GRAY)
    PCHAR2(13, '-',  S_vodoor, "vertical open door", "open door", CLR_BROWN)
    PCHAR2(14, '|',  S_hodoor, "horizontal open door", "open door", CLR_BROWN)
    PCHAR2(15, '+',  S_vcdoor, "vertical closed door",
                               "closed door", CLR_BROWN)
    PCHAR2(16, '+',  S_hcdoor, "horizontal closed door",
                               "closed door", CLR_BROWN)
    PCHAR( 17, '#',  S_bars,   "iron bars", HI_METAL)
    PCHAR( 18, '#',  S_tree,   "tree", CLR_GREEN)
    PCHAR( 19, '.',  S_room,   "floor of a room", CLR_GRAY)
    PCHAR( 20, '.',  S_darkroom, "dark part of a room", CLR_BLACK)
    PCHAR2(21, '`',  S_engroom, "engraving in a room", "engraving",
                                CLR_BRIGHT_BLUE)
    PCHAR2(22, '#',  S_corr,   "dark corridor", "corridor", CLR_GRAY)
    PCHAR( 23, '#',  S_litcorr, "lit corridor", CLR_GRAY)
    PCHAR2(24, '#',  S_engrcorr, "engraving in a corridor", "engraving",
                                 CLR_BRIGHT_BLUE)
    PCHAR2(25, '<',  S_upstair, "up stairs", "staircase up", CLR_GRAY)
    PCHAR2(26, '>',  S_dnstair, "down stairs", "staircase down", CLR_GRAY)
    PCHAR2(27, '<',  S_upladder, "up ladder", "ladder up", CLR_BROWN)
    PCHAR2(28, '>',  S_dnladder, "down ladder", "ladder down", CLR_BROWN)
    PCHAR( 29, '<',  S_brupstair, "branch staircase up", CLR_YELLOW)
    PCHAR( 30, '>',  S_brdnstair, "branch staircase down", CLR_YELLOW)
    PCHAR( 31, '<',  S_brupladder, "branch ladder up", CLR_YELLOW)
    PCHAR( 32, '>',  S_brdnladder, "branch ladder down", CLR_YELLOW)
    /* end cmap A */
    PCHAR( 33, '_',  S_altar,  "altar", CLR_GRAY)
    /* start cmap B */
    PCHAR( 34, '|',  S_grave,  "grave", CLR_WHITE)
    PCHAR2(35, '\\', S_throne, "throne", "opulent throne", HI_GOLD)
    PCHAR( 36, '{',  S_sink,   "sink", CLR_WHITE)
    PCHAR( 37, '{',  S_fountain, "fountain", CLR_BRIGHT_BLUE)
    /* the S_pool symbol is used for both POOL terrain and MOAT terrain */
    PCHAR2(38, '}',  S_pool,   "pool", "water", CLR_BLUE)
    PCHAR( 39, '.',  S_ice,    "ice", CLR_CYAN)
    PCHAR( 40, '}',  S_lava,   "molten lava", CLR_RED)
    PCHAR( 41, '}',  S_lavawall,  "wall of lava", CLR_ORANGE)
    PCHAR2(42, '.',  S_vodbridge, "vertical open drawbridge",
                                  "lowered drawbridge", CLR_BROWN)
    PCHAR2(43, '.',  S_hodbridge, "horizontal open drawbridge",
                                  "lowered drawbridge", CLR_BROWN)
    PCHAR2(44, '#',  S_vcdbridge, "vertical closed drawbridge",
                                  "raised drawbridge", CLR_BROWN)
    PCHAR2(45, '#',  S_hcdbridge, "horizontal closed drawbridge",
                                  "raised drawbridge", CLR_BROWN)
    PCHAR( 46, ' ',  S_air,    "air", CLR_CYAN)
    PCHAR( 47, '#',  S_cloud,  "cloud", CLR_GRAY)
    /* the S_water symbol is used for WATER terrain: wall of water in the
       dungeon and Plane of Water in the endgame */
    PCHAR( 48, '}',  S_water,  "water", CLR_BRIGHT_BLUE)
    /* end dungeon characters                                          */
    /*                                                                 */
    /* begin traps                                                     */
    /*                                                                 */
    PCHAR( 49, '^',  S_arrow_trap, "arrow trap", HI_METAL)
    PCHAR( 50, '^',  S_dart_trap, "dart trap", HI_METAL)
    PCHAR( 51, '^',  S_falling_rock_trap, "falling rock trap", CLR_GRAY)
    PCHAR( 52, '^',  S_squeaky_board, "squeaky board", CLR_BROWN)
    PCHAR( 53, '^',  S_bear_trap, "bear trap", HI_METAL)
    PCHAR( 54, '^',  S_land_mine, "land mine", CLR_RED)
    PCHAR( 55, '^',  S_rolling_boulder_trap, "rolling boulder trap", CLR_GRAY)
    PCHAR( 56, '^',  S_sleeping_gas_trap, "sleeping gas trap", HI_ZAP)
    PCHAR( 57, '^',  S_rust_trap, "rust trap", CLR_BLUE)
    PCHAR( 58, '^',  S_fire_trap, "fire trap", CLR_ORANGE)
    PCHAR( 59, '^',  S_pit, "pit", CLR_BLACK)
    PCHAR( 60, '^',  S_spiked_pit, "spiked pit", CLR_BLACK)
    PCHAR( 61, '^',  S_hole, "hole", CLR_BROWN)
    PCHAR( 62, '^',  S_trap_door, "trap door", CLR_BROWN)
    PCHAR( 63, '^',  S_teleportation_trap, "teleportation trap", CLR_MAGENTA)
    PCHAR( 64, '^',  S_level_teleporter, "level teleporter", CLR_MAGENTA)
    PCHAR( 65, '^',  S_magic_portal, "magic portal", CLR_BRIGHT_MAGENTA)
    PCHAR( 66, '"',  S_web, "web", CLR_GRAY)
    PCHAR( 67, '^',  S_statue_trap, "statue trap", CLR_GRAY)
    PCHAR( 68, '^',  S_magic_trap, "magic trap", HI_ZAP)
    PCHAR2(69, '^',  S_anti_magic_trap, "anti magic trap", "anti-magic field",
                                        HI_ZAP)
    PCHAR( 70, '^',  S_polymorph_trap, "polymorph trap", CLR_BRIGHT_GREEN)
    PCHAR( 71, '~',  S_vibrating_square, "vibrating square", CLR_MAGENTA)
    PCHAR( 72, '^',  S_trapped_door, "trapped door", CLR_ORANGE)
    PCHAR( 73, '^',  S_trapped_chest, "trapped chest", CLR_ORANGE)
    /* end traps                                                       */
    /* end cmap B */
    /*                                                                   */
    /* begin special effects                                             */
    /*                                                                   */
    /* zap colors are changed by reset_glyphmap() to match type of beam */
    /*                                                                   */
    PCHAR2(74, '|',  S_vbeam, "vertical beam", "", CLR_GRAY)
    PCHAR2(75, '-',  S_hbeam, "horizontal beam", "", CLR_GRAY)
    PCHAR2(76, '\\', S_lslant, "left slant beam", "", CLR_GRAY)
    PCHAR2(77, '/',  S_rslant, "right slant beam", "", CLR_GRAY)
    /* start cmap C */
    PCHAR2(78, '*',  S_digbeam, "dig beam", "", CLR_WHITE)
    PCHAR2(79, '!',  S_flashbeam, "flash beam", "", CLR_WHITE)
    PCHAR2(80, ')',  S_boomleft, "boom left", "", HI_WOOD)
    PCHAR2(81, '(',  S_boomright, "boom right", "", HI_WOOD)
    /* 4 magic shield symbols                                          */
    PCHAR2(82, '0',  S_ss1, "shield1", "", HI_ZAP)
    PCHAR2(83, '#',  S_ss2, "shield2", "", HI_ZAP)
    PCHAR2(84, '@',  S_ss3, "shield3", "", HI_ZAP)
    PCHAR2(85, '*',  S_ss4, "shield4", "", HI_ZAP)
    PCHAR( 86, '#',  S_poisoncloud, "poison cloud", CLR_BRIGHT_GREEN)
    /* for a time S_goodpos was a question mark, but dollar sign is the
       default keystroke for getpos() to toggle goodpos glyphs on or off */
    PCHAR( 87, '$',  S_goodpos, "valid position", HI_ZAP)
    /* end cmap C */
    /*                                                             */
    /* The 8 swallow symbols.  Do NOT separate.                    */
    /* To change order or add, see the function swallow_to_glyph() */
    /* in display.c. swallow colors are changed by                 */
    /* reset_glyphmap() to match the engulfing monst.              */
    /*                                                             */
    /*  Order:                                                     */
    /*                                                             */
    /*      1 2 3                                                  */
    /*      4 5 6                                                  */
    /*      7 8 9                                                  */
    /*                                                             */
    PCHAR2(88, '/',  S_sw_tl, "swallow top left", "", CLR_GREEN)      /*1*/
    PCHAR2(89, '-',  S_sw_tc, "swallow top center", "", CLR_GREEN)    /*2*/
    PCHAR2(90, '\\', S_sw_tr, "swallow top right", "", CLR_GREEN)     /*3*/
    PCHAR2(91, '|',  S_sw_ml, "swallow middle left", "", CLR_GREEN)   /*4*/
    PCHAR2(92, '|',  S_sw_mr, "swallow middle right", "", CLR_GREEN)  /*6*/
    PCHAR2(93, '\\', S_sw_bl, "swallow bottom left", "", CLR_GREEN)   /*7*/
    PCHAR2(94, '-',  S_sw_bc, "swallow bottom center", "", CLR_GREEN) /*8*/
    PCHAR2(95, '/',  S_sw_br, "swallow bottom right", "", CLR_GREEN)  /*9*/
    /*                                                             */
    /* explosion colors are changed by reset_glyphmap() to match   */
    /* the type of expl.                                           */
    /*                                                             */
    /*    Ex.                                                      */
    /*                                                             */
    /*      /-\                                                    */
    /*      |@|                                                    */
    /*      \-/                                                    */
    /*                                                             */
    PCHAR2(96, '/',  S_expl_tl, "explosion top left", "", CLR_ORANGE)
    PCHAR2(97, '-',  S_expl_tc, "explosion top center", "", CLR_ORANGE)
    PCHAR2(98, '\\', S_expl_tr, "explosion top right", "", CLR_ORANGE)
    PCHAR2(99, '|',  S_expl_ml, "explosion middle left", "", CLR_ORANGE)
    PCHAR2(100, ' ',  S_expl_mc, "explosion middle center", "", CLR_ORANGE)
    PCHAR2(101, '|',  S_expl_mr, "explosion middle right", "", CLR_ORANGE)
    PCHAR2(102, '\\', S_expl_bl, "explosion bottom left", "", CLR_ORANGE)
    PCHAR2(103, '-', S_expl_bc, "explosion bottom center", "", CLR_ORANGE)
    PCHAR2(104, '/', S_expl_br, "explosion bottom right", "", CLR_ORANGE)
#undef PCHAR
#undef PCHAR2
#endif /* PCHAR_S_ENUM || PCHAR_PARSE || PCHAR_DRAWING || PCHAR_TILES
        * || DUMP_ENUMS_PCHAR */

#if defined(MONSYMS_S_ENUM)                     \
    || defined(MONSYMS_DEFCHAR_ENUM)            \
    || defined(MONSYMS_PARSE)                   \
    || defined(MONSYMS_DRAWING)                 \
    || defined(DUMP_ENUMS_MONSYMS)              \
    || defined(DUMP_ENUMS_MONSYMS_DEFCHAR)

/*
    MONSYM(idx, ch, sym desc)
        idx:     index used in enum
        ch:      character symbol
        sym:     symbol name for parsing purposes
        desc:    description
*/

#if defined(MONSYMS_S_ENUM)
/* sym.h */
#define MONSYM(idx, ch, basename, sym, desc) sym = idx,

#elif defined(MONSYMS_DEFCHAR_ENUM)
/* sym.h */
#define MONSYM(idx, ch, basename, sym,  desc) DEF_##basename = ch,

#elif defined(MONSYMS_PARSE)
/* symbols.c */
#define MONSYM(idx, ch, basename, sym, desc) \
    { SYM_MON, sym + SYM_OFF_M, #sym },

#elif defined(MONSYMS_DRAWING)
/* drawing.c */
#define MONSYM(idx, ch, basename, sym, desc) { DEF_##basename, "", desc },

/* allmain.c */
#elif defined(DUMP_ENUMS_MONSYMS)
#define MONSYM(idx, ch, basename, sym, desc) { sym, #sym },

#elif defined(DUMP_ENUMS_MONSYMS_DEFCHAR)
#define MONSYM(idx, ch, basename, sym, desc) \
    { DEF_##basename, "DEF_" #basename },

#endif

    MONSYM( 1, 'a', ANT, S_ANT,   "ant or other insect")
    MONSYM( 2, 'b', BLOB, S_BLOB, "blob")
    MONSYM( 3, 'c', COCKATRICE, S_COCKATRICE, "cockatrice")
    MONSYM( 4, 'd', DOG, S_DOG, "dog or other canine")
    MONSYM( 5, 'e', EYE, S_EYE, "eye or sphere")
    MONSYM( 6, 'f', FELINE, S_FELINE, "cat or other feline")
    MONSYM( 7, 'g', GREMLIN, S_GREMLIN, "gremlin")
    /* small humanoids: hobbit, dwarf */
    MONSYM( 8, 'h', HUMANOID, S_HUMANOID, "humanoid")
    /* minor demons */
    MONSYM( 9, 'i', IMP, S_IMP, "imp or minor demon")
    MONSYM(10, 'j', JELLY, S_JELLY, "jelly")
    MONSYM(11, 'k', KOBOLD, S_KOBOLD, "kobold")
    MONSYM(12, 'l', LEPRECHAUN, S_LEPRECHAUN, "leprechaun")
    MONSYM(13, 'm', MIMIC, S_MIMIC, "mimic")
    MONSYM(14, 'n', NYMPH, S_NYMPH, "nymph")
    MONSYM(15, 'o', ORC, S_ORC, "orc")
    MONSYM(16, 'p', PIERCER, S_PIERCER, "piercer")
    /* quadruped excludes horses */
    MONSYM(17, 'q', QUADRUPED, S_QUADRUPED, "quadruped")
    MONSYM(18, 'r', RODENT, S_RODENT, "rodent")
    MONSYM(19, 's', SPIDER, S_SPIDER, "arachnid or centipede")
    MONSYM(20, 't', TRAPPER, S_TRAPPER, "trapper or lurker above")
    /* unicorn, horses */
    MONSYM(21, 'u', UNICORN, S_UNICORN, "unicorn or horse")
    MONSYM(22, 'v', VORTEX, S_VORTEX, "vortex")
    MONSYM(23, 'w', WORM, S_WORM, "worm")
    MONSYM(24, 'x', XAN, S_XAN, "xan or other mythical/fantastic insect")
    /* yellow light, black light */
    MONSYM(25, 'y', LIGHT, S_LIGHT, "light")
    MONSYM(26, 'z', ZRUTY, S_ZRUTY, "zruty")
    MONSYM(27, 'A', ANGEL, S_ANGEL, "angelic being")
    MONSYM(28, 'B', BAT, S_BAT, "bat or bird")
    MONSYM(29, 'C', CENTAUR, S_CENTAUR, "centaur")
    MONSYM(30, 'D', DRAGON, S_DRAGON, "dragon")
    /* elemental includes invisible stalker */
    MONSYM(31, 'E', ELEMENTAL, S_ELEMENTAL, "elemental")
    MONSYM(32, 'F', FUNGUS, S_FUNGUS, "fungus or mold")
    MONSYM(33, 'G', GNOME, S_GNOME, "gnome")
    /* large humanoid: giant, ettin, minotaur */
    MONSYM(34, 'H', GIANT, S_GIANT, "giant humanoid")
    MONSYM(35, 'I', INVISIBLE, S_invisible, "invisible monster")
    MONSYM(36, 'J', JABBERWOCK, S_JABBERWOCK, "jabberwock")
    MONSYM(37, 'K', KOP, S_KOP, "Keystone Kop")
    MONSYM(38, 'L', LICH, S_LICH, "lich")
    MONSYM(39, 'M', MUMMY, S_MUMMY, "mummy")
    MONSYM(40, 'N', NAGA, S_NAGA, "naga")
    MONSYM(41, 'O', OGRE, S_OGRE, "ogre")
    MONSYM(42, 'P', PUDDING, S_PUDDING, "pudding or ooze")
    MONSYM(43, 'Q', QUANTMECH, S_QUANTMECH, "quantum mechanic")
    MONSYM(44, 'R', RUSTMONST, S_RUSTMONST, "rust monster or disenchanter")
    MONSYM(45, 'S', SNAKE, S_SNAKE, "snake")
    MONSYM(46, 'T', TROLL, S_TROLL, "troll")
    /* umber hulk */
    MONSYM(47, 'U', UMBER, S_UMBER, "umber hulk")
    MONSYM(48, 'V', VAMPIRE, S_VAMPIRE, "vampire")
    MONSYM(49, 'W', WRAITH, S_WRAITH, "wraith")
    MONSYM(50, 'X', XORN, S_XORN, "xorn")
    /* apelike creature includes owlbear, monkey */
    MONSYM(51, 'Y', YETI, S_YETI, "apelike creature")
    MONSYM(52, 'Z', ZOMBIE, S_ZOMBIE, "zombie")
    MONSYM(53, '@', HUMAN, S_HUMAN, "human or elf")
    /* space symbol*/
    MONSYM(54, ' ', GHOST, S_GHOST, "ghost")
    MONSYM(55, '\'', GOLEM, S_GOLEM, "golem")
    MONSYM(56, '&', DEMON, S_DEMON, "major demon")
    /* fish */
    MONSYM(57, ';', EEL, S_EEL,  "sea monster")
    /* reptiles */
    MONSYM(58, ':', LIZARD, S_LIZARD, "lizard")
    MONSYM(59, '~', WORM_TAIL, S_WORM_TAIL, "long worm tail")
    MONSYM(60, ']', MIMIC_DEF, S_MIMIC_DEF, "mimic")

#undef MONSYM
#endif /* MONSYMS_S_ENUM || MONSYMS_DEFCHAR_ENUM || MONSYMS_PARSE
        * || MONSYMS_DRAWING || DUMP_ENUMS_MONSYMS)
        * || DUMP_ENUMS_MONSYMS_DEFCHAR */

#if defined(OBJCLASS_S_ENUM)                    \
    || defined(OBJCLASS_DEFCHAR_ENUM)           \
    || defined(OBJCLASS_CLASS_ENUM)             \
    || defined(OBJCLASS_PARSE)                  \
    || defined(OBJCLASS_DRAWING)                \
    || defined(DUMP_ENUMS_OBJCLASS_DEFCHARS)    \
    || defined(DUMP_ENUMS_OBJCLASS_CLASSES)     \
    || defined(DUMP_ENUMS_OBJCLASS_SYMS)

/*
    OBJCLASS(idx, ch, basename, sym, name, explain)
        idx:      index used in enum
        ch:       default character
        basename: unadorned base name of objclass, used
                  to construct enums through suffixes/prefixes
        sym:      symbol name for enum and parsing purposes
        name:     used in object_detect()
        explain:  used in do_look()

    OBJCLASS2(idx, ch, basename, sname, sym, name, explain)
        idx:      index used in enum
        ch:       default character
        basename: unadorned base name of objclass, used
                  to construct enums through suffixes/prefixes
        sname:    hardcoded *_SYM value for this entry (required
                  only because basename and GOLD_SYM differ
        sym:      symbol name for enum and parsing purposes
        name:     used in object_detect()
        explain:  used in do_look()
*/

#if defined(OBJCLASS_CLASS_ENUM)
/* objclass.h */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    basename##_CLASS = idx,

#elif defined(OBJCLASS_DEFCHAR_ENUM)
/* objclass.h */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    basename##_SYM = ch,

#elif defined(OBJCLASS_S_ENUM)
/* objclass.h */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    sym = idx,

#elif defined(OBJCLASS_PARSE)
/* symbols.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { SYM_OC, sym + SYM_OFF_O, #sym },

#elif defined(OBJCLASS_DRAWING)
/* drawing.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { basename##_SYM, name, explain },

#elif defined(DUMP_ENUMS_OBJCLASS_DEFCHARS)
/* allmain.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { basename##_SYM, #basename "_SYM" },

#elif defined(DUMP_ENUMS_OBJCLASS_CLASSES)
/* allmain.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { basename##_CLASS, #basename "_CLASS" },

#elif defined(DUMP_ENUMS_OBJCLASS_SYMS)
/* allmain.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { sym , #sym },
#endif

/* OBJCLASS with extra arg */
#if defined(OBJCLASS_DEFCHAR_ENUM)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    sname = ch,
#elif defined(OBJCLASS_DRAWING)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { sname, name, explain },
#elif defined(DUMP_ENUMS_OBJCLASS_DEFCHARS)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { sname, #sname },
#elif defined(DUMP_ENUMS_OBJCLASS_CLASSES)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { basename##_CLASS, #basename "_CLASS" },
#elif defined(DUMP_ENUMS_OBJCLASS_SYMS)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { sym , #sym },
#else
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    OBJCLASS(idx, ch, basename, sym, name, explain)
#endif

    OBJCLASS( 1,  ']', ILLOBJ, S_strange_obj, "illegal objects",
                                              "strange object")
    OBJCLASS( 2,  ')', WEAPON, S_weapon, "weapons", "weapon")
    OBJCLASS( 3,  '[', ARMOR,  S_armor, "armor", "suit or piece of armor")
    OBJCLASS( 4,  '=', RING,   S_ring, "rings", "ring")
    OBJCLASS( 5,  '"', AMULET, S_amulet, "amulets", "amulet")
    OBJCLASS( 6,  '(', TOOL,   S_tool, "tools",
                                       "useful item (pick-axe, key, lamp...)")
    OBJCLASS( 7,  '%', FOOD,   S_food, "food", "piece of food")
    OBJCLASS( 8,  '!', POTION, S_potion, "potions", "potion")
    OBJCLASS( 9,  '?', SCROLL, S_scroll, "scrolls", "scroll")
    OBJCLASS(10,  '+', SPBOOK, S_book, "spellbooks", "spellbook")
    OBJCLASS(11,  '/', WAND,   S_wand, "wands", "wand")
    OBJCLASS2(12, '$', COIN,   GOLD_SYM, S_coin, "coins", "pile of coins")
    OBJCLASS(13,  '*', GEM,    S_gem, "rocks", "gem or rock")
    OBJCLASS(14,  '`', ROCK,   S_rock, "large stones", "boulder or statue")
    OBJCLASS(15,  '0', BALL,   S_ball, "iron balls", "iron ball")
    OBJCLASS(16,  '_', CHAIN,  S_chain, "chains", "iron chain")
    OBJCLASS(17,  '.', VENOM,  S_venom, "venoms", "splash of venom")

#undef OBJCLASS
#undef OBJCLASS2
#endif /* OBJCLASS_S_ENUM || OBJCLASS_DEFCHAR_ENUM || OBJCLASS_CLASS_ENUM
        * || OBJCLASS_PARSE || OBJCLASS_DRAWING
        * || DUMP_ENUMS_OBJCLASS_DEFCHARS || DUMP_ENUMS_OBJCLASS_CLASSES
        * || DUMP_ENUMS_OBJCLASS_SYMS */

#ifdef DEBUG
#if !defined(PCHAR_S_ENUM) && !defined(PCHAR_DRAWING) \
    && !defined(PCHAR_PARSE) && !defined(PCHAR_TILES) \
    && !defined(DUMP_ENUMS_PCHAR) \
    && !defined(MONSYMS_S_ENUM) && !defined(MONSYMS_DEFCHAR_ENUM) \
    && !defined(MONSYMS_PARSE) && !defined(MONSYMS_DRAWING) \
    && !defined(DUMP_ENUMS_MONSYMS) \
    && !defined(DUMP_ENUMS_MONSYMS_DEFCHAR) \
    && !defined(OBJCLASS_S_ENUM) && !defined(OBJCLASS_DEFCHAR_ENUM) \
    && !defined(OBJCLASS_CLASS_ENUM) && !defined(OBJCLASS_PARSE) \
    && !defined (OBJCLASS_DRAWING) \
    && !defined(DUMP_ENUMS_OBJCLASS_DEFCHARS) \
    && !defined(DUMP_ENUMS_OBJCLASS_CLASSES) \
    && !defined(DUMP_ENUMS_OBJCLASS_SYMS)
#error Non-productive inclusion of defsym.h
#endif
#endif /* DEBUG */

/* end of defsym.h */
