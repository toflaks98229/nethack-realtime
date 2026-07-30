/* NetHack 5.0	global.h	$NHDT-Date: 1704225560 2024/01/02 19:59:20 $  $NHDT-Branch: keni-luabits2 $:$NHDT-Revision: 1.159 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file global.h
 * @brief What the game assumes about the machine it is built for.
 *
 * Read after config.h, and its job is to turn the choices made there into the concrete facts the rest of the code relies on. A build option says which platform
 * this is; this file works out from that what the integer types are called, what the port is called, what an exit code is, whether a hangup can be handled.
 *
 * Four kinds of thing live here.
 *
 * The data file names come first. They are gathered so that the same name is used by the game and by the tools that build the data, and because a file may be a
 * loose file or a member of an archive without the name changing.
 *
 * Then the types the whole game is written in. Small integers with a known width, the boolean, the coordinate. They are defined here rather than used directly
 * because a platform may need a different underlying type, and because the widths are part of the save format.
 *
 * Then the memory allocation interface. The comment beside it is worth reading: the game does not use the standard allocator directly, because the standard one
 * returns nothing when memory runs out and the game's does not return at all. That single decision is why no caller in the game tests an allocation for failure.
 *
 * Finally the sizes -- how large the map is, how long a message may be, how many rooms a level may hold. The standing warning above them is real: these are not
 * preferences.
 *
 * @note Several definitions are guarded so a platform header may have set them already. Where that is so, this file supplies a default rather than the value.
 * @warning The map dimensions and buffer sizes are depended upon in ways that are not local to their use. The existing comment warns that changing one carelessly
 *          can leave the built game unable to run.
 */

/**
 * @file global.h
 * @brief 게임이 자신이 빌드되는 기계에 대해 전제하는 것.
 *
 * config.h 다음에 읽히며, 그 일은 그곳에서 내려진 선택을 나머지 코드가 의존하는 구체적인 사실로 바꾸는 것이다. 빌드 옵션이 이것이 어느 플랫폼인지 말하고, 이 파일이 그것으로부터 정수 타입이 무엇이라 불리는지, 포트가 무엇이라 불리는지, 종료
 * 코드가 무엇인지, 접속 끊김을 다룰 수 있는지를 알아낸다.
 *
 * 네 종류가 여기에 산다.
 *
 * 먼저 데이터 파일 이름들. 게임과 그 데이터를 만드는 도구가 같은 이름을 쓰도록 모여 있고, 파일이 이름을 바꾸지 않고도 낱개 파일이거나 아카이브 구성원일 수 있기 때문에 모여 있다.
 *
 * 다음으로 게임 전체가 쓰여 있는 타입들. 폭이 정해진 작은 정수, 논리값, 좌표. 직접 쓰이지 않고 여기서 정의되는 것은, 플랫폼이 다른 바탕 타입을 필요로 할 수 있고 그 폭이 저장 형식의 일부이기 때문이다.
 *
 * 다음으로 메모리 할당 인터페이스. 그 곁의 주석은 읽어 볼 가치가 있다. 게임은 표준 할당기를 직접 쓰지 않는다. 표준 것은 메모리가 다하면 아무것도 반환하지 않고, 게임의 것은 아예 반환하지 않기 때문이다. 그 하나의 결정이 게임의 어느 호출자도
 * 할당 실패를 검사하지 않는 이유다.
 *
 * 마지막으로 크기들. 지도가 얼마나 큰지, 메시지가 얼마나 길 수 있는지, 레벨이 몇 개의 방을 담을 수 있는지. 그 위의 상시 경고는 진짜다. 이들은 선호가 아니다.
 *
 * @note 여러 정의가 플랫폼 헤더가 이미 설정했을 수 있도록 보호되어 있다. 그런 곳에서 이 파일은 값이 아니라 기본값을 제공한다.
 * @warning 지도 크기와 버퍼 크기는 그것이 쓰이는 곳에 국지적이지 않은 방식으로 의존된다. 기존 주석은 그 중 하나를 부주의하게 바꾸면 빌드된 게임이 돌아갈 수 없게 될 수 있다고 경고한다.
 */

#ifndef GLOBAL_H
#define GLOBAL_H

/**
 * @name Data file names
 * @brief The files the game reads from its data directory.
 *
 * Named here so the game and the tools that build the data agree, and because a file may be a loose file or a member of an archive without the name changing --
 * the layer that opens them takes a name and decides where to look.
 *
 * @note Most are text the player is shown: the help, the license, the meanings of symbols. A few are lists the game draws randomly from -- rumors, epitaphs,
 *       engravings.
 * @{
 */
/**
 * @name 데이터 파일 이름
 * @brief 게임이 자기 데이터 디렉토리에서 읽는 파일들.
 *
 * 게임과 그 데이터를 만드는 도구가 일치하도록 여기서 이름 붙는다. 그리고 파일이 이름을 바꾸지 않고도 낱개 파일이거나 아카이브 구성원일 수 있기 때문이다. 그것들을 여는 계층이 이름을 받아 어디를 볼지 정한다.
 *
 * @note 대부분은 플레이어에게 보여지는 글이다. 도움말, 라이선스, 심볼의 뜻. 몇 개는 게임이 무작위로 뽑는 목록이다. 소문, 비문, 새김.
 * @{
 */
/*
 * Files expected to exist in the playground directory (possibly inside
 * a dlb container file).
 */

#define RECORD "record"         /* file containing list of topscorers */
#define HELP "help"             /* file containing command descriptions */
#define SHELP "hh"              /* abbreviated form of the same */
#define KEYHELP "keyhelp"       /* explanatory text for 'whatdoes' command */
#define DEBUGHELP "wizhelp"     /* file containing debug mode cmds */
#define RUMORFILE "rumors"      /* file with fortune cookies */
#define ORACLEFILE "oracles"    /* file with oracular information */
#define DATAFILE "data"         /* file giving the meaning of symbols used */
#define CMDHELPFILE "cmdhelp"   /* file telling what commands do */
#define HISTORY "history"       /* file giving nethack's history */
#define LICENSE "license"       /* file with license information */
#define OPTIONFILE "opthelp"    /* file explaining runtime options */
#define OPTMENUHELP "optmenu"   /* file explaining #options command */
#define USAGEHELP "usagehlp"    /* file explaining command line use */
#define OPTIONS_USED "options"  /* compile-time options, for #version */
#define SYMBOLS "symbols"       /* replacement symbol sets */
#define EPITAPHFILE "epitaph"   /* random epitaphs on graves */
#define ENGRAVEFILE "engrave"   /* random engravings on the floor */
#define BOGUSMONFILE "bogusmon" /* hallucinatory monsters */
#define TRIBUTEFILE "tribute"   /* 3.6 tribute to Terry Pratchett */
#define LEV_EXT ".lua"          /* extension for special level files */
/** @} */

