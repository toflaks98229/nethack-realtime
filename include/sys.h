/* NetHack 5.0	sys.h	$NHDT-Date: 1693083207 2023/08/26 20:53:27 $  $NHDT-Branch: keni-crashweb2 $:$NHDT-Revision: 1.41 $ */
/* Copyright (c) Kenneth Lorber, Kensington, Maryland, 2008. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file sys.h
 * @brief Settings that belong to the installation rather than to the player.
 *
 * Who may enter wizard mode, how many players may be logged in, where a crash
 * report goes, how many score entries one person may hold -- decisions made by
 * whoever installed the game, not by whoever is playing it.
 *
 * That distinction is the point: these come from a system configuration file the
 * player cannot write, which is why they are separate from options entirely.
 *
 * @note Several are lists of user names. They govern access, so they are read
 *       from the system file and not from anything the player controls.
 * @warning @c env_dbgfl exists because an environment variable and the system
 *          file can both set debug files, and which wins depends on which was
 *          seen first -- so it is a three-state record, not a flag.
 */

/**
 * @file sys.h
 * @brief 플레이어가 아니라 설치 환경에 속한 설정들.
 *
 * 누가 마법사 모드에 들어갈 수 있는지, 몇 명이 동시에 접속할 수 있는지, 크래시 보고가
 * 어디로 가는지, 한 사람이 점수표에서 몇 자리를 차지할 수 있는지 -- 게임을 설치한 쪽이
 * 내리는 결정이며, 플레이하는 쪽이 내리는 결정이 아니다.
 *
 * 그 구분이 핵심이다. 이 값들은 플레이어가 쓸 수 없는 시스템 설정 파일에서 오며, 그래서
 * 옵션과 완전히 분리되어 있다.
 *
 * @note 여럿은 사용자 이름 목록이다. 접근 권한을 좌우하므로 시스템 파일에서 읽으며,
 *       플레이어가 통제하는 어떤 것에서도 읽지 않는다.
 * @warning @c env_dbgfl 이 존재하는 이유는 환경 변수와 시스템 파일이 모두 디버그 파일을
 *          지정할 수 있고, 어느 쪽이 이기는지가 무엇을 먼저 보았는지에 달려 있기 때문이다.
 *          그래서 플래그가 아니라 세 가지 상태를 갖는 기록이다.
 */

#ifndef SYS_H
#define SYS_H

/**
 * @brief The installation's settings, as read from the system configuration.
 * @warning Strings here are owned by this structure and outlive any one game, so
 *          they must not be replaced without freeing what was there.
 */
/**
 * @brief 시스템 설정에서 읽어 들인 설치 환경의 설정들.
 * @warning 여기의 문자열은 이 구조체가 소유하며 한 판의 게임보다 오래 남는다. 기존
 *          내용을 해제하지 않고 교체해서는 안 된다.
 */
struct sysopt_s {
    char *support; /* local support contact */
    char *recover; /* how to run recover - may be overridden by win port */
    char *wizards; /* space-separated list of usernames */
    char *fmtd_wizard_list; /* formatted version of wizards; null or "one"
                               or "one or two" or "one, two, or three", &c */
    char *explorers;  /* like wizards, but for access to explore mode */
    char *shellers;   /* like wizards, for ! command (-DSHELL); also ^Z */
    char *genericusers; /* usernames that prompt for user name */
    char *debugfiles; /* files to show debugplines in. '*' is all. */
    char *msghandler;
#ifdef DUMPLOG
    char *dumplogfile; /* where the dump file is saved */
#endif
    int env_dbgfl;    /*  1: debugfiles comes from getenv("DEBUGFILES")
                       *     so sysconf's DEBUGFILES shouldn't override it;
                       *  0: getenv() hasn't been attempted yet;
                       * -1: getenv() didn't find a value for DEBUGFILES.
                       */
    int maxplayers;
    int maxrerollrate;
    int seduce;
    int check_save_uid; /* restoring savefile checks UID? */
    int check_plname; /* use plname for checking wizards/explorers/shellers */
    int bones_pools;
    long livelog; /* LL_foo events to livelog */

    /* record file */
    int persmax;
    int pers_is_uid;
    int entrymax;
    int pointsmin;
    int tt_oname_maxrank;

    /* panic options */
    char *gdbpath;
    char *greppath;
    char *crashreporturl;
    int panictrace_gdb;
    int panictrace_libc;

    /* save and bones format */
    int saveformat[2];    /* primary and onetime conversion */
    int bonesformat[2];   /* primary and onetime conversion */

    /* enable accessibility options */
    int accessibility;
#ifdef WIN32
    int portable_device_paths;  /* nethack config for a portable device */
#endif

    /* nethack's interactive help menu */
    int hideusage;      /* 0: include 'command-line usage' entry in help menu;
                         * 1: suppress it */
};

extern struct sysopt_s sysopt;

#define SYSOPT_SEDUCE sysopt.seduce

#endif /* SYS_H */
