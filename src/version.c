/* NetHack 5.0	version.c	$NHDT-Date: 1781973072 2026/06/20 16:31:12 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.118 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2018. */
/* NetHack may be freely redistributed.  See license for details. */


/**
 * @file version.c
 * @brief Reporting what this build is, and deciding what it can read.
 *
 * The version is not only something to show the player. A save file, a level
 * file and a bones file each carry the version and build settings that
 * produced them, and this is where they are compared against what is running
 * now. The comparison is about compatibility rather than equality: some
 * differences merely need reporting, while others make the data unreadable.
 *
 * @note Reads its data from the DLB archive when one is in use, so the build
 *       and the data it was packaged with cannot drift apart.
 * @warning Accepting an incompatible file is worse than refusing it: the game
 *          would read fields that mean something else and behave strangely
 *          rather than fail.
 */

/**
 * @file version.c
 * @brief 이 빌드가 무엇인지 알리고, 무엇을 읽을 수 있는지 판단한다.
 *
 * 버전은 플레이어에게 보여 주기만 하는 것이 아니다. 저장 파일, 레벨 파일, 본즈
 * 파일은 각각 자신을 만들어 낸 버전과 빌드 설정을 지니고 있으며, 지금 실행 중인
 * 것과 비교하는 곳이 여기다. 비교의 기준은 동일함이 아니라 호환성이다. 어떤
 * 차이는 알리기만 하면 되지만, 어떤 차이는 데이터를 읽을 수 없게 만든다.
 *
 * @note DLB 아카이브를 쓰는 빌드에서는 그 안에서 데이터를 읽는다. 빌드와 함께
 *       묶인 데이터가 서로 어긋날 수 없게 하기 위함이다.
 * @warning 호환되지 않는 파일을 받아들이는 것은 거부하는 것보다 나쁘다. 게임이
 *          다른 의미를 지닌 필드를 읽고, 실패하는 대신 이상하게 동작할 것이기
 *          때문이다.
 */

#include "hack.h"
#include "dlb.h"

#ifndef MINIMAL_FOR_RECOVER
#ifndef SFCTOOL

#ifndef OPTIONS_AT_RUNTIME
#define OPTIONS_AT_RUNTIME
#endif

staticfn void insert_rtoption(char *) NONNULLARG1;

/**
 * @brief The version as a short string, into the caller's buffer.
 *
 * @param buf where to write it
 * @param bufsz how much room there is
 * @return the buffer
 * @note Falls back to composing the version from its parts when the build information is unavailable, and the accompanying comment gives the case that matters: an entry written to the failure log after that information has
 *       been released. Something has already gone wrong at that point, and a version string is exactly what such an entry most needs.
 * @note The reason it takes a buffer rather than returning a constant is in the comment too -- a caller that only wants the version should not have to include the build information to get it.
 */
/**
 * @brief 판본을 짧은 문자열로. 호출자의 버퍼에.
 *
 * @param buf 그것을 쓸 곳
 * @param bufsz 자리가 얼마나 있는지
 * @return 그 버퍼
 * @note 빌드 정보를 쓸 수 없을 때는 판본을 그 부분들에서 조립하는 쪽으로 돌아가며, 딸린 주석이 중요한 경우를 밝힌다. 그 정보가 놓아진 뒤에 실패 로그에 기록되는 항목. 그 지점에서는 이미 무언가가 잘못되어 있고, 판본 문자열이 바로 그런 항목이 가장 필요로 하는 것이다.
 * @note 상수를 반환하는 대신 버퍼를 받는 이유도 그 주석에 있다. 판본만을 원하는 호출자가 그것을 얻기 위해 빌드 정보를 포함해야 하지는 않아야 한다.
 */
/* fill buffer with short version (so caller can avoid including date.h)
 * buf cannot be NULL */
char *
version_string(char *buf, size_t bufsz)
{
    Snprintf(buf, bufsz, "%s",
             ((nomakedefs.version_string && nomakedefs.version_string[0])
              ? nomakedefs.version_string
              /* in case we try to write a paniclog entry after releasing
                 the 'nomakedefs' data */
              : mdlib_version_string(buf, ".")));
    return buf;
}

/**
 * @brief The version with everything known about the build appended.
 *
 * The version, then in parentheses whatever the build recorded -- the port it was built for, the source revision, the branch, the source prefix. Any of those may be missing, and the composition has to read correctly whichever
 * combination is present.
 *
 * @param buf where to write it
 * @param bufsz how much room there is
 * @return the buffer
 * @note The care here is entirely about punctuation. The opening parenthesis is written before it is known whether anything will follow it and removed again if nothing does, and a trailing full stop is taken off before the
 *       parenthesis and put back after -- so the result reads as a sentence rather than as fields joined by chance.
 * @note The branch is included only in a build that is not a release, since a released version's branch tells a player nothing they need.
 */
/**
 * @brief 빌드에 대해 알려진 모든 것이 덧붙은 판본.
 *
 * 판본, 그다음 괄호 안에 빌드가 기록한 것 무엇이든. 빌드 대상 포트, 소스 리비전, 브랜치, 소스 접두어. 그 중 어느 것도 없을 수 있으며, 그 조립은 어떤 조합이 있든 올바르게 읽혀야 한다.
 *
 * @param buf 그것을 쓸 곳
 * @param bufsz 자리가 얼마나 있는지
 * @return 그 버퍼
 * @note 여기의 조심스러움은 전적으로 구두점에 관한 것이다. 여는 괄호는 그 뒤에 무엇이 따를지 알기 전에 쓰이고 아무것도 따르지 않으면 다시 없애지며, 끝의 마침표는 괄호 앞에서 떼어졌다가 뒤에 되붙는다. 그래서 결과가 우연히 이어진 필드가 아니라 문장처럼 읽힌다.
 * @note 브랜치는 릴리스가 아닌 빌드에서만 포함된다. 릴리스된 판본의 브랜치는 플레이어에게 필요한 것을 아무것도 알려 주지 않기 때문이다.
 */
/* fill and return the given buffer with the long nethack version string */
char *
getversionstring(char *buf, size_t bufsz)
{
    Strcpy(buf, nomakedefs.version_id);

    {
        int c = 0;
#if defined(RUNTIME_PORT_ID)
        char tmpbuf[BUFSZ], *tmp;
#endif
        char *p = eos(buf);
        boolean dotoff = (p > buf && p[-1] == '.');

        if (dotoff)
            --p;
        Strcpy(p, " (");
#if defined(RUNTIME_PORT_ID)
        tmp = get_port_id(tmpbuf);
        if (tmp)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s%s", c++ ? "," : "", tmp);
#endif
        if (nomakedefs.git_sha)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s%s", c++ ? "," : "", nomakedefs.git_sha);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
        if (nomakedefs.git_branch)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%sbranch:%s",
                     c++ ? "," : "", nomakedefs.git_branch);
#endif
        if (nomakedefs.git_prefix)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%sprefix:%s",
                     c++ ? "," : "", nomakedefs.git_prefix);
        if (c)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s", ")");
        else /* if nothing has been added, strip " (" back off */
            *p = '\0';
        if (dotoff)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s", ".");
    }
    return buf;
}

/**
 * @brief The version as it should appear on the status line, where space is scarce.
 *
 * A different problem from the other two: the status line has very little room, so this is about what to leave out. The accompanying comment sets out the forms, and the interesting rule is the omission -- where the game's name is
 * a prefix of the branch name, showing both would repeat itself, so only the branch is shown.
 *
 * @param buf where to write it
 * @param bufsz how much room there is
 * @param indent whether to leave room at the front for the status line's own layout
 * @return the buffer
 * @note Which parts appear at all is the player's own setting, so this composes from what they asked for rather than from what is known.
 * @note The comment's remark that a variant should use its own name rather than NetHack's applies to this fork, which is a modified copy.
 */
