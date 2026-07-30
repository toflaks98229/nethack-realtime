/* NetHack 5.0	rumors.c	$NHDT-Date: 1781973065 2026/06/20 16:31:05 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.93 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file rumors.c
 * @brief Rumors, oracles, and the epitaphs and engravings the dungeon is
 *        littered with.
 *
 * Rumors come in two kinds -- true and false -- and which the hero gets is a
 * matter of luck and of who is speaking. A fortune cookie may lie; the Oracle,
 * paid properly, does not.
 *
 * The text lives in data files rather than in the program, so this reads it at
 * the moment it is needed. Rather than load a whole file to pick one line, it
 * seeks to a random offset and takes the line it lands in, which is why the
 * files record where the true entries end and the false ones begin.
 *
 * @note The same mechanism serves epitaphs, engravings and hallucinatory
 *       bogus monster names; they are different files read the same way.
 */

/**
 * @file rumors.c
 * @brief 소문과 신탁, 그리고 던전 곳곳에 흩어진 묘비명과 각인.
 *
 * 소문에는 참과 거짓 두 종류가 있고, 영웅이 어느 쪽을 듣는지는 운과 말하는 이에
 * 달려 있다. 포춘 쿠키는 거짓말을 할 수 있지만, 값을 제대로 치른 신탁은 그러지
 * 않는다.
 *
 * 문구는 프로그램이 아니라 데이터 파일에 있으므로 필요한 순간에 읽어 온다. 한
 * 줄을 고르자고 파일 전체를 불러오는 대신 무작위 위치로 건너뛰어 거기 걸린 줄을
 * 취한다. 파일이 참 항목이 끝나고 거짓 항목이 시작되는 지점을 기록해 두는 이유가
 * 그것이다.
 *
 * @note 같은 방식이 묘비명, 각인, 환각 상태의 가짜 몬스터 이름에도 쓰인다.
 *       파일만 다를 뿐 읽는 방법은 같다.
 */

#include "hack.h"
#include "dlb.h"

/*      [Note:  this comment is fairly old, but still accurate for 3.1;
 *       it's no longer accurate for 5.0 but may still be of interest.]
 * Rumors have been entirely rewritten to speed up the access.  This is
 * essential when working from floppies.  Using fseek() the way that's done
 * here means rumors following longer rumors are output more often than those
 * following shorter rumors.  Also, you may see the same rumor more than once
 * in a particular game (although the odds are highly against it), but
 * this also happens with real fortune cookies.  -dgk
 */

/*      3.6
 * The rumors file consists of a "do not edit" line, then a line containing
 * three sets of three counts (first two in decimal, third in hexadecimal).
 * The first set has the number of true rumors, the count in bytes for all
 * true rumors, and the file offset to the first one.  The second set has
 * the same group of numbers for the false rumors.  The third set has 0 for
 * count, 0 for size, and the file offset for end-of-file.  The offset of
 * the first true rumor plus the size of the true rumors matches the offset
 * of the first false rumor.  Likewise, the offset of the first false rumor
 * plus the size of the false rumors matches the offset for end-of-file.
 */

/*      3.1     [now obsolete for rumors but still accurate for oracles]
 * The rumors file consists of a "do not edit" line, a hexadecimal number
 * giving the number of bytes of useful/true rumors, followed by those
 * true rumors (one per line), followed by the useless/false/misleading/cute
 * rumors (also one per line).  Number of bytes of untrue rumors is derived
 * via fseek(EOF)+ftell().
 *
 * The oracles file consists of a "do not edit" comment, a decimal count N
 * and set of N+1 hexadecimal fseek offsets, followed by N multiple-line
 * records, separated by "---" lines.  The first oracle is a special case,
 * and placed there by 'makedefs'.
 */

#ifndef SFCTOOL
staticfn void unpadline(char *);
staticfn void init_rumors(dlb *);
staticfn char *get_rnd_line(dlb *, char *, unsigned, int (*)(int),
                          long, long, unsigned);
staticfn void init_oracles(dlb *);
staticfn void others_check(const char *ftype, const char *, winid *);
staticfn void couldnt_open_file(const char *);
staticfn void init_CapMons(void);

/**
 * @name The capitalised-monster-name list
 *
 * A cache of every monster name that begins with a capital letter, real and hallucinatory alike, built on first use and kept for the rest of the game.
 *
 * It exists because a name that is capitalised in the data is a proper name -- "Woodland-elf", "Cyclops" -- and a proper name must not be given an article or lower-cased when it appears in a sentence. Working that out from the data
 * every time a name is printed would mean scanning the whole monster table and the hallucinatory name file, so the answers are collected once.
 *
 * @note Kept as plain file-scope data rather than in the game's globals structure, as the existing comment records. It is derived entirely from files that do not change during a game, so there is nothing here worth saving.
 * @note The size counts one more than the two counts together, for the terminator.
 * @{
 */

/** How many real monster names and how many hallucinatory ones were collected, and the allocated size. All zero until the list is built. */
/** 실제 몬스터 이름 몇 개와 환각 이름 몇 개가 모아졌는지, 그리고 할당된 크기. 목록이 만들어지기 전까지 모두 영이다. */
static unsigned CapMonstCnt = 0, CapBogonCnt = 0,
                CapMonSiz = 0; /* CapMonstCnt+CapBogonCnt+1 when non-zero */

/** The names themselves. Null until built, which is how the code knows whether to build it. */
/** 이름 자체. 만들어지기 전까지 널이며, 그것이 코드가 그것을 만들어야 하는지 아는 방법이다. */
static const char **CapMons = 0;

/** @} */

/**
 * @brief The characters a hallucinatory monster name may begin with to say what sort it is.
 *
 * Defined elsewhere and shared here rather than duplicated, because both files must agree about them: this file strips the codes when reading names, and the other applies their meaning. A name whose code one file knows and the other does
 * not would be printed with its code showing.
 *
 * @note Documented where the names live, in the hallucinatory name data file, rather than here.
 */
/**
 * @brief 환각 몬스터 이름이 자기가 어떤 종류인지 말하기 위해 시작할 수 있는 문자들.
 *
 * 다른 곳에서 정의되어 여기서 복제되는 대신 공유된다. 두 파일이 그것들에 대해 일치해야 하기 때문이다. 이 파일은 이름을 읽을 때 그 부호를 떼고, 다른 파일은 그 뜻을 적용한다. 한 파일은 알고 다른 파일은 모르는 부호를 가진 이름은 그 부호가 보이는 채로 인쇄될 것이다.
 *
 * @note 여기가 아니라 이름이 있는 곳인 환각 이름 데이터 파일에 문서화되어 있다.
 */
extern const char bogon_codes[]; /* from do_name.c */

/**
 * @brief Remove the padding the data-generating tool added to a short line.
 *
 * The tool pads short entries with trailing underscores, as the existing comment records. That is not decoration: the seek-to-a-random-offset method makes a line's chance of being chosen proportional to its length, so padding short
 * entries evens out the odds. The padding has to come off before the line is shown.
 *
 * @param line the line, shortened in place
 * @note Also removes a newline if one is still there. The caller is supposed to have done that, as the comment notes, so this is a second line of defence rather than the intended path -- but a rumor printed with its newline attached
 *       would break the message line, so it is worth checking twice.
 * @warning An entry that genuinely ends in an underscore cannot be represented. Anything written that way in the data files silently loses its final character, and the data files are written by hand.
 */
/**
 * @brief 데이터 생성 도구가 짧은 줄에 더한 채움을 없앤다.
 *
 * 기존 주석이 기록하듯 그 도구는 짧은 항목을 뒤따르는 밑줄로 채운다. 그것은 장식이 아니다. 무작위 위치로 건너뛰는 방법은 한 줄이 골라질 가능성을 그 길이에 비례하게 만들므로, 짧은 항목을 채우는 것이 그 확률을 고르게 한다. 그 채움은 줄이 보이기 전에 떼어져야 한다.
 *
 * @param line 그 줄. 제자리에서 짧아진다
 * @note 줄바꿈이 아직 있으면 그것도 없앤다. 주석이 적듯 호출자가 그것을 했어야 하므로 이것은 의도된 길이 아니라 두 번째 방어선이다. 그러나 줄바꿈이 붙은 채 인쇄된 소문은 메시지 줄을 망칠 것이므로 두 번 검사할 만하다.
 * @warning 진짜로 밑줄로 끝나는 항목은 나타내어질 수 없다. 데이터 파일에 그렇게 쓰인 것은 조용히 마지막 문자를 잃으며, 데이터 파일은 손으로 쓰인다.
 */
staticfn void
unpadline(char *line)
{
    char *p = eos(line);

    /* remove newline if still present; caller should have stripped it */
    if (p > line && p[-1] == '\n')
        --p;

    /* remove padding */
    while (p > line && p[-1] == '_')
        --p;

    *p = '\0';
}

DISABLE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Read the rumor file's header, learning where the true and false rumors lie.
 *
 * The header exists so that a rumor can be fetched without reading the file. It gives, for each kind, how many there are, how many bytes they occupy and where the first one starts -- from which the end of each range is computed. The
 * format is described in detail in the comments above.
 *
 * @param fp the open rumor file
 * @note The counts of rumors are read but not used. Only the byte ranges matter, because a rumor is chosen by offset rather than by number.
 * @note The file's own consistency -- that the true range ends exactly where the false one begins -- is checked only by assertions that are commented out. They record what the format promises without enforcing it.
 * @note A rumor file with no rumors of one kind is rejected as well as an unreadable header, because the choosing code would draw within a range of zero length.
 * @warning Failure is recorded by setting the true-rumor size to minus one, and the file is closed here. The caller must not go on to read from it, and must recognise that value -- there is no return value saying so.
 */
/**
 * @brief 소문 파일의 머리글을 읽어 참 소문과 거짓 소문이 어디 있는지 안다.
 *
 * 머리글이 존재하는 것은 파일을 읽지 않고 소문을 가져올 수 있게 하기 위해서다. 그것은 각 종류에 대해 몇 개가 있는지, 몇 바이트를 차지하는지, 첫 번째가 어디서 시작하는지를 주며, 그것으로부터 각 범위의 끝이 계산된다. 그 형식은 위의 주석들에 자세히 서술되어 있다.
 *
 * @param fp 열린 소문 파일
 * @note 소문의 개수는 읽히지만 쓰이지 않는다. 바이트 범위만 중요하다. 소문이 번호가 아니라 위치로 골라지기 때문이다.
 * @note 파일 자신의 일관성, 곧 참 범위가 거짓 범위가 시작하는 곳에서 정확히 끝난다는 것은 주석 처리된 단정문으로만 검사된다. 그것들은 형식이 약속하는 것을 강제하지 않으면서 기록한다.
 * @note 한 종류의 소문이 없는 소문 파일은 읽을 수 없는 머리글과 마찬가지로 거부된다. 고르는 코드가 길이 영인 범위 안에서 뽑을 것이기 때문이다.
 * @warning 실패는 참 소문 크기를 마이너스 일로 설정함으로써 기록되며, 파일은 여기서 닫힌다. 호출자는 이어서 그것에서 읽어서는 안 되며 그 값을 알아보아야 한다. 그렇다고 말하는 반환 값은 없다.
 */