/**
 * @name Line padding in random-access text files
 * @brief How long every line in a randomly-sampled file is made, so that sampling is fair.
 *
 * A random line is chosen by jumping to a random byte and taking the next line. That is fast and needs no index, but it is biased: a long line has more bytes to land
 * in and so is chosen more often. Padding every line to one length removes the bias.
 *
 * @note So the padding is not formatting. It is what makes the selection uniform.
 * @note Two values because the two kinds of file have very different line lengths, and padding short lines up to a length chosen for long ones would waste
 *       considerable space.
 * @warning A value of zero disables padding, which saves file size at the cost of biased selection, as the existing comment records. Both sides of that trade are
 *          deliberate choices rather than one being simply better.
 * @warning Used by both the game and the tool that builds the files. They must agree, and nothing checks it -- a mismatch produces selection that is quietly wrong
 *          rather than a failure.
 * @{
 */
/**
 * @name 무작위 접근 텍스트 파일에서의 줄 채우기
 * @brief 무작위로 뽑히는 파일의 모든 줄을 얼마나 길게 만드는지. 뽑기가 공평해지도록.
 *
 * 무작위 줄은 무작위 바이트로 건너뛴 뒤 다음 줄을 취해 골라진다. 그것은 빠르고 색인이 필요하지 않지만 편향된다. 긴 줄은 착지할 바이트가 더 많으므로 더 자주 골라진다. 모든 줄을 한 길이로 채우면 그 편향이 없어진다.
 *
 * @note 그래서 이 채우기는 서식이 아니다. 뽑기를 균일하게 만드는 것이다.
 * @note 두 값인 것은 두 종류의 파일의 줄 길이가 크게 다르고, 짧은 줄을 긴 줄에 맞춘 길이까지 채우면 상당한 공간을 낭비하기 때문이다.
 * @warning 기존 주석이 기록하듯 값 0은 채우기를 끄며, 편향된 뽑기를 대가로 파일 크기를 아낀다. 그 절충의 양쪽 모두 한쪽이 그냥 더 낫다는 것이 아니라 의도적인 선택이다.
 * @warning 게임과 그 파일을 만드는 도구가 함께 쓴다. 둘이 일치해야 하며, 그것을 검사하는 것은 없다. 불일치는 실패가 아니라 조용히 틀린 뽑기를 낸다.
 * @{
 */
/* padding amounts for files that have lines chosen by fseek to random spot,
   advancing to the next line, and using that line; makedefs forces shorter
   lines to be padded to these lengths; value of 0 will inhibit any padding,
   avoiding an increase in files' sizes, but resulting in biased selection;
   used by makedefs while building and by core's callers of get_rnd_text() */
#define MD_PAD_RUMORS 60u /* for RUMORFILE, EPITAPHFILE, and ENGRAVEFILE */
#define MD_PAD_BOGONS 20u /* for BOGUSMONFILE */
/** @} */

/* Assorted definitions that may depend on selections in config.h. */

/*
 * for DUMB preprocessor and compiler, e.g., cpp and pcc supplied
 * with Microport SysV/AT, which have small symbol tables;
 * DUMB if needed is defined in CFLAGS
 */
#ifdef DUMB
#ifdef BITFIELDS
#undef BITFIELDS
#endif
#ifndef STUPID
#define STUPID
#endif
#endif /* DUMB */

/**
 * @name The game's own integer types
 * @brief Named types for the sizes the game stores things in.
 *
 * Defined here rather than used directly for two reasons. A platform may need a different underlying type, and these widths are part of the save format -- so a
 * saved game depends on what these are, and changing one changes what every save file means.
 *
 * @note @c coordxy and @c xint16 are the same width and are separate names on purpose: one is a coordinate and one is not, and keeping them apart makes it visible
 *       which a field is. The existing comment records why coordinates outgrew a single byte -- screen positions passed that limit in practice.
 * @{
 */
/**
 * @name 게임 자체의 정수 타입
 * @brief 게임이 무언가를 저장하는 크기에 붙인 이름들.
 *
 * 직접 쓰지 않고 여기서 정의되는 이유는 둘이다. 플랫폼이 다른 바탕 타입을 필요로 할 수 있고, 이 폭들이 저장 형식의 일부다. 그래서 저장 게임이 이것들이 무엇인지에 의존하며, 그 중 하나를 바꾸면 모든 저장 파일의 뜻이 바뀐다.
 *
 * @note @c coordxy 와 @c xint16 은 같은 폭이며 의도적으로 별개의 이름이다. 하나는 좌표이고 하나는 아니며, 그것들을 떼어 놓으면 어떤 필드가 어느 쪽인지 눈에 보인다. 기존 주석은 좌표가 왜 한 바이트를 넘어섰는지 기록한다. 화면 위치가 실제로 그
 *       한계를 지났다.
 * @{
 */
/*
 * type xint8: small integers (typedef'd as signed,
 * in the range -127 - 127).
 */
typedef int8_t xint8;
/*
 * type coordxy: integers (typedef'd as signed,
 * in the range -32768 to 32767), mostly coordinates.
 * Note that in 2022, screen coordinates easily
 * surpass an upper limit of 127.
 */
typedef int16_t coordxy;
/*
 * type xint16: integers (typedef'd as signed,
 * in the range -32768 to 32767), non-coordinates.
 */
typedef int16_t xint16;
/** @} */

/**
 * @brief The game's boolean type.
 * @note Not the language's own boolean. The game predates it, and this is one byte where the language's is not guaranteed to be -- which matters because booleans are
 *       saved.
 * @warning Its underlying type differs between platforms, as the guards here show, to avoid a clash with a system header. So it is one byte but not always the same
 *          one byte, and its signedness is not something to rely on.
 */
/**
 * @brief 게임의 논리값 타입.
 * @note 언어 자체의 논리값이 아니다. 게임이 그것보다 앞서며, 이것은 한 바이트인데 언어의 것은 그렇다는 보장이 없다. 논리값이 저장되기 때문에 그것이 중요하다.
 * @warning 여기의 보호가 보여 주듯, 시스템 헤더와의 충돌을 피하기 위해 그 바탕 타입이 플랫폼마다 다르다. 그래서 한 바이트이지만 항상 같은 한 바이트는 아니며, 그 부호 여부에 의존해서는 안 된다.
 */
#ifdef __MINGW32__
/* Resolve conflict with Qt 5 and MinGW-w32 */
typedef unsigned char boolean; /* 0 or 1 */
#else
#ifndef SKIP_BOOLEAN
typedef schar boolean; /* 0 or 1 */
#endif
#endif

/**
 * @brief The type holding what has been seen from where.
 * @note One type for two arrays that hold different things, as the existing comment records: one an eight-bit mask and one a five-bit field. They share a type
 *       because they are close enough in size, not because they mean the same thing.
 * @warning So a value of this type is not interpretable without knowing which array it came from.
 */
