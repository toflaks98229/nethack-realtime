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

/**
 * @brief A player-supplied rule matching messages by pattern.
 *
 * The player configures these to tame the message stream: a pattern is
 * compiled once into a regex and every message is tested against the list, so
 * routine chatter can be hidden and dangerous news made to stop the game.
 *
 * @note Both the compiled @c regex and the original @c pattern are kept, the
 *       latter so the rule can be shown back to the player.
 */
/**
 * @brief 플레이어가 지정한, 패턴으로 메시지를 걸러 내는 규칙.
 *
 * 플레이어는 메시지 흐름을 다스리기 위해 이것을 설정한다. 패턴은 한 번 정규식으로
 * 컴파일되고 모든 메시지가 이 목록과 대조되므로, 일상적인 잡음은 감추고 위험한
 * 소식은 게임을 멈추게 할 수 있다.
 *
 * @note 컴파일된 @c regex 와 원본 @c pattern 을 모두 보관한다. 후자는 규칙을
 *       플레이어에게 다시 보여 주기 위한 것이다.
 */
struct plinemsg_type {
    xint16 msgtype;  /* one of MSGTYP_foo */
    struct nhregex *regex;
    char *pattern;
    struct plinemsg_type *next;
};

/**
 * @brief What to do with a message that matches a rule.
 * @note @c MSGTYP_STOP does more than display: it interrupts multi-turn
 *       activity, which is the point of flagging a message as important.
 */
/**
 * @brief 규칙에 일치한 메시지를 어떻게 처리할지.
 * @note @c MSGTYP_STOP 은 표시에 그치지 않고 여러 턴에 걸친 행동을 중단시킨다.
 *       메시지를 중요하다고 표시하는 목적이 바로 그것이다.
 */
#define MSGTYP_NORMAL   0
#define MSGTYP_NOREP    1
#define MSGTYP_NOSHOW   2
#define MSGTYP_STOP     3
/* bitmask for callers of hide_unhide_msgtypes() */
#define MSGTYP_MASK_REP_SHOW ((1 << MSGTYP_NOREP) | (1 << MSGTYP_NOSHOW))

/**
 * @brief Emit a message that is a single runtime string.
 * @param cstr The string to print; may contain characters that would otherwise
 *             be read as format directives.
 * @warning Use these instead of passing the string directly as the format
 *          argument. A runtime string containing a percent sign would
 *          otherwise be interpreted as a conversion and read arguments that
 *          were never passed.
 */
/**
 * @brief 런타임 문자열 하나로 이루어진 메시지를 출력한다.
 * @param cstr 출력할 문자열. 형식 지시자로 해석될 수 있는 문자를 포함할 수 있다.
 * @warning 문자열을 형식 인자로 직접 넘기지 말고 이 래퍼들을 쓸 것. 퍼센트 기호가
 *          들어 있는 런타임 문자열은 변환 지시자로 해석되어, 전달된 적 없는 인자를
 *          읽게 된다.
 */
/* pline (et al) for a single string argument (suppress compiler warning) */
#define pline1(cstr) pline("%s", cstr)
#define Your1(cstr) Your("%s", cstr)
#define You1(cstr) You("%s", cstr)
#define verbalize1(cstr) verbalize("%s", cstr)
#define You_hear1(cstr) You_hear("%s", cstr)
#define Sprintf1(buf, cstr) Sprintf(buf, "%s", cstr)
#define panic1(cstr) panic("%s", cstr)

/**
 * @brief Ask the player a yes/no question with the conventional default.
 * @param query The question to display.
 * @return The character the player chose.
 * @note The name encodes the answer set and the default: @c ynaq offers
 *       yes/no/abort defaulting to yes, @c nyaq the same set defaulting to no.
 * @warning @c YN() does not record the answer in the do-again buffer, so
 *          repeating the command asks again rather than reusing the reply.
 */
/**
 * @brief 관례적인 기본값과 함께 플레이어에게 예/아니오를 묻는다.
 * @param query 표시할 질문.
 * @return 플레이어가 선택한 문자.
 * @note 이름이 선택지 집합과 기본값을 나타낸다. @c ynaq 는 예/아니오/중단을
 *       제시하며 기본은 예, @c nyaq 는 같은 집합에 기본이 아니오다.
 * @warning @c YN() 은 답을 do-again 버퍼에 기록하지 않으므로, 명령을 반복하면
 *          답을 재사용하지 않고 다시 묻는다.
 */
/*** some utility macros ***/
#define y_n(query) yn_function(query, ynchars, 'n', TRUE)
#define ynq(query) yn_function(query, ynqchars, 'q', TRUE)
#define ynaq(query) yn_function(query, ynaqchars, 'y', TRUE)
#define nyaq(query) yn_function(query, ynaqchars, 'n', TRUE)
#define nyNaq(query) yn_function(query, ynNaqchars, 'n', TRUE)
#define ynNaq(query) yn_function(query, ynNaqchars, 'y', TRUE)
/* YN() is same as y_n() except doesn't save the response in do-again buffer */
#define YN(query) yn_function(query, ynchars, 'n', FALSE)

/**
 * @brief Non-default handling for one particular message.
 *
 * Ordinarily a message is displayed, recorded in history, and subject to the
 * player's message-type rules. These bits override that for a single call --
 * to keep a repeated message from stuttering, to bypass the player's own
 * filters for something that must be seen, or to mark text as speech.
 *
 * @note @c OVERRIDE_MSGTYPE deliberately ignores the player's configuration,
 *       so it is reserved for messages the game must not let them suppress.
 */
/**
 * @brief 특정 메시지 하나에 대한 기본과 다른 처리.
 *
 * 보통 메시지는 표시되고, 기록에 남으며, 플레이어의 메시지 유형 규칙을 따른다.
 * 이 비트들은 한 번의 호출에 한해 그것을 무시한다. 반복되는 메시지가 계속 튀어
 * 나오지 않게 하거나, 반드시 보아야 할 것에 대해 플레이어의 필터를 우회하거나,
 * 발화로 표시하기 위해서다.
 *
 * @note @c OVERRIDE_MSGTYPE 은 플레이어 설정을 의도적으로 무시하므로, 게임이
 *       감춰지도록 두어서는 안 되는 메시지에만 쓴다.
 */
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
