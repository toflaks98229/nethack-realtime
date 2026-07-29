/* NetHack 5.0	nh_fileprefix.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_fileprefix.h
 * @brief Directory classes used to locate NetHack's data and state files.
 *
 * A build may keep read-only data, per-user saves, and shared score and lock
 * files in different places. Rather than hard-code paths, the code names the
 * *kind* of file it wants and the port supplies a prefix for each kind. The
 * default is an empty prefix for every class, which yields the traditional
 * behavior of looking in the current directory.
 *
 * @note The comments record each class's sharing and permission expectations
 *       (shared vs per-user, read-only vs read-write); a port that redirects a
 *       prefix must honor them.
 * @warning @c DATAPREFIX must match the corresponding value in @c dlb.c.
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_fileprefix.h
 * @brief NetHack 의 데이터·상태 파일 위치를 지정하는 디렉토리 분류.
 *
 * 빌드에 따라 읽기 전용 데이터, 사용자별 저장 파일, 공유되는 점수·잠금 파일을
 * 서로 다른 곳에 둘 수 있다. 코드는 경로를 하드코딩하는 대신 원하는 파일의
 * *종류* 를 지정하고, 포팅 코드가 종류마다 접두 경로를 제공한다. 기본값은 모든
 * 분류에 대해 빈 접두사이며, 이는 현재 디렉토리를 찾는 전통적 동작이 된다.
 *
 * @note 각 분류의 주석은 공유 여부와 권한 기대치(공유/사용자별, 읽기 전용/
 *       읽기 쓰기)를 기록한다. 접두사를 바꾸는 포팅 코드는 이를 지켜야 한다.
 * @warning @c DATAPREFIX 는 @c dlb.c 의 대응 값과 일치해야 한다.
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_FILEPREFIX_H
#define NH_FILEPREFIX_H

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
#define HACKPREFIX      0  /* shared, RO */
#define LEVELPREFIX     1  /* per-user, RW */
#define SAVEPREFIX      2  /* per-user, RW */
#define BONESPREFIX     3  /* shared, RW */
#define DATAPREFIX      4  /* dungeon/dlb; must match value in dlb.c */
#define SCOREPREFIX     5  /* shared, RW */
#define LOCKPREFIX      6  /* shared, RW */
#define SYSCONFPREFIX   7  /* shared, RO */
#define CONFIGPREFIX    8
#define TROUBLEPREFIX   9  /* shared or per-user, RW (append-only) */
#define PREFIX_COUNT   10
/* used in files.c; xxconf.h can override if needed */
#ifndef FQN_MAX_FILENAME
#define FQN_MAX_FILENAME 512
#endif

#if defined(NOCWD_ASSUMPTIONS) || defined(VAR_PLAYGROUND)
/* the bare-bones stuff is unconditional above to simplify coding; for
 * ports that actually use prefixes, add some more localized things
 */
#define PREFIXES_IN_USE
#endif

#endif /* NH_FILEPREFIX_H */
