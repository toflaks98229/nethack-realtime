/* NetHack 5.0	fnamesiz.h	$NHDT-Date: 1781973080 2026/06/20 16:31:20 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.5 $ */
/*-Copyright (c) Michael Allison, 2020. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file fnamesiz.h
 * @brief How long the game's file names can be, agreed between two programs.
 *
 * The recover tool has to find and read the same lock and bones files the game
 * writes, so both must size their buffers identically. Keeping the sizes in one
 * header is what stops the two from drifting apart -- as the existing comment
 * says, the point is less chance of deviation.
 *
 * The values differ per platform because the naming conventions do: a VMS name
 * carries a version, a Windows lock name carries the user name.
 *
 * @note Several sizes are derived from a sample name via @c sizeof, so the
 *       template and the size cannot disagree.
 * @warning Changing a name's shape here changes it for both programs. A game and
 *          a recover built from different values would not find each other's
 *          files.
 */

/**
 * @file fnamesiz.h
 * @brief 게임의 파일 이름 길이. 두 프로그램이 합의하는 값.
 *
 * recover 도구는 게임이 쓰는 잠금 파일과 본즈 파일을 찾아 읽어야 하므로, 양쪽이 버퍼
 * 크기를 똑같이 잡아야 한다. 크기를 한 헤더에 두는 것이 둘이 어긋나지 않게 하는 방법이며,
 * 기존 주석이 말하듯 요점은 편차 가능성을 줄이는 것이다.
 *
 * 값이 플랫폼마다 다른 것은 이름 규칙이 다르기 때문이다. VMS 이름에는 버전이 붙고,
 * 윈도우 잠금 이름에는 사용자 이름이 붙는다.
 *
 * @note 여러 크기가 표본 이름에서 @c sizeof 로 유도된다. 그래서 서식과 크기가 서로
 *       어긋날 수 없다.
 * @warning 여기서 이름의 형태를 바꾸면 두 프로그램 모두에 대해 바뀐다. 서로 다른 값으로
 *          빌드된 게임과 recover 는 상대의 파일을 찾지 못한다.
 */

#ifndef FNAMESIZ_H
#define FNAMESIZ_H

/*
 *  Declarations for some file name sizes, some of which need to be shared
 *  between NetHack and recover. By placing them in here, there is less
 *  chance of deviation between the two.
 */

#if !defined(MICRO) && !defined(VMS) && !defined(WIN32)
#define LOCKNAMESIZE (PL_NSIZ + 14) /* long enough for uid+name+.99 */
#define LOCKNAMEINIT "1lock"
#define BONESINIT "bonesnn.xxx.le"
#define BONESSIZE sizeof(BONESINIT)
#else
#if defined(MICRO)
#define LOCKNAMESIZE FILENAME
#define LOCKNAMEINIT ""
#define BONESINIT ""
#define BONESSIZE FILENAME
#endif
#if defined(VMS)
#define LOCKNAMESIZE (PL_NSIZ + 17) /* long enough for _uid+name+.99;1 */
#define LOCKNAMEINIT "1lock"
#define BONESINIT "bonesnn.xxx_le;1"
#define BONESSIZE sizeof(BONESINIT)
#endif
#if defined(WIN32)
#define LOCKNAMESIZE (PL_NSIZ + 25) /* long enough for username+-+name+.99 */
#define LOCKNAMEINIT ""
#define BONESINIT "bonesnn.xxx.le"
#define BONESSIZE sizeof(BONESINIT)
#endif
#endif

#define INDEXT ".xxxxxx"           /* largest indicator suffix */
#define INDSIZE sizeof(INDEXT)

#if defined(UNIX) || defined(__BEOS__)
#define SAVEX "save/99999.e"
#ifndef SAVE_EXTENSION
#define SAVE_EXTENSION ""
#endif
#else /* UNIX || __BEOS__ */
#ifdef VMS
#define SAVEX "[.save]nnnnn.e;1"
#ifndef SAVE_EXTENSION
#define SAVE_EXTENSION ""
#endif
#else /* VMS */
#if defined(WIN32) || defined(MICRO)
#define SAVEX ""
#if !defined(SAVE_EXTENSION)
#ifdef MICRO
#define SAVE_EXTENSION ".svh"
#endif
#ifdef WIN32
#define SAVE_EXTENSION ".NetHack-saved-game"
#endif
#endif /* !SAVE_EXTENSION */
#endif /* WIN32 || MICRO */
#endif /* else !VMS */
#endif /* else !(UNIX || __BEOS__) */

#ifndef SAVE_EXTENSION
#define SAVE_EXTENSION ""
#endif

#ifndef MICRO
#define SAVESIZE (PL_NSIZ + sizeof(SAVEX) + sizeof(SAVE_EXTENSION) + INDSIZE)
#else
#define SAVESIZE FILENAME
#endif

#endif /* FNAMESIZ_H */
