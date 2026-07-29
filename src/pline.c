/* NetHack 5.0	pline.c	$NHDT-Date: 1781973061 2026/06/20 16:31:01 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.150 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2018. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file pline.c
 * @brief Everything the game says to the player.
 *
 * @c pline() and its relatives format a message and hand it to the window
 * port, but most of this file is about the decisions made on the way there:
 * whether the player has asked for this kind of message to be hidden or to
 * stop the game, whether it repeats the last one, whether it should be
 * recorded in the session's history, the dump log, or the public live log.
 *
 * The variants exist because the same sentence needs different grammar
 * depending on who it is about -- @c You(), @c Your(), @c pline_mon() -- and
 * because a message may need to point at a place on the map so the interface
 * can draw attention there.
 *
 * @note Messages are formatted into a buffer several times @c BUFSZ so that
 *       long configuration text can be decorated before being truncated, which
 *       is why the working buffer here is larger than the one callers see.
 */

/**
 * @file pline.c
 * @brief 게임이 플레이어에게 하는 모든 말.
 *
 * @c pline() 과 그 친척들은 메시지를 형식화해 윈도우 포팅에 넘기지만, 이 파일의
 * 대부분은 거기에 이르기까지의 판단에 관한 것이다. 플레이어가 이런 종류의
 * 메시지를 감추라고 했는지 혹은 게임을 멈추라고 했는지, 직전 메시지의 반복인지,
 * 세션 기록·덤프 로그·공개 라이브 로그에 남겨야 하는지 등이다.
 *
 * 변형들이 존재하는 이유는 같은 문장이라도 누구에 관한 것이냐에 따라 다른 문법이
 * 필요하기 때문이며(@c You(), @c Your(), @c pline_mon()), 또 메시지가 지도 위의
 * 한 지점을 가리켜 인터페이스가 그쪽으로 주의를 끌게 해야 할 때가 있기 때문이다.
 *
 * @note 메시지는 @c BUFSZ 의 몇 배가 되는 버퍼에 형식화된다. 긴 설정 텍스트를
 *       장식한 뒤 잘라내기 위함이며, 그래서 여기의 작업 버퍼가 호출자가 보는
 *       것보다 크다.
 */

#include "hack.h"

#define BIGBUFSZ (5 * BUFSZ) /* big enough to format a 4*BUFSZ string (from
                              * config file parsing) with modest decoration;
                              * result will then be truncated to BUFSZ-1 */

staticfn void putmesg(const char *);
staticfn char *You_buf(int);
staticfn void execplinehandler(const char *);
#ifdef USER_SOUNDS
extern void maybe_play_sound(const char *);
#endif
#ifdef DUMPLOG_CORE

/* keep the most recent DUMPLOG_MSG_COUNT messages */
void
dumplogmsg(const char *line)
{
    /*
     * TODO:
     *  This essentially duplicates message history, which is
     *  currently implemented in an interface-specific manner.
     *  The core should take responsibility for that and have
     *  this share it.
     */
    unsigned indx = gs.saved_pline_index; /* next slot to use */
    char *oldest = gs.saved_plines[indx]; /* current content of that slot */

    if (!strncmp(line, "Unknown command", 15))
        return;
    if (oldest && strlen(oldest) >= strlen(line)) {
        /* this buffer will gradually shrink until the 'else' is needed;
           there's no pressing need to track allocation size instead */
        Strcpy(oldest, line);
    } else {
        if (oldest)
            free((genericptr_t) oldest);
        gs.saved_plines[indx] = dupstr(line);
    }
    gs.saved_pline_index = (indx + 1) % DUMPLOG_MSG_COUNT;
}

/* called during save (unlike the interface-specific message history,
   this data isn't saved and restored); end-of-game releases saved_plines[]
   while writing its contents to the final dump log */