/**
 * @brief 자리가 부족한 상태줄에 나타나야 하는 대로의 판본.
 *
 * 나머지 둘과는 다른 문제다. 상태줄에는 자리가 아주 적으므로, 이것은 무엇을 빼놓을지에 관한 것이다. 딸린 주석이 그 형태들을 밝히며, 흥미로운 규칙은 그 생략이다. 게임의 이름이 브랜치 이름의 접두어인 곳에서 둘 다를 보이면 스스로를 되풀이하게 되므로, 브랜치만이 보여진다.
 *
 * @param buf 그것을 쓸 곳
 * @param bufsz 자리가 얼마나 있는지
 * @param indent 상태줄 자신의 배치를 위해 앞에 자리를 남길지
 * @return 그 버퍼
 * @note 어느 부분이 아예 나타나는지는 플레이어 자신의 설정이므로, 이것은 알려진 것이 아니라 그가 요청한 것에서 조립한다.
 * @note 변종은 NetHack 의 이름이 아니라 자기 이름을 써야 한다는 그 주석의 언급은 수정된 사본인 이 포크에 적용된다.
 */
/* version info that could be displayed on status lines;
     "<game name> <git branch name> <x.y.z version number>";
   if game name is a prefix of--or same as--branch name, it is omitted
     "<git branch name> <x.y.z version number>";
   after release--or if branch info is unavailable--it will be
     "<game name> <x.y.z version number>";
   game name or branch name or both can be requested via flags */
char *
status_version(char *buf, size_t bufsz, boolean indent)
{
    const char *name = NULL, *altname = NULL, *indentation;
    unsigned vflags = flags.versinfo;
    boolean shownum = ((vflags & VI_NUMBER) != 0),
            showname = ((vflags & VI_NAME) != 0),
            showbranch = ((vflags & VI_BRANCH) != 0);

    /* game's name {variants should use own name, not "NetHack"} */
    if (showname) {
#ifdef VERS_GAME_NAME /* can be set to override default (base of filename) */
        name = VERS_GAME_NAME;
#else
        name = nh_basename(gh.hname, FALSE); /* hname is from xxxmain.c */
#endif
        if (!name || !*name) /* shouldn't happen */
            showname = FALSE;
    }
    /* git branch name, if available */
    if (showbranch) {
#if 1   /*#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)*/
        altname = nomakedefs.git_branch;
#endif
        if (!altname || !*altname)
            showbranch = FALSE;
    }
    if (showname && showbranch) {
        if (!strncmpi(name, altname, strlen(name)))
            showname = FALSE;
#if 0
        /* note: it's possible for branch name to be a prefix of game name
           but that's unlikely enough that we won't bother with it; having
           branch "nethack-5.0" be a superset of game "nethack" seems like
           including both is redundant, but having branch "net" be a subset
           of game "nethack" doesn't feel that way; optimizing "net" out
           seems like it would be a mistake */
        else if (!strncmpi(altname, name, strlen(altname)))
            showbranch = FALSE;
#endif
    } else if (!showname && !showbranch) {
        /* flags.versinfo could be set to only 'branch' but it might not
           be available */
        shownum = TRUE;
    }

    *buf = '\0';
    indentation = indent ? " " : "";
    if (showname) {
        Snprintf(eos(buf), bufsz - strlen(buf), "%s%s", indentation, name);
        indentation = " "; /* forced separator rather than optional indent */
    }
    if (showbranch) {
        Snprintf(eos(buf), bufsz - strlen(buf), "%s%s", indentation, altname);
        indentation = " ";
    }
    if (shownum) {
        /* x.y.z version number */
        Snprintf(eos(buf), bufsz - strlen(buf), "%s%s", indentation,
                 (nomakedefs.version_string && nomakedefs.version_string[0])
                     ? nomakedefs.version_string
                     : mdlib_version_string(buf, "."));
    }
    return buf;
}

/**
 * @brief Show the version as a single line.
 * @return the command result
 * @note Redirects to the full listing when the player asked for a menu, which is how one key serves both -- a player wanting detail does not have to remember a second command.
 */
/**
 * @brief 판본을 한 줄로 보인다.
 * @return 명령 결과
 * @note 플레이어가 메뉴를 요청했으면 전체 목록으로 넘긴다. 그것이 하나의 키가 둘 다를 맡는 방식이다. 자세한 것을 원하는 플레이어가 두 번째 명령을 기억해야 하지는 않다.
 */
/* the #versionshort command */
int
doversion(void)
{
    char buf[BUFSZ];

    if (iflags.menu_requested)
        return doextversion();

    pline("%s", getversionstring(buf, sizeof buf));
    return ECMD_OK;
}

/**
 * @brief Show the version along with everything the build was configured with.
 *
 * The full account: the version, the build details, and the list of compile-time options this copy was made with. That last is why the command exists -- two copies of the same version can behave differently, and this is how a player
 * or whoever they are reporting a problem to finds out which options are in force.
 *
 * @return the command result
 * @note The options list is read from a data file rather than compiled in, so it is written once when the game is built and does not have to be maintained alongside the options themselves.
 * @note Also reachable from the help menu, as the accompanying comment notes, since a player looking for it is more likely to be looking for help than for a command.
 */
/**
 * @brief 판본과 함께 이 빌드가 무엇으로 설정되었는지 전부를 보인다.
 *
 * 온전한 보고다. 판본, 빌드 세부, 그리고 이 사본이 어떤 컴파일 시점 선택지로 만들어졌는지의 목록. 마지막 것이 이 명령이 존재하는 이유다. 같은 판본의 두 사본이 다르게 동작할 수 있고, 이것이 플레이어나 그가 문제를 알리는 상대가 어떤 선택지가 유효한지 알아내는 방식이다.
 *
 * @return 명령 결과
 * @note 선택지 목록은 컴파일에 포함되는 대신 데이터 파일에서 읽힌다. 그래서 게임이 빌드될 때 한 번 쓰이고 선택지 자체와 나란히 유지되어야 하지는 않는다.
 * @note 딸린 주석이 밝히듯 도움말 메뉴에서도 닿을 수 있다. 그것을 찾는 플레이어는 명령을 찾는 것보다 도움을 찾는 것일 가능성이 크기 때문이다.
 */
/* the '#version' command; also a choice for '?' */
int
doextversion(void)
{
    int rtcontext = 0;
    const char *rtbuf;
    dlb *f = (dlb *) 0;
    char buf[BUFSZ], *p = 0;
    winid win = create_nhwindow(NHW_TEXT);
    boolean use_dlb = TRUE,
            done_rt = FALSE,
            done_dlb = FALSE,
            prolog;
    /* lua_info[] moved to util/mdlib.c and rendered via do_runtime_info() */

#if defined(OPTIONS_AT_RUNTIME)
    use_dlb = FALSE;
#else
    done_rt = TRUE;
#endif

    /* instead of using ``display_file(OPTIONS_USED,TRUE)'' we handle
       the file manually so we can include dynamic version info */

    (void) getversionstring(buf, sizeof buf);
    /* if extra text (git info) is present, put it on separate line
       but don't wrap on (x86) */
    if (strlen(buf) >= COLNO)
        p = strrchr(buf, '(');
    if (p && p > buf && p[-1] == ' ' && p[1] != 'x')
        p[-1] = '\0';
    else
        p = 0;
    putstr(win, 0, buf);
    if (p) {
        *--p = ' ';
        putstr(win, 0, p);
    }

    if (use_dlb) {
        f = dlb_fopen(OPTIONS_USED, "r");
        if (!f) {
            putstr(win, 0, "");
            Sprintf(buf, "[Configuration '%s' not available?]", OPTIONS_USED);
            putstr(win, 0, buf);
            done_dlb = TRUE;
        }
    }
    /*
     * already inserted above:
     * + outdented program name and version plus build date and time
     * dat/options; display contents with lines prefixed by '-' deleted:
     * - blank-line
     * -     indented program name and version
     *   blank-line
     *   outdented feature header
     * - blank-line
     *       indented feature list
     *       spread over multiple lines
     *   blank-line
     *   outdented windowing header
     * - blank-line
     *       indented windowing choices with
     *       optional second line for default
     * - blank-line
     * - EOF
     */

    prolog = TRUE; /* to skip indented program name */
    for (;;) {
        if (use_dlb && !done_dlb) {
            if (!dlb_fgets(buf, BUFSZ, f)) {
                done_dlb = TRUE;
                continue;
            }
        } else if (!done_rt) {
            if (!(rtbuf = do_runtime_info(&rtcontext))) {
                done_rt = TRUE;
                continue;
            }
            (void) strncpy(buf, rtbuf, BUFSZ - 1);
            buf[BUFSZ - 1] = '\0';
        } else {
            break;
        }
        (void) strip_newline(buf);
        if (strchr(buf, '\t') != 0)
            (void) tabexpand(buf);

        if (*buf && *buf != ' ') {
            /* found outdented header; insert a separator since we'll
               have skipped corresponding blank line inside the file */
            putstr(win, 0, "");
            prolog = FALSE;
        }
        /* skip blank lines and prolog (progame name plus version) */
        if (prolog || !*buf)
            continue;

        if (strchr(buf, ':'))
            insert_rtoption(buf);

        if (*buf)
            putstr(win, 0, buf);
    }
    if (use_dlb)
        (void) dlb_fclose(f);
    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);
    return ECMD_OK;
}

