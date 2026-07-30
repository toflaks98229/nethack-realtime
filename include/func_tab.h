/* NetHack 5.0	func_tab.h	$NHDT-Date: 1781973080 2026/06/20 16:31:20 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.29 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file func_tab.h
 * @brief The command table, and what a command is allowed to be.
 *
 * Every command the player can give is an entry with a name, a description, a
 * function and a set of flags. Dispatch is a lookup in that table rather than a
 * switch, which is what lets a key, an extended command name, a mouse click and
 * a command the game generated itself all reach the same entry.
 *
 * The flags are how a command declares its own nature: whether it takes game
 * time, whether it works while buried, whether it may be bound to a mouse
 * button, whether the fuzzer may run it, whether it is a prefix expecting
 * another command after it.
 *
 * @note Bindings are a separate list from the table, so the same command can be
 *       reached from several keys and the player's changes do not touch the
 *       table itself.
 * @warning @c GENERALCMD marks a command that does not consume game time.
 *          Getting that wrong is not cosmetic -- a command that takes time when
 *          it should not lets the world act for free.
 */

/**
 * @file func_tab.h
 * @brief 명령 표와, 명령이 무엇일 수 있는지.
 *
 * 플레이어가 내릴 수 있는 모든 명령은 이름과 설명, 함수, 플래그 집합을 가진 항목이다.
 * 디스패치는 switch 가 아니라 그 표에 대한 조회이며, 덕분에 키와 확장 명령 이름,
 * 마우스 클릭, 게임이 스스로 만든 명령이 모두 같은 항목에 도달한다.
 *
 * 플래그는 명령이 자기 성질을 스스로 선언하는 방식이다. 게임 시간을 쓰는지, 매몰된
 * 상태에서도 동작하는지, 마우스 버튼에 바인딩될 수 있는지, 퍼저가 실행해도 되는지,
 * 뒤에 다른 명령을 기대하는 접두 명령인지다.
 *
 * @note 바인딩은 표와 별개의 목록이다. 그래서 같은 명령에 여러 키로 도달할 수 있고,
 *       플레이어의 변경이 표 자체를 건드리지 않는다.
 * @warning @c GENERALCMD 는 게임 시간을 소모하지 않는 명령을 표시한다. 이를 틀리는
 *          것은 표시상의 문제가 아니다. 소모하지 말아야 할 명령이 시간을 쓰면 세계가
 *          공짜로 한 번 움직인다.
 */

#ifndef FUNC_TAB_H
#define FUNC_TAB_H

/**
 * @brief What a command declares about itself.
 * @note Several flags exist for callers other than the player: the fuzzer, the
 *       sanity checker, and commands the game issues internally.
 */
/**
 * @brief 명령이 자기 자신에 대해 선언하는 것.
 * @note 여러 플래그는 플레이어가 아닌 호출자를 위해 존재한다. 퍼저, 정합성 검사기,
 *       그리고 게임이 내부적으로 발행하는 명령들이다.
 */
/* extended command flags */
#define IFBURIED     0x0001 /* can do command when buried */
#define AUTOCOMPLETE 0x0002 /* command autocompletes */
#define WIZMODECMD   0x0004 /* wizard-mode command */
#define GENERALCMD   0x0008 /* general command, does not take game time */
#define CMD_NOT_AVAILABLE 0x0010 /* recognized but non-functional (!SHELL,&c)*/
#define NOFUZZERCMD  0x0020 /* fuzzer cannot execute this command */
#define INTERNALCMD  0x0040 /* only for internal use, not for user */
#define CMD_M_PREFIX 0x0080 /* accepts menu prefix */
#define CMD_gGF_PREFIX 0x0100 /* accepts g/G/F prefix */
#define CMD_MOVE_PREFIXES  (CMD_M_PREFIX | CMD_gGF_PREFIX)
#define PREFIXCMD    0x0200 /* prefix command, requires another one after it */
#define MOVEMENTCMD  0x0400 /* used to move hero/cursor */
#define MOUSECMD     0x0800 /* cmd allowed to be bound to mouse button */
#define CMD_INSANE   0x1000 /* suppress sanity check (for ^P and ^R) */
#define AUTOCOMP_ADJ 0x2000 /* user changed command autocompletion */
#define CMD_PARAM    0x4000 /* command requires a param from key bind */

/* flags for extcmds_match() */
#define ECM_NOFLAGS       0
#define ECM_IGNOREAC   0x01 /* ignore !autocomplete commands */
#define ECM_EXACTMATCH 0x02 /* needs exact match of findstr */
#define ECM_NO1CHARCMD 0x04 /* ignore commands like '?' and '#' */

/**
 * @brief One key, and the command it currently reaches.
 * @note A list rather than an array indexed by key, since a binding may carry a
 *       parameter and several keys may reach one command.
 * @note @c userbind distinguishes the player's own bindings from the defaults,
 *       which is what lets configuration be written back out.
 */
/**
 * @brief 키 하나와, 그것이 현재 도달하는 명령.
 * @note 키로 색인하는 배열이 아니라 목록이다. 바인딩이 매개변수를 지닐 수 있고, 여러
 *       키가 하나의 명령에 도달할 수도 있기 때문이다.
 * @note @c userbind 는 플레이어 자신의 바인딩을 기본값과 구분한다. 덕분에 설정을 다시
 *       내보낼 수 있다.
 */
/* a key bound to ext_func_tab */
struct Cmd_bind {
    uchar key;
    boolean userbind; /* added by user */
    char *param;
    const struct ext_func_tab *cmd;
    struct Cmd_bind *next;
};

/**
 * @brief One command: what it is called, what it does, and how it behaves.
 * @note @c ef_funct returns @c ECMD_ flags rather than a result, since a command
 *       reports what happened -- whether time passed, whether it was cancelled --
 *       rather than a value.
 * @warning The default key here is a starting point only; what actually reaches
 *          this command is whatever is bound to it.
 */
/**
 * @brief 명령 하나. 무엇으로 불리고, 무엇을 하며, 어떻게 행동하는지.
 * @note @c ef_funct 는 결과값이 아니라 @c ECMD_ 플래그를 반환한다. 명령은 값을
 *       돌려주는 것이 아니라 무슨 일이 일어났는지 -- 시간이 지났는지, 취소되었는지 --
 *       를 보고하기 때문이다.
 * @warning 여기의 기본 키는 출발점일 뿐이다. 실제로 이 명령에 도달하는 것은 그것에
 *          바인딩된 무엇이든이다.
 */
struct ext_func_tab {
    uchar key;
    const char *ef_txt, *ef_desc;
    int (*ef_funct)(void); /* must return ECMD_foo flags */
    unsigned flags;
    const char *f_text;
};

extern struct ext_func_tab extcmdlist[];

#endif /* FUNC_TAB_H */