/**
 * @brief 어디에서 무엇이 보였는지를 담는 타입.
 * @note 기존 주석이 기록하듯 서로 다른 것을 담는 두 배열에 대해 하나의 타입이다. 하나는 8비트 마스크이고 하나는 5비트 필드다. 뜻이 같아서가 아니라 크기가 충분히 비슷해서 타입을 공유한다.
 * @warning 그래서 이 타입의 값은 그것이 어느 배열에서 왔는지 모르고는 해석할 수 없다.
 */
/* vision seen vectors: viz_array[][] and levl[][].seenv, which use different
   values from each other but are close enough in size to share a type;
   viz_array contains 8-bit bitmasks, lev->seenv is a 5-bit bitfield */
typedef unsigned char seenV; /* no need for uint8_t */

/**
 * @brief The type a low-level read expects for a length.
 * @note Differs between platforms in signedness, which is why it is named rather than written -- passing the wrong one produces a warning on some systems and a
 *       conversion on others.
 */
/**
 * @brief 저수준 읽기가 길이에 대해 기대하는 타입.
 * @note 플랫폼마다 부호 여부가 다르며, 그래서 적어 넣지 않고 이름이 붙어 있다. 잘못된 것을 넘기면 어떤 시스템에서는 경고가 나고 어떤 시스템에서는 변환이 일어난다.
 */
/* Type for third parameter of read(2) */
#if defined(BSD) || defined(ULTRIX)
typedef int readLenType;
#else /* e.g. SYSV, __TURBOC__ */
typedef unsigned readLenType;
#endif

/**
 * @name Boolean values
 * @brief True and false, cast to the game's boolean type.
 * @note Guarded because some systems define them already, as the existing comment records. The casts matter: an uncast value assigned to a one-byte boolean is a
 *       conversion, and comparing a boolean field against an uncast literal is not always what it looks like.
 * @{
 */
/**
 * @name 논리값
 * @brief 참과 거짓. 게임의 논리값 타입으로 형변환된 것.
 * @note 기존 주석이 기록하듯 어떤 시스템이 이미 정의하고 있어서 보호되어 있다. 그 형변환이 중요하다. 형변환되지 않은 값을 한 바이트 논리값에 대입하는 것은 변환이며, 논리값 필드를 형변환되지 않은 리터럴과 비교하는 것이 항상 보이는 그대로는 아니다.
 * @{
 */
#ifndef TRUE /* defined in some systems' native include files */
#define TRUE ((boolean) 1)
#define FALSE ((boolean) 0)
#endif
/** @} */

/**
 * @def BOOL_RANDOM
 * @brief A third answer where a boolean is expected: decide at random.
 * @warning Negative, and so not a boolean value at all. It is passed where an option's setting is expected, and code that stores it into a boolean field will lose it
 *          -- it must be resolved to true or false before being stored.
 */
/**
 * @def BOOL_RANDOM
 * @brief 논리값이 기대되는 곳에서의 세 번째 답. 무작위로 정하라.
 * @warning 음수이며, 그래서 논리값이 전혀 아니다. 옵션의 설정이 기대되는 곳에 전달되고, 그것을 논리값 필드에 저장하는 코드는 그것을 잃는다. 저장되기 전에 참이나 거짓으로 결정되어야 한다.
 */
#define BOOL_RANDOM (-1)

/**
 * @brief The type a drawable symbol is held in.
 * @note Named because symbols are loaded from configuration and stored in tables, and the width has to be the same in both -- so it cannot be a plain character in
 *       one place and something wider in another.
 */
/**
 * @brief 그릴 수 있는 심볼을 담는 타입.
 * @note 심볼이 설정에서 적재되고 표에 저장되며 그 폭이 양쪽에서 같아야 하므로 이름이 붙어 있다. 그래서 한곳에서는 평범한 문자이고 다른 곳에서는 더 넓은 것일 수는 없다.
 */
/*
 * type nhsym: loadable symbols go into this type
 */
typedef uchar nhsym;

#ifndef STRNCMPI
/* SAS/C already shifts to stricmp */
#if !defined(__SASC_60) && !defined(CROSS_TO_AMIGA)
#define strcmpi(a, b) strncmpi((a), (b), -1)
#endif
#endif
#ifdef CROSS_TO_AMIGA
#define strcmpi(a, b) stricmp(a, b)
#endif

/* #define SPECIALIZATION */ /* do "specialized" version of new topology */

/**
 * @def Bitfield
 * @brief Declare a member as a bit field, or as a whole byte where bit fields are not usable.
 * @param x the member's name
 * @param n how many bits it needs
 * @warning The two expansions are not equivalent. Where bit fields are unavailable each member becomes a full byte, so a structure declared with these is a different
 *          size on such a platform -- and the width @c n is then not enforced at all, so a value too large for the field is silently stored.
 * @note This is why almost every small field in the game's structures is declared through this rather than directly: the choice has to be made in one place.
 */
/**
 * @def Bitfield
 * @brief 멤버를 비트 필드로 선언한다. 또는 비트 필드를 쓸 수 없는 곳에서는 온전한 한 바이트로.
 * @param x 멤버의 이름
 * @param n 필요한 비트 수
 * @warning 두 확장은 동등하지 않다. 비트 필드를 쓸 수 없는 곳에서는 각 멤버가 온전한 한 바이트가 되므로, 이것으로 선언된 구조체는 그런 플랫폼에서 크기가 다르다. 그리고 그때 폭 @c n 은 전혀 강제되지 않으므로, 그 필드에 너무 큰 값이 조용히
 *          저장된다.
 * @note 게임 구조체의 거의 모든 작은 필드가 직접 선언되지 않고 이것을 통해 선언되는 이유가 그것이다. 그 선택이 한곳에서 내려져야 한다.
 */
#ifdef BITFIELDS
#define Bitfield(x, n) unsigned x : n
#else
#define Bitfield(x, n) uchar x
#endif

/**
 * @def SIZE
 * @brief How many elements an array has.
 * @param x the array
 * @warning Only correct for an array, not for a pointer. Handed a pointer it compiles and yields the ratio of the pointer's size to its target's -- a plausible small
 *          number that means nothing. An array that has decayed to a pointer by being passed as a parameter is the usual way this goes wrong.
 */
/**
 * @def SIZE
 * @brief 배열의 원소가 몇 개인지.
 * @param x 그 배열
 * @warning 배열에 대해서만 올바르고 포인터에 대해서는 그렇지 않다. 포인터를 넘기면 컴파일되고 포인터 크기와 그 대상 크기의 비를 낸다. 그럴듯하지만 아무 뜻이 없는 작은 숫자다. 매개변수로 전달되어 포인터로 붕괴한 배열이 이것이 잘못되는 흔한 경로다.
 */
#define SIZE(x) (int)(sizeof(x) / sizeof(x[0]))

