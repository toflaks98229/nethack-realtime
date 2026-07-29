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

/**
 * @brief What kind of item a queue entry holds.
 *
 * A queued entry is not always a keystroke: the code may need to supply a
 * whole extended command, a direction, or a numeric argument that the player
 * would otherwise have typed.
 *
 * @note @c CMDQ_USER_INPUT is a placeholder rather than a value; it marks a
 *       point at which the queue defers to the player.
 */
/**
 * @brief 큐 항목이 담고 있는 것의 종류.
 *
 * 큐에 넣는 항목이 항상 키 입력인 것은 아니다. 코드가 확장 명령 전체, 방향,
 * 또는 플레이어가 입력했을 숫자 인자를 직접 공급해야 할 때도 있다.
 *
 * @note @c CMDQ_USER_INPUT 은 값이 아니라 자리표시자다. 큐가 플레이어에게
 *       입력을 넘기는 지점을 표시한다.
 */
enum cmdq_cmdtypes {
    CMDQ_KEY = 0, /* a literal character, cmdq_add_key() */
    CMDQ_EXTCMD,  /* extended command, cmdq_add_ec() */
    CMDQ_DIR,     /* direction, cmdq_add_dir() */
    CMDQ_USER_INPUT, /* placeholder for user input, cmdq_add_userinput() */
    CMDQ_INT,     /* integer value, cmdq_add_int() */
};

/**
 * @brief One pending entry in a command queue.
 *
 * The @c typ field selects which of the remaining members is meaningful, so a
 * single node can stand for a key, an extended command, a direction, or an
 * integer.
 *
 * @note Entries are heap-allocated and consumed by @c cmdq_pop(), which
 *       transfers ownership to the caller; the caller frees the node.
 */
/**
 * @brief 명령 큐에 대기 중인 항목 하나.
 *
 * @c typ 필드가 나머지 멤버 중 어느 것이 유효한지를 정하므로, 노드 하나가 키,
 * 확장 명령, 방향, 정수 중 무엇이든 나타낼 수 있다.
 *
 * @note 항목은 힙에 할당되며 @c cmdq_pop() 이 꺼내면서 소유권을 호출자에게
 *       넘긴다. 노드 해제는 호출자의 몫이다.
 */
struct _cmd_queue {
    int typ;
    char key;
    schar dirx, diry, dirz;
    int intval;
    const struct ext_func_tab *ec_entry;
    struct _cmd_queue *next;
};

/**
 * @brief Which queue an entry belongs to.
 *
 * Canned sequences the code generates are kept apart from what the player
 * typed, so that repeating a command replays the player's intent rather than
 * the machinery that carried it out.
 */
/**
 * @brief 항목이 속한 큐의 종류.
 *
 * 코드가 생성한 정해진 순서열은 플레이어가 실제로 입력한 것과 분리해 보관한다.
 * 그래야 명령을 반복할 때 그것을 수행한 내부 절차가 아니라 플레이어의 의도가
 * 재생된다.
 */
enum {
    CQ_CANNED = 0, /* internal canned sequence */
    CQ_REPEAT,     /* user-inputted, if gi.in_doagain, replayed */
    NUM_CQS
};

/**
 * @brief Repetition count attached to a command.
 * @note Signed and long enough to hold the largest count a player can type;
 *       commands treat a zero or negative count as "no count given".
 */
/**
 * @brief 명령에 붙는 반복 횟수.
 * @note 부호가 있으며 플레이어가 입력할 수 있는 최대 횟수를 담을 만큼 크다.
 *       명령들은 0 또는 음수 횟수를 "횟수 미지정"으로 취급한다.
 */
typedef long cmdcount_nht;    /* Command counts */


/**
 * @brief Keys that steer an interaction rather than naming a command.
 *
 * These are the keys that mean something *inside* a prompt: cancelling, giving
 * a count, choosing a direction, or moving the cursor around the map while
 * picking a position. They are bindable, so they live in a table rather than
 * being hard-coded at each prompt.
 *
 * @note @c NUM_NHKF sizes @c struct cmd's @c spkeys array; adding a function
 *       enlarges every saved binding set.
 */
/**
 * @brief 명령을 지칭하는 대신 상호작용을 조종하는 키들.
 *
 * 프롬프트 *안에서* 의미를 갖는 키들이다. 취소, 횟수 입력, 방향 선택, 위치를
 * 고르는 동안 지도 위에서 커서를 옮기는 일 등이다. 이들은 재바인딩할 수 있으므로
 * 각 프롬프트에 하드코딩하지 않고 테이블에 둔다.
 *
 * @note @c NUM_NHKF 가 @c struct cmd 의 @c spkeys 배열 크기를 정한다. 기능을
 *       추가하면 저장되는 모든 바인딩 집합이 커진다.
 */
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

/**
 * @brief The resolved key bindings the dispatcher works from.
 *
 * Options alone do not determine what a key does: number-pad mode, keyboard
 * layout, and user rebindings interact. Rather than re-derive that at every
 * keystroke, the result is resolved once into this structure, and @c serialno
 * lets interested code notice when it has been rebuilt.
 *
 * @note @c dirchars and @c alphadirchars differ only when the number pad is
 *       active; the alphabetic set is retained so commands that always want
 *       letter directions can reach it.
 * @warning The layout flags are only meaningful in one number-pad state each
 *          (@c pcHack_compat and @c phone_layout when it is on, @c swap_yz
 *          when it is off); reading them otherwise is not meaningful.
 */
/**
 * @brief 디스패처가 사용하는, 해석이 끝난 키 바인딩.
 *
 * 키가 무슨 일을 하는지는 옵션만으로 정해지지 않는다. 숫자 패드 모드, 키보드
 * 배열, 사용자 재바인딩이 서로 얽힌다. 키를 누를 때마다 이를 다시 계산하는 대신
 * 결과를 한 번 이 구조체로 해석해 두며, @c serialno 로 재구성 여부를 알 수 있다.
 *
 * @note @c dirchars 와 @c alphadirchars 는 숫자 패드가 켜져 있을 때만 다르다.
 *       항상 문자 방향을 원하는 명령이 참조할 수 있도록 알파벳 집합을 함께
 *       보관한다.
 * @warning 배열 관련 플래그는 각각 한쪽 숫자 패드 상태에서만 의미가 있다
 *          (@c pcHack_compat 과 @c phone_layout 은 켜졌을 때, @c swap_yz 는
 *          꺼졌을 때). 그 밖의 경우에 읽는 것은 의미가 없다.
 */
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