void
dumplogfreemessages(void)
{
    unsigned i;

    for (i = 0; i < DUMPLOG_MSG_COUNT; ++i)
        if (gs.saved_plines[i])
            free((genericptr_t) gs.saved_plines[i]), gs.saved_plines[i] = 0;
    gs.saved_pline_index = 0;
}
#endif

/* keeps windowprocs usage out of pline() */
staticfn void
putmesg(const char *line)
{
    int attr = ATR_NONE;

    if (iflags.debug_prevent_pline)
        return;

    if ((gp.pline_flags & URGENT_MESSAGE) != 0
        && (windowprocs.wincap2 & WC2_URGENT_MESG) != 0)
        attr |= ATR_URGENT;
    if ((gp.pline_flags & SUPPRESS_HISTORY) != 0
        && (windowprocs.wincap2 & WC2_SUPPRESS_HIST) != 0)
        attr |= ATR_NOHISTORY;
    putstr(WIN_MESSAGE, attr, line);
    SoundSpeak(line);
}

/* set the direction where next message happens */
void
set_msg_dir(int dir)
{
    dirtocoord(&a11y.msg_loc, dir);
    a11y.msg_loc.x += u.ux;
    a11y.msg_loc.y += u.uy;
}

/* set the coordinate where next message happens */
void
set_msg_xy(coordxy x, coordxy y)
{
    a11y.msg_loc.x = x;
    a11y.msg_loc.y = y;
}

staticfn void vpline(const char *, va_list);

DISABLE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Say something to the player.
 * @param[in] line Printf-style format for the message.
 * @note Subject to the player's message-type rules, so a message may be
 *       suppressed or may stop multi-turn activity.
 * @warning Never pass a runtime string as the format. Use @c pline1() for
 *          that; a stray percent sign would otherwise be read as a conversion.
 */
/**
 * @brief 플레이어에게 무언가를 말한다.
 * @param[in] line printf 형식의 메시지 서식.
 * @note 플레이어의 메시지 유형 규칙을 따르므로, 메시지가 억제되거나 여러 턴짜리
 *       행동을 중단시킬 수 있다.
 * @warning 런타임 문자열을 서식으로 넘기지 말 것. 그런 경우 @c pline1() 을 쓴다.
 *          그러지 않으면 문자열에 섞인 퍼센트 기호가 변환 지시자로 해석된다.
 */
void
pline(const char *line, ...)
{
    va_list the_args;

    va_start(the_args, line);
    vpline(line, the_args);
    va_end(the_args);
}

void
pline_dir(int dir, const char *line, ...)
{
    va_list the_args;

    set_msg_dir(dir);

    va_start(the_args, line);
    vpline(line, the_args);
    va_end(the_args);
}

void
pline_xy(coordxy x, coordxy y, const char *line, ...)
{
    va_list the_args;

    set_msg_xy(x, y);

    va_start(the_args, line);
    vpline(line, the_args);
    va_end(the_args);
}

void
pline_mon(struct monst *mtmp, const char *line, ...)
{
    va_list the_args;

    if (mtmp == &gy.youmonst)
        set_msg_xy(0, 0);
    else
        set_msg_xy(mtmp->mx, mtmp->my);

    va_start(the_args, line);
    vpline(line, the_args);
    va_end(the_args);
}

/**
 * @brief Format a message and decide what becomes of it.
 * @param[in] line     Printf-style format for the message.
 * @param[in] the_args Arguments for that format.
 * @note This is where the player's configuration is honoured: a matching
 *       message may be hidden, may interrupt what the hero is doing, or may be
 *       kept out of the history. It is also where a message identical to the
 *       previous one is suppressed.
 * @warning Reentered indirectly in some paths -- a message can be produced
 *          while one is being handled -- so it guards against recursion rather
 *          than assuming it cannot happen.
 */