/**
 * @def LARGEST_INT
 * @brief The largest value the game lets certain quantities reach.
 *
 * Deliberately not the machine's own integer limit. The existing comment gives the reason and it is a good one: scores must be comparable between players on different
 * machines, and a limit that varied with the hardware would make them incomparable. So the value is the smallest maximum the language guarantees, and every platform
 * behaves like the most limited one.
 *
 * @warning Not a description of the platform. Using it as one -- assuming an integer cannot exceed it -- is wrong on nearly every machine the game runs on.
 */
/**
 * @def LARGEST_INT
 * @brief 게임이 특정 수량이 이르도록 허용하는 최대값.
 *
 * 의도적으로 기계 자신의 정수 한계가 아니다. 기존 주석이 그 이유를 밝히고 그것은 좋은 이유다. 점수는 다른 기계의 플레이어들 사이에서 비교될 수 있어야 하고, 하드웨어에 따라 달라지는 한계는 그것을 비교할 수 없게 만든다. 그래서 그 값은 언어가
 * 보장하는 가장 작은 최대값이고, 모든 플랫폼이 가장 제한된 것처럼 동작한다.
 *
 * @warning 플랫폼에 대한 기술이 아니다. 그것으로 쓰는 것 -- 정수가 그것을 넘을 수 없다고 가정하는 것 -- 은 게임이 돌아가는 거의 모든 기계에서 틀리다.
 */
#define LARGEST_INT 32767

#include "coord.h"

/* define USING_ADDRESS_SANITIZER if ASAN is in use */
#if defined(__clang__)
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define USING_ADDRESS_SANITIZER
#endif  /* __has_feature */
#endif  /* __has_feature(address_sanitizer) */
#else   /* ?__clang__ */
#if defined(__GNUC__) || defined(_MSC_VER)
#ifdef __SANITIZE_ADDRESS__
#define USING_ADDRESS_SANITIZER
#endif  /* __SANITIZE_ADDRESS__ */
#endif  /* __GNUC__ || _MSC_VER */
#endif  /* !__clang__ */

/*
 * Automatic inclusions for the subsidiary files.
 * Please don't change the order.  It does matter.
 */

#ifdef VMS
#include "vmsconf.h"
#endif

#ifdef UNIX
#include "unixconf.h"
#endif

#ifdef MSDOS
#include "pcconf.h"
#endif

#ifdef WIN32
#include "windconf.h"
#endif

#include "warnings.h"

/* amiconf.h needs to be the last nested #include of config.h because
   'make depend' will turn it into a comment, hiding anything after it */
#ifdef AMIGA
#include "amiconf.h"
#endif

/* Displayable name of this port; don't redefine if defined in *conf.h */
#ifndef PORT_ID
#ifdef AMIGA
#define PORT_ID "Amiga"
#endif
#ifdef MAC68K
#define PORT_ID "Mac"
#endif
#ifdef __APPLE__
#define PORT_ID "MacOS"
#endif
#ifdef MSDOS
#ifdef PC9800
#define PORT_ID "PC-9800"
#else
#define PORT_ID "PC"
#endif
#ifdef DJGPP
#define PORT_SUB_ID "djgpp"
#else
#ifdef OVERLAY
#define PORT_SUB_ID "overlaid"
#else
#define PORT_SUB_ID "non-overlaid"
#endif
#endif
#endif
#ifdef OS2
#define PORT_ID "OS/2"
#endif
#ifdef TOS
#define PORT_ID "ST"
#endif
/* Check again in case something more specific has been defined above. */
#ifndef PORT_ID
#ifdef UNIX
#define PORT_ID "Unix"
#endif
#endif
#ifdef VMS
#define PORT_ID "VMS"
#endif
#ifdef WIN32
#define PORT_ID "Windows"
#endif
#endif

#if !defined(CROSSCOMPILE)
#if defined(MICRO)
#if !defined(AMIGA) && !defined(TOS) && !defined(OS2_HPFS)
#define SHORT_FILENAMES /* filenames are 8.3 */
#endif
#endif
#endif

#include "fnamesiz.h" /* file sizes shared between nethack and recover */

#ifdef VMS
/* vms_exit() (sys/vms/vmsmisc.c) expects the non-VMS EXIT_xxx values below.
 * these definitions allow all systems to be treated uniformly, provided
 * main() routines do not terminate with return(), whose value is not
 * so massaged.
 */
#ifdef EXIT_SUCCESS
#undef EXIT_SUCCESS
#endif
#ifdef EXIT_FAILURE
#undef EXIT_FAILURE
#endif
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#if defined(X11_GRAPHICS) || defined(QT_GRAPHICS) || defined(GNOME_GRAPHICS) \
    || defined(MSWIN_GRAPHICS)
#ifndef NO_TILE_C
#ifndef TILES_IN_GLYPHMAP
#define TILES_IN_GLYPHMAP
#endif
#endif
#endif
#if defined(AMII_GRAPHICS) || defined(GEM_GRAPHICS)
#ifndef NO_TILE_C
#ifndef TILES_IN_GLYPHMAP
#define TILES_IN_GLYPHMAP
#endif
#endif
#endif

#if defined(UNIX) || defined(VMS) || defined(__EMX__) || defined(WIN32)
#define HANGUPHANDLING
#endif
#if defined(SAFERHANGUP) \
    && (defined(NOSAVEONHANGUP) || !defined(HANGUPHANDLING))
#undef SAFERHANGUP
#endif

/**
 * @name String operations, with the return value discarded
 * @brief The standard string routines, wrapped so their unused result is discarded deliberately.
 *
 * These routines return a value nobody wants, and a compiler configured strictly says so at every call. Discarding it explicitly once here silences that everywhere,
 * and does it as a statement of intent rather than by turning the warning off.
 *
 * @note @c Strlen is different from the others -- it is not a discard but a call to the game's own routine, which is given the caller's location so that a length
 *       computed on something unexpected can be traced.
 * @warning These are the plain routines with no bounds checking, not safer versions. Wrapping them does nothing about buffer overruns; the fixed buffer sizes above are
 *          what guards against those.
 * @{
 */
/**
 * @name 반환값을 버리는 문자열 연산
 * @brief 표준 문자열 루틴들. 쓰이지 않는 결과를 의도적으로 버리도록 감싼 것.
 *
 * 이 루틴들은 아무도 원하지 않는 값을 반환하고, 엄격하게 설정된 컴파일러는 모든 호출 지점에서 그것을 지적한다. 여기서 한 번 명시적으로 버리면 그것이 어디서나 잠잠해지며, 경고를 끄는 방식이 아니라 의도의 선언으로 그렇게 한다.
 *
 * @note @c Strlen 은 나머지와 다르다. 버림이 아니라 게임 자체 루틴에 대한 호출이며, 그 루틴에 호출자의 위치가 주어진다. 그래서 예상치 못한 것에 대해 계산된 길이를 추적할 수 있다.
 * @warning 이들은 더 안전한 판본이 아니라 경계 검사가 없는 평범한 루틴이다. 감싸는 것은 버퍼 넘침에 대해 아무것도 하지 않는다. 위의 고정 버퍼 크기들이 그것을 막는 것이다.
 * @{
 */
