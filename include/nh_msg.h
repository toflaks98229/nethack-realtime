/* NetHack 5.0	nh_msg.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Consolidated 2026-07 from hack.h for the real-time fork (see
   MODIFICATIONS.md).  Content is unchanged; gathered at the position of the
   earliest constituent block, so every definition is available no later than
   before. */

/**
 * @file nh_msg.h
 * @brief Telling the player things, and asking them questions.
 *
 * Three related concerns are gathered here. The player can classify messages by
 * pattern so that ones they find noisy are suppressed or made to stop the game;
 * @c struct plinemsg_type and the @c MSGTYP_ values implement that. Callers
 * that want non-default behavior for a single message -- do not repeat it, do
 * not record it in history, treat it as urgent or as speech -- pass the
 * @c PLINE_ mask values. Finally the @c yn_function wrappers pose the standard
 * yes/no questions with their conventional default answers.
 *
 * @note The single-argument wrappers (@c pline1, @c You1, ...) exist to pass a
 *       runtime string safely through a format-string parameter; they are not
 *       merely shorthand.
 * @warning @c YN() differs from @c y_n() in that the answer is not recorded in
 *          the do-again buffer, so a repeated command will ask again.
 */

/**
 * @file nh_msg.h
 * @brief 플레이어에게 알리고, 플레이어에게 묻는 일.
 *
 * 서로 관련된 세 가지 관심사를 모았다. 플레이어는 메시지를 패턴으로 분류해
 * 성가신 것을 감추거나 게임을 멈추게 할 수 있으며, @c struct plinemsg_type 과
 * @c MSGTYP_ 값이 이를 구현한다. 메시지 하나에 대해 기본과 다른 처리를 원하는
 * 호출자 -- 반복하지 말 것, 기록에 남기지 말 것, 긴급으로 다룰 것, 발화로
 * 취급할 것 -- 는 @c PLINE_ 마스크 값을 넘긴다. 마지막으로 @c yn_function
 * 래퍼들이 표준 예/아니오 질문을 관례적인 기본 답과 함께 제시한다.
 *
 * @note 인자 하나짜리 래퍼(@c pline1, @c You1 등)는 런타임 문자열을 형식 문자열
 *       인자로 안전하게 전달하기 위한 것이며, 단순한 축약이 아니다.
 * @warning @c YN() 은 @c y_n() 과 달리 답을 do-again 버퍼에 기록하지 않으므로,
 *          명령을 반복하면 다시 묻는다.
 */

#ifndef NH_MSG_H
#define NH_MSG_H

struct plinemsg_type {
    xint16 msgtype;  /* one of MSGTYP_foo */
    struct nhregex *regex;
    char *pattern;
    struct plinemsg_type *next;
};

#define MSGTYP_NORMAL   0
#define MSGTYP_NOREP    1
#define MSGTYP_NOSHOW   2
#define MSGTYP_STOP     3
/* bitmask for callers of hide_unhide_msgtypes() */
#define MSGTYP_MASK_REP_SHOW ((1 << MSGTYP_NOREP) | (1 << MSGTYP_NOSHOW))

/* pline (et al) for a single string argument (suppress compiler warning) */
#define pline1(cstr) pline("%s", cstr)
#define Your1(cstr) Your("%s", cstr)
#define You1(cstr) You("%s", cstr)
#define verbalize1(cstr) verbalize("%s", cstr)
#define You_hear1(cstr) You_hear("%s", cstr)
#define Sprintf1(buf, cstr) Sprintf(buf, "%s", cstr)
#define panic1(cstr) panic("%s", cstr)

/*** some utility macros ***/
#define y_n(query) yn_function(query, ynchars, 'n', TRUE)
#define ynq(query) yn_function(query, ynqchars, 'q', TRUE)
#define ynaq(query) yn_function(query, ynaqchars, 'y', TRUE)
#define nyaq(query) yn_function(query, ynaqchars, 'n', TRUE)
#define nyNaq(query) yn_function(query, ynNaqchars, 'n', TRUE)
#define ynNaq(query) yn_function(query, ynNaqchars, 'y', TRUE)
/* YN() is same as y_n() except doesn't save the response in do-again buffer */
#define YN(query) yn_function(query, ynchars, 'n', FALSE)

/* pline_flags; mask values for custompline()'s first argument */
/* #define PLINE_ORDINARY 0 */
#define PLINE_NOREPEAT   1
#define OVERRIDE_MSGTYPE 2
#define SUPPRESS_HISTORY 4
#define URGENT_MESSAGE   8
#define PLINE_VERBALIZE 16
#define PLINE_SPEECH    32
#define NO_CURS_ON_U    64

#endif /* NH_MSG_H */