/**
 * @brief Report the version before the game has started, for the command line.
 *
 * A version request from the command line has to be answered without a display, since the point of asking is often to find out what is installed rather than to play. So this writes plainly and does not use any of the game's message
 * machinery.
 *
 * @param pastebuf whether to put the text where the player can copy it, on a platform that can
 * @note That option exists because the answer's main use is being quoted in a problem report, and expecting someone to retype a version string with a revision hash in it is asking for a transcription error.
 */
/**
 * @brief 게임이 시작되기 전에 판본을 알린다. 명령줄을 위한 것.
 *
 * 명령줄에서의 판본 요청은 표시부 없이 답해져야 한다. 묻는 요점이 플레이하는 것이 아니라 무엇이 설치되어 있는지 알아내는 것인 경우가 많기 때문이다. 그래서 이것은 담담하게 쓰며 게임의 메시지 기제를 전혀 쓰지 않는다.
 *
 * @param pastebuf 그럴 수 있는 플랫폼에서, 그 글을 플레이어가 복사할 수 있는 곳에 둘지
 * @note 그 선택지가 있는 것은 그 답의 주된 용도가 문제 보고에 인용되는 것이기 때문이다. 리비전 해시가 든 판본 문자열을 누군가 다시 입력하기를 기대하는 것은 필사 오류를 부르는 일이다.
 */
void
early_version_info(boolean pastebuf)
{
    char buf1[BUFSZ], buf2[BUFSZ];
    char *buf, *tmp;

    Snprintf(buf1, sizeof buf1, "test");
    /* this is early enough that we have to do our own line-splitting */
    getversionstring(buf1, sizeof buf1);
    tmp = strstri(buf1, " ("); /* split at start of version info */
    if (tmp) {
        /* retain one buffer so that it all goes into the paste buffer */
        *tmp++ = '\0';
        Snprintf(buf2, sizeof (buf2),"%s\n%s", buf1, tmp);
        buf = buf2;
    } else {
        buf = buf1;
    }

    raw_printf("%s", buf);

    if (pastebuf) {
#if defined(RUNTIME_PASTEBUF_SUPPORT) && !defined(LIBNH)
        /*
         * Call a platform/port-specific routine to insert the
         * version information into a paste buffer. Useful for
         * easy inclusion in bug reports.
         */
        port_insert_pastebuf(buf);
#else
        raw_printf("%s", "Paste buffer copy is not available.\n");
#endif
    }
}

extern const char regex_id[];

/*
 * makedefs should put the first token into dat/options; we'll substitute
 * the second value for it.  The token must contain at least one colon
 * so that we can spot it, and should not contain spaces so that makedefs
 * won't split it across lines.  Ideally the length should be close to
 * that of the substituted value since we don't do phrase-splitting/line-
 * wrapping when displaying it.
 */
static struct rt_opt {
    const char *token, *value;
} rt_opts[] = {
    { ":PATMATCH:", regex_id },
    { ":LUAVERSION:", (const char *) gl.lua_ver },
    { ":LUACOPYRIGHT:", (const char *) gl.lua_copyright },
};

/**
 * @brief Fill in the option values the build tool could not know.
 *
 * The options listing is written when the game is built, but a few of its values cannot be known then. The accompanying comment gives the reason: whether a facility is present depends on which pieces were linked into the finished
 * program, which the tool that wrote the listing does not see. So the listing leaves a placeholder and this substitutes the answer as the file is read.
 *
 * @param buf one line of the listing, altered in place
 * @note Does not stop at the first substitution, as the accompanying comment says, because one line may hold more than one placeholder.
 * @note The scripting language's version is one of these, and is fetched on first use rather than at startup -- a player who never asks for the version listing never pays for it.
 */
/**
 * @brief 빌드 도구가 알 수 없었던 선택지 값을 채운다.
 *
 * 선택지 목록은 게임이 빌드될 때 쓰이지만, 그 값 중 몇 개는 그때 알 수 없다. 딸린 주석이 그 이유를 밝힌다. 어떤 기능이 있는지가 완성된 프로그램에 어느 조각이 링크되었는지에 달려 있고, 그 목록을 쓴 도구는 그것을 보지 못한다. 그래서 그 목록은 자리 표시를 남기고, 이것이 파일이 읽히면서 그 답을
 * 갈아 넣는다.
 *
 * @param buf 그 목록의 한 줄. 제자리에서 바뀐다
 * @note 딸린 주석이 말하듯 첫 번째 갈아 넣기에서 멈추지 않는다. 한 줄이 자리 표시를 둘 넘게 담을 수 있기 때문이다.
 * @note 스크립트 언어의 판본이 그 중 하나이며, 시작할 때가 아니라 처음 쓰일 때 가져온다. 판본 목록을 결코 요청하지 않는 플레이어는 그것에 대한 비용을 치르지 않는다.
 */
/*
 * 3.6.0
 * Some optional stuff is no longer available to makedefs because
 * it depends which of several object files got linked into the
 * game image, so we insert those options here.
 */
staticfn void
insert_rtoption(char *buf)
{
    int i;

    if (!gl.lua_ver[0])
        get_lua_version();

    for (i = 0; i < SIZE(rt_opts); ++i) {
        if (strstri(buf, rt_opts[i].token) && *rt_opts[i].value) {
            (void) strsubst(buf, rt_opts[i].token, rt_opts[i].value);
        }
        /* we don't break out of the loop after a match; there might be
           other matches on the same line */
    }
}

#ifdef MICRO
boolean
comp_times(long filetime)
{
    /* BUILD_TIME is constant but might have L suffix rather than UL;
       'filetime' is historically signed but ought to have been unsigned */
    return ((unsigned long) filetime < (unsigned long) nomakedefs.build_time);
}
#endif
#endif /* !SFCTOOL */

#ifdef SFCTOOL
#ifdef wait_synch
#undef wait_synch
#endif
#define wait_synch()
#endif /* SFCTOOL */

/**
 * @brief Whether a file's recorded version is one this build can read.
 *
 * The gate every saved file passes through. Three things are compared, and the version number alone is not enough: two builds of one version may differ in which options were compiled in, and either may have a different number of
 * monsters and objects -- and a file built against one is meaningless to the other.
 *
 * @param version_data what the file recorded, which this may modify
 * @param filename the file's name, for a message
 * @param complain whether to tell the player what was wrong
 * @param utdflags which of the comparisons to relax
 * @return whether the file may be read
 * @note The flags exist because not every comparison matters for every kind of file. A bones file may be accepted where a save file would not, since it carries less of the game's structure.
 * @warning Modifies what it was given: it strips the mark saying the file came from the format-conversion tool, and records that fact elsewhere. So a caller cannot compare the same data twice and expect the same answer.
 * @note Complaining requires a filename, and passing one without the other is caught in a build with the extra checks enabled -- since a message about an unnamed file would say nothing useful.
 */