#define Sprintf (void) sprintf
#define Strcat (void) strcat
#define Strcpy (void) strcpy
#define Strlen(s) Strlen_(s,__func__,__LINE__)
#ifdef NEED_VARARGS
#define Vprintf (void) vprintf
#define Vfprintf (void) vfprintf
#define Vsprintf (void) vsprintf
#endif
/** @} */

/**
 * @note The comment below is the most consequential in this file. The game does not use the standard allocator directly: the standard one returns nothing when memory
 *       runs out and the game's does not return at all, ending the game instead. That single decision is why no caller anywhere in the game tests an allocation for
 *       failure -- and it is why introducing a direct use of the standard allocator would introduce a null pointer the surrounding code is not written to expect.
 */
/**
 * @note 아래의 주석이 이 파일에서 가장 결과가 큰 것이다. 게임은 표준 할당기를 직접 쓰지 않는다. 표준 것은 메모리가 다하면 아무것도 반환하고, 게임의 것은 아예 반환하지 않고 대신 게임을 끝낸다. 그 하나의 결정이 게임의 어디에서도 호출자가 할당 실패를
 *       검사하지 않는 이유이며, 표준 할당기를 직접 쓰는 곳을 만들면 둘레의 코드가 기대하도록 쓰여 있지 않은 널 포인터가 생기는 이유다.
 */
/*
 *  Memory allocation.  Functions are declared here rather than in
 *  extern.h so that source files which use config.h instead of hack.h
 *  will see the declarations.
 *
 *  NetHack does not use malloc() [except to implement alloc() in alloc.c]
 *  or realloc() or calloc().  They return Null if memory runs out and
 *  nethack's code relies on alloc() to intercept that so that a zillion
 *  callers don't need to test for Null result.  alloc() never returns
 *  Null; if memory runs out, it calls panic() and does not return at all.
 */

/* dupstr() is unconditional in alloc.c but not used when MONITOR_HEAP
   is enabled; some utility programs link with alloc.{o,obj} and need it
   if nethack is built with MONITOR_HEAP enabled and they aren't; this
   declaration has been moved out of the '#else' below to avoid getting
   a complaint from -Wmissing-prototypes when building with MONITOR_HEAP */
extern char *dupstr(const char *) NONNULL NONNULLARG1;
/* same, but return strlen(string) in extra argument */
extern char *dupstr_n(const char *string,
                      unsigned *lenout) NONNULL NONNULLPTRS;

/*
 * MONITOR_HEAP is conditionally used for primitive memory leak debugging.
 * When enabled, NH_HEAPLOG (if defined in the environment) is used as the
 * name of a log file to create for capturing allocations and releases.
 * [The 'heaputil' program to analyze that file isn't included in releases.]
 *
 * See alloc.c.
 */
#ifdef MONITOR_HEAP
/* plain alloc() is not declared except in alloc.c */
extern long *nhalloc(unsigned int, const char *, int) NONNULL NONNULLARG2;
extern long *nhrealloc(long *, unsigned int, const char *,
                       int) NONNULL NONNULLARG3;
extern void nhfree(genericptr_t, const char *, int) NONNULLARG2;
extern char *nhdupstr(const char *, const char *, int) NONNULL NONNULLPTRS;
/* this predates C99's __func__; that is trickier to use conditionally
   because it is not implemented as a preprocessor macro; MONITOR_HEAP
   wouldn't gain much benefit from it anyway so continue to live without it;
   if func's caller were accessible, that would be a very different issue */
#define alloc(a) nhalloc(a, __FILE__, (int) __LINE__)
#define re_alloc(a,n) nhrealloc(a, n, __FILE__, (int) __LINE__)
#define free(a) nhfree(a, __FILE__, (int) __LINE__)
#define dupstr(s) nhdupstr(s, __FILE__, (int) __LINE__)
#else /* !MONITOR_HEAP */
/* declare alloc.c's alloc(); allocations made with it use ordinary free() */
extern long *alloc(unsigned int) NONNULL;  /* alloc.c */
extern long *re_alloc(long *, unsigned int) NONNULL;
#endif /* ?MONITOR_HEAP */

/**
 * @brief What a data file must agree with the game about.
 *
 * Three numbers checked when a data file is opened. The version alone is not enough: two builds of the same version may differ in which options were compiled in, and
 * either may have a different number of monsters and objects -- and a data file built against one is meaningless to the other. So all three are compared.
 *
 * @note Declared here rather than with the game's other structures so the tools that build the data files can see it, as the existing comment records. They include the
 *       configuration but not the game.
 */
/**
 * @brief 데이터 파일이 게임과 일치해야 하는 것.
 *
 * 데이터 파일을 열 때 검사되는 세 숫자. 버전만으로는 충분하지 않다. 같은 버전의 두 빌드가 어떤 옵션이 컴파일에 포함되었는지에서 다를 수 있고, 어느 쪽이든 몬스터와 물건의 수가 다를 수 있다. 그리고 한쪽에 맞춰 만든 데이터 파일은 다른 쪽에게 무의미하다.
 * 그래서 셋 모두가 비교된다.
 *
 * @note 기존 주석이 기록하듯 게임의 다른 구조체들과 함께가 아니라 여기에 선언되어 있어, 데이터 파일을 만드는 도구가 그것을 볼 수 있다. 그 도구들은 설정은 포함하지만 게임은 포함하지 않는다.
 */
/* Used for consistency checks of various data files; declare it here so
   that utility programs which include config.h but not hack.h can see it. */
struct version_info {
    unsigned long incarnation;   /* actual version number */
    unsigned long feature_set;   /* bitmask of config settings */
    unsigned long entity_count;  /* # of monsters and objects */
};

/**
 * @brief Facts about how this copy was built.
 *
 * The build date, the source revision, the version as text and as a number. Gathered here because the game reports them, and because they must be the same numbers the
 * data files were checked against.
 *
 * @note The existing comment records that these used to be produced by a build tool into a generated header. Holding them in a structure instead means a change of
 *       revision does not force a rebuild of everything that mentions the version.
 */
/**
 * @brief 이 사본이 어떻게 빌드되었는지에 대한 사실들.
 *
 * 빌드 날짜, 소스 리비전, 글로서의 버전과 숫자로서의 버전. 게임이 그것을 보고하기 때문에, 그리고 그것이 데이터 파일이 검사받은 것과 같은 숫자여야 하기 때문에 여기에 모여 있다.
 *
 * @note 기존 주석은 이것들이 예전에는 빌드 도구가 생성 헤더로 만들어 냈다고 기록한다. 대신 구조체에 담아 두면 리비전이 바뀔 때 버전을 언급하는 모든 것이 다시 빌드되지 않는다.
 */