staticfn void
init_rumors(dlb *fp)
{
    static const char rumors_header[] = "%d,%ld,%lx;%d,%ld,%lx;0,0,%lx\n";
    int true_count, false_count; /* in file but not used here */
    unsigned long eof_offset;
    char line[BUFSZ];

    (void) dlb_fgets(line, sizeof line, fp); /* skip "don't edit" comment */
    (void) dlb_fgets(line, sizeof line, fp);
    if (sscanf(line, rumors_header, &true_count, &gt.true_rumor_size,
               &gt.true_rumor_start, &false_count, &gf.false_rumor_size,
               &gf.false_rumor_start, &eof_offset) == 7
        && gt.true_rumor_size > 0L
        && gf.false_rumor_size > 0L) {
        gt.true_rumor_end = (long) gt.true_rumor_start + gt.true_rumor_size;
        /* assert( gt.true_rumor_end == false_rumor_start ); */
        gf.false_rumor_end = (long) gf.false_rumor_start + gf.false_rumor_size;
        /* assert( gf.false_rumor_end == eof_offset ); */
    } else {
        gt.true_rumor_size = -1L; /* init failed */
        (void) dlb_fclose(fp);
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Fetch a rumor, true or false as asked.
 *
 * The truth argument is not simply a choice between two ranges: zero means either, and the decision is made by adding a coin flip to the argument. That arithmetic is worth reading in the table above it -- asking for true always gets
 * true, asking for false always gets false, and asking for either gets one of each half the time.
 *
 * @param truth 1 for true, -1 for false, 0 for either
 * @param rumor_buf where to write the rumor
 * @param exclude_cookie whether to refuse rumors that mention fortune cookies
 * @return the buffer
 * @note Some false rumors refer to the cookie the hero is eating. Those cannot be used where there is no cookie -- graffiti on a wall, for instance -- so such rumors are marked in the data file and skipped when asked. As the existing
 *       comment records, the flag has no effect on true rumors because none of them mention cookies.
 * @note The marker is stripped from the rumor when cookies are allowed, so the mark is never shown to a player either way.
 * @note Asking for a rumor exercises wisdom, and getting a true one exercises it favourably. Suppressed while a level is being made, as the existing comment notes -- graffiti generated during level creation is not the hero learning
 *       anything.
 * @warning Retries up to fifty times when excluding cookie rumors and complains if it cannot find one. That bound exists because the exclusion is a filter over random draws rather than a narrowed range, so there is no guarantee of
 *          success; a data file where most false rumors mention cookies would make this slow and occasionally fail.
 * @note A file that cannot be opened records failure in the same place initialisation does, which stops it being retried for the rest of the game rather than complaining on every fortune cookie.
 */
/**
 * @brief 요청받은 대로 참 또는 거짓 소문을 가져온다.
 *
 * 진실 인자는 단순히 두 범위 사이의 선택이 아니다. 영은 어느 쪽이든을 뜻하고, 그 결정은 인자에 동전 던지기를 더함으로써 이루어진다. 그 산술은 그 위의 표에서 읽을 만하다. 참을 요청하면 언제나 참을 얻고, 거짓을 요청하면 언제나 거짓을 얻고, 어느 쪽이든을 요청하면 절반씩 하나를 얻는다.
 *
 * @param truth 참에 1, 거짓에 -1, 어느 쪽이든에 0
 * @param rumor_buf 소문을 쓸 곳
 * @param exclude_cookie 포춘 쿠키를 언급하는 소문을 거부할지
 * @return 그 버퍼
 * @note 어떤 거짓 소문은 영웅이 먹고 있는 쿠키를 가리킨다. 그런 것은 쿠키가 없는 곳, 예컨대 벽의 낙서에는 쓰일 수 없다. 그래서 그런 소문이 데이터 파일에 표시되고 요청받을 때 건너뛰어진다. 기존 주석이 기록하듯 그 표시는 참 소문에는 아무 영향이 없다. 그중 무엇도 쿠키를 언급하지 않기
 *       때문이다.
 * @note 쿠키가 허용될 때 그 표시는 소문에서 떼어지므로, 어느 쪽이든 그 표시가 플레이어에게 보이는 일은 없다.
 * @note 소문을 요청하는 것이 지혜를 단련하고, 참인 것을 얻는 것이 그것을 유리하게 단련한다. 기존 주석이 적듯 층이 만들어지는 동안은 억제된다. 층 생성 중에 만들어지는 낙서는 영웅이 무엇을 배우는 것이 아니다.
 * @warning 쿠키 소문을 배제할 때 오십 번까지 다시 시도하고, 찾을 수 없으면 불평한다. 그 한계가 존재하는 것은 그 배제가 좁혀진 범위가 아니라 무작위 뽑기 위의 걸름이어서 성공이 보장되지 않기 때문이다. 거짓 소문 대부분이 쿠키를 언급하는 데이터 파일은 이것을 느리게 만들고 이따금 실패하게
 *          만들 것이다.
 * @note 열 수 없는 파일은 초기화가 하는 것과 같은 곳에 실패를 기록하며, 그것이 포춘 쿠키마다 불평하는 대신 남은 게임 동안 다시 시도되지 않게 한다.
 */
char *
getrumor(
    int truth, /* 1=true, -1=false, 0=either */
    char *rumor_buf,
    boolean exclude_cookie)
{
    dlb *rumors;
    long beginning, ending;
    char line[BUFSZ];
    static const char *cookie_marker = "[cookie] ";
    const int marklen = strlen(cookie_marker);

    rumor_buf[0] = '\0';
    if (gt.true_rumor_size < 0L) /* a previous try failed to open RUMORFILE */
        return rumor_buf;

    rumors = dlb_fopen(RUMORFILE, "r");
    if (rumors) {
        int count = 0;
        int adjtruth;

        do {
            rumor_buf[0] = '\0';
            if (gt.true_rumor_size == 0L) { /* if this is 1st outrumor() */
                init_rumors(rumors);
                if (gt.true_rumor_size < 0L) { /* init failed */
                    Sprintf(rumor_buf, "Error reading \"%.80s\".", RUMORFILE);
                    return rumor_buf;
                }
            }
            /*
             *  input:      1    0   -1
             *   rn2 \ +1  2=T  1=T  0=F
             *   adj./ +0  1=T  0=F -1=F
             */
            switch (adjtruth = truth + rn2(2)) {
            case 2: /*(might let a bogus input arg sneak thru)*/
            case 1:
                beginning = (long) gt.true_rumor_start;
                ending = gt.true_rumor_end;
                break;
            case 0: /* once here, 0 => false rather than "either"*/
            case -1:
                beginning = (long) gf.false_rumor_start;
                ending = gf.false_rumor_end;
                break;
            default:
                impossible("strange truth value for rumor");
                return strcpy(rumor_buf, "Oops...");
            }
            Strcpy(rumor_buf,
                   get_rnd_line(rumors, line, (unsigned) sizeof line, rn2,
                                beginning, ending, MD_PAD_RUMORS));
        } while (count++ < 50 && exclude_cookie
                 && !strncmp(rumor_buf, cookie_marker, marklen));
        (void) dlb_fclose(rumors);
        if (count >= 50)
            impossible("Can't find non-cookie rumor?");
        else if (!gi.in_mklev) /* avoid exercising wisdom for graffiti */
            exercise(A_WIS, (adjtruth > 0));
    } else {
        couldnt_open_file(RUMORFILE);
        gt.true_rumor_size = -1; /* don't try to open it again */
    }
    if (!exclude_cookie
        && !strncmp(rumor_buf, cookie_marker, marklen)) {
        /* remove cookie_marker from the string */
        char *src = rumor_buf + marklen;
        char *dst = rumor_buf;
        for (; *src != '\0'; ++src, ++dst) {
            *dst = *src;
        }
        *dst = '\0'; /* terminator wasn't copied */
    }
    return rumor_buf;
}

/**
 * @brief A wizard-mode report on whether the data files are shaped as expected.
 *
 * Shows the recorded offsets and then the rumors actually found at them -- the first and last of each kind. That is the only way to tell whether a rumor file is right: the offsets can be internally consistent and still point into the
 * middle of lines, and the symptom is rumors that begin mid-sentence.
 *
 * @note Shows the rumors decrypted. The data files are lightly scrambled so that a player browsing them does not spoil the game for themselves; this is a deliberate exception, since the point is to inspect them.
 * @note The last rumor of each kind is found by reading forward until the range ends, because nothing records where the last one begins.
 * @note Does not reopen a file that a previous attempt failed on. As the existing comment records, that is why an unreadable rumor file produces one report rather than a repeated complaint -- and the epitaph, engraving and monster-name
 *       files are still checked afterwards regardless.
 * @note The three other files are checked by others_check() rather than here, because they have a different and simpler format. As the existing comment records, they are checked at all because the first implementation of them had a bug.
 */
/**
 * @brief 데이터 파일이 예상된 모양인지에 대한 마법사 모드 보고.
 *
 * 기록된 위치를 보인 다음 그 위치에서 실제로 발견된 소문을 보인다. 각 종류의 첫 번째와 마지막 것. 그것이 소문 파일이 옳은지 말할 수 있는 유일한 방법이다. 위치는 내부적으로 일관되면서도 줄 가운데를 가리킬 수 있고, 그 증상은 문장 중간에서 시작하는 소문이다.
 *
 * @note 소문을 복호화해 보인다. 데이터 파일은 그것을 뒤져 보는 플레이어가 스스로 게임을 망치지 않도록 가볍게 뒤섞여 있다. 이것은 의도된 예외다. 요점이 그것을 살펴보는 것이기 때문이다.
 * @note 각 종류의 마지막 소문은 범위가 끝날 때까지 앞으로 읽어서 찾는다. 마지막 것이 어디서 시작하는지 기록하는 것이 없기 때문이다.
 * @note 앞선 시도가 실패한 파일을 다시 열지 않는다. 기존 주석이 기록하듯 그것이 읽을 수 없는 소문 파일이 되풀이되는 불평 대신 하나의 보고를 내는 이유다. 그리고 묘비명, 각인, 몬스터 이름 파일은 그와 무관하게 뒤에 여전히 검사된다.
 * @note 다른 세 파일은 여기가 아니라 others_check()가 검사한다. 그것들이 다르고 더 단순한 형식을 갖기 때문이다. 기존 주석이 기록하듯 그것들이 검사되는 것은 애초에 그것들의 첫 구현에 결함이 있었기 때문이다.
 */
void
rumor_check(void)
{
    dlb *rumors;
    winid tmpwin = WIN_ERR;
    char *endp, line[BUFSZ], xbuf[BUFSZ], rumor_buf[BUFSZ];

    rumors = (gt.true_rumor_size >= 0) ? dlb_fopen(RUMORFILE, "r") : 0;
    if (rumors) {
        long ftell_rumor_start = 0L;

        rumor_buf[0] = '\0';
        if (gt.true_rumor_size == 0L) { /* if this is 1st outrumor() */
            init_rumors(rumors);
            if (gt.true_rumor_size < 0L) {
                rumors = (dlb *) 0; /* init_rumors() closes it upon failure */
                goto no_rumors; /* init failed */
            }
        }
        tmpwin = create_nhwindow(NHW_TEXT);

        /*
         * reveal the values.
         */
        Sprintf(rumor_buf,
               "T start=%06ld (%06lx), end=%06ld (%06lx), size=%06ld (%06lx)",
            (long) gt.true_rumor_start, gt.true_rumor_start,
            gt.true_rumor_end, (unsigned long) gt.true_rumor_end,
            gt.true_rumor_size,(unsigned long) gt.true_rumor_size);
        putstr(tmpwin, 0, rumor_buf);
        Sprintf(rumor_buf,
               "F start=%06ld (%06lx), end=%06ld (%06lx), size=%06ld (%06lx)",
            (long) gf.false_rumor_start, gf.false_rumor_start,
            gf.false_rumor_end, (unsigned long) gf.false_rumor_end,
            gf.false_rumor_size, (unsigned long) gf.false_rumor_size);
        putstr(tmpwin, 0, rumor_buf);

        /*
         * check the first rumor (start of true rumors) by
         * skipping the first two lines.
         *
         * Then seek to the start of the false rumors (based on
         * the value read in rumors, and display it.
         */
        rumor_buf[0] = '\0';
        (void) dlb_fseek(rumors, (long) gt.true_rumor_start, SEEK_SET);
        ftell_rumor_start = dlb_ftell(rumors);
        (void) dlb_fgets(line, sizeof line, rumors);
        if ((endp = strchr(line, '\n')) != 0)
            *endp = 0;
        Sprintf(rumor_buf, "T %06ld %s", ftell_rumor_start,
                xcrypt(line, xbuf));
        putstr(tmpwin, 0, rumor_buf);
        /* find last true rumor */
        while (dlb_fgets(line, sizeof line, rumors)
               && dlb_ftell(rumors) < gt.true_rumor_end)
            continue;
        if ((endp = strchr(line, '\n')) != 0)
            *endp = 0;
        Sprintf(rumor_buf, "  %6s %s", "", xcrypt(line, xbuf));
        putstr(tmpwin, 0, rumor_buf);

        rumor_buf[0] = '\0';
        (void) dlb_fseek(rumors, (long) gf.false_rumor_start, SEEK_SET);
        ftell_rumor_start = dlb_ftell(rumors);
        (void) dlb_fgets(line, sizeof line, rumors);
        if ((endp = strchr(line, '\n')) != 0)
            *endp = 0;
        Sprintf(rumor_buf, "F %06ld %s", ftell_rumor_start,
                xcrypt(line, xbuf));
        putstr(tmpwin, 0, rumor_buf);
        /* find last false rumor */
        while (dlb_fgets(line, sizeof line, rumors)
               && dlb_ftell(rumors) < gf.false_rumor_end)
            continue;
        if ((endp = strchr(line, '\n')) != 0)
            *endp = 0;
        Sprintf(rumor_buf, "  %6s %s", "", xcrypt(line, xbuf));
        putstr(tmpwin, 0, rumor_buf);

        (void) dlb_fclose(rumors);

    /* if a previous attempt couldn't open file or rejected its contents,
       we didn't bother trying again this time */
    } else if (gt.true_rumor_size < 0L) {
 no_rumors: /* file could be opened but init_rumors() didn't like it */
        pline("rumors not accessible.");
        /* engravings, epitaphs, and bogus monsters will still be shown,
           and in tmpwin rather than via additional pline() calls */
        display_nhwindow(WIN_MESSAGE, TRUE); /* --more-- */

    /* first attempt to open file has just failed */
    } else {
        couldnt_open_file(RUMORFILE);
        gt.true_rumor_size = -1; /* don't try to open it again */
    }

    /* initial implementation of default epitaph/engraving/bogusmon
       contained an error; check those along with rumors */
    others_check("Engravings:", ENGRAVEFILE, &tmpwin);
    others_check("Epitaphs:", EPITAPHFILE, &tmpwin);
    others_check("Bogus monsters:", BOGUSMONFILE, &tmpwin);

    if (tmpwin != WIN_ERR) {
        display_nhwindow(tmpwin, TRUE);
        destroy_nhwindow(tmpwin);
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Check and report on one of the simpler data files.
 *
 * The epitaph, engraving and hallucinatory-name files have no header of offsets -- every line is an entry, after a comment line. So checking them is a matter of confirming that structure and showing enough entries for a person to see
 * whether the contents look right: the first two and the last, with the total count.
 *
 * @param ftype the heading to show
 * @param fname the file to check
 * @param winptr the window to write into; created here if there is none yet, and the caller receives it
 * @note Creates the window only when there is something to report, and shares one window across all three files, which is why the caller passes a pointer rather than a window. A separate window per file would make the report three
 *       dismissals long.
 * @note A first line that is not a comment is shown both as it is and decrypted, because as the existing comment concedes there is no way to tell which state the file is in -- and a file mistakenly left unencrypted looks exactly like a
 *       corrupt one.
 * @note Cannot verify what the first line says, only that it exists. The tool that builds the file inserts a known line, but as the existing comment records that value is not available here.
 * @note Complains rather than ending the game when the window cannot be made, deliberately, as the existing comment notes: this is a wizard-mode inspection and should not be able to end a game.
 */
/**
 * @brief 더 단순한 데이터 파일 중 하나를 검사하고 보고한다.
 *
 * 묘비명, 각인, 환각 이름 파일은 위치 머리글을 갖지 않는다. 주석 줄 뒤로 모든 줄이 항목이다. 그래서 그것들을 검사하는 것은 그 구조를 확인하고, 사람이 내용이 옳아 보이는지 볼 만큼의 항목을 보이는 문제다. 처음 둘과 마지막, 그리고 전체 개수.
 *
 * @param ftype 보일 제목
 * @param fname 검사할 파일
 * @param winptr 쓸 창. 아직 없으면 여기서 만들어지고 호출자가 그것을 받는다
 * @note 보고할 것이 있을 때만 창을 만들고, 세 파일 전부에 걸쳐 한 창을 공유한다. 그것이 호출자가 창이 아니라 포인터를 넘기는 이유다. 파일마다 따로 된 창은 보고를 세 번 닫아야 하는 길이로 만들 것이다.
 * @note 주석이 아닌 첫 줄은 그대로도 복호화해서도 보인다. 기존 주석이 인정하듯 파일이 어느 상태인지 말할 방법이 없기 때문이며, 실수로 암호화되지 않고 남겨진 파일은 손상된 파일과 정확히 똑같아 보인다.
 * @note 첫 줄이 무엇을 말하는지는 검증할 수 없고 그것이 존재하는지만 검증할 수 있다. 파일을 만드는 도구가 알려진 줄을 넣지만, 기존 주석이 기록하듯 그 값을 여기서 쓸 수 없다.
 * @note 기존 주석이 적듯 창을 만들 수 없을 때 의도적으로 게임을 끝내는 대신 불평한다. 이것은 마법사 모드 점검이며 게임을 끝낼 수 있어서는 안 된다.
 */
staticfn void
others_check(
    const char *ftype, /* header: "{Engravings|Epitaphs|Bogus monsters}:" */
    const char *fname, /* filename: {ENGRAVEFILE|EPITAPHFILE|BOGUSMONFILE} */
    winid *winptr)     /* text window for output; created here if necessary */
{
    static const char errfmt[] = "others_check(\"%s\"): %s";
    dlb *fh;
    char line[BUFSZ], xbuf[BUFSZ], *endp;
    winid tmpwin = *winptr;
    int entrycount = 0;

    fh = dlb_fopen(fname, "r");
    if (fh) {
        if (tmpwin == WIN_ERR) {
            *winptr = tmpwin = create_nhwindow(NHW_TEXT);
            if (tmpwin == WIN_ERR) {
                /* should panic, but won't for wizard mode check operation */
                impossible(errfmt, fname, "can't create temporary window");
                goto closeit;
            }
        }
        putstr(tmpwin, 0, "");
        putstr(tmpwin, 0, ftype);
        /* "don't edit" comment */
        *line = '\0';
        if (!dlb_fgets(line, sizeof line, fh)) {
            Sprintf(xbuf, errfmt, fname, "error; can't read comment line");
            putstr(tmpwin, 0, xbuf);
            goto closeit;
        }
        if (*line != '#') {
            Sprintf(xbuf, errfmt, fname,
                    "malformed; first line is not a comment line:");
            putstr(tmpwin, 0, xbuf);
            /* show the bad line; we don't know whether it has been
               encrypted via xcrypt() so show it both ways */
            if ((endp = strchr(line, '\n')) != 0)
                *endp = 0;
            putstr(tmpwin, 0, "- first line, as is");
            putstr(tmpwin, 0, line);
            putstr(tmpwin, 0, "- xcrypt of first line");
            putstr(tmpwin, 0, xcrypt(line, xbuf));
            goto closeit;
        }
        /* first line; should be default one inserted by makedefs when
           building the file but we don't have the expected value so
           can only require a line to exist */
        *line = '\0';
        if (!dlb_fgets(line, sizeof line, fh) || *line == '\n') {
            Sprintf(xbuf, errfmt, fname,
                    !*line ? "can't read first non-comment line"
                           : "first non-comment line is empty");
            putstr(tmpwin, 0, xbuf);
            goto closeit;
        }
        ++entrycount;
        if ((endp = strchr(line, '\n')) != 0)
            *endp = 0;
        putstr(tmpwin, 0, xcrypt(line, xbuf));
        if (!dlb_fgets(line, sizeof line, fh)) {
            putstr(tmpwin, 0, "(no second entry)");
        } else {
            ++entrycount;
            if ((endp = strchr(line, '\n')) != 0)
                *endp = 0;
            putstr(tmpwin, 0, xcrypt(line, xbuf));
            while (dlb_fgets(line, sizeof line, fh)) {
                ++entrycount;
                if ((endp = strchr(line, '\n')) != 0)
                    *endp = 0;
                (void) xcrypt(line, xbuf);
            }
            /* count will be 2 if the default entry and the first ordinary
               entry are the only ones present (if either of those were
               missing, we wouldn't have gotten here...) */
            if (entrycount == 2) {
                putstr(tmpwin, 0, "(only two entries)");
            } else {
                /* showing an ellipsis avoids ambiguity about whether
                   there are other lines; doing so three times (once for
                   each file) results in total output being 24 lines,
                   forcing a --More-- prompt if using a 24 line screen;
                   displaying 23 lines and --More-- followed by second
                   page with 1 line doesn't look very good but isn't
                   incorrect, and taller screens where that won't be an
                   issue are more common than 24 line terminals nowadays */
                if (entrycount > 3)
                    putstr(tmpwin, 0, " ...");
                putstr(tmpwin, 0, xbuf); /* already decrypted */
            }
        }

 closeit:
        (void) dlb_fclose(fh);
    } else {
        /* since this comes out via impossible(), it won't be integrated
           with the text window of values, but it shouldn't ever happen
           so we won't waste effort integrating it */
        couldnt_open_file(fname);
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Pick one line at random from a stretch of a file, without reading the file.
 *
 * The mechanism the whole file rests on. It seeks to a random byte, which almost always lands in the middle of a line, discards the remainder of that line and takes the next one. That costs one seek regardless of how large the file is,
 * which is why rumors are not held in memory.
 *
 * The method has a bias: a line following a long line is more likely to be chosen than one following a short line, because a longer line offers more bytes to land in. The padding is how that is corrected. When entries have been padded to
 * a known length, a landing position is accepted only if it falls within the last stretch of the line -- effectively treating every line as the same length -- so each line is equally likely. Without a padding length the bias remains, as
 * both this function's comment and the very old comment at the top of the file describe.
 *
 * @param fh the already-open file
 * @param buf where to write the line
 * @param bufsiz its size
 * @param rng which random number routine to use
 * @param startpos where the lines of interest begin
 * @param endpos one past where they end, or zero for end-of-file
 * @param padlength the length entries were padded to, or zero if they were not
 * @return the buffer, holding the line decrypted and with its padding and newline removed
 * @note The end position is treated exactly like end of file, and the existing comment gives the reason: a landing in the last true rumor would otherwise read on into the first false one, so asking for a truth would sometimes get a lie.
 * @note Landing past the end wraps back to the first line rather than failing. That makes the first line slightly likelier than the rest, which is accepted as the cost of never failing.
 * @note Retries a limited number of times when a padded landing is rejected, then takes whatever it has. The limit means the even distribution is approximate rather than exact -- but a bounded loop that is nearly even is preferable to an
 *       unbounded one that is exactly even.
 * @note An empty range returns an empty line silently. As the existing comment explains, complaining would repeat endlessly and the usual advice to save and restore would not help.
 * @note Decryption uses a temporary buffer, allocated only if the line is too long for the one on the stack. Lines that long are not expected, so the allocation is the unusual path rather than the normal one.
 */
/**
 * @brief 파일을 읽지 않고 파일의 한 구간에서 무작위로 한 줄을 고른다.
 *
 * 이 파일 전체가 놓여 있는 방식이다. 무작위 바이트로 건너뛰는데, 그것은 거의 언제나 어떤 줄 가운데에 떨어진다. 그 줄의 남은 부분을 버리고 다음 줄을 취한다. 그것은 파일이 얼마나 큰지와 무관하게 한 번의 건너뛰기를 들이며, 그것이 소문이 메모리에 담기지 않는 이유다.
 *
 * 그 방법에는 편향이 있다. 긴 줄 다음의 줄이 짧은 줄 다음의 줄보다 골라질 가능성이 크다. 더 긴 줄이 떨어질 바이트를 더 많이 내주기 때문이다. 채움이 그것을 바로잡는 방법이다. 항목이 알려진 길이로 채워졌을 때, 떨어진 위치는 그 줄의 마지막 구간 안에 들 때만 받아들여진다. 사실상 모든 줄을 같은 길이로 다루는 것이며, 그래서 각 줄이 똑같이 있을 법해진다. 채움 길이가 없으면 그 편향이 남는데, 이 함수의 주석과 파일 맨 위의 아주 오래된 주석이 모두 그것을 서술한다.
 *
 * @param fh 이미 열린 파일
 * @param buf 줄을 쓸 곳
 * @param bufsiz 그 크기
 * @param rng 어느 무작위 수 함수를 쓸지
 * @param startpos 관심 있는 줄들이 시작하는 곳
 * @param endpos 그것들이 끝나는 곳 하나 다음, 또는 파일 끝을 뜻하는 영
 * @param padlength 항목이 채워진 길이, 또는 채워지지 않았으면 영
 * @return 복호화되고 채움과 줄바꿈이 없어진 줄을 담은 그 버퍼
 * @note 끝 위치가 파일 끝과 정확히 똑같이 다뤄지며, 기존 주석이 그 이유를 준다. 마지막 참 소문에 떨어지면 그러지 않으면 첫 거짓 소문으로 이어 읽을 것이므로, 참을 요청하는 것이 이따금 거짓말을 얻을 것이다.
 * @note 끝을 넘어 떨어지는 것은 실패하는 대신 첫 줄로 되돌아 감싼다. 그것이 첫 줄을 나머지보다 조금 더 있을 법하게 만드는데, 결코 실패하지 않는 것의 값으로 받아들여진다.
 * @note 채워진 떨어짐이 거부될 때 한정된 횟수만큼 다시 시도하고, 그다음 가진 것을 그대로 취한다. 그 한계는 고른 분포가 정확한 것이 아니라 근사한 것이라는 뜻이다. 그러나 거의 고른 한정된 되돌기가 정확히 고른 한정 없는 되돌기보다 낫다.
 * @note 빈 범위는 조용히 빈 줄을 돌려준다. 기존 주석이 설명하듯 불평하는 것은 끝없이 되풀이될 것이고, 저장하고 복원하라는 보통의 조언은 도움이 되지 않을 것이다.
 * @note 복호화가 임시 버퍼를 쓰는데, 줄이 스택의 것에 너무 길 때만 할당된다. 그렇게 긴 줄은 예상되지 않으므로 그 할당은 보통의 길이 아니라 드문 길이다.
 */
staticfn char *
get_rnd_line(
    dlb *fh,            /* already opened file */
    char *buf,          /* output buffer */
    unsigned bufsiz,    /* (unsigned) sizeof buf */
    int (*rng)(int),    /* random number routine; rn2(N) or similar, 0..N-1 */
    long startpos,      /* location in file of first line of interest */
    long endpos,        /* location one byte past last line of interest;
                         * if 0, end-of-file will be used */
    unsigned padlength) /* expected line length; 0 if no expectations */
{
    char *newl, *xbufp, xbuf[BUFSZ];
    long filechunksize, chunkoffset;
    int trylimit;

    *buf = '\0';
    if (!endpos) {
        (void) dlb_fseek(fh, 0L, SEEK_END);
        endpos = dlb_ftell(fh);
    }
    filechunksize = endpos - startpos;

    /* might be zero (only if file is empty); should complain in that
       case but it could happen over and over, also the suggestion
       that save and restore might fix the problem wouldn't be useful */
    if (filechunksize < 1L)
        return buf;
    /* 'rumors' is about 3/4 of the way to the limit on a 16-bit config
       for the whole, roughly 3/8 of the way for either half; all active
       configurations these days are at least 32-bits anyway */
    nhassert(filechunksize <= INT_MAX); /* essential for rn2() */

    /*
     * Position randomly which will probably be in the middle of a line.
     * (Occasionally by chance it will happen to be at the very start of
     * a line, but we'll have no way of knowing that so have to behave
     * as if it were positioned in the middle.)
     * Read the rest of that line, then use the next one.  If there's no
     * next line (ie, end of file), go back to beginning and use first.
     *
     * When short lines have been padded to length N, only accept long
     * lines if we land within last N+1 characters (+1 is for newline
     * which hasn't been stripped away yet), effectively shortening
     * them to normal length.  That yields even selection distribution.
     */
    for (trylimit = 10; trylimit > 0; --trylimit) {
        chunkoffset = (long) (*rng)((int) filechunksize);
        (void) dlb_fseek(fh, startpos + chunkoffset, SEEK_SET);
        (void) dlb_fgets(buf, bufsiz, fh);
        /* if padlength is 0, accept any position; when non-zero,
           padlength does not count the newline but strlen(buf) does */
        if (!padlength || (unsigned) strlen(buf) <= padlength + 1)
            break;
    }
    /* use next line; for rumors, caller takes care of whether startpos
       and endpos cover just true rumors or just false rumors; reaching
       endpos is equivalent to end-of-file in order to avoid using the
       first false rumor if fseek for a true one lands within the last one */
    if (dlb_ftell(fh) >= endpos || !dlb_fgets(buf, bufsiz, fh)) {
        /* assume failure is due to end-of-file; go back to start */
        (void) dlb_fseek(fh, startpos, SEEK_SET);
        (void) dlb_fgets(buf, bufsiz, fh);
    }
    if ((newl = strchr(buf, '\n')) != 0)
        *newl = '\0';
    /* decrypt line; make sure that our intermediate buffer is big enough */
    xbufp = (strlen(buf) <= sizeof xbuf - 1) ? &xbuf[0]
            : (char *) alloc((unsigned) strlen(buf) + 1);
    Strcpy(buf, xcrypt(buf, xbufp));
    if (xbufp != &xbuf[0])
        free((genericptr_t) xbufp);
    /* strip padding that makedefs adds to short lines */
    if (padlength)
        unpadline(buf);
    return buf;
}

/**
 * @brief Pick a random line from one of the simple data files.
 *
 * What epitaphs, engravings and hallucinatory monster names come through. Unlike rumors these files have no sections, so the whole file after its comment line is one range and no header needs reading -- which is why this can open, draw and
 * close in one call while rumors need initialisation kept between calls.
 *
 * @param fname the file
 * @param buf where to write the line
 * @param rng which random number routine to use
 * @param padlength the length entries were padded to, or zero
 * @return the buffer, holding the line ready to show
 * @note The random routine is a parameter because some callers must not disturb the game's own random sequence -- a name shown while hallucinating should not change what happens next.
 * @note Returns an empty line if the file cannot be opened, having complained. Callers must have something to fall back on; the line is not guaranteed.
 */
/**
 * @brief 단순한 데이터 파일 중 하나에서 무작위 줄을 고른다.
 *
 * 묘비명, 각인, 환각 몬스터 이름이 나오는 곳이다. 소문과 달리 이 파일들은 구역을 갖지 않으므로, 주석 줄 뒤의 파일 전체가 하나의 범위이고 읽어야 할 머리글이 없다. 그것이 이것은 한 번의 호출로 열고 뽑고 닫을 수 있는데 소문은 호출 사이에 지켜지는 초기화를 필요로 하는 이유다.
 *
 * @param fname 그 파일
 * @param buf 줄을 쓸 곳
 * @param rng 어느 무작위 수 함수를 쓸지
 * @param padlength 항목이 채워진 길이, 또는 영
 * @return 보일 준비가 된 줄을 담은 그 버퍼
 * @note 무작위 함수가 인자인 것은, 어떤 호출자들은 게임 자신의 무작위 수열을 흐트러뜨려서는 안 되기 때문이다. 환각 중에 보이는 이름이 다음에 무엇이 일어나는지를 바꾸어서는 안 된다.
 * @note 파일을 열 수 없으면 불평한 뒤 빈 줄을 돌려준다. 호출자는 물러설 것을 가지고 있어야 한다. 그 줄은 보장되지 않는다.
 */
char *
get_rnd_text(
    const char *fname,
    char *buf,
    int (*rng)(int),
    unsigned padlength)
{
    dlb *fh = dlb_fopen(fname, "r");

    buf[0] = '\0';
    if (fh) {
        long starttxt = 0L;
        char line[BUFSZ];

        /* skip "don't edit" comment */
        (void) dlb_fgets(line, sizeof line, fh);
        /* obtain current file position */
        (void) dlb_fseek(fh, 0L, SEEK_CUR);
        starttxt = dlb_ftell(fh);

        /* get a randomly chosen line; it comes back decrypted and unpadded */
        Strcpy(buf, get_rnd_line(fh, line, (unsigned) sizeof line, rng,
                                 starttxt, 0L, padlength));
        (void) dlb_fclose(fh);
    } else {
        couldnt_open_file(fname);
    }
    return buf;
}

/**
 * @brief Deliver a rumor to the hero by whichever means is doing the delivering.
 *
 * How the rumor arrives changes what is said around it, not just the wrapping: the Oracle speaks the rumor aloud, a cookie announces a scrap of paper first, and paper is simply read. That is the whole reason the means is a parameter.
 *
 * @param truth 1 for true, -1 for false, 0 for either
 * @param mechanism which means is delivering it
 * @note Reading requires sight, so a blind hero gets told they cannot read it -- and still gets told the cookie contained something, because finding the paper does not require sight. A fainting hero eating a cookie gets nothing at all,
 *       since they are not in a position to notice.
 * @note Cookie rumors are excluded for every means except reading, because a rumor mentioning the cookie makes no sense spoken by the Oracle.
 * @note Falls back to a fixed line when no rumor could be obtained, so a broken data file produces a joke rather than silence -- silence at a fortune cookie would read as the game having failed.
 * @note The Oracle's delivery varies its adverb at random and speaks with her own voice. Wisdom is not exercised again here; getrumor() already did it, as the existing comment records.
 */
/**
 * @brief 전달하고 있는 수단이 무엇이든 그것으로 영웅에게 소문을 전한다.
 *
 * 소문이 어떻게 도착하는지가 그 둘레에서 무엇이 말해지는지를 바꾼다. 포장만이 아니다. 신탁은 소문을 소리 내어 말하고, 쿠키는 먼저 종이쪽을 알리고, 종이는 그냥 읽힌다. 그것이 수단이 인자인 이유 전부다.
 *
 * @param truth 참에 1, 거짓에 -1, 어느 쪽이든에 0
 * @param mechanism 어느 수단이 그것을 전하는지
 * @note 읽기는 볼 수 있어야 하므로, 눈먼 영웅은 그것을 읽을 수 없다는 말을 듣는다. 그리고 쿠키에 무언가가 들어 있었다는 말도 여전히 듣는다. 종이를 발견하는 것은 볼 수 있어야 하는 일이 아니기 때문이다. 쿠키를 먹으며 기절하고 있는 영웅은 아무것도 얻지 못한다. 그가 알아챌 처지가 아니기 때문이다.
 * @note 읽기를 뺀 모든 수단에서 쿠키 소문이 배제된다. 쿠키를 언급하는 소문은 신탁이 말할 때 뜻이 통하지 않기 때문이다.
 * @note 소문을 얻을 수 없었을 때 고정된 줄로 물러나므로, 망가진 데이터 파일이 침묵 대신 농담을 낸다. 포춘 쿠키에서의 침묵은 게임이 실패한 것으로 읽힐 것이다.
 * @note 신탁의 전달은 자기 부사를 무작위로 바꾸며 자기 목소리로 말한다. 기존 주석이 기록하듯 지혜는 여기서 다시 단련되지 않는다. getrumor()가 이미 그것을 했다.
 */
void
outrumor(
    int truth, /* 1=true, -1=false, 0=either */
    int mechanism)
{
    static const char fortune_msg[] =
        "This cookie has a scrap of paper inside.";
    const char *line;
    char buf[BUFSZ];
    boolean reading = (mechanism == BY_COOKIE || mechanism == BY_PAPER);

    if (reading) {
        /* deal with various things that prevent reading */
        if (is_fainted() && mechanism == BY_COOKIE) {
            return;
        } else if (Blind) {
            if (mechanism == BY_COOKIE)
                pline(fortune_msg);
            pline("What a pity that you cannot read it!");
            return;
        }
    }

    line = getrumor(truth, buf, reading ? FALSE : TRUE);
    if (!*line)
        line = "NetHack rumors file closed for renovation.";
    switch (mechanism) {
    case BY_ORACLE:
        /* Oracle delivers the rumor */
        pline("True to her word, the Oracle %s%s: ",
              (!rn2(4) ? "offhandedly "
                       : (!rn2(3) ? "casually "
                                  : (rn2(2) ? "nonchalantly " : ""))),
              says());
        SetVoice((struct monst *) 0, 0, 80, voice_oracle);
        verbalize1(line);
        /* [WIS exercised by getrumor()] */
        return;
    case BY_COOKIE:
        pline(fortune_msg);
        FALLTHROUGH;
    /* FALLTHRU */
    case BY_PAPER:
        pline("It reads:");
        break;
    }
    pline1(line);
}

/**
 * @brief Read the oracle file's table of offsets.
 *
 * Oracles differ from rumors in being several lines each, so a random byte offset would not do -- an oracle must be found at its start. The file therefore carries an explicit offset per oracle, and this reads them all into memory.
 *
 * @param fp the open oracle file
 * @note The list is held in the saved part of the game's state rather than beside the file, because an oracle is consumed once delivered and which ones remain must survive a save.
 * @note Assumes it is called only once, as the existing comment records. It allocates without freeing anything first, so a second call would lose the previous list.
 * @note A count that cannot be read, or is not positive, leaves the count at zero and allocates nothing. The caller must check the count rather than assuming the list exists.
 * @warning The offsets are read without checking that each one was parsed. A malformed line leaves that entry holding whatever it held before -- which for a freshly allocated list is unspecified, and seeking there would read from an
 *          arbitrary point in the file.
 */
/**
 * @brief 신탁 파일의 위치 표를 읽는다.
 *
 * 신탁은 각각 여러 줄이라는 점에서 소문과 다르므로, 무작위 바이트 위치로는 안 된다. 신탁은 자기 시작점에서 발견되어야 한다. 그래서 파일이 신탁마다 명시적인 위치를 지니며, 이것이 그것을 모두 메모리로 읽어 온다.
 *
 * @param fp 열린 신탁 파일
 * @note 그 목록은 파일 옆이 아니라 게임 상태의 저장되는 부분에 담긴다. 신탁은 한번 전해지면 소진되고, 어느 것이 남았는지가 저장을 넘어 살아남아야 하기 때문이다.
 * @note 기존 주석이 기록하듯 한 번만 호출된다고 가정한다. 무엇도 먼저 해제하지 않고 할당하므로, 두 번째 호출은 앞선 목록을 잃을 것이다.
 * @note 읽을 수 없거나 양수가 아닌 개수는 개수를 영으로 남기고 아무것도 할당하지 않는다. 호출자는 목록이 존재한다고 가정하는 대신 개수를 검사해야 한다.
 * @warning 위치들은 각각이 해석되었는지 검사되지 않고 읽힌다. 잘못된 줄은 그 항목이 앞서 담고 있던 무엇이든 담은 채로 남기며, 갓 할당된 목록에 대해 그것은 명시되지 않은 값이고, 거기로 건너뛰는 것은 파일의 임의 지점에서 읽을 것이다.
 */
staticfn void
init_oracles(dlb *fp)
{
    int i;
    char line[BUFSZ];
    int cnt = 0;

    /* this assumes we're only called once */
    (void) dlb_fgets(line, sizeof line, fp); /* skip "don't edit" comment*/
    (void) dlb_fgets(line, sizeof line, fp);
    if (sscanf(line, "%5d\n", &cnt) == 1 && cnt > 0) {
        svo.oracle_cnt = (unsigned) cnt;
        svo.oracle_loc = (unsigned long *) alloc((unsigned) cnt * sizeof(long));
        for (i = 0; i < cnt; i++) {
            (void) dlb_fgets(line, sizeof line, fp);
            (void) sscanf(line, "%5lx\n", &svo.oracle_loc[i]);
        }
    }
    return;
}

/**
 * @brief Write out which oracles remain undelivered, and optionally release the list.
 *
 * The list must be saved because it shrinks as the game goes on: a delivered oracle is removed so it cannot be heard twice. Without saving it, restoring a game would restore every oracle the hero had already paid for.
 *
 * @param nhfp the save file
 * @note Writing and releasing are separate decisions, both taken from the file's own state. A save that keeps playing writes without releasing; ending the game releases without necessarily writing.
 * @note Clears the initialised flag along with the count when releasing, so a later request reads the file again rather than consulting a freed list.
 * @note The offsets are written one at a time rather than as a block, which is what keeps the save readable by a build with a different integer width.
 */
/**
 * @brief 어느 신탁이 아직 전해지지 않았는지 써내고, 선택적으로 목록을 해제한다.
 *
 * 그 목록은 게임이 진행되며 줄어들기 때문에 저장되어야 한다. 전해진 신탁은 두 번 들릴 수 없도록 제거된다. 그것을 저장하지 않으면, 게임을 복원하는 것이 영웅이 이미 값을 치른 모든 신탁을 복원할 것이다.
 *
 * @param nhfp 저장 파일
 * @note 쓰기와 해제하기는 따로 된 결정이며, 둘 다 파일 자신의 상태에서 취해진다. 계속 진행하는 저장은 해제 없이 쓰고, 게임을 끝내는 것은 반드시 쓰지 않으면서 해제한다.
 * @note 해제할 때 개수와 함께 초기화 표시도 지우므로, 나중의 요청이 해제된 목록을 참조하는 대신 파일을 다시 읽는다.
 * @note 위치들이 덩이가 아니라 하나씩 쓰이며, 그것이 저장 파일을 다른 정수 너비의 빌드가 읽을 수 있게 유지하는 것이다.
 */
void
save_oracles(NHFILE *nhfp)
{
    int i;

    if (update_file(nhfp)) {
        Sfo_unsigned(nhfp, &svo.oracle_cnt, "oracle-oracle_cnt");
        if (svo.oracle_cnt) {
            for (i = 0; (unsigned) i < svo.oracle_cnt; ++i) {
                Sfo_ulong(nhfp, &svo.oracle_loc[i], "oracle-oracle_loc");
            }
        }
    }
    if (release_data(nhfp)) {
        if (svo.oracle_cnt) {
            svo.oracle_cnt = 0, go.oracle_flg = 0;
        }
        if (svo.oracle_loc) {
            free((genericptr_t) svo.oracle_loc);
            svo.oracle_loc = 0;
        }
    }
}
#endif /* !SFCTOOL */

/**
 * @brief Read back which oracles remain undelivered.
 * @param nhfp the save file
 * @note Marks the list as initialised, so the file is not read again -- as the existing comment records, init_oracles() need not be called. That matters for correctness, not speed: reading the file again would restore the full list and
 *       give back oracles the hero has already heard.
 * @note Built outside the excluded region, unlike its saving counterpart, because the format-conversion tool must be able to read a save file even though it never writes or delivers an oracle.
 * @note A saved count of zero leaves the list unallocated and the flag unset, so the file will be read on first use. That is the correct outcome for a game saved before any oracle was consulted.
 */
/**
 * @brief 어느 신탁이 아직 전해지지 않았는지 되읽는다.
 * @param nhfp 저장 파일
 * @note 목록을 초기화된 것으로 표시하므로 파일이 다시 읽히지 않는다. 기존 주석이 기록하듯 init_oracles()가 호출될 필요가 없다. 그것은 속도가 아니라 옳음에 중요하다. 파일을 다시 읽는 것은 전체 목록을 복원하고 영웅이 이미 들은 신탁을 되돌려줄 것이다.
 * @note 저장하는 짝과 달리 제외된 구역 밖에 빌드된다. 형식 변환 도구가 신탁을 쓰거나 전하는 일이 결코 없어도 저장 파일을 읽을 수 있어야 하기 때문이다.
 * @note 저장된 개수가 영이면 목록을 할당되지 않은 채로, 표시를 설정되지 않은 채로 남기므로 처음 쓸 때 파일이 읽힐 것이다. 그것은 어떤 신탁도 찾기 전에 저장된 게임에 대해 옳은 결과다.
 */
void
restore_oracles(NHFILE *nhfp)
{
    int i;

    Sfi_unsigned(nhfp, &svo.oracle_cnt, "oracle-oracle_cnt");
    if (svo.oracle_cnt) {
        svo.oracle_loc =
            (unsigned long *) alloc(svo.oracle_cnt * sizeof (unsigned long));
        for (i = 0; (unsigned) i < svo.oracle_cnt; ++i) {
            Sfi_ulong(nhfp, &svo.oracle_loc[i], "oracle-oracle_loc");
        }
        go.oracle_flg = 1; /* no need to call init_oracles() */
    }
}

#ifndef SFCTOOL
/**
 * @brief Deliver one oracle and remove it from those remaining.
 *
 * An oracle is consumed by being delivered. That is the point of the whole offset list: a hero who pays repeatedly hears something new each time, and eventually there is nothing left to hear.
 *
 * Removal is done by moving the last remaining offset into the delivered one's slot and shortening the list. The order of the remaining oracles is not preserved, which does not matter because they are chosen at random anyway.
 *
 * @param special whether this is the one major oracle rather than a minor one
 * @param delphi whether the Oracle herself is speaking, as opposed to the text being found written
 * @note The major oracle is the first entry and is not consumed -- the same one is delivered every time it is paid for, which is why it is excluded from the shortening. As the code's comment records, the remaining entries are the minor
 *       ones.
 * @note Reading the same major oracle again is intentional: it is the one piece of genuine guidance the game gives, and a hero who paid for it should be able to hear it again.
 * @note An unopenable file is recorded so it is not retried, and an exhausted list returns immediately. Both checks happen before the file is opened, so a hero who has heard everything is not made to wait on a file read.
 * @note Each oracle's text ends at a separator line rather than at a recorded length, so the text may be any number of lines.
 */
/**
 * @brief 신탁 하나를 전하고 그것을 남은 것들에서 제거한다.
 *
 * 신탁은 전해짐으로써 소진된다. 그것이 위치 목록 전체의 요점이다. 되풀이해 값을 치르는 영웅은 매번 새로운 것을 듣고, 마침내 들을 것이 남지 않는다.
 *
 * 제거는 남은 마지막 위치를 전해진 것의 자리로 옮기고 목록을 짧게 함으로써 이루어진다. 남은 신탁들의 순서는 보존되지 않는데, 그것들이 어차피 무작위로 골라지므로 문제가 되지 않는다.
 *
 * @param special 이것이 작은 것이 아니라 하나의 큰 신탁인지
 * @param delphi 문구가 쓰인 채로 발견되는 것이 아니라 신탁 자신이 말하고 있는지
 * @note 큰 신탁은 첫 항목이며 소진되지 않는다. 값이 치러질 때마다 같은 것이 전해지며, 그것이 그것이 짧게 하기에서 제외되는 이유다. 코드의 주석이 기록하듯 남은 항목들이 작은 것들이다.
 * @note 같은 큰 신탁을 다시 읽는 것은 의도된 것이다. 그것은 게임이 주는 진짜 안내의 한 조각이며, 그것에 값을 치른 영웅은 그것을 다시 들을 수 있어야 한다.
 * @note 열 수 없는 파일은 다시 시도되지 않도록 기록되고, 소진된 목록은 곧바로 돌아온다. 두 검사 모두 파일이 열리기 전에 일어나므로, 모든 것을 들은 영웅이 파일 읽기를 기다리게 되지 않는다.
 * @note 각 신탁의 문구는 기록된 길이가 아니라 구분 줄에서 끝나므로, 그 문구는 몇 줄이든 될 수 있다.
 */
void
outoracle(boolean special, boolean delphi)
{
    winid tmpwin;
    dlb *oracles;
    int oracle_idx;
    char *endp, line[COLNO], xbuf[BUFSZ];

    /* early return if we couldn't open ORACLEFILE on previous attempt,
       or if all the oracularities are already exhausted */
    if (go.oracle_flg < 0 || (go.oracle_flg > 0 && svo.oracle_cnt == 0))
        return;

    oracles = dlb_fopen(ORACLEFILE, "r");

    if (oracles) {
        if (go.oracle_flg == 0) { /* if this is the first outoracle() */
            init_oracles(oracles);
            go.oracle_flg = 1;
            if (svo.oracle_cnt == 0)
                goto close_oracles;
        }
        /* oracle_loc[0] is the special oracle;
           oracle_loc[1..oracle_cnt-1] are normal ones */
        if (svo.oracle_cnt <= 1 && !special)
            goto close_oracles; /*(shouldn't happen)*/
        oracle_idx = special ? 0 : rnd((int) svo.oracle_cnt - 1);
        (void) dlb_fseek(oracles, (long) svo.oracle_loc[oracle_idx], SEEK_SET);
        if (!special) /* move offset of very last one into this slot */
            svo.oracle_loc[oracle_idx] = svo.oracle_loc[--svo.oracle_cnt];

        tmpwin = create_nhwindow(NHW_TEXT);
        if (delphi)
            putstr(tmpwin, 0,
                   special
                     ? "The Oracle scornfully takes all your gold and says:"
                     : "The Oracle meditates for a moment and then intones:");
        else
            putstr(tmpwin, 0, "The message reads:");
        putstr(tmpwin, 0, "");

        while (dlb_fgets(line, COLNO, oracles) && strcmp(line, "---\n")) {
            if ((endp = strchr(line, '\n')) != 0)
                *endp = 0;
            putstr(tmpwin, 0, xcrypt(line, xbuf));
        }
        display_nhwindow(tmpwin, TRUE);
        destroy_nhwindow(tmpwin);
 close_oracles:
        (void) dlb_fclose(oracles);
    } else {
        couldnt_open_file(ORACLEFILE);
        go.oracle_flg = -1; /* don't try to open it again */
    }
}

/**
 * @brief Pay the Oracle for a consultation and receive it.
 *
 * Two grades are offered. A minor consultation is cheap and yields a true rumor. A major one costs far more -- and more the higher the hero's experience -- and yields one of the multi-line oracles.
 *
 * @param oracl the Oracle
 * @return whether time passed
 * @note A hero who cannot afford the major price is not refused. They pay what they have and receive the major oracle, which is the same one every time and which the game itself describes as being taken scornfully. So underpaying is
 *       possible but pointless, and the message says so.
 * @note Underpaying also forfeits the experience for a first major consultation and exercises wisdom unfavourably. That is the mechanical statement of the same judgement the message makes in words.
 * @note Experience is granted only for the first consultation of each grade, and much less if the other grade was already done. The consultation is worth something as a discovery, not as a repeatable source of experience.
 * @note The major consultation is not offered when only one oracle remains or the file could not be read. The one remaining is the major one, which the hero can already hear, so offering to sell it as new would be taking money for
 *       nothing.
 * @note Refuses before asking for money when the Oracle is hostile or the hero has none, so a hero with an empty purse is not walked through the prices.
 */
/**
 * @brief 신탁에게 상담 값을 치르고 그것을 받는다.
 *
 * 두 등급이 제공된다. 작은 상담은 값이 싸고 참인 소문을 낸다. 큰 상담은 훨씬 더 비싸고 영웅의 경험이 높을수록 더 비싸며, 여러 줄로 된 신탁 중 하나를 낸다.
 *
 * @param oracl 신탁
 * @return 시간이 지났는지
 * @note 큰 값을 치를 수 없는 영웅은 거절되지 않는다. 그는 가진 것을 치르고 큰 신탁을 받는데, 그것은 매번 같은 것이며 게임 자신이 비웃으며 받아 간다고 서술하는 것이다. 그러니 덜 치르는 것은 가능하지만 무의미하고, 그 메시지가 그렇게 말한다.
 * @note 덜 치르는 것은 또한 첫 큰 상담에 대한 경험을 잃게 하고 지혜를 불리하게 단련한다. 그것은 그 메시지가 말로 하는 것과 같은 판단의 기계적 진술이다.
 * @note 경험은 각 등급의 첫 상담에만 주어지고, 다른 등급이 이미 이루어졌으면 훨씬 적게 주어진다. 상담은 되풀이할 수 있는 경험의 원천이 아니라 발견으로서 값어치가 있다.
 * @note 신탁이 하나만 남았거나 파일을 읽을 수 없었을 때 큰 상담은 제공되지 않는다. 남은 하나가 큰 것이고 영웅은 이미 그것을 들을 수 있으므로, 그것을 새것으로 파겠다고 하는 것은 아무것도 아닌 것에 돈을 받는 일일 것이다.
 * @note 신탁이 적대적이거나 영웅이 돈이 없을 때 돈을 요구하기 전에 거절하므로, 빈 지갑의 영웅이 값을 하나하나 듣게 되지 않는다.
 */
int
doconsult(struct monst *oracl)
{
    long umoney;
    int u_pay, minor_cost = 50, major_cost = 500 + 50 * u.ulevel;
    int add_xpts;
    char qbuf[QBUFSZ];

    gm.multi = 0;
    umoney = money_cnt(gi.invent);

    if (!oracl) {
        There("is no one here to consult.");
        return ECMD_OK;
    } else if (!oracl->mpeaceful) {
        pline("%s is in no mood for consultations.", Monnam(oracl));
        return ECMD_OK;
    } else if (!umoney) {
        You("have no gold.");
        return ECMD_OK;
    }

    Sprintf(qbuf, "\"Wilt thou settle for a minor consultation?\" (%d %s)",
            minor_cost, currency((long) minor_cost));
    switch (ynq(qbuf)) {
    default:
    case 'q':
        return ECMD_OK;
    case 'y':
        if (umoney < (long) minor_cost) {
            You("don't even have enough gold for that!");
            return ECMD_OK;
        }
        u_pay = minor_cost;
        break;
    case 'n':
        if (umoney <= (long) minor_cost /* don't even ask */
            || (svo.oracle_cnt == 1 || go.oracle_flg < 0))
            return ECMD_OK;
        Sprintf(qbuf, "\"Then dost thou desire a major one?\" (%d %s)",
                major_cost, currency((long) major_cost));
        if (y_n(qbuf) != 'y')
            return ECMD_OK;
        u_pay = (umoney < (long) major_cost) ? (int) umoney : major_cost;
        break;
    }
    money2mon(oracl, (long) u_pay);
    disp.botl = TRUE;
    if (!u.uevent.major_oracle && !u.uevent.minor_oracle)
        record_achievement(ACH_ORCL);
    add_xpts = 0; /* first oracle of each type gives experience points */
    if (u_pay == minor_cost) {
        outrumor(1, BY_ORACLE);
        if (!u.uevent.minor_oracle)
            add_xpts = u_pay / (u.uevent.major_oracle ? 25 : 10);
        /* 5 pts if very 1st, or 2 pts if major already done */
        u.uevent.minor_oracle = TRUE;
    } else {
        boolean cheapskate = u_pay < major_cost;

        outoracle(cheapskate, TRUE);
        if (!cheapskate && !u.uevent.major_oracle)
            add_xpts = u_pay / (u.uevent.minor_oracle ? 25 : 10);
        /* ~100 pts if very 1st, ~40 pts if minor already done */
        u.uevent.major_oracle = TRUE;
        exercise(A_WIS, !cheapskate);
    }
    if (add_xpts) {
        more_experienced(add_xpts, u_pay / 50);
        newexplevel();
    }
    return ECMD_TIME;
}

/**
 * @brief Complain that a data file could not be opened, without suggesting a save and restore.
 *
 * The general complaint machinery advises the player to save and restore, because most internal errors are transient state that a reload clears. A missing data file is not: the file will still be missing, and the advice would send the
 * player through a pointless exercise. So this reports the problem while temporarily withdrawing the claim that there is anything worth saving.
 *
 * @param filename the file
 * @note The suppression is skipped when the fuzzer is running, as the existing comment records, because there the complaint is escalated to an outright failure and that escalation is the point.
 * @note Restores the previous setting afterwards rather than leaving it cleared, since the game genuinely does have state worth saving -- it just cannot be fixed by saving it.
 */
/**
 * @brief 데이터 파일을 열 수 없었다고 불평하되, 저장하고 복원하기를 권하지 않는다.
 *
 * 일반적인 불평 기계는 플레이어에게 저장하고 복원하라고 권한다. 대부분의 내부 오류가 다시 불러오기로 지워지는 일시적 상태이기 때문이다. 없는 데이터 파일은 그렇지 않다. 파일은 여전히 없을 것이고, 그 권유는 플레이어를 무의미한 수고로 보낼 것이다. 그래서 이것은 저장할 가치가 있는 것이 있다는 주장을 잠시 철회하면서 문제를 보고한다.
 *
 * @param filename 그 파일
 * @note 기존 주석이 기록하듯 퍼저가 돌고 있을 때는 그 억제가 건너뛰어진다. 거기서는 불평이 완전한 실패로 격상되며, 그 격상이 요점이기 때문이다.
 * @note 뒤에 앞선 설정을 지워진 채로 남기는 대신 되돌린다. 게임은 진짜로 저장할 가치가 있는 상태를 갖는다. 다만 그것을 저장함으로써 이것이 고쳐질 수 없을 뿐이다.
 */
staticfn void
couldnt_open_file(const char *filename)
{
    int save_something = program_state.something_worth_saving;

    /* most likely the file is missing, so suppress impossible()'s
       "saving and restoring might fix this" (unless the fuzzer,
       which escalates impossible to panic, is running) */
    if (!iflags.debug_fuzzer)
        program_state.something_worth_saving = 0;

    impossible("Can't open '%s' file.", filename);
    program_state.something_worth_saving = save_something;
}

/**
 * @brief Does this capitalised word name a monster that takes "the"?
 *
 * Asked when a string must be given an article and there is no context to decide from -- only the text itself. A capital letter alone does not settle it: "Mordor Orc" is a kind of thing and takes "the", while "Izchak" is a person and takes
 * nothing. The distinction is drawn from the monster data, where a unique monster known by a title rather than a personal name is marked as taking an article.
 *
 * @param word the word, possibly followed by more text
 * @return whether it is such a name
 * @note Answered from a cached list rather than by scanning the monster table each time, as the existing comment records. The list is built on first use, so the first question costs a file read and the rest cost a walk of a few dozen
 *       entries.
 * @note Matches whole words only. As the existing comment explains, "Foo" matches "Foo", "Foo bar" and "Foo's bar" but not "Foobar" -- so a name that is a prefix of an unrelated word does not produce a false article.
 * @note Matching is case-sensitive, deliberately: the capital letter is part of what is being tested, and a case-insensitive match would accept the lower-case common noun that shares the name.
 * @note A word that does not start with a capital is refused immediately without building the list, which is the common case.
 * @note Simpler than the general name-to-monster lookup because it needs only a yes or no -- not the longest match, not the gender, not where the name ends.
 */
/**
 * @brief 이 대문자로 시작하는 낱말이 "the"를 취하는 몬스터를 이름 짓는가?
 *
 * 어떤 문자열에 관사가 주어져야 하는데 판단할 문맥이 없고 그 글 자체만 있을 때 물어진다. 대문자만으로는 정해지지 않는다. "모르도르 오크"는 사물의 한 종류이고 "the"를 취하며, "이즈착"은 사람이고 아무것도 취하지 않는다. 그 구별은 몬스터 데이터에서 끌어와지는데, 거기서 개인 이름이 아니라 직함으로 알려진 유일한 몬스터가 관사를 취하는 것으로 표시되어 있다.
 *
 * @param word 그 낱말. 뒤에 더 많은 글이 따를 수 있다
 * @return 그것이 그런 이름인지
 * @note 기존 주석이 기록하듯 매번 몬스터 표를 훑는 대신 저장된 목록에서 답한다. 목록은 처음 쓸 때 만들어지므로, 첫 질문은 파일 읽기를 들이고 나머지는 수십 항목의 걸어가기를 들인다.
 * @note 온전한 낱말만 맞춘다. 기존 주석이 설명하듯 "Foo"는 "Foo", "Foo bar", "Foo's bar"와 맞지만 "Foobar"와는 맞지 않는다. 그래서 무관한 낱말의 앞부분인 이름이 잘못된 관사를 내지 않는다.
 * @note 맞추기가 의도적으로 대소문자를 가린다. 대문자가 검사되는 것의 일부이며, 대소문자를 가리지 않는 맞추기는 그 이름을 공유하는 소문자 보통 명사를 받아들일 것이다.
 * @note 대문자로 시작하지 않는 낱말은 목록을 만들지 않고 곧바로 거부되며, 그것이 흔한 경우다.
 * @note 일반적인 이름에서 몬스터 찾기보다 단순하다. 그것은 예 또는 아니오만 필요하기 때문이다. 가장 긴 맞음도, 성별도, 이름이 어디서 끝나는지도 필요하지 않다.
 */
boolean
CapitalMon(
    const char *word) /* potential monster name; a name might be followed by
                       * something like " corpse" */
{
    const char *nam;
    unsigned i, wln, nln;

    if (!word || !*word || *word == lowc(*word))
        return FALSE; /* 'word' is not a capitalized monster name */

    if (!CapMons)
        init_CapMons();
    assert(CapMons != 0);

    wln = (unsigned) strlen(word);
    for (i = 0; i < CapMonSiz - 1; ++i) {
        nam = CapMons[i];
        nln = (unsigned) strlen(nam);
        if (wln < nln)
            continue;
        /*
         * Unlike name_to_mon(), we don't need to find the longest match
         * or return the gender or a pointer to trailing stuff.  We do
         * check full words though: "Foo" matches "Foo" and "Foo bar" and
         * "Foo's bar" but not "Foobar".  We use case-sensitive matching.
         */
        if (!strncmp(nam, word, nln)
            && (!word[nln] || word[nln] == ' ' || word[nln] == '\''))
            return TRUE; /* 'word' is a capitalized monster name */
    }
    return FALSE;
}

/**
 * @brief Build the list of capitalised monster names that take an article.
 *
 * Two sources, gathered in that order and kept in that order: the real monster table, then the hallucinatory names file. The order matters because freeing depends on it -- the first stretch is names owned by the monster table and the second
 * is copies owned by this list.
 *
 * Done in two passes over both sources: the first counts, then the list is allocated, then the second fills it. Counting first is necessary because the number of names is not known in advance -- both sources are data that can be edited.
 *
 * @note Unique monsters are included only when they are known by a title rather than a personal name, since only those take an article.
 * @note Every gender's name is considered, not just one, because the two can differ in capitalisation.
 * @note The hallucinatory file must be rewound between passes. It is harmless on the first pass and essential on the second, as the existing comment records.
 * @note Hallucinatory names carry a leading code saying what sort of name they are, which is stripped before the name is judged. Names marked as personal are excluded here, the same rule applied to the real monsters.
 * @note A terminating null entry is added although nothing relies on it. As the existing comment explains, its real purpose is to guarantee the list is never of zero size, which could happen if both sources were edited to contain no
 *       applicable names.
 * @note Frees any existing list first, described as a precaution. It should not be needed, since this runs once, but rebuilding over the old list would leak the hallucinatory copies.
 * @note There is a debugging dump of the finished list, and the comment above it describes the surprisingly indirect sequence needed to trigger it -- a consequence of the list being built only on demand.
 */
/**
 * @brief 관사를 취하는 대문자 몬스터 이름 목록을 만든다.
 *
 * 두 원천이 그 순서로 모아지고 그 순서로 유지된다. 실제 몬스터 표, 그다음 환각 이름 파일. 그 순서가 중요한 것은 해제가 그것에 달려 있기 때문이다. 첫 구간은 몬스터 표가 소유한 이름이고 두 번째는 이 목록이 소유한 사본이다.
 *
 * 두 원천에 대해 두 번 지나가며 이루어진다. 첫 번째가 세고, 그다음 목록이 할당되고, 그다음 두 번째가 그것을 채운다. 먼저 세는 것이 필요한 것은 이름의 개수가 미리 알려지지 않기 때문이다. 두 원천 모두 편집될 수 있는 데이터다.
 *
 * @note 유일한 몬스터는 개인 이름이 아니라 직함으로 알려질 때만 포함된다. 그런 것들만 관사를 취하기 때문이다.
 * @note 하나가 아니라 모든 성별의 이름이 고려된다. 그 둘이 대문자 여부에서 다를 수 있기 때문이다.
 * @note 환각 파일은 지나가기 사이에 되감겨야 한다. 기존 주석이 기록하듯 첫 번째 지나가기에서는 무해하고 두 번째에서는 필수적이다.
 * @note 환각 이름은 자기가 어떤 종류의 이름인지 말하는 앞선 부호를 지니며, 그것은 이름이 판단되기 전에 떼어진다. 개인 이름으로 표시된 이름은 여기서 배제되며, 실제 몬스터에 적용된 것과 같은 규칙이다.
 * @note 무엇도 그것에 의존하지 않지만 종료 널 항목이 더해진다. 기존 주석이 설명하듯 그것의 진짜 목적은 목록이 결코 크기 영이 되지 않도록 보장하는 것이며, 두 원천이 해당하는 이름을 하나도 담지 않도록 편집되면 그것이 일어날 수 있다.
 * @note 예방책이라고 서술되며 기존 목록을 먼저 해제한다. 이것이 한 번 실행되므로 필요하지 않아야 하지만, 옛 목록 위에 다시 만드는 것은 환각 사본을 새게 할 것이다.
 * @note 완성된 목록의 디버깅용 출력이 있으며, 그 위의 주석이 그것을 일으키는 데 필요한 놀랍도록 우회적인 절차를 서술한다. 목록이 요청될 때만 만들어지는 것의 결과다.
 */
staticfn void
init_CapMons(void)
{
    unsigned pass;
    dlb *bogonfile = dlb_fopen(BOGUSMONFILE, "r");

    if (CapMons) /* sanity precaution */
        free_CapMons();

    /* first pass: count the number of relevant monster names, then
       allocate memory for CapMons[]; second pass: populate CapMons[] */
    for (pass = 1; pass <= 2; ++pass) {
        struct permonst *mptr;
        const char *nam;
        unsigned mndx, mgend;

        /* the first CapMonstCnt entries come from mons[].pmnames[] and
           the next CapBogonCnt entries from the 'bogusmons' file;
           there is an extra entry for Null at the end, but that is only
           useful to force non-zero array size in case both mons[] and
           bogusmons get modified to have no applicable monster names */
        CapMonstCnt = CapBogonCnt = 0;

        /* gather applicable actual monsters */
        for (mndx = LOW_PM; mndx < NUMMONS; ++mndx) {
            mptr = &mons[mndx];
            if ((mptr->geno & G_UNIQ) != 0 && !the_unique_pm(mptr))
                continue;
            for (mgend = MALE; mgend < NUM_MGENDERS; ++mgend) {
                nam = mptr->pmnames[mgend];
                if (nam && *nam != lowc(*nam)) {
                    if (pass == 2)
                        CapMons[CapMonstCnt] = nam;
                    ++CapMonstCnt;
                }
            }
        }

        /* now gather applicable hallucinatory monsters */
        if (bogonfile) {
            char hline[BUFSZ], xbuf[BUFSZ], *endp, *startp, code;

            /* rewind; effectively a no-op for pass 1; essential for pass 2 */
            (void) dlb_fseek(bogonfile, 0L, SEEK_SET);
            /* skip "don't edit" comment (first line of file) */
            (void) dlb_fgets(hline, sizeof hline, bogonfile);

            /* one monster name per line in rudimentary encrypted format;
               some are prefixed by a classification code to indicate
               gender and/or to distinguish an individual from a type
               (code is a single punctuation character when present) */
            while (dlb_fgets(hline, sizeof hline, bogonfile)) {
                if ((endp = strchr(hline, '\n')) != 0)
                    *endp = '\0'; /* strip newline */
                (void) xcrypt(hline, xbuf);
                unpadline(xbuf);

                if (!xbuf[0] || !strchr(bogon_codes, xbuf[0]))
                    code = '\0', startp = &xbuf[0]; /* ordinary */
                else
                    code = xbuf[0], startp = &xbuf[1]; /* special */

                if (*startp != lowc(*startp) && !bogon_is_pname(code)) {
                    if (pass == 2)
                        CapMons[CapMonstCnt + CapBogonCnt] = dupstr(startp);
                    ++CapBogonCnt;
                }
            }
        }

        /* finish the current pass */
        if (pass == 1) {
            CapMonSiz = CapMonstCnt + CapBogonCnt + 1; /* +1: terminator */
            CapMons = (const char **) alloc(CapMonSiz * sizeof *CapMons);
        } else { /* pass == 2 */
            /* terminator; not strictly needed */
            CapMons[CapMonSiz - 1] = (const char *) 0;

            if (bogonfile)
                (void) dlb_fclose(bogonfile), bogonfile = (dlb *) 0;
        }
    }
#ifdef DEBUG
    /*
     * CapMons[] init doesn't kick in until needed.  To force this name
     * dump, set DEBUGFILES to "CapMons" in your environment (or in
     * sysconf) prior to starting nethack, wish for a statue of an Archon
     * and drop it if held, then step away and apply a stethoscope towards
     * it to trigger a message that passes "Archon" to the() which will
     * then call CapitalMon() which in turn will call init_CapMons().
     */
    if (wizard && explicitdebug("CapMons")) {
        char buf[BUFSZ];
        unsigned i;
        winid tmpwin = create_nhwindow(NHW_TEXT);

        putstr(tmpwin, 0,
              "Capitalized monster type names normally preceded by \"the\":");
        for (i = 0; i < CapMonSiz - 1; ++i) {
            Sprintf(buf, "  %.77s", CapMons[i]);
            putstr(tmpwin, 0, buf);
        }
        display_nhwindow(tmpwin, TRUE);
        destroy_nhwindow(tmpwin);
    }
#endif
    return;
}

/**
 * @brief Release the capitalised-name list.
 *
 * Only part of the list is owned by it. The entries from the monster table are that table's own strings and must not be freed; the entries from the hallucinatory file are copies made here and must be. The two are told apart by position --
 * the count of real monster names is where the owned stretch begins -- which is why the build order is not an arrangement of convenience.
 *
 * @warning Freeing the whole list would free strings belonging to the monster table, and failing to free the second stretch would leak. The boundary is a count, not a mark on each entry, so anything that changed the order or interleaved
 *          the two sources would break this without any local sign of why.
 * @note Clears the size unconditionally, even when there was no list. That keeps the size and the pointer from disagreeing, since the pointer being null is what the rest of the code takes as "not built".
 */
/**
 * @brief 대문자 이름 목록을 해제한다.
 *
 * 목록의 일부만이 그것에 의해 소유된다. 몬스터 표에서 온 항목은 그 표 자신의 문자열이며 해제되어서는 안 된다. 환각 파일에서 온 항목은 여기서 만들어진 사본이며 해제되어야 한다. 그 둘은 위치로 구별되며, 실제 몬스터 이름의 개수가 소유된 구간이 시작하는 곳이다. 그것이 만드는 순서가 편의의 배치가 아닌 이유다.
 *
 * @warning 목록 전체를 해제하는 것은 몬스터 표에 속한 문자열을 해제할 것이고, 두 번째 구간을 해제하지 않는 것은 새게 할 것이다. 그 경계는 각 항목의 표시가 아니라 개수이므로, 순서를 바꾸거나 두 원천을 뒤섞는 무엇이든 왜인지에 대한 어떤 국지적 표시도 없이 이것을 망가뜨릴 것이다.
 * @note 목록이 없었을 때조차 크기를 조건 없이 지운다. 그것이 크기와 포인터가 어긋나지 않게 한다. 포인터가 널인 것을 나머지 코드가 "만들어지지 않음"으로 여기기 때문이다.
 */
void
free_CapMons(void)
{
    /* note: some elements of CapMons[] are string literals from
       mons[].pmnames[] and should not be freed, others are dynamically
       allocated copies of hallucinatory monster names and should be freed */
    if (CapMons) {
        unsigned idx;

        /* skip 0..MonstCnt-1, free MonstCnt..(MonstCnt+BogonCnt-1) */
        for (idx = CapMonstCnt; idx < CapMonSiz - 1; ++idx)
            free((genericptr_t) CapMons[idx]); /* cast: discard 'const' */
        free((genericptr_t) CapMons), CapMons = (const char **) 0;
    }
    CapMonSiz = 0;
}
#endif /* !SFCTOOL */

/*rumors.c*/