/**
 * @brief 어떤 파일에 기록된 판본이 이 빌드가 읽을 수 있는 것인지.
 *
 * 저장된 모든 파일이 지나는 관문이다. 세 가지가 비교되며, 판본 번호만으로는 충분하지 않다. 한 판본의 두 빌드가 어떤 선택지로 컴파일되었는지에서 다를 수 있고, 어느 쪽이든 몬스터와 물건의 수가 다를 수 있다. 그리고 한쪽에 맞춰 만든 파일은 다른 쪽에게 무의미하다.
 *
 * @param version_data 그 파일이 기록한 것. 이것이 그것을 바꿀 수 있다
 * @param filename 그 파일의 이름. 메시지를 위한 것
 * @param complain 무엇이 잘못되었는지 플레이어에게 알릴지
 * @param utdflags 그 비교 중 어느 것을 느슨하게 할지
 * @return 그 파일을 읽어도 되는지
 * @note 그 플래그가 있는 것은 모든 종류의 파일에 모든 비교가 중요하지는 않기 때문이다. 유골 파일은 저장 파일이 받아들여지지 않을 곳에서 받아들여질 수 있다. 게임의 구조를 덜 지니기 때문이다.
 * @warning 건네받은 것을 바꾼다. 그 파일이 형식 변환 도구에서 왔다는 표시를 벗겨 내고 그 사실을 다른 곳에 기록한다. 그래서 호출자가 같은 데이터를 두 번 비교하며 같은 답을 기대할 수 없다.
 * @note 불평하려면 파일 이름이 필요하며, 하나를 다른 것 없이 넘기는 것은 추가 검사가 켜진 빌드에서 잡힌다. 이름 없는 파일에 대한 메시지는 쓸모 있는 것을 아무것도 말하지 않기 때문이다.
 */
boolean
check_version(
    struct version_info *version_data,
    const char *filename,
    boolean complain,
    unsigned long utdflags)
{
    if (!filename) {
#ifdef EXTRA_SANITY_CHECKS
        if (complain)
            impossible("check_version() called with"
                       " 'complain'=True but 'filename'=Null");
#endif
        complain = FALSE; /* 'complain' requires 'filename' for pline("%s") */
    }
    if ((version_data->feature_set & SFCTOOL_BIT) != 0) {
        gc.converted_savefile_loaded = TRUE;
        version_data->feature_set &= ~(SFCTOOL_BIT);
    }
    if (
#ifdef VERSION_COMPATIBILITY /* patchlevel.h */
        version_data->incarnation < VERSION_COMPATIBILITY
        || version_data->incarnation > nomakedefs.version_number
#else
        version_data->incarnation != nomakedefs.version_number
#endif
        ) {
#ifndef SFCTOOL
        if (complain) {
            pline("Version mismatch for file \"%s\".", filename);
            if (WIN_MESSAGE != WIN_ERR)
                 display_nhwindow(WIN_MESSAGE, TRUE);
        }
#endif
        return FALSE;
    } else if (
        (version_data->feature_set & ~nomakedefs.ignored_features)
            != (nomakedefs.version_features & ~nomakedefs.ignored_features)
        || ((utdflags & UTD_SKIP_SANITY1) == 0
             && version_data->entity_count != nomakedefs.version_sanity1)
        ) {
#ifndef SFCTOOL
        if (complain) {
            pline("Configuration incompatibility for file \"%s\".", filename);
            display_nhwindow(WIN_MESSAGE, TRUE);
        }
#endif
        return FALSE;
    }
    return TRUE;
}

#ifndef SFCTOOL
#ifdef AMIGA
const char amiga_version_string[] = AMIGA_VERSION_STRING;
#endif

/**
 * @brief Turn a version a player wrote into the packed form the game compares against.
 *
 * Exists for the setting by which a player says which version's new-feature notices they have already seen, so the game stops repeating them. The player writes a version the way a version is written; the game holds it packed.
 *
 * @param str the version as text, which this copies before altering
 * @return the packed version, or zero if nothing was given
 * @note Copies the text first, so the caller's string is untouched -- which matters because this is called on a value read from a configuration file that may be reported back in an error message.
 * @warning Strict: all three parts are required and anything but digits and separators is refused. A version with fewer parts, or with any other character in it, yields zero -- which the caller must distinguish from a genuine zero
 *          rather than treating as "no notices seen".
 */
/**
 * @brief 플레이어가 적은 판본을 게임이 비교하는 압축 형태로 바꾼다.
 *
 * 플레이어가 어느 판본의 새 기능 알림을 이미 보았는지 말하는 설정을 위해 존재한다. 그래서 게임이 그것을 되풀이하기를 그만둔다. 플레이어는 판본을 적는 방식대로 적고, 게임은 그것을 압축해서 보관한다.
 *
 * @param str 글로서의 판본. 이것이 바꾸기 전에 그것을 복사한다
 * @return 압축된 판본. 아무것도 주어지지 않았으면 0
 * @note 글을 먼저 복사하므로 호출자의 문자열은 건드려지지 않는다. 이것이 설정 파일에서 읽힌 값에 대해 호출되고 그 값이 오류 메시지에 되보여질 수 있으므로 그것이 중요하다.
 * @warning 엄격하다. 세 부분 모두가 필수이고 숫자와 구분자 말고는 거부된다. 부분이 더 적은 판본이나 다른 문자가 든 판본은 0을 낸다. 호출자는 그것을 "알림을 본 적 없음"으로 취급하는 대신 진짜 0과 구별해야 한다.
 */
unsigned long
get_feature_notice_ver(char *str)
{
    char buf[BUFSZ];
    int ver_maj, ver_min, patch;
    char *istr[3];
    int j = 0;

    if (!str)
        return 0L;
    str = strcpy(buf, str);
    istr[j] = str;
    while (*str) {
        if (*str == '.') {
            *str++ = '\0';
            j++;
            istr[j] = str;
            if (j == 2)
                break;
        } else if (strchr("0123456789", *str) != 0) {
            str++;
        } else
            return 0L;
    }
    if (j != 2)
        return 0L;
    ver_maj = atoi(istr[0]);
    ver_min = atoi(istr[1]);
    patch = atoi(istr[2]);
    return FEATURE_NOTICE_VER(ver_maj, ver_min, patch);
    /* macro from hack.h */
}