/* This is used to store some build-info data that used
   to be present in makedefs-generated header file date.h */

struct nomakedefs_s {
    const char *build_date;
    const char *copyright_banner_c;
    const char *git_sha;
    const char *git_branch;
    const char *git_prefix;
    const char *version_string;
    const char *version_id;
    unsigned long version_number;
    unsigned long version_features;
    unsigned long ignored_features;
    unsigned long version_sanity1;
    unsigned long build_time;
};
extern struct nomakedefs_s nomakedefs;

/*
 * Configurable internal parameters.
 *
 * Please be very careful if you are going to change one of these.  Any
 * changes in these parameters, unless properly done, can render the
 * executable inoperative.
 */

/**
 * @name Map dimensions
 * @brief How large a dungeon level is.
 *
 * Fixed rather than adjustable, and not because nobody thought of it: the size is assumed by the level descriptions, by the save format, by the vision calculations, and
 * by the display. Changing it means changing all of those.
 *
 * @note The screen must be a few lines taller than the map, as the existing comment notes, to leave room for the message line and the status lines.
 * @warning Both are part of the save format. A save from a build with different dimensions cannot be read.
 * @{
 */
/**
 * @name 지도 크기
 * @brief 던전 레벨 하나가 얼마나 큰지.
 *
 * 조절할 수 있는 것이 아니라 고정되어 있으며, 아무도 그것을 생각해 보지 않아서가 아니다. 그 크기는 레벨 기술, 저장 형식, 시야 계산, 표시부가 전제한다. 그것을 바꾸는 것은 그 전부를 바꾸는 일이다.
 *
 * @note 기존 주석이 밝히듯 화면은 지도보다 몇 줄 더 높아야 한다. 메시지 줄과 상태줄을 위한 자리를 남겨야 한다.
 * @warning 둘 다 저장 형식의 일부다. 크기가 다른 빌드의 저장은 읽을 수 없다.
 * @{
 */
/* size of terminal screen is (at least) (ROWNO+3) by COLNO */
#define COLNO 80
#define ROWNO 21
/** @} */

/**
 * @name Level structure limits
 * @brief How many rooms and doors a level may contain.
 * @note The door figure is an increment rather than a total, as its comment says: doors are allocated in batches of this size as more are needed, so it is a growth step
 *       and not a limit.
 * @warning The two room figures are limits, and they size fixed arrays. A level description asking for more rooms than fit does not grow the array.
 * @{
 */
/**
 * @name 레벨 구조 한계
 * @brief 레벨이 담을 수 있는 방과 문의 수.
 * @note 문 수치는 그 주석이 말하듯 총합이 아니라 증분이다. 더 필요해지면 문이 이 크기의 묶음으로 할당되므로, 한계가 아니라 성장 단위다.
 * @warning 두 방 수치는 한계이며 고정 배열의 크기를 정한다. 들어갈 수 있는 것보다 많은 방을 요청하는 레벨 기술이 그 배열을 늘리지는 않는다.
 * @{
 */
#define MAXNROFROOMS 40 /* max number of rooms per level */
#define MAX_SUBROOMS 24 /* max # of subrooms in a given room */
#define DOORINC      20 /* number of doors per level, increment */
/** @} */

/**
 * @name Text buffer sizes
 * @brief How long the game's various pieces of text may be.
 *
 * Three sizes because the three uses have genuinely different needs, and the largest is the interesting one. Its comment records the reasoning: the longest message the
 * game can produce names three monsters at full length plus the longest prefix plus a few words, and the size is that sum rather than a round number.
 *
 * @warning These are the sizes of fixed buffers and the game is written to them throughout. A string longer than its buffer is the classic hazard here, and the sizes
 *          are chosen so it cannot happen rather than checked for at each use.
 * @{
 */
/**
 * @name 텍스트 버퍼 크기
 * @brief 게임의 여러 글 조각이 얼마나 길 수 있는지.
 *
 * 세 가지 크기인 것은 세 용도의 필요가 실제로 다르기 때문이며, 가장 큰 것이 흥미로운 것이다. 그 주석이 그 계산을 기록한다. 게임이 만들 수 있는 가장 긴 메시지는 몬스터 셋을 온전한 길이로 지칭하고 거기에 가장 긴 접두어와 몇 낱말이 더해진 것이며, 그
 * 크기는 어림수가 아니라 그 합이다.
 *
 * @warning 이들은 고정 버퍼의 크기이고 게임 전반이 그것에 맞춰 쓰여 있다. 자기 버퍼보다 긴 문자열이 여기서의 고전적인 위험이며, 그 크기들은 쓰일 때마다 검사되는 대신 그런 일이 일어날 수 없도록 골라져 있다.
 * @{
 */
#define BUFSZ 256  /* for getlin buffers */
#define QBUFSZ 128 /* for building question text */
#define TBUFSZ 300 /* gt.toplines[] buffer max msg: 3 81-char names
                    * plus longest prefix plus a few extra words */
/** @} */

/**
 * @def COLBUFSZ
 * @brief A buffer big enough for either a line of text or a full map row.
 * @note Worked out rather than written, so that whichever of the two is larger is the one used. That matters because the same buffer is used for both.
 */
/**
 * @def COLBUFSZ
 * @brief 텍스트 한 줄이나 지도 한 행 중 어느 쪽이든 담을 만큼 큰 버퍼.
 * @note 적어 넣지 않고 계산되므로, 둘 중 더 큰 쪽이 쓰인다. 같은 버퍼가 둘 다에 쓰이기 때문에 그것이 중요하다.
 */
/* COLBUFSZ is the larger of BUFSZ and COLNO */
#if BUFSZ > COLNO
#define COLBUFSZ BUFSZ
#else
#define COLBUFSZ COLNO
#endif

#define PL_NSIZ 32 /* name of player, ghost, shopkeeper */
#define PL_CSIZ 32 /* sizeof pl_character */
#define PL_FSIZ 32 /* fruit name */
#define PL_PSIZ 63 /* player-given names for pets, other monsters, objects */
/* room for "name-role-race-gend-algn" plus 1 character playmode code */
#define PL_NSIZ_PLUS (PL_NSIZ + 4 * (1 + 3) + 1) /* 49 */

#define MAXDUNGEON 16 /* current maximum number of dungeons */
#define MAXLEVEL 32   /* max number of levels in one dungeon */
#define MAXSTAIRS 1   /* max # of special stairways in a dungeon */
#define ALIGNWEIGHT 4 /* generation weight of alignment */

#define MAXULEV 30 /* max character experience level */

#define MAXMONNO 120 /* extinct monst after this number created */
#define MHPMAX 500   /* maximum monster hp */

#ifndef MAX_MSG_HISTORY
#define MAX_MSG_HISTORY 128 /* max # of lines in msg_history */
#endif

