/* NetHack 5.0	mail.h	$NHDT-Date: 1781973081 2026/06/20 16:31:21 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.12 $ */
/*      Copyright (c) 2015 by Kenneth Lorber              */
/* NetHack may be freely redistributed.  See license for details. */

/* used by ckmailstatus() to pass information to the mail-daemon in newmail()
 */

/**
 * @file mail.h
 * @brief Delivering something from outside the game to inside it.
 *
 * On a system where the player may receive mail, the game notices and sends a
 * mail daemon to hand over a scroll -- so an event outside the process becomes
 * an object in the dungeon.
 *
 * What the daemon says, what the scroll is called, and what reading it does are
 * all supplied by whatever detected the message, because only that knows what
 * arrived.
 *
 * @note The command carried here is executed later, when the scroll is read, not
 *       when the mail arrives.
 */

/**
 * @file mail.h
 * @brief 게임 밖의 무언가를 게임 안으로 배달하기.
 *
 * 플레이어가 메일을 받을 수 있는 환경에서는 게임이 그것을 알아채고 메일 데몬을 보내
 * 두루마리를 건넨다. 프로세스 밖의 사건이 던전 안의 객체가 되는 것이다.
 *
 * 데몬이 무슨 말을 하고, 두루마리를 뭐라고 부르며, 그것을 읽으면 무슨 일이 일어나는지는
 * 모두 메시지를 감지한 쪽이 제공한다. 무엇이 도착했는지는 그쪽만 알기 때문이다.
 *
 * @note 여기 담긴 명령은 메일이 도착할 때가 아니라 두루마리를 읽을 때 나중에 실행된다.
 */

#ifndef MAIL_H
#define MAIL_H

/**
 * @brief How intrusive an arriving message is.
 * @note The distinction decides how the daemon behaves: an interruption is
 *       announced more insistently than a piece of mail.
 */
/**
 * @brief 도착한 메시지가 얼마나 방해가 되는지.
 * @note 이 구분이 데몬의 행동을 정한다. 방해성 알림은 평범한 메일보다 더 강하게
 *       전달된다.
 */
#define MSG_OTHER 0 /* catch-all; none of the below... */
#define MSG_MAIL 1  /* unimportant, uninteresting mail message */
#define MSG_CALL 2  /* annoying phone/talk/chat-type interruption */

struct mail_info {
    int message_typ;          /* MSG_foo value */
    const char *display_txt;  /* text for daemon to verbalize */
    const char *object_nam;   /* text to tag object with */
    const char *response_cmd; /* command to eventually execute */
};

#endif /* MAIL_H */