/**
 * @brief 메시지를 형식화하고 그것이 어떻게 될지 결정한다.
 * @param[in] line     printf 형식의 메시지 서식.
 * @param[in] the_args 그 서식에 대한 인자들.
 * @note 플레이어의 설정이 반영되는 지점이다. 규칙에 걸린 메시지는 감춰지거나,
 *       영웅이 하던 일을 중단시키거나, 기록에서 빠질 수 있다. 직전과 동일한
 *       메시지를 억제하는 것도 여기서 이루어진다.
 * @warning 일부 경로에서 간접적으로 재진입된다. 메시지를 처리하는 도중에 또
 *          다른 메시지가 생길 수 있기 때문이며, 그래서 그런 일이 없다고 가정하지
 *          않고 재귀를 방어한다.
 */
staticfn void
vpline(const char *line, va_list the_args)
{
    static int in_pline = 0;
    char pbuf[BIGBUFSZ]; /* will get chopped down to BUFSZ-1 if longer */
    int ln;
    int msgtyp;
    boolean no_repeat;
    coord a11y_mesgxy;

    a11y_mesgxy = a11y.msg_loc; /* save a11y.msg_loc before reseting it */
    /* always reset a11y.msg_loc whether we end up using it or not */
    a11y.msg_loc.x = a11y.msg_loc.y = 0;

    if (!line || !*line)
        return;
#ifdef HANGUPHANDLING
    if (program_state.done_hup)
        return;
#endif
    if (program_state.wizkit_wishing)
        return;

    /* when accessiblemsg is set and a11y.msg_loc is nonzero, use the latter
       to insert a location prefix in front of current message */
    if (a11y.accessiblemsg && isok(a11y_mesgxy.x, a11y_mesgxy.y)) {
        char *tmp, *dirstr, dirstrbuf[QBUFSZ];

        dirstr = coord_desc(a11y_mesgxy.x, a11y_mesgxy.y, dirstrbuf,
                            ((iflags.getpos_coords == GPCOORDS_NONE)
                             ? GPCOORDS_COMFULL : iflags.getpos_coords));
        tmp = (char *) alloc(strlen(line) + sizeof ": " + strlen(dirstr));
        Strcpy(tmp, dirstr);
        Strcat(tmp, ": ");
        Strcat(tmp, line);
        vpline(tmp, the_args);
        free((genericptr_t) tmp);
        return;
    }

    if (!strchr(line, '%')) {
        /* format does not specify any substitutions; use it as-is */
        ln = (int) strlen(line);
    } else if (line[0] == '%' && line[1] == 's' && !line[2]) {
        /* "%s" => single string; skip format and use its first argument;
           unlike with the format, it is irrelevant whether the argument
           contains any percent signs */
        line = va_arg(the_args, const char *); /*VA_NEXT(line,const char *);*/
        ln = (int) strlen(line);
    } else {
        /* perform printf() formatting */
        ln = vsnprintf(pbuf, sizeof pbuf, line, the_args);
        line = pbuf;
        /* note: 'ln' is number of characters attempted, not necessarily
           strlen(line); that matters for the overflow check; if we avoid
           the extremely-too-long panic then 'ln' will be actual length */
    }
    if (ln > (int) sizeof pbuf - 1) /* extremely too long */
        panic("pline attempting to print %d characters!", ln);

    if (ln > BUFSZ - 1) {
        /* too long but modestly so; allow but truncate, preserving final
           3 chars: "___ extremely long text" -> "___ extremely l...ext"
           (this may be suboptimal if overflow is less than 3) */
        if (line != pbuf) /* no '%' was present or format was just "%s" */
            (void) strncpy(pbuf, line, BUFSZ - 1); /* caveat: unterminated */
        pbuf[BUFSZ - 1 - 6] = pbuf[BUFSZ - 1 - 5] = pbuf[BUFSZ - 1 - 4] = '.';
        /* avoid strncpy; buffers could overlap if excess is small */
        pbuf[BUFSZ - 1 - 3] = line[ln - 3];
        pbuf[BUFSZ - 1 - 2] = line[ln - 2];
        pbuf[BUFSZ - 1 - 1] = line[ln - 1];
        pbuf[BUFSZ - 1] = '\0';
        line = pbuf;
    }
    msgtyp = MSGTYP_NORMAL;

#ifdef DUMPLOG_CORE
    /* We hook here early to have options-agnostic output.
     * Unfortunately, that means Norep() isn't honored (general issue) and
     * that short lines aren't combined into one longer one (tty behavior).
     */
    if ((gp.pline_flags & SUPPRESS_HISTORY) == 0)
        dumplogmsg(line);
#endif
    /* use raw_print() if we're called too early (or perhaps too late
       during shutdown) or if we're being called recursively (probably
       via debugpline() in the interface code) */
    if (in_pline++ || !iflags.window_inited) {
        /* [we should probably be using raw_printf("\n%s", line) here] */
        raw_print(line);
        iflags.last_msg = PLNMSG_UNKNOWN;
        goto pline_done;
    }

    no_repeat = (gp.pline_flags & PLINE_NOREPEAT) ? TRUE : FALSE;
    if ((gp.pline_flags & OVERRIDE_MSGTYPE) == 0) {
        msgtyp = msgtype_type(line, no_repeat);
#ifdef USER_SOUNDS
        if (msgtyp == MSGTYP_NORMAL || msgtyp == MSGTYP_NOSHOW)
            maybe_play_sound(line);
#endif
        if ((gp.pline_flags & URGENT_MESSAGE) == 0
            && (msgtyp == MSGTYP_NOSHOW
                || (msgtyp == MSGTYP_NOREP && !strcmp(line, gp.prevmsg))))
            /* FIXME: we need a way to tell our caller that this message
             * was suppressed so that caller doesn't set iflags.last_msg
             * for something that hasn't been shown, otherwise a subsequent
             * message which uses alternate wording based on that would be
             * doing so out of context and probably end up seeming silly.
             * (Not an issue for no-repeat but matters for no-show.)
             */
            goto pline_done;
    }

    if (gv.vision_full_recalc) {
        int tmp_in_pline = in_pline;

        in_pline = 0;
        vision_recalc(0);
        in_pline = tmp_in_pline;
    }
    if (u.ux)
        flush_screen((gp.pline_flags & NO_CURS_ON_U) ? 0 : 1); /* %% */

    putmesg(line);

    execplinehandler(line);

    /* this gets cleared after every pline message */
    iflags.last_msg = PLNMSG_UNKNOWN;
    (void) strncpy(gp.prevmsg, line, BUFSZ), gp.prevmsg[BUFSZ - 1] = '\0';
    if (msgtyp == MSGTYP_STOP)
        display_nhwindow(WIN_MESSAGE, TRUE); /* --more-- */
 pline_done:
#ifdef SND_SPEECH
    /* clear the SPEECH flag so caller never has to */
    gp.pline_flags &= ~PLINE_SPEECH;
#endif
    --in_pline;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* pline() variant which can override MSGTYPE handling or suppress
   message history (tty interface uses pline() to issue prompts and
   they shouldn't be blockable via MSGTYPE=hide) */
/**
 * @brief Say something, overriding how it would normally be handled.
 * @param[in] pflags Mask of @c PLINE_ values; see nh_msg.h.
 * @param[in] line   Printf-style format for the message.
 * @note Used where the default is wrong for one particular message -- to stop
 *       a repeat from stuttering, to keep something out of history, or to mark
 *       text as speech.
 * @warning @c OVERRIDE_MSGTYPE ignores the player's own filters, so it belongs
 *          only on messages the game must not let them hide.
 */
/**
 * @brief 평소의 처리 방식을 무시하고 무언가를 말한다.
 * @param[in] pflags @c PLINE_ 값들의 마스크. nh_msg.h 참고.
 * @param[in] line   printf 형식의 메시지 서식.
 * @note 특정 메시지 하나에 대해 기본 동작이 부적절할 때 쓴다. 반복되는 메시지가
 *       계속 튀어나오지 않게 하거나, 기록에 남기지 않거나, 발화로 표시할 때다.
 * @warning @c OVERRIDE_MSGTYPE 은 플레이어 자신의 필터를 무시하므로, 감춰지도록
 *          두어서는 안 되는 메시지에만 쓴다.
 */
void
custompline(unsigned pflags, const char *line, ...)
{
    va_list the_args;

    va_start(the_args, line);
    gp.pline_flags = pflags;
    vpline(line, the_args);
    gp.pline_flags = 0;
    va_end(the_args);
}

/* if player has dismissed --More-- with ESC to suppress further messages
   until next input request, tell the interface that it should override that
   and re-enable them; equivalent to custompline(URGENT_MESSAGE, line, ...)
   but slightly simpler to use */
void
urgent_pline(const char *line, ...)
{
    va_list the_args;

    va_start(the_args, line);
    gp.pline_flags = URGENT_MESSAGE;
    vpline(line, the_args);
    gp.pline_flags = 0;
    va_end(the_args);
}

void
Norep(const char *line, ...)
{
    va_list the_args;

    va_start(the_args, line);
    gp.pline_flags = PLINE_NOREPEAT;
    vpline(line, the_args);
    gp.pline_flags = 0;
    va_end(the_args);
}

staticfn char *
You_buf(int siz)
{
    if (siz > gy.you_buf_siz) {
        if (gy.you_buf)
            free((genericptr_t) gy.you_buf);
        gy.you_buf_siz = siz + 10;
        gy.you_buf = (char *) alloc((unsigned) gy.you_buf_siz);
    }
    return gy.you_buf;
}

void
free_youbuf(void)
{
    if (gy.you_buf)
        free((genericptr_t) gy.you_buf), gy.you_buf = (char *) 0;
    gy.you_buf_siz = 0;
}

/* `prefix' must be a string literal, not a pointer */
#define YouPrefix(pointer, prefix, text) \
    Strcpy((pointer = You_buf((int) (strlen(text) + sizeof prefix))), prefix)

#define YouMessage(pointer, prefix, text) \
    strcat((YouPrefix(pointer, prefix, text), pointer), text)

void
You(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    vpline(YouMessage(tmp, "You ", line), the_args);
    va_end(the_args);
}

void
Your(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    vpline(YouMessage(tmp, "Your ", line), the_args);
    va_end(the_args);
}

void
You_feel(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    if (Unaware)
        YouPrefix(tmp, "You dream that you feel ", line);
    else
        YouPrefix(tmp, "You feel ", line);
    vpline(strcat(tmp, line), the_args);
    va_end(the_args);
}

void
You_cant(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    vpline(YouMessage(tmp, "You can't ", line), the_args);
    va_end(the_args);
}

void
pline_The(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    vpline(YouMessage(tmp, "The ", line), the_args);
    va_end(the_args);
}

void
There(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    vpline(YouMessage(tmp, "There ", line), the_args);
    va_end(the_args);
}

void
You_hear(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    if ((Deaf && !Unaware) || !flags.acoustics)
        return;
    va_start(the_args, line);
    if (Underwater)
        YouPrefix(tmp, "You barely hear ", line);
    else if (Unaware)
        YouPrefix(tmp, "You dream that you hear ", line);
    else
        YouPrefix(tmp, "You hear ", line);  /* Deaf-aware */
    vpline(strcat(tmp, line), the_args);
    va_end(the_args);
}

void
You_see(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    if (Unaware)
        YouPrefix(tmp, "You dream that you see ", line);
    else if (Blind) /* caller should have caught this... */
        YouPrefix(tmp, "You sense ", line);
    else
        YouPrefix(tmp, "You see ", line);
    vpline(strcat(tmp, line), the_args);
    va_end(the_args);
}

/* Print a message inside double-quotes.
 * The caller is responsible for checking deafness.
 * Gods can speak directly to you in spite of deafness.
 */
/**
 * @brief Report something as spoken aloud, in double quotes.
 * @param[in] line Printf-style format for what is said.
 * @warning Does not check whether the hero can hear. The caller must, since
 *          gods are heard through deafness and most other speakers are not.
 */
/**
 * @brief 소리 내어 말한 것으로, 큰따옴표를 둘러 보고한다.
 * @param[in] line printf 형식의 발화 내용 서식.
 * @warning 영웅이 들을 수 있는지 검사하지 않는다. 확인은 호출자의 몫이다. 신은
 *          난청 상태에서도 들리지만 다른 화자들은 그렇지 않기 때문이다.
 */
void
verbalize(const char *line, ...)
{
    va_list the_args;
    char *tmp;

    va_start(the_args, line);
    gp.pline_flags |= PLINE_VERBALIZE;
    tmp = You_buf((int) strlen(line) + sizeof "\"\"");
    Strcpy(tmp, "\"");
    Strcat(tmp, line);
    Strcat(tmp, "\"");
    vpline(tmp, the_args);
    gp.pline_flags &= ~PLINE_VERBALIZE;
    va_end(the_args);
}

#ifdef CHRONICLE

void
gamelog_add(long glflags, long gltime, const char *str)
{
    struct gamelog_line *tmp;
    struct gamelog_line *lst = gg.gamelog;

    tmp = (struct gamelog_line *) alloc(sizeof (struct gamelog_line));
    tmp->turn = gltime;
    tmp->flags = glflags;
    tmp->text = dupstr(str);
    tmp->next = NULL;
    while (lst && lst->next)
        lst = lst->next;
    if (!lst)
        gg.gamelog = tmp;
    else
        lst->next = tmp;
}

void
livelog_printf(long ll_type, const char *line, ...)
{
    char gamelogbuf[BUFSZ * 2];
    va_list the_args;

    va_start(the_args, line);
    (void) vsnprintf(gamelogbuf, sizeof gamelogbuf, line, the_args);
    va_end(the_args);

    gamelog_add(ll_type, svm.moves, gamelogbuf);
    strNsubst(gamelogbuf, "\t", "_", 0);
    livelog_add(ll_type, gamelogbuf);
}

#else

void
gamelog_add(
    long glflags UNUSED, long gltime UNUSED, const char *msg UNUSED)
{
    ; /* nothing here */
}

void
livelog_printf(
    long ll_type UNUSED, const char *line UNUSED, ...)
{
    ; /* nothing here */
}

#endif /* !CHRONICLE */

staticfn void vraw_printf(const char *, va_list);

void
raw_printf(const char *line, ...)
{
    va_list the_args;

    va_start(the_args, line);
    vraw_printf(line, the_args);
    va_end(the_args);
    if (!program_state.beyond_savefile_load)
        ge.early_raw_messages++;
}

DISABLE_WARNING_FORMAT_NONLITERAL

staticfn void
vraw_printf(const char *line, va_list the_args)
{
    char pbuf[BIGBUFSZ]; /* will be chopped down to BUFSZ-1 if longer */

    if (strchr(line, '%')) {
        (void) vsnprintf(pbuf, sizeof(pbuf), line, the_args);
        line = pbuf;
    }
    if ((int) strlen(line) > BUFSZ - 1) {
        if (line != pbuf)
            line = strncpy(pbuf, line, BUFSZ - 1);
        /* unlike pline, we don't futz around to keep last few chars */
        pbuf[BUFSZ - 1] = '\0'; /* terminate strncpy or truncate vsprintf */
    }
    raw_print(line);
    execplinehandler(line);
    if (!program_state.beyond_savefile_load)
        ge.early_raw_messages++;
}

void
impossible(const char *s, ...)
{
    va_list the_args;
    char pbuf[BIGBUFSZ]; /* will be chopped down to BUFSZ-1 if longer */
    char pbuf2[BUFSZ];

    va_start(the_args, s);
    if (program_state.in_impossible)
        panic("impossible called impossible");

    program_state.in_impossible = 1;
    (void) vsnprintf(pbuf, sizeof pbuf, s, the_args);
    va_end(the_args);
    pbuf[BUFSZ - 1] = '\0'; /* sanity */
    paniclog("impossible", pbuf);
    if (iflags.debug_fuzzer == fuzzer_impossible_panic)
        panic("%s", pbuf);

    gp.pline_flags = URGENT_MESSAGE;
    pline("%s", pbuf);
    gp.pline_flags = 0;

    if (program_state.in_sanity_check) {
        /* skip rest of multi-line feedback */
        program_state.in_impossible = 0;
        return;
    }

    Strcpy(pbuf2, "Program in disorder!");
    if (program_state.something_worth_saving)
        Strcat(pbuf2, "  (Saving and reloading may fix this problem.)");
    pline("%s", pbuf2);
    pline("Please report these messages to %s.", DEVTEAM_EMAIL);
    if (sysopt.support) {
        pline("Alternatively, contact local support: %s", sysopt.support);
    }

#ifdef CRASHREPORT
    if (sysopt.crashreporturl) {
        boolean report = ('y' == yn_function("Report now?", ynchars,
                                             'n', FALSE));

        raw_print(""); /* prove to the user the character was accepted */
        if (report) {
            submit_web_report(1, "Impossible", pbuf);
        }
    }
#endif

    program_state.in_impossible = 0;
}

RESTORE_WARNING_FORMAT_NONLITERAL

static boolean use_pline_handler = TRUE;

staticfn void
execplinehandler(const char *line)
{
#if defined(UNIX) && (defined(POSIX_TYPES) || defined(__GNUC__))
    int f;
#endif
    const char *args[3];

    if (!use_pline_handler || !sysopt.msghandler)
        return;

#if defined(UNIX) && (defined(POSIX_TYPES) || defined(__GNUC__))
    f = fork();
    if (f == 0) { /* child */
        args[0] = sysopt.msghandler;
        args[1] = line;
        args[2] = NULL;
        (void) setgid(getgid());
        (void) setuid(getuid());
        (void) execv(args[0], (char *const *) args);
        perror((char *) 0);
        (void) fprintf(stderr, "Exec to message handler %s failed.\n", sysopt.msghandler);
        nh_terminate(EXIT_FAILURE);
    } else if (f > 0) {
        int status;

        waitpid(f, &status, 0);
    } else if (f == -1) {
        perror((char *) 0);
        use_pline_handler = FALSE;
        pline("%s", "Fork to message handler failed.");
    }
#elif defined(WIN32)
    {
        intptr_t ret;
        args[0] = sysopt.msghandler;
        args[1] = line;
        args[2] = NULL;
        ret = _spawnv(_P_NOWAIT, sysopt.msghandler, args);
        nhUse(ret);  /* -Wunused-but-set-variable */
    }
#else
    use_pline_handler = FALSE;
    nhUse(args);
    nhUse(line);
#endif
}

/* nhassert_failed is called when an nhassert's condition is false */
void
nhassert_failed(const char *expression, const char *filepath, int line)
{
    const char *filename, *p;

    /* Attempt to get filename from path.
       TODO: we really need a port provided function to return a filename
       from a path. */
    filename = filepath;
    if ((p = strrchr(filename, '/')) != 0)
        filename = p + 1;
    if ((p = strrchr(filename, '\\')) != 0)
        filename = p + 1;
#ifdef VMS
    /* usually "device:[directory]name"
       but might be "device:[root.][directory]name"
       and either "[directory]" or "[root.]" or both can be delimited
       by <> rather than by []; find the last of ']', '>', and ':'  */
    if ((p = strrchr(filename, ']')) != 0)
        filename = p + 1;
    if ((p = strrchr(filename, '>')) != 0)
        filename = p + 1;
    if ((p = strrchr(filename, ':')) != 0)
        filename = p + 1;
#endif

    impossible("nhassert(%s) failed in file '%s' at line %d",
               expression, filename, line);
}

/*pline.c*/