#include "color.h"

/*
 * Version 5.0.x has aspirations of portable file formats. We
 * make a distinction between MAIL functionality and MAIL_STRUCTURES
 * so that the underlying structures are consistent, whether MAIL is
 * defined or not.
 */
#define MAIL_STRUCTURES

/* PANICTRACE: Always defined for NH_DEVEL_STATUS != NH_STATUS_RELEASED
   but only for supported platforms. */
#ifndef NOPANICTRACE
#ifdef UNIX
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
/* see end.c */
#if !defined(CROSS_TO_WASM) && !defined(CROSS_TO_MSDOS)
#ifndef PANICTRACE
#define PANICTRACE
#endif  /* PANICTRACE */
#endif  /* CROSS_TO_WASM |  CROSS_TO_MSDOS */
#endif  /* NH_DEVEL_STATUS != NH_STATUS_RELEASED */
#endif  /* UNIX */
#endif  /* !NOPANICTRACE */

/* The following are meaningless if PANICTRACE is not defined: */
#if defined(__linux__) && defined(__GLIBC__) && (__GLIBC__ >= 2)
#define PANICTRACE_LIBC
#endif
#if defined(MACOS)
#define PANICTRACE_LIBC
#endif
#ifdef UNIX
#if !defined(CROSS_TO_WASM) /* no popen in WASM */
#define PANICTRACE_GDB
#endif
#ifdef CROSS_TO_WASM
#undef COMPRESS
#endif
#endif

/* Supply nethack_enter macro if not supplied by port */
#ifndef nethack_enter
#define nethack_enter(argc, argv) ((void) 0)
#endif

