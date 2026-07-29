/* NetHack 5.0	nh_cmd.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_cmd.h
 * @brief Command queue, special key bindings, and the command dispatch table.
 *
 * Groups the definitions that describe how player input becomes a game
 * command: the queue that lets the code feed synthetic keystrokes and extended
 * commands through the normal path, the enumeration of special (non-command)
 * keys, and @c struct cmd, which caches the current key bindings so the
 * dispatcher can index them directly instead of scanning the command list.
 *
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_cmd.h
 * @brief 명령 큐, 특수 키 바인딩, 명령 디스패치 테이블.
 *
 * 플레이어 입력이 게임 명령으로 바뀌는 과정을 서술하는 정의들을 모았다.
 * 코드가 합성 키 입력과 확장 명령을 정상 경로로 흘려보낼 수 있게 하는 큐,
 * 특수 키(명령이 아닌 키) 열거, 그리고 디스패처가 명령 목록을 훑는 대신
 * 곧바로 색인할 수 있도록 현재 키 바인딩을 캐시하는 @c struct cmd 가 있다.
 *
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_CMD_H
#define NH_CMD_H

/*
 * Rudimentary command queue.
 * Allows the code to put keys and extended commands into the queue,
 * and they're executed just as if the user did them.  Time passes
 * normally when doing queued actions.  The queue will get cleared
 * if hero is interrupted.
 */
enum cmdq_cmdtypes {
    CMDQ_KEY = 0, /* a literal character, cmdq_add_key() */
    CMDQ_EXTCMD,  /* extended command, cmdq_add_ec() */
    CMDQ_DIR,     /* direction, cmdq_add_dir() */
    CMDQ_USER_INPUT, /* placeholder for user input, cmdq_add_userinput() */
    CMDQ_INT,     /* integer value, cmdq_add_int() */
};

struct _cmd_queue {
    int typ;
    char key;
    schar dirx, diry, dirz;
    int intval;
    const struct ext_func_tab *ec_entry;
    struct _cmd_queue *next;
};

enum {
    CQ_CANNED = 0, /* internal canned sequence */
    CQ_REPEAT,     /* user-inputted, if gi.in_doagain, replayed */
    NUM_CQS
};

typedef long cmdcount_nht;    /* Command counts */


/* special key functions */
enum nh_keyfunc {
    NHKF_ESC = 0,

    NHKF_GETDIR_SELF,
    NHKF_GETDIR_SELF2,
    NHKF_GETDIR_HELP,
    NHKF_GETDIR_MOUSE,   /* simulated click for #therecmdmenu; use '_' as
                          * direction to initiate, then getpos() finishing
                          * with ',' (left click) or '.' (right click) */
    NHKF_COUNT,
    NHKF_GETPOS_SELF,
    NHKF_GETPOS_PICK,
    NHKF_GETPOS_PICK_Q,  /* quick */
    NHKF_GETPOS_PICK_O,  /* once */
    NHKF_GETPOS_PICK_V,  /* verbose */
    NHKF_GETPOS_SHOWVALID,
    NHKF_GETPOS_AUTODESC,
    NHKF_GETPOS_MON_NEXT,
    NHKF_GETPOS_MON_PREV,
    NHKF_GETPOS_OBJ_NEXT,
    NHKF_GETPOS_OBJ_PREV,
    NHKF_GETPOS_DOOR_NEXT,
    NHKF_GETPOS_DOOR_PREV,
    NHKF_GETPOS_UNEX_NEXT,
    NHKF_GETPOS_UNEX_PREV,
    NHKF_GETPOS_INTERESTING_NEXT,
    NHKF_GETPOS_INTERESTING_PREV,
    NHKF_GETPOS_VALID_NEXT,
    NHKF_GETPOS_VALID_PREV,
    NHKF_GETPOS_HELP,
    NHKF_GETPOS_MENU,
    NHKF_GETPOS_LIMITVIEW,
    NHKF_GETPOS_MOVESKIP,

    NUM_NHKF
};

/* commands[] is used to directly access cmdlist[] instead of looping
   through it to find the entry for a given input character;
   move_X is the character used for moving one step in direction X;
   alphadirchars corresponds to old sdir,
   dirchars corresponds to ``iflags.num_pad ? ndir : sdir'';
   pcHack_compat and phone_layout only matter when num_pad is on,
   swap_yz only matters when it's off */
struct cmd {
    unsigned serialno;     /* incremented after each update */
    boolean num_pad;       /* same as iflags.num_pad except during updates */
    boolean pcHack_compat; /* for numpad:  affects 5, M-5, and M-0 */
    boolean phone_layout;  /* inverted keypad:  1,2,3 above, 7,8,9 below */
    boolean swap_yz;       /* QWERTZ keyboards; use z to move NW, y to zap */
    const char *dirchars;      /* current movement/direction characters */
    const char *alphadirchars; /* same as dirchars if !numpad */
    struct Cmd_bind *cmdbinds;
    const struct ext_func_tab *mousebtn[NUM_MOUSE_BUTTONS];
    char spkeys[NUM_NHKF];
    char extcmd_char;      /* key that starts an extended command ('#') */
};

#endif /* NH_CMD_H */