unsigned long
get_current_feature_ver(void)
{
    return FEATURE_NOTICE_VER(VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
}

/**
 * @brief One line of the copyright notice, by number.
 * @param indx which line, counting from one
 * @return the line, or empty text for a line this build does not have
 * @note Fetched a line at a time rather than as a block so a display can lay them out however suits it -- a terminal centres them and a graphical port may not.
 * @note One of the lines is produced when the game runs rather than compiled in, which is why they cannot simply be a table.
 * @warning An index outside the range yields empty text rather than nothing, so a caller printing them in a loop stops on its own and does not need to know how many there are.
 */
/**
 * @brief 저작권 고지의 한 줄. 번호로.
 * @param indx 몇 번째 줄인지. 1부터
 * @return 그 줄. 이 빌드에 없는 줄에 대해서는 빈 글
 * @note 덩어리가 아니라 한 줄씩 가져온다. 그래서 표시부가 자신에게 맞는 대로 배치할 수 있다. 터미널은 그것을 가운데 맞추고 그래픽 포트는 그러지 않을 수 있다.
 * @note 그 줄 중 하나는 컴파일에 포함되는 대신 게임이 돌아갈 때 만들어진다. 그것이 그것들이 그냥 표일 수 없는 이유다.
 * @warning 범위 밖의 색인은 아무것도가 아니라 빈 글을 낸다. 그래서 반복문으로 그것들을 인쇄하는 호출자가 스스로 멈추고 몇 개인지 알 필요가 없다.
 */
/*ARGUSED*/
const char *
copyright_banner_line(int indx)
{
#ifdef COPYRIGHT_BANNER_A
    if (indx == 1)
        return COPYRIGHT_BANNER_A;
#endif
#ifdef COPYRIGHT_BANNER_B
    if (indx == 2)
        return COPYRIGHT_BANNER_B;
#endif

    if (indx == 3)
        return nomakedefs.copyright_banner_c;

#ifdef COPYRIGHT_BANNER_D
    if (indx == 4)
        return COPYRIGHT_BANNER_D;
#endif
    return "";
}

/**
 * @brief Print the three numbers a file must match, for a tool to read.
 *
 * Not for a player. It writes the version, the compiled-in options and the entity count in a fixed layout, so that a script deciding whether a save file matches this build can compare without understanding anything about the game.
 *
 * @note The program's own name is included and truncated from the front rather than the back if too long, so what survives is the name and not the directories leading to it.
 * @note The ignored options are masked out, so two builds differing only in options that do not affect file compatibility report the same number -- which is the whole point of there being ignored options at all.
 */
/**
 * @brief 파일이 맞춰야 하는 세 숫자를 인쇄한다. 도구가 읽기 위한 것.
 *
 * 플레이어를 위한 것이 아니다. 판본, 컴파일에 포함된 선택지, 개체 수를 고정된 배치로 쓴다. 그래서 저장 파일이 이 빌드와 맞는지 정하는 스크립트가 게임에 대해 아무것도 이해하지 않고 비교할 수 있다.
 *
 * @note 프로그램 자신의 이름이 포함되며 너무 길면 뒤가 아니라 앞에서 잘린다. 그래서 살아남는 것이 그 이름이고 그것에 이르는 디렉토리가 아니다.
 * @note 무시되는 선택지는 마스킹되어 빠진다. 그래서 파일 호환성에 영향을 주지 않는 선택지에서만 다른 두 빌드가 같은 숫자를 알린다. 그것이 무시되는 선택지가 아예 있는 요점 전체다.
 */
/* called by argcheck(allmain.c) from early_options(sys/xxx/xxxmain.c) */
void
dump_version_info(void)
{
    char buf[BUFSZ];
    const char *hname = gh.hname ? gh.hname : "nethack";

    if (strlen(hname) > 33)
        hname = eos(nhStr(hname)) - 33; /* discard const for eos() */
    runtime_info_init();
    Snprintf(buf, sizeof buf, "%-12.33s %08lx %08lx %08lx",
             hname,
             nomakedefs.version_number,
             (nomakedefs.version_features & ~nomakedefs.ignored_features),
             nomakedefs.version_sanity1);
    raw_print(buf);
    release_runtime_info();
    return;
}
/**
 * @brief Write this build's three identifying numbers at the head of a file.
 * @param nhfp the file being written
 * @note Written first, before anything else, because the check that reads them decides whether the rest of the file may be read at all. That ordering is why the version structure's save routines are generated separately from every
 *       other type's.
 */
/**
 * @brief 이 빌드의 세 식별 숫자를 파일 앞머리에 쓴다.
 * @param nhfp 기록 중인 파일
 * @note 다른 무엇보다 먼저 쓰인다. 그것을 읽는 검사가 파일의 나머지를 아예 읽어도 되는지 정하기 때문이다. 그 순서가 판본 구조체의 저장 루틴이 다른 모든 타입의 것과 따로 생성되는 이유다.
 */
void
store_version(NHFILE *nhfp)
{
    struct version_info version_data = {
        0UL,
        0UL,
        0UL,
    };
    /* actual version number */
    version_data.incarnation = nomakedefs.version_number;
    /* bitmask of config settings */
    version_data.feature_set = nomakedefs.version_features;
    /* # of monsters and objects */
    version_data.entity_count = nomakedefs.version_sanity1;

    /* bwrite() before bufon() uses plain write() */
    if (nhfp->structlevel)
        bufoff(nhfp->fd);

    store_critical_bytes(nhfp);
    Sfo_version_info(nhfp, (struct version_info *) &version_data,
                     "version_info");

    if (nhfp->structlevel)
        bufon(nhfp->fd);
    return;
}
#endif /* !SFCTOOL */
#endif /* MINIMAL_FOR_RECOVER */

/**
 * @brief One type whose size must agree between the build that wrote a file and the build reading it.
 * @note The name is carried alongside the size so a mismatch can say which type disagreed. Reporting "byte 43 differs" would be useless; reporting the structure's name is what makes such a failure diagnosable.
 */
/**
 * @brief 파일을 쓴 빌드와 읽는 빌드 사이에서 크기가 일치해야 하는 타입 하나.
 * @note 크기와 함께 이름이 지녀지므로, 불일치가 어느 타입이 어긋났는지 말할 수 있다. "43번째 바이트가 다르다"고 알리는 것은 쓸모가 없다. 그 구조체의 이름을 알리는 것이 그런 실패를 진단할 수 있게 하는 것이다.
 */
struct critical_sizes_with_names {
    uchar ucsize;
    const char *nm;
};

/**
 * @brief Every type whose size is part of the save format.
 *
 * The version numbers say which build wrote a file; this says whether that build laid its data out the same way. The two are different questions, because the same source compiled by a different compiler, or for a different data
 * model, produces the same version and an incompatible file.
 *
 * The table is in two parts. The simple types come first -- the widths everything else is built from -- and the compound ones follow, which is why a single-byte difference in an integer type shows up as many mismatches rather than
 * one: everything containing it disagrees too. The first mismatch is the informative one.
 *
 * @note Every size is stored in a single byte, which is enough for every type here and is what keeps the whole check to a few dozen bytes at the head of a file.
 * @note Nine unused entries are reserved at the end, as the accompanying comment records, so a type can be added later without changing the table's size -- and therefore without invalidating files written before it was added.
 * @note The last entry is not a size at all but the save format's own revision level, placed here so it is compared by the same machinery rather than needing its own.
 */
/**
 * @brief 크기가 저장 형식의 일부인 모든 타입.
 *
 * 판본 번호는 어느 빌드가 파일을 썼는지 말한다. 이것은 그 빌드가 자기 데이터를 같은 방식으로 놓았는지 말한다. 그 둘은 다른 질문이다. 같은 소스를 다른 컴파일러로, 또는 다른 데이터 모델로 컴파일하면 같은 판본과 호환되지 않는 파일이 나오기 때문이다.
 *
 * 이 표는 두 부분이다. 단순 타입이 먼저 온다. 다른 모든 것이 그것으로 지어지는 폭들이다. 그리고 복합 타입이 따른다. 그것이 어떤 정수 타입의 한 바이트 차이가 하나가 아니라 여러 불일치로 나타나는 이유다. 그것을 담은 모든 것도 어긋난다. 첫 번째 불일치가 정보를 주는 것이다.
 *
 * @note 모든 크기가 한 바이트에 저장되며, 여기의 모든 타입에 충분하고 그것이 이 검사 전체를 파일 앞머리의 수십 바이트로 유지하는 것이다.
 * @note 딸린 주석이 기록하듯 끝에 쓰이지 않는 항목 아홉 개가 예약되어 있다. 그래서 표의 크기를 바꾸지 않고 나중에 타입을 더할 수 있고, 따라서 그것이 더해지기 전에 쓰인 파일을 무효로 만들지 않는다.
 * @note 마지막 항목은 크기가 전혀 아니라 저장 형식 자신의 개정 단계다. 자기 기제를 따로 필요로 하는 대신 같은 기제로 비교되도록 여기에 놓여 있다.
 */
struct critical_sizes_with_names critical_sizes[] = {
    { 0, "unused" },
    /* simple types, that don't have subfields */
    { (uchar) sizeof(short), "short" },
    { (uchar) sizeof(int), "int" },
    { (uchar) sizeof(long), "long" },
    { (uchar) sizeof(long long), "long long" },
    { (uchar) sizeof(genericptr_t), "genericptr_t" },
    { (uchar) sizeof(aligntyp), "aligntyp" },
    { (uchar) sizeof(boolean), "boolean" },
    { (uchar) sizeof(coordxy), "coordxy" },
    { (uchar) sizeof(int16), "int16" },
    { (uchar) sizeof(int32), "int32" },
    { (uchar) sizeof(int64), "int64" },
    { (uchar) sizeof(schar), "schar" },
    { (uchar) sizeof(size_t), "size_t" },
    { (uchar) sizeof(uchar), "uchar" },
    { (uchar) sizeof(uint16), "uint16" },
    { (uchar) sizeof(uint32), "uint32" },
    { (uchar) sizeof(uint64), "uint64" },
    { (uchar) sizeof(ulong), "ulong" },
    { (uchar) sizeof(unsigned), "unsigned" },
    { (uchar) sizeof(ushort), "ushort" },
    { (uchar) sizeof(xint16), "xint16" },
    { (uchar) sizeof(xint8), "xint8" },
    /* complex - they break down into one or more simple types */
    { (uchar) sizeof(struct arti_info), "struct arti_info" },
    { (uchar) sizeof(struct nhrect), "struct nhrect" },
    { (uchar) sizeof(struct branch), "struct branch" },
    { (uchar) sizeof(struct bubble), "struct bubble" },
    { (uchar) sizeof(struct cemetery), "struct cemetery" },
    { (uchar) sizeof(struct context_info), "struct context_info" },
    { (uchar) sizeof(struct nhcoord), "struct nhcoord" },
    { (uchar) sizeof(struct damage), "struct damage" },
    { (uchar) sizeof(struct dest_area), "struct dest_area" },
    { (uchar) sizeof(struct dgn_topology), "struct dgn_topology" },
    { (uchar) sizeof(struct dungeon), "struct dungeon" },
    { (uchar) sizeof(struct d_level), "struct d_level" },
    { (uchar) sizeof(struct ebones), "struct ebones" },
    { (uchar) sizeof(struct edog), "struct edog" },
    { (uchar) sizeof(struct egd), "struct egd" },
    { (uchar) sizeof(struct emin), "struct emin" },
    { (uchar) sizeof(struct engr), "struct engr" },
    { (uchar) sizeof(struct epri), "struct epri" },
    { (uchar) sizeof(struct eshk), "struct eshk" },
    { (uchar) sizeof(struct fe), "struct fe" },
    { (uchar) sizeof(struct flag), "struct flag" },
    { (uchar) sizeof(struct fruit), "struct fruit" },
    { (uchar) sizeof(struct gamelog_line), "struct gamelog_line" },
    { (uchar) sizeof(struct kinfo), "struct kinfo" },
    { (uchar) sizeof(struct levelflags), "struct levelflags" },
    { (uchar) sizeof(struct ls_t), "struct ls_t" },
    { (uchar) sizeof(struct linfo), "struct linfo" },
    { (uchar) sizeof(struct mapseen_feat), "struct mapseen_feat" },
    { (uchar) sizeof(struct mapseen_flags), "struct mapseen_flags" },
    { (uchar) sizeof(struct mapseen_rooms), "struct mapseen_rooms" },
    { (uchar) sizeof(struct mextra), "struct mextra" },
    { (uchar) sizeof(struct mkroom), "struct mkroom" },
    { (uchar) sizeof(struct monst), "struct monst" },
    { (uchar) sizeof(struct mvitals), "struct mvitals" },
    { (uchar) sizeof(struct obj), "struct obj" },
    { (uchar) sizeof(struct objclass), "struct objclass" },
    { (uchar) sizeof(struct oextra), "struct oextra" },
    { (uchar) sizeof(struct q_score), "struct q_score" },
    { (uchar) sizeof(struct rm), "struct rm" },               /* [61] */
    { (uchar) sizeof(struct spell), "struct spell" },
    { (uchar) sizeof(struct stairway), "struct stairway" },
    { (uchar) sizeof(struct s_level), "struct s_level" },
    { (uchar) sizeof(struct trap), "struct trap" },
    { (uchar) sizeof(struct version_info), "struct version_info" },
    { (uchar) sizeof(anything), "anything" },
    /* struct you requires 2 bytes */
    { (uchar) ((sizeof(struct you) & 0x00FF)), "you_LO" },
    { (uchar) ((sizeof(struct you) & 0xFF00) >> 8), "you_HI" },
#ifdef SF_INCLUDE_SUBSTRUCTS
    /*
     * the ones below are substructures of the ones
     * above, so there is no need to check these directly.
     */
    { (uchar) sizeof(struct attribs), "struct attribs" },
    { (uchar) sizeof(struct dig_info), "struct dig_info" },
    { (uchar) sizeof(struct tin_info), "struct tin_info" },
    { (uchar) sizeof(struct book_info), "struct book_info" },
    { (uchar) sizeof(struct takeoff_info), "struct takeoff_info" },
    { (uchar) sizeof(struct victual_info), "struct victual_info" },
    { (uchar) sizeof(struct engrave_info), "struct engrave_info" },
    { (uchar) sizeof(struct warntype_info), "struct warntype_info" },
    { (uchar) sizeof(struct polearm_info), "struct polearm_info" },
    { (uchar) sizeof(struct obj_split), "struct obj_split" },
    { (uchar) sizeof(struct tribute_info), "struct tribute_info" },
    { (uchar) sizeof(struct novel_tracking), "struct novel_tracking" },
    { (uchar) sizeof(struct achievement_tracking),
      "struct achievement_tracking" },
    { (uchar) sizeof(struct d_flags), "struct d_flags" },
    { (uchar) sizeof(struct mapseen), "struct mapseen" },
    { (uchar) sizeof(struct fakecorridor), "struct fakecorridor" },
    { (uchar) sizeof(struct bill_x), "struct bill_x" },
    { (uchar) sizeof(union vptrs), "union vptrs" },
    { (uchar) sizeof(struct prop), "struct prop" },
    { (uchar) sizeof(struct skills), "struct skills" },
    { (uchar) sizeof(union vlaunchinfo), "union vlaunchinfo" },
    { (uchar) sizeof(struct u_have), "struct u_have" },
    { (uchar) sizeof(struct u_event), "struct u_event" },
    { (uchar) sizeof(struct u_realtime), "struct u_realtime" },
    { (uchar) sizeof(struct u_conduct), "struct u_conduct" },
    { (uchar) sizeof(struct u_roleplay), "struct u_roleplay" },
#endif /* SF_INCLUDE_SUBSTRUCTS */
    /* 10 for future expansion without changing array size */
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { (uchar) SAVEFILE_REVISION_LEVEL, "savefile_revision_level" },
};

/**
 * @var cscbuf
 * @brief Where the sizes read from a file are put, for comparing against this build's.
 * @note Sized from the table itself, so it cannot be too small. A file recording a different number of sizes is refused before this is filled, which is why one buffer serves both a matching and a mismatching file.
 */
/**
 * @var cscbuf
 * @brief 파일에서 읽힌 크기가 놓이는 곳. 이 빌드의 것과 비교하기 위한 것.
 * @note 표 자체에서 크기가 정해지므로 너무 작을 수 없다. 다른 개수의 크기를 기록한 파일은 이것이 채워지기 전에 거부되며, 그것이 하나의 버퍼가 맞는 파일과 맞지 않는 파일 둘 다를 맡는 이유다.
 */
uchar cscbuf[SIZE(critical_sizes)];

/**
 * @brief How many sizes are compared.
 * @note Asked rather than known, so the reading side does not have to be kept in step with the table by hand.
 */
/**
 * @brief 몇 개의 크기가 비교되는지.
 * @note 알고 있는 대신 물어진다. 그래서 읽는 쪽이 손으로 그 표와 보조를 맞추어야 하지는 않다.
 */
int
get_critical_size_count(void)
{
    return SIZE(critical_sizes);
}

/**
 * @brief Write which save format was used and every size that must match.
 *
 * @param nhfp the file being written
 * @note The format marker comes first because the reader must know how to read the rest before reading it. A single character rather than a name, since it is read before anything about the file has been established.
 * @note The count is written before the sizes so a reader can refuse a file with a different number of them without reading them -- which is what makes a file from a build that added a type a clean refusal rather than a
 *       misaligned comparison.
 * @warning Writes nothing at all when the file is being read rather than written. So it is safe to call in either direction, and a caller need not know which.
 */
/**
 * @brief 어느 저장 형식이 쓰였는지와 일치해야 하는 모든 크기를 쓴다.
 *
 * @param nhfp 기록 중인 파일
 * @note 형식 표시가 먼저 온다. 읽는 쪽이 나머지를 읽기 전에 그것을 어떻게 읽을지 알아야 하기 때문이다. 이름이 아니라 문자 하나인 것은, 그 파일에 대해 아무것도 확정되기 전에 읽히기 때문이다.
 * @note 개수가 크기들보다 먼저 쓰이므로, 읽는 쪽이 개수가 다른 파일을 그것들을 읽지 않고 거부할 수 있다. 그것이 타입을 더한 빌드의 파일을 어긋난 비교가 아니라 깔끔한 거부로 만드는 것이다.
 * @warning 파일이 기록되는 것이 아니라 읽히는 중일 때는 아무것도 쓰지 않는다. 그래서 어느 방향에서든 호출해도 안전하고, 호출자가 어느 쪽인지 알 필요가 없다.
 */
#ifndef MINIMAL_FOR_RECOVER
void
store_critical_bytes(NHFILE *nhfp)
{
    int i, cnt;
    char indicate = 'u', csc_count = (char) SIZE(critical_sizes);
    /* int cmc = 0; */

    if (nhfp->mode & WRITING) {
        indicate = (nhfp->structlevel) ? 'h'
                                       : (nhfp->fnidx == exportascii)
                                         ? 'a'
                                         : '?';
        Sfo_char(nhfp, &indicate, "indicate-format", 1);
        Sfo_char(nhfp, &csc_count, "count-critical_sizes", 1);
        cnt = (int) csc_count;
        for (i = 0; i < cnt; ++i) {
            Sfo_uchar(nhfp, &critical_sizes[i].ucsize, "critical_sizes");
        }
    }
}

/**
 * @brief Whether a file may be read by this build, and if not, precisely why.
 *
 * The full compatibility check: the version numbers and then every size that must agree. What makes it worth reading is the list of answers -- it does not merely refuse, it names the mismatch, and several of the named cases are
 * specific pairings of data models.
 *
 * That specificity is deliberate. "This save file is from a 64-bit build and you are running a 32-bit one" is something a player can act on; "incompatible file" is not. So the common cross-platform mistakes each have their own
 * answer rather than falling into the general one.
 *
 * @param nhfp the file
 * @param name the file's name, for a message
 * @param utdflags which comparisons to relax
 * @return one of the listed results; zero means the file is usable
 * @note It examines the file's contents. The accompanying comment records that it once used the file's date instead, which was both less reliable and dependent on the operating system.
 * @note The size-count mismatch is answered before any size is compared, since a file recording a different number of them cannot be compared meaningfully at all.
 */
/**
 * @brief 어떤 파일을 이 빌드가 읽어도 되는지, 그렇지 않다면 정확히 왜인지.
 *
 * 온전한 호환성 검사다. 판본 번호, 그다음 일치해야 하는 모든 크기. 이것을 읽어 볼 가치가 있게 만드는 것은 그 답의 목록이다. 그저 거부하는 것이 아니라 그 불일치의 이름을 밝히며, 이름 붙은 경우 중 여러 개가 데이터 모델의 특정한 짝이다.
 *
 * 그 구체성은 의도적이다. "이 저장 파일은 64비트 빌드의 것이고 당신은 32비트를 돌리고 있다"는 플레이어가 조치할 수 있는 것이다. "호환되지 않는 파일"은 그렇지 않다. 그래서 흔한 플랫폼 간 실수 각각이 일반적인 것에 떨어지는 대신 자기 답을 가진다.
 *
 * @param nhfp 그 파일
 * @param name 그 파일의 이름. 메시지를 위한 것
 * @param utdflags 어느 비교를 느슨하게 할지
 * @return 나열된 결과 중 하나. 0은 그 파일을 쓸 수 있다는 뜻이다
 * @note 파일의 내용을 살핀다. 딸린 주석은 그것이 한때 대신 파일의 날짜를 썼다고 기록하며, 그것은 덜 믿을 만하면서 운영 체제에 의존적이었다.
 * @note 크기 개수 불일치는 어느 크기가 비교되기 전에 답해진다. 그 개수가 다른 파일은 아예 의미 있게 비교될 수 없기 때문이다.
 */
/* this used to be based on file date and somewhat OS-dependent,
 *  but now examines the initial part of the file's contents.
 *
 * returns:
 *
 *   SF_UPTODATE                     (0) everything matched and looks good
 *   SF_OUTDATED                     (1) savefile is outdated
 *   SF_CRITICAL_BYTE_COUNT_MISMATCH (2) critical size count mismatch
 *   SF_DM_IL32LLP64_ON_ILP32LL64    (3) Windows x64 savefile on x86
 *   SF_DM_I32LP64_ON_ILP32LL64      (4) Unix 64 savefile on x86
 *   SF_DM_ILP32LL64_ON_I32LP64      (5) x86 savefile on Unix 64
 *   SF_DM_ILP32LL64_ON_IL32LLP64    (6) x86 savefile on Windows x64
 *   SF_DM_I32LP64_ON_IL32LLP64      (7) Unix 64 savefile on Windows x64
 *   SF_DM_IL32LLP64_ON_I32LP64      (8) Windows x64 savefile on Unix 64
 *   SF_DM_MISMATCH                  (9) some other mismatch
 */
int
uptodate(NHFILE *nhfp, const char *name, unsigned long utdflags)
{
#ifdef SFCTOOL
    extern struct version_info vers_info;
#else
    struct version_info vers_info;
#endif
    char indicator;
    int sfstatus = 0, idx_1st_mismatch = 0;
    boolean quietly = (utdflags & UTD_QUIETLY) != 0;
    boolean verbose = name ? TRUE : FALSE;

    Sfi_char(nhfp, &indicator, "indicate-format", 1);
    if ((sfstatus = compare_critical_bytes(nhfp, &idx_1st_mismatch, utdflags))
                                                             != SF_UPTODATE) {
        if (sfstatus > 0 && idx_1st_mismatch) {
            if (!quietly)
                raw_printf("comparison of critical bytes mismatched at %d (%s).",
                           critical_sizes[idx_1st_mismatch].ucsize,
                           critical_sizes[idx_1st_mismatch].nm);
        }
    }

    Sfi_version_info(nhfp, &vers_info, "version_info");
    if (!check_version(&vers_info, name, verbose, utdflags)) {
        if (verbose) {
            if ((utdflags & UTD_WITHOUT_WAITSYNCH_PERFILE) == 0) {
                wait_synch();
            }
        }
        return SF_OUTDATED;
    }
    return sfstatus;
}

/*
 * returns:
 *
 *   SF_UPTODATE                     (0) everything matched and looks good
 *   SF_OUTDATED                     (1) savefile is outdated
 *   SF_CRITICAL_BYTE_COUNT_MISMATCH (2) critical size count mismatch
 *   SF_DM_IL32LLP64_ON_ILP32LL64    (3) Windows x64 savefile on x86
 *   SF_DM_I32LP64_ON_ILP32LL64      (4) Unix 64 savefile on x86
 *   SF_DM_ILP32LL64_ON_I32LP64      (5) x86 savefile on Unix 64
 *   SF_DM_ILP32LL64_ON_IL32LLP64    (6) x86 savefile on Windows x64
 *   SF_DM_I32LP64_ON_IL32LLP64      (7) Unix 64 savefile on Windows x64
 *   SF_DM_IL32LLP64_ON_I32LP64      (8) Windows x64 savefile on Unix 64
 *   SF_DM_MISMATCH                  (9) some other mismatch
 */
/**
 * @brief Read a file's recorded sizes and work out what kind of mismatch, if any, they show.
 *
 * The part that turns a difference into a diagnosis. Rather than reporting that some byte differs, it looks at the pattern of differences and recognises the common data models -- so a save file from a sixty-four-bit build read by a
 * thirty-two-bit one is named as exactly that.
 *
 * @param nhfp the file
 * @param idx_1st_mismatch receives which entry differed first
 * @param utdflags which comparisons to relax
 * @return one of the listed results
 * @note The first mismatch is reported rather than all of them, because a single differing integer width makes every structure containing it differ too. The first is the cause and the rest are its consequences.
 * @warning Refuses immediately if the file records a different number of sizes, without comparing any. Comparing a shorter or longer list entry by entry would produce a mismatch at whichever entry the lists first diverge, which
 *          says nothing about what actually differs.
 */
/**
 * @brief 파일에 기록된 크기를 읽고, 그것이 어떤 종류의 불일치를 보이는지 알아낸다.
 *
 * 차이를 진단으로 바꾸는 부분이다. 어떤 바이트가 다르다고 알리는 대신, 차이의 양상을 보고 흔한 데이터 모델을 알아본다. 그래서 64비트 빌드의 저장 파일이 32비트가 읽는 경우가 정확히 그것으로 이름 붙는다.
 *
 * @param nhfp 그 파일
 * @param idx_1st_mismatch 어느 항목이 먼저 달랐는지를 받는다
 * @param utdflags 어느 비교를 느슨하게 할지
 * @return 나열된 결과 중 하나
 * @note 전부가 아니라 첫 번째 불일치가 알려진다. 하나의 정수 폭이 다르면 그것을 담은 모든 구조체도 달라지기 때문이다. 첫 번째가 원인이고 나머지는 그 결과다.
 * @warning 파일이 다른 개수의 크기를 기록했으면 어느 것도 비교하지 않고 즉시 거부한다. 더 짧거나 긴 목록을 항목별로 비교하면 두 목록이 처음 갈라지는 항목에서 불일치가 나오며, 그것은 실제로 무엇이 다른지에 대해 아무것도 말하지 않는다.
 */
int
compare_critical_bytes(NHFILE *nhfp, int *idx_1st_mismatch, unsigned long utdflags)
{
    char active_csc_count = (char) SIZE(critical_sizes),
         file_csc_count;
    int i, cnt = (int) active_csc_count,
        dmmismatch = SF_DM_MISMATCH;
    boolean quietly = (utdflags & UTD_QUIETLY) != 0;

    Sfi_char(nhfp, &file_csc_count, "count-critical_sizes", 1);
    if (file_csc_count > cnt) {
        if (!quietly)
            raw_printf("critical byte counts do not match"
                       ", file:%d, critical_sizes:%d.",
                       file_csc_count, SIZE(critical_sizes));
        return SF_CRITICAL_BYTE_COUNT_MISMATCH;
    }
    for (i = 0; i < (int) file_csc_count; ++i) {
        Sfi_uchar(nhfp, &cscbuf[i], "critical_sizes");
    }

    for (i = 1; i < cnt; ++i) {
#ifndef SFCTOOL
        if (cscbuf[i] != critical_sizes[i].ucsize && i == cnt - 1) {
            /* SAVEFILE_REVISION_LEVEL mismatch; attempt to deal with it */
            int file_rev_level = cscbuf[i];

            if (revision_increment(file_rev_level,
                                                   file_csc_count,
                                                   cscbuf))
                continue;
        }
#endif
        if (cscbuf[i] != critical_sizes[i].ucsize) {
            const char *dm = datamodel(0), *dmfile;

            dmfile = what_datamodel_is_this(0,
                                            cscbuf[1],  /* short */
                                            cscbuf[2],  /* int */
                                            cscbuf[3],  /* long */
                                            cscbuf[4],  /* long long */
                                            cscbuf[5]); /* ptr */

            if (!strcmp(dmfile, "IL32LLP64") && !strcmp(dm, "ILP32LL64")) {
                /*  Windows x64 savefile on x86 */
                dmmismatch = SF_DM_IL32LLP64_ON_ILP32LL64;
            } else if (!strcmp(dmfile, "I32LP64")
                       && !strcmp(dm, "ILP32LL64")) {
                /* Unix 64 savefile on x86*/
                dmmismatch = SF_DM_I32LP64_ON_ILP32LL64;
            } else if (!strcmp(dmfile, "ILP32LL64")
                       && !strcmp(dm, "I32LP64")) {
                /*  x86 savefile on Unix 64 */
                dmmismatch = SF_DM_ILP32LL64_ON_I32LP64;
            } else if (!strcmp(dmfile, "ILP32LL64")
                       && !strcmp(dm, "IL32LLP64")) {
                /* x86 savefile on Windows x64 */
                dmmismatch = SF_DM_ILP32LL64_ON_IL32LLP64;
            } else if (!strcmp(dmfile, "I32LP64")
                       && !strcmp(dm, "IL32LLP64")) {
                /* Unix 64 savefile on Windows x64 */
                dmmismatch = SF_DM_I32LP64_ON_IL32LLP64;
            } else if (!strcmp(dmfile, "IL32LLP64")
                       && !strcmp(dm, "I32LP64")) {
                /* Windows x64 savefile on Unix 64 */
                dmmismatch = SF_DM_IL32LLP64_ON_I32LP64;
            }
            if (idx_1st_mismatch)
                *idx_1st_mismatch = i;
            return dmmismatch;
        }
    }
    return SF_UPTODATE; /* everything matched */
}

/*
 * returns:
 *
 *   SF_UPTODATE                     (0) everything matched and looks good
 *   SF_OUTDATED                     (1) savefile is outdated
 *   SF_CRITICAL_BYTE_COUNT_MISMATCH (2) critical size count mismatch
 *   SF_DM_IL32LLP64_ON_ILP32LL64    (3) Windows x64 savefile on x86
 *   SF_DM_I32LP64_ON_ILP32LL64      (4) Unix 64 savefile on x86
 *   SF_DM_ILP32LL64_ON_I32LP64      (5) x86 savefile on Unix 64
 *   SF_DM_ILP32LL64_ON_IL32LLP64    (6) x86 savefile on Windows x64
 *   SF_DM_I32LP64_ON_IL32LLP64      (7) Unix 64 savefile on Windows x64
 *   SF_DM_IL32LLP64_ON_I32LP64      (8) Windows x64 savefile on Unix 64
 *   SF_DM_MISMATCH                  (9) some other mismatch
 */
/**
 * @brief Check a file and, if it is unusable, tell the player why in words.
 *
 * The player-facing wrapper. The checking routine returns a code; this turns that code into an explanation, which is the difference that matters -- a player who has moved a save file between two machines needs to be told that, not
 * given a number.
 *
 * @param nhfp the file
 * @param name the file's name
 * @param without_waitsynch_perfile whether to skip pausing after each file's message
 * @return the same result the check produced; zero means usable
 * @note The pause is suppressible because several files may be checked in a row -- pausing after each would make examining a directory of save files tedious rather than informative.
 */
/**
 * @brief 파일을 검사하고, 쓸 수 없으면 그 이유를 플레이어에게 말로 알린다.
 *
 * 플레이어를 향한 감싸기다. 검사 루틴은 코드를 반환하고, 이것이 그 코드를 설명으로 바꾼다. 그것이 중요한 차이다. 저장 파일을 두 기계 사이에서 옮긴 플레이어는 숫자를 받는 것이 아니라 그것을 들어야 한다.
 *
 * @param nhfp 그 파일
 * @param name 그 파일의 이름
 * @param without_waitsynch_perfile 파일마다의 메시지 뒤에 멈추기를 건너뛸지
 * @return 검사가 낸 것과 같은 결과. 0은 쓸 수 있다는 뜻이다
 * @note 그 멈춤을 억제할 수 있는 것은 여러 파일이 잇따라 검사될 수 있기 때문이다. 각각마다 멈추면 저장 파일 디렉토리를 살피는 일이 정보를 주기보다 번거로워진다.
 */
int
validate(NHFILE *nhfp, const char *name, boolean without_waitsynch_perfile,
         int additional_utd_flags)
{
    unsigned long utdflags = 0L;
    int validsf = 0;

if (additional_utd_flags)
        utdflags |= additional_utd_flags;
#ifdef SFCTOOL
    utdflags |= UTD_QUIETLY;
#endif
    if (nhfp->structlevel)
        utdflags |= UTD_CHECKSIZES;
    if (without_waitsynch_perfile)
        utdflags |= UTD_WITHOUT_WAITSYNCH_PERFILE;
    if (nhfp->fieldlevel)
        utdflags |= UTD_CHECKFIELDCOUNTS | UTD_SKIP_SANITY1;
    validsf = uptodate(nhfp, name, utdflags);
    return validsf;
}
#endif /* MINIMAL_FOR_RECOVER */

/*version.c*/