/* Supply nhassert macro if not supplied by port */
#ifndef nhassert
#define nhassert(expression) (void)((!!(expression)) || \
        (nhassert_failed(#expression, __FILE__, __LINE__), 0))
#endif

/* Macros for meta and ctrl modifiers:
 *   M and C return the meta/ctrl code for the given character;
 *     e.g., (C('c') is ctrl-c
 */
#ifndef M
#ifndef NHSTDC
#define M(c) (0x80 | (c))
#else
#define M(c) ((c) - 128)
#endif /* NHSTDC */
#endif

#ifndef C
#define C(c) (0x1f & (c))
#endif

#define unmeta(c) (0x7f & (c))

/* Game log message type flags */
#define LL_NONE       0x0000L /* No message is livelogged */
#define LL_WISH       0x0001L /* Report stuff people type at the wish prompt */
#define LL_ACHIEVE    0x0002L /* Achievements bitfield + invocation, planes */
#define LL_UMONST     0x0004L /* defeated unique monster */
#define LL_DIVINEGIFT 0x0008L /* Sacrifice gifts, crowning */
#define LL_LIFESAVE   0x0010L /* Use up amulet of lifesaving */
#define LL_CONDUCT    0x0020L /* Break conduct - not reported early-game */
#define LL_ARTIFACT   0x0040L /* bestowed, found, or manufactured */
#define LL_GENOCIDE   0x0080L /* Logging of genocides */
#define LL_KILLEDPET  0x0100L /* Killed a tame monster */
#define LL_ALIGNMENT  0x0200L /* changed alignment, temporary or permanent */
#define LL_DUMP_ASC   0x0400L /* Log URL for dumplog if ascended */
#define LL_DUMP_ALL   0x0800L /* Log dumplog url for all games */
#define LL_MINORAC    0x1000L /* Log 'minor' achievements - can be spammy */
#define LL_SPOILER    0x2000L /* reveals information so don't show in-game
                               * via #chronicle unless in wizard mode */
#define LL_DUMP       0x4000L /* none of the above but should be in dumplog */
#define LL_DEBUG      0x8000L /* For debugging messages and other spam */

/*
 * Lua sandbox
 */
/* Control block for setting up a Lua state with nhl_init(). */
typedef struct nhl_sandbox_info {
    uint32_t  flags;       /* see below */
    uint32_t  memlimit;    /* approximate memory limit */
    uint32_t  steps;       /* instruction limit for state OR ... */
    uint32_t  perpcall;    /* ... instruction limit per nhl_pcall */
} nhl_sandbox_info;

/* For efficiency, we only check every NHL_SB_STEPSIZE instructions. */
#ifndef NHL_SB_STEPSIZE
#define NHL_SB_STEPSIZE 1000
#endif

/* High level groups.  Use these flags. */
    /* Safe functions. */
#define NHL_SB_SAFE        0x80000000
    /* Access to Lua version information. */
#define NHL_SB_VERSION     0x40000000
    /* Debugging library - mostly unsafe. */
#define NHL_SB_DEBUGGING   0x08000000

/* Low level groups.  If you need these, you probably need to define
 * a new high level group instead. */
#define NHL_SB_STRING      0x00000001
#define NHL_SB_TABLE       0x00000002
#define NHL_SB_COROUTINE   0x00000004
#define NHL_SB_MATH        0x00000008
#define NHL_SB_UTF8        0x00000010
#ifdef notyet
#define NHL_SB_IO          0x00000020
#endif
#define NHL_SB_OS          0x00000040

#define NHL_SB_BASEMASK    0x00000f80
#define NHL_SB_BASE_BASE   0x00000080
#define NHL_SB_BASE_ERROR  0x00000100
#define NHL_SB_BASE_META   0x00000200
#define NHL_SB_BASE_GC     0x00000400
#define NHL_SB_BASE_UNSAFE 0x00000800

#define NHL_SB_DBMASK      0x00003000
#define NHL_SB_DB_DB       0x00001000
#define NHL_SB_DB_SAFE     0x00002000

#define NHL_SB_OSMASK      0x0000c000
#define NHL_SB_OS_TIME     0x00004000
#define NHL_SB_OS_FILES    0x00008000

#define NHL_SB_ALL         0x0000ffff

/* return codes */
#define NHL_SBRV_DENY 1
#define NHL_SBRV_ACCEPT 2
#define NHL_SBRV_FAIL 3

/* NHL_pcall_handle action values */
typedef enum NHL_pcall_action {
    NHLpa_panic,
    NHLpa_impossible
} NHL_pcall_action;

/**
 * @brief Whether a listing is of what to include or what to leave out.
 * @note Some option lists are more compactly written as exclusions, so an option's value may be a set plus which sense it is meant in.
 */
/**
 * @brief 목록이 포함할 것에 대한 것인지 빼 둘 것에 대한 것인지.
 * @note 어떤 옵션 목록은 배제로 적는 것이 더 간결하다. 그래서 옵션의 값이 묶음과 그것이 어느 의미로 쓰였는지를 함께 가질 수 있다.
 */
enum optchoice { opt_in, opt_out};

/**
 * @brief Who may set an option and when.
 *
 * Not a permission scale but a list of distinct situations, and the differences matter. Some options can only be set before the game starts because changing them later
 * would be incoherent. Some may be changed by an external program but not from inside the game. Some are for debugging and are refused outside debug mode -- and one of
 * those is additionally refused to automated testing, since a test that changed it would invalidate itself.
 *
 * @note The last is not a restriction at all but a marker for entries that exist to hold a name prefix and are never shown.
 * @warning The accompanying validity test brackets the real values, so it depends on the ordering: adding a restriction outside that bracket makes it silently invalid.
 */
/**
 * @brief 누가 언제 옵션을 설정할 수 있는지.
 *
 * 권한의 척도가 아니라 서로 구별되는 상황의 목록이며, 그 차이들이 중요하다. 어떤 옵션은 게임이 시작하기 전에만 설정될 수 있다. 나중에 바꾸면 앞뒤가 맞지 않기 때문이다. 어떤 것은 외부 프로그램으로는 바꿀 수 있으나 게임 안에서는 그럴 수 없다. 어떤 것은
 * 디버깅용이고 디버그 모드 밖에서는 거절된다. 그리고 그 중 하나는 자동 시험에게도 추가로 거절된다. 그것을 바꾸는 시험은 자기 자신을 무효로 만들기 때문이다.
 *
 * @note 마지막 것은 제약이 전혀 아니라 이름 접두어를 담기 위해 존재하고 결코 보여지지 않는 항목을 위한 표시다.
 * @warning 딸린 유효성 검사가 실제 값들을 괄호처럼 감싸므로 그 순서에 의존한다. 그 괄호 밖에 제약을 더하면 그것이 조용히 무효가 된다.
 */
/*
 * option setting restrictions
 */
enum optset_restrictions {
    set_in_sysconf = 0, /* system config file option only */
    set_in_config = 1,  /* config file option only */
    set_viaprog = 2,    /* may be set via extern program, not seen in game */
    set_gameview = 3,   /* may be set via extern program, displayed in game */
    set_in_game = 4,    /* may be set via extern program or set in the game */
    set_wizonly = 5,    /* may be set in the game if wizmode */
    set_wiznofuz = 6,   /* wizard-mode only, but not by fuzzer */
    set_hidden = 7      /* placeholder for prefixed entries, never show it  */
};

/**
 * @brief Where an option's current value came from.
 *
 * An option may be set in five or six different places, and the later one wins. What is recorded is not the value but its provenance, and that is what makes overriding
 * work: a value from a configuration file may be replaced by one from the command line, but not the other way round.
 *
 * @note The order is the precedence and is therefore not arbitrary. Reading them in order also reads as the sequence in which they are applied.
 * @warning Despite the similar shape, these are not the setting restrictions above. The existing comment says so plainly, and the two enumerations have overlapping
 *          values -- so one used in place of the other compiles and means something else.
 */
/**
 * @brief 옵션의 현재 값이 어디서 왔는지.
 *
 * 옵션은 다섯이나 여섯 군데에서 설정될 수 있고, 나중의 것이 이긴다. 기록되는 것은 값이 아니라 그 출처이며, 그것이 덮어쓰기가 작동하게 하는 것이다. 설정 파일에서 온 값은 명령줄에서 온 값으로 대체될 수 있지만 그 반대는 아니다.
 *
 * @note 그 순서가 우선순위이므로 임의적이지 않다. 순서대로 읽으면 그것들이 적용되는 순서로도 읽힌다.
 * @warning 모양이 비슷하지만 이들은 위의 설정 제약이 아니다. 기존 주석이 그것을 분명히 말하며, 두 열거는 값이 겹친다. 그래서 하나를 다른 하나 자리에 쓰면 컴파일되고 다른 것을 뜻한다.
 */
/* these aren't the same as set_xxx */
enum option_phases {
    phase_not_set = 0,
    builtin_opt = 1, /* compiled-in default value of an option */
    syscf_opt,       /* sysconf setting of an option, overrides builtin */
    rc_file_opt, /* player's run-time config file setting, overrides syscf */
    environ_opt, /* player's environment NETHACKOPTIONS, overrides rc_file */
    cmdline_opt, /* program invocation command-line, overrides environ */
    play_opt,    /* 'O' command, interactively set so overrides all */
    num_opt_phases
};

/**
 * @def SET__IS_VALUE_VALID
 * @brief Whether a restriction value is one of the real ones.
 * @param s the restriction value
 * @warning Yields true for an @e invalid value, despite the name. It is a range check written as its own negation, so a caller reading it as "is valid" has the sense
 *          backwards.
 * @warning Depends on the restriction enumeration's ordering, since it brackets the range rather than listing the values.
 */
/**
 * @def SET__IS_VALUE_VALID
 * @brief 제약 값이 실제 값 중 하나인지.
 * @param s 그 제약 값
 * @warning 이름과 달리 @e 유효하지 않은 값에 대해 참을 낸다. 자기 자신의 부정으로 쓰인 범위 검사이므로, 그것을 "유효한가"로 읽는 호출자는 의미를 거꾸로 쥐고 있는 것이다.
 * @warning 값들을 나열하는 대신 범위를 감싸므로 제약 열거의 순서에 의존한다.
 */
#define SET__IS_VALUE_VALID(s) ((s < set_in_sysconf) || (s > set_wiznofuz))
#include "optlist.h"
/**
 * @brief Every option the game has, generated from the option list.
 * @note Read from the same file that defines the options themselves, so an option's name, its enumerator and its handling come from one description and cannot disagree.
 * @note @c opt_prefix_only is negative because it is not an option but a marker for a name that is only a prefix of others.
 */
/**
 * @brief 게임이 가진 모든 옵션. 옵션 목록에서 생성된다.
 * @note 옵션 자체를 정의하는 것과 같은 파일에서 읽어 오므로, 옵션의 이름, 그 열거자, 그 처리가 하나의 기술에서 나오고 서로 어긋날 수 없다.
 * @note @c opt_prefix_only 가 음수인 것은 그것이 옵션이 아니라 다른 이름들의 접두어일 뿐인 이름을 위한 표시이기 때문이다.
 */
enum opt {
    opt_prefix_only = -1,
#define NHOPT_ENUM
#include "optlist.h"
#undef NHOPT_ENUM
    OPTCOUNT
};

/**
 * @def SFCTOOL_BIT
 * @brief A high bit marking a value as coming from the save-format tool rather than the game.
 * @note Placed high so it cannot collide with any real value it accompanies, which is what lets one field carry both.
 */
/**
 * @def SFCTOOL_BIT
 * @brief 어떤 값이 게임이 아니라 저장 형식 도구에서 왔음을 표시하는 상위 비트.
 * @note 높은 곳에 놓여 있어 자신이 딸린 실제 값과 충돌할 수 없다. 그것이 하나의 필드가 둘 다를 지닐 수 있게 하는 것이다.
 */
#define SFCTOOL_BIT (1UL << 30)

#endif /* GLOBAL_H */
