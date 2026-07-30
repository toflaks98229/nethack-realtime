/* NetHack 5.0	earlyarg.c	$NHDT-Date: 1782016695 2026/06/20 23:38:15 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.11 $ */
/* Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file earlyarg.c
 * @brief Command-line arguments that must be handled before anything exists.
 *
 * A few arguments cannot wait for normal option processing: asking for the
 * version or the file paths must work without a game, and choosing a window
 * system has to happen before there is a window to report errors through.
 *
 * So these are recognized in a first pass over the command line, acted on, and
 * removed from what the ordinary parser will later see.
 *
 * @warning Runs before the window system, so nothing here may report through
 *          it; output goes to the terminal and errors end the process.
 */

/**
 * @file earlyarg.c
 * @brief 아직 아무것도 존재하지 않는 시점에 처리해야 하는 명령행 인자들.
 *
 * 일부 인자는 일반적인 옵션 처리를 기다릴 수 없다. 버전이나 파일 경로를 묻는 일은
 * 게임 없이도 동작해야 하고, 윈도우 시스템을 고르는 일은 오류를 알릴 창이 생기기
 * 전에 이루어져야 한다.
 *
 * 그래서 이 인자들은 명령행에 대한 첫 번째 훑기에서 인식되어 처리되고, 나중에
 * 일반 파서가 보게 될 목록에서 제거된다.
 *
 * @warning 윈도우 시스템보다 먼저 실행되므로 여기의 어떤 것도 그것을 통해 알릴 수
 *          없다. 출력은 터미널로 가고, 오류는 프로세스를 끝낸다.
 */

#include "hack.h"
#include "dlb.h"

staticfn void debug_fields(char *);
#ifndef NODUMPENUMS
staticfn void dump_enums(void);
#endif
ATTRNORETURN staticfn void opt_terminate(void) NORETURN;
ATTRNORETURN staticfn void opt_usage(const char *) NORETURN;
ATTRNORETURN staticfn void scores_only(int, char **, const char *) NORETURN;
staticfn char *lopt(char *, int, const char *, const char *, int *, char ***);
staticfn void consume_arg(int, int *, char ***);
staticfn void consume_two_args(int, int *, char ***);

#ifdef UNIX
extern boolean whoami(void);
#endif

/*
 * Argument processing helpers - for xxmain() to share
 * and call.
 *
 * These should return TRUE if the argument matched,
 * whether the processing of the argument was
 * successful or not.
 *
 * Most of these do their thing, then after returning
 * to xxmain(), the code exits without starting a game.
 *
 */

/**
 * @var earlyopts
 * @brief The arguments recognised in this first pass, and how to recognise them.
 *
 * Each entry gives the argument's name, how many characters of it must be typed to be accepted, and whether it may be abbreviated at all. So "-vers" is accepted as the version request while "-ver" is not -- the minimum length is what
 * stops a shorter abbreviation from becoming ambiguous with something added later.
 *
 * @note Several entries exist only in some builds. An argument that is not compiled in is not merely ignored but unrecognised, so it falls through to the ordinary parser and is reported as unknown -- which is the right answer for a build
 *       that cannot honour it.
 * @warning The minimum lengths are chosen by hand and nothing checks them against each other. Two entries whose minimum-length prefixes coincide would both match the same abbreviation, and the first in the table would silently win.
 */
/**
 * @var earlyopts
 * @brief 이 첫 번째 훑기에서 인식되는 인자들, 그리고 그것을 어떻게 인식하는지.
 *
 * 각 항목은 인자의 이름, 받아들여지려면 그것의 몇 문자가 입력되어야 하는지, 그리고 그것이 줄여질 수 있기는 한지를 준다. 그래서 "-vers"는 판본 요청으로 받아들여지고 "-ver"는 아니다. 최소 길이가 더 짧은 줄임말이 나중에 더해진 무엇과 애매해지는 것을 막는 것이다.
 *
 * @note 몇몇 항목은 어떤 빌드에만 존재한다. 컴파일에 포함되지 않은 인자는 그저 무시되는 것이 아니라 인식되지 않으므로, 일반 파서로 떨어져 알 수 없는 것으로 보고된다. 그것을 존중할 수 없는 빌드에 대해 그것이 옳은 답이다.
 * @warning 최소 길이는 손으로 골라지며 무엇도 그것들을 서로 견주어 검사하지 않는다. 최소 길이 앞부분이 겹치는 두 항목은 둘 다 같은 줄임말과 맞을 것이고, 표에서 앞선 것이 조용히 이길 것이다.
 */
static const struct early_opt earlyopts[] = {
    { ARG_DEBUG, "debug", 5, TRUE },
    { ARG_VERSION, "version", 4, TRUE },
    { ARG_SHOWPATHS, "showpaths", 8, FALSE },
#ifndef NODUMPENUMS
    { ARG_DUMPENUMS, "dumpenums", 9, FALSE },
#endif
    { ARG_DUMPGLYPHIDS, "dumpglyphnames", 12, FALSE },
    { ARG_DUMPMONGEN, "dumpmongen", 10, FALSE },
    { ARG_DUMPWEIGHTS, "dumpweights", 11, FALSE },
#ifdef WIN32
    { ARG_WINDOWS, "windows", 4, TRUE },
#endif
#if defined(CRASHREPORT)
    { ARG_BIDSHOW, "bidshow", 7, FALSE },
#endif
};

/**
 * @brief What is reported as an argument's value when it had none.
 *
 * A placeholder rather than an empty string, so that an error message about a missing value reads as a sentence instead of trailing off. The reader of "unexpected value [nothing]" can tell what happened.
 *
 * @warning Deliberately not constant, as the existing comment records. It is handed back where an actual argument value would be, and the caller may write into what it receives -- so it cannot be a string literal.
 */
/**
 * @brief 인자가 값을 갖지 않았을 때 그 값으로 보고되는 것.
 *
 * 빈 문자열이 아니라 자리표여서, 없는 값에 대한 오류 메시지가 흐려지는 대신 문장으로 읽힌다. "예상치 못한 값 [nothing]"을 읽는 사람은 무슨 일이 있었는지 말할 수 있다.
 *
 * @warning 기존 주석이 기록하듯 의도적으로 상수가 아니다. 그것은 실제 인자 값이 있을 자리에 돌려주어지고, 호출자가 자기가 받은 것에 써넣을 수 있다. 그러니 그것은 문자열 상수일 수 없다.
 */
static char ArgVal_novalue[] = "[nothing]"; /* note: not 'const' */

/**
 * @brief How to treat one argument: whether it takes a value, how it is named, and whether to complain.
 *
 * Three independent choices packed into one value, each occupying its own bits with a mask to extract it. They are combined rather than passed separately because they always travel together.
 *
 * @note The value-handling choice has three states, not two. An argument may require a value, accept one optionally, or refuse one -- and refusing is different from not requiring, because "-showpaths=x" should be reported as wrong rather
 *       than accepted with the value ignored.
 * @note The zero values are the defaults that fall out of not setting anything: a value is required, the name is a long one, and errors are silent. So an argument described by zero is the ordinary case.
 * @warning The masks must match the values they cover. A choice extracted with the wrong mask yields a state that looks valid, so adding a fourth choice means widening the earlier masks rather than only appending bits.
 */
/**
 * @brief 한 인자를 어떻게 다룰지. 값을 취하는지, 어떻게 이름 지어지는지, 불평할지.
 *
 * 세 독립적인 선택이 하나의 값에 압축되며, 각각이 자기 비트를 차지하고 그것을 뽑아낼 가리개를 갖는다. 그것들이 따로 넘겨지는 대신 합쳐지는 것은 그것들이 언제나 함께 다니기 때문이다.
 *
 * @note 값 다루기 선택은 둘이 아니라 세 상태를 갖는다. 인자는 값을 요구할 수도, 선택적으로 받아들일 수도, 거부할 수도 있다. 그리고 거부하는 것은 요구하지 않는 것과 다르다. "-showpaths=x"는 값이 무시된 채 받아들여지는 대신 틀렸다고 보고되어야 하기 때문이다.
 * @note 영 값들은 무엇도 설정하지 않은 데서 떨어지는 기본값이다. 값이 요구되고, 이름은 긴 것이고, 오류는 조용하다. 그러니 영으로 서술되는 인자가 보통의 경우다.
 * @warning 가리개는 자기가 덮는 값과 맞아야 한다. 틀린 가리개로 뽑아낸 선택은 유효해 보이는 상태를 내므로, 네 번째 선택을 더하는 것은 비트를 덧붙이기만 하는 것이 아니라 앞선 가리개를 넓히는 일을 뜻한다.
 */
enum cmdlinearg {
    ArgValRequired = 0,
    ArgValOptional = 1,
    ArgValDisallowed = 2,
    ArgVal_mask = (1 | 2),
    ArgNamOneLetter = 4,
    ArgNam_mask = 4,
    ArgErrSilent = 0,
    ArgErrComplain = 8,
    ArgErr_mask = 8
};

/**
 * @brief Match one long-form argument and find its value, in the manner of the standard option parser.
 *
 * A local approximation of what the system's long-option parser does, written because this runs before anything is initialised and cannot rely on a library that may not be present. As the existing comment notes, the examples throughout name
 * one particular argument but nothing here is specific to it.
 *
 * A value can arrive three ways: joined by an equals or a colon, as the following separate token, or not at all. The three are accepted or refused according to whether this argument requires a value, permits one, or forbids one.
 *
 * @param arg the token, whose beginning matches the argument's name
 * @param lflags how to treat this argument -- whether a value is required, whether a one-letter form is allowed, whether to complain
 * @param optname the argument's name
 * @param origarg the token as the user typed it, used in messages; @p arg may have had a leading dash removed
 * @param argc_p the remaining argument count, decreased if the following token is taken as the value
 * @param argv_p the remaining arguments, advanced likewise
 * @return the value, or null if the token did not match or the value was wrong
 * @note Taking the value from the following token consumes that token, which is why the count and list are passed by pointer. A caller that ignored the change would then parse the value again as though it were an argument of its own.
 * @note The following token is only considered a value if it does not itself begin with a dash. That is a guess, not a rule: it means an argument requiring a value cannot be given one that looks like another argument, which is accepted
 *       because no such value exists.
 * @note An argument that permits a value but was given none yields the placeholder rather than null, so the caller can tell "present without a value" from "not present". Returning null for both would collapse that distinction.
 * @note Complaining is optional because this is called speculatively -- trying each known argument against a token in turn -- and a failed match is not an error until every argument has failed.
 * @warning Only the first letter is compared before anything else, as a quick rejection. Two arguments sharing a first letter both reach the full comparison, so the one-letter form can only belong to one of them.
 * @note There is disabled code for accepting a colon immediately after a one-letter name. As its comment records, it would work but callers do not expect it.
 */
/**
 * @brief 표준 옵션 파서의 방식으로 긴 형태 인자 하나를 맞추고 그 값을 찾는다.
 *
 * 시스템의 긴 옵션 파서가 하는 일을 국지적으로 근사한 것이며, 이것이 아무것도 초기화되기 전에 실행되고 없을 수도 있는 라이브러리에 의존할 수 없기 때문에 쓰였다. 기존 주석이 적듯 곳곳의 예시가 특정한 한 인자를 이름 짓지만 여기의 무엇도 그것에 국한되지 않는다.
 *
 * 값은 세 방식으로 도착할 수 있다. 등호나 쌍점으로 이어져서, 뒤따르는 따로 된 토큰으로, 또는 아예 오지 않고. 그 셋은 이 인자가 값을 요구하는지, 허용하는지, 금하는지에 따라 받아들여지거나 거부된다.
 *
 * @param arg 시작이 인자 이름과 맞는 토큰
 * @param lflags 이 인자를 어떻게 다룰지. 값이 요구되는지, 한 글자 형태가 허용되는지, 불평할지
 * @param optname 인자의 이름
 * @param origarg 사용자가 입력한 그대로의 토큰. 메시지에 쓰인다. @p arg 는 앞선 붙임표가 없어졌을 수 있다
 * @param argc_p 남은 인자 개수. 뒤따르는 토큰이 값으로 취해지면 줄어든다
 * @param argv_p 남은 인자들. 마찬가지로 나아간다
 * @return 그 값, 또는 토큰이 맞지 않았거나 값이 틀렸으면 널
 * @note 뒤따르는 토큰에서 값을 취하는 것은 그 토큰을 소비하며, 그것이 개수와 목록이 포인터로 넘겨지는 이유다. 그 변경을 무시한 호출자는 그다음 그 값을 자기 나름의 인자인 것처럼 다시 해석할 것이다.
 * @note 뒤따르는 토큰은 그것 자체가 붙임표로 시작하지 않을 때만 값으로 여겨진다. 그것은 규칙이 아니라 짐작이다. 값을 요구하는 인자가 다른 인자처럼 보이는 값을 받을 수 없다는 뜻이며, 그런 값이 없으므로 받아들여진다.
 * @note 값을 허용하지만 아무것도 받지 않은 인자는 널이 아니라 자리표를 낸다. 그래서 호출자가 "값 없이 있음"과 "없음"을 구별할 수 있다. 둘 다에 널을 돌려주는 것은 그 구별을 무너뜨릴 것이다.
 * @note 불평하기가 선택적인 것은 이것이 추측으로 호출되기 때문이다. 알려진 각 인자를 토큰에 차례로 견주어 보는 것이며, 맞추기 실패는 모든 인자가 실패하기 전까지는 오류가 아니다.
 * @warning 다른 무엇보다 먼저 첫 글자만이 빠른 거부로서 비교된다. 첫 글자를 공유하는 두 인자는 둘 다 온전한 비교에 이르므로, 한 글자 형태는 그중 하나에만 속할 수 있다.
 * @note 한 글자 이름 바로 뒤의 쌍점을 받아들이는 비활성화된 코드가 있다. 그 주석이 기록하듯 그것은 동작하겠지만 호출자들이 그것을 예상하지 않는다.
 */
staticfn char *
lopt(char *arg,  /* command line token; beginning matches 'optname' */
     int lflags, /* cmdlinearg | errorhandling */
     const char *optname, /* option's name; "-windowtype" in examples below */
     const char *origarg, /* 'arg' might have had a dash prefix removed */
     int *argc_p,         /* argc that can have changes passed to caller */
     char ***argv_p)      /* argv[] ditto */
{
    int argc = *argc_p;
    char **argv = *argv_p;
    char *p, *nextarg = (argc > 1 && argv[1][0] != '-') ? argv[1] : 0;
    int l, opttype = (lflags & ArgVal_mask);
    boolean oneletterok = ((lflags & ArgNam_mask) == ArgNamOneLetter),
            complain = ((lflags & ArgErr_mask) == ArgErrComplain);

    /* first letter must match */
    if (arg[1] != optname[1]) {
    loptbail:
        if (complain)
            config_error_add("Unknown option: %.60s", origarg);
        return (char *) 0;
    loptnotallowed:
        if (complain)
            config_error_add("Value not allowed: %.60s", origarg);
        return (char *) 0;
    loptrequired:
        if (complain)
            config_error_add("Missing required value: %.60s", origarg);
        return (char *) 0;
    }

    if ((p = strchr(arg, '=')) == 0)
        p = strchr(arg, ':');
    if (p && opttype == ArgValDisallowed)
        goto loptnotallowed;

    l = (int) (p ? (long) (p - arg) : (long) strlen(arg));
    if ((l > 2 || oneletterok) && !strncmp(arg, optname, l)) {
        /* "-windowtype[=foo]" */
        if (p)
            ++p; /* past '=' or ':' */
        else if (opttype == ArgValRequired)
            p = eos(arg); /* we have "-w[indowtype]" w/o "=foo"
                           * so we'll take foo from next element */
        else
            return ArgVal_novalue;
    } else if (oneletterok) {
        /* "-w..." but not "-w[indowtype[=foo]]" */
        if (!p) {
            p = &arg[2]; /* past 'w' of "-wfoo" */
#if 0 /* -x:value could work but is not supported (callers don't expect it) \
       */
        } else if (p == arg + 2) {
            ++p; /* past ':' of "-w:foo" */
#endif
        } else {
            /* "-w...=foo" but not "-w[indowtype]=foo" */
            goto loptbail;
        }
    } else {
        goto loptbail;
    }
    if (!p || !*p) {
        /* "-w[indowtype]" w/o '='/':' if there is a next element, use
           it for "foo"; if not, supply a non-Null bogus value */
        if (nextarg
            && (opttype == ArgValRequired || opttype == ArgValOptional))
            p = nextarg, --(*argc_p), ++(*argv_p);
        else if (opttype == ArgValRequired)
            goto loptrequired;
        else
            p = ArgVal_novalue; /* there is no next element */
    }
    return p;
}
/**
 * @brief Hide an argument this pass has dealt with, so the ordinary parser will not see it.
 *
 * Done by moving the token to the end of the list and shortening the count, rather than by removing it. Nothing is discarded, as the existing comment records -- the tokens are only reordered -- which matters because the list belongs to the
 * system and something else may still want to read the whole of it.
 *
 * @param ndx which argument to hide
 * @param ac_p the count, reduced by one
 * @param av_p the list, whose contents are rearranged
 * @note Leaves a single-argument list alone apart from the count, since there is nowhere to move the only token to.
 * @warning Every argument after the hidden one shifts down by one position. A caller iterating by index must not advance after calling this, or it will skip the token that moved into the vacated slot.
 */
/**
 * @brief 이 훑기가 처리한 인자를 감추어 일반 파서가 그것을 보지 않게 한다.
 *
 * 제거함으로써가 아니라 토큰을 목록 끝으로 옮기고 개수를 줄임으로써 이루어진다. 기존 주석이 기록하듯 무엇도 버려지지 않고 토큰들이 순서만 바뀐다. 그것이 중요한 것은 그 목록이 시스템의 것이고 다른 무엇이 여전히 그 전체를 읽고 싶어 할 수 있기 때문이다.
 *
 * @param ndx 어느 인자를 감출지
 * @param ac_p 개수. 하나 줄어든다
 * @param av_p 목록. 그 내용이 재배치된다
 * @note 인자가 하나뿐인 목록은 개수 말고는 그대로 둔다. 유일한 토큰을 옮길 곳이 없기 때문이다.
 * @warning 감춰진 것 뒤의 모든 인자가 한 자리씩 내려간다. 색인으로 순회하는 호출자는 이것을 호출한 뒤 나아가서는 안 된다. 그러지 않으면 비워진 자리로 옮겨 온 토큰을 건너뛸 것이다.
 */
staticfn void
consume_arg(int ndx, int *ac_p, char ***av_p)
{
    char *gone, **av = *av_p;
    int i, ac = *ac_p;

    /* "-one -two -three -four" -> "-two -three -four -one" */
    if (ac > 2) {
        gone = av[ndx];
        for (i = ndx + 1; i < ac; ++i)
            av[i - 1] = av[i];
        av[ac - 1] = gone;
    }
    --(*ac_p);
}

/**
 * @brief Hide an argument and the separate token holding its value, keeping the two together.
 *
 * Not the same as hiding twice. Hiding an argument moves it to the end, so hiding the name and then the value would leave them at the end in swapped order -- the existing comment gives the exact before and after. Since the hidden tokens
 * remain in the list, that reversal would be visible to anything that reads past the shortened count.
 *
 * The order is preserved by briefly restoring the slot just vacated, so the second hide moves the value past the name rather than in front of it.
 *
 * @param ndx which argument to hide, the value being the one after it
 * @param ac_p the count, reduced by two
 * @param av_p the list, whose contents are rearranged
 * @note The count is nudged up and back down around the second hide. That is not a correction of a mistake but the mechanism itself: the temporarily visible slot is what the value is moved into.
 */
/**
 * @brief 인자와 그 값을 담은 따로 된 토큰을 감추면서, 그 둘을 함께 유지한다.
 *
 * 두 번 감추는 것과 같지 않다. 인자를 감추는 것은 그것을 끝으로 옮기므로, 이름을 감추고 그다음 값을 감추면 그것들이 뒤바뀐 순서로 끝에 남을 것이다. 기존 주석이 정확한 전과 후를 준다. 감춰진 토큰들이 목록에 남으므로, 그 뒤바뀜은 짧아진 개수를 넘어 읽는 무엇에게든 보일 것이다.
 *
 * 순서는 방금 비워진 자리를 잠시 되돌림으로써 보존되며, 그래서 두 번째 감추기가 값을 이름 앞이 아니라 뒤로 옮긴다.
 *
 * @param ndx 어느 인자를 감출지. 값은 그 다음의 것이다
 * @param ac_p 개수. 둘 줄어든다
 * @param av_p 목록. 그 내용이 재배치된다
 * @note 개수가 두 번째 감추기를 둘러싸고 올려졌다 다시 내려진다. 그것은 실수의 교정이 아니라 방식 자체다. 잠시 보이게 된 그 자리가 값이 옮겨 들어가는 곳이다.
 */
staticfn void
consume_two_args(int ndx, int *ac_p, char ***av_p)
{
    /* when consuming "-two arg" from "-two arg -three -four",
       the *ac_p manipulation results in "-three -four -two arg"
       rather than the "-three -four arg -two" that would happen
       with just two ordinary consume_arg() calls */
    consume_arg(ndx, ac_p, av_p);
    ++(*ac_p); /* bring the final slot back into view */
    consume_arg(ndx, ac_p, av_p);
    --(*ac_p); /* take away restored slot */
}

/**
 * @brief Walk the command line, act on the arguments that cannot wait, and hide them from the ordinary parser.
 *
 * Dispatched on the character after the leading dash rather than by comparing whole names, which is what keeps a single pass over the command line from being a comparison against every known argument.
 *
 * Several of these arguments end the process rather than returning: asking for the version, the usage, the scores or one of the data dumps is a request to report something and stop, not a way to start a game.
 *
 * @param argc_p the argument count, reduced as arguments are hidden
 * @param argv_p the arguments, rearranged as they are hidden
 * @param hackdir_p the game directory, which the directory argument replaces
 * @note The loop does not advance when an argument was consumed, because hiding one shifts the rest down into its place. That is why the index is advanced conditionally rather than by the loop itself.
 * @note A doubled dash is accepted as a single one, but only for names longer than one character -- so both "-windowtype" and "--windowtype" work while "--w" does not. The check is written out rather than being a rule the parser knows.
 * @note A bare question mark is treated as a request for usage, since that is what a player would try. As the existing comment concedes, the shell usually eats it, and it does not work if it follows a directory argument.
 * @note The program name is skipped by starting at the second element. As the existing comment records, the argument-checking helper does not mind either way but the score printer counts arguments, which is why the scores case adjusts the
 *       pointer back by one before handing over.
 * @note The window system and configuration file are copied rather than pointed at, because the command line's memory is not the game's to keep.
 * @note Showing the paths is deferred rather than done here -- it records the request and returns. The paths are not all known yet at this point, so reporting them now would report them wrongly.
 * @note The directory argument refuses a value beginning with a particular letter, to avoid matching two other arguments that share its first letter. That is a consequence of dispatching on one character, and the comment at that point
 *       names the two.
 * @note Errors are collected and counted rather than reported one at a time, so a command line with several mistakes produces one report.
 */
/**
 * @brief 명령행을 걸어가며, 기다릴 수 없는 인자에 대해 행동하고, 그것을 일반 파서에게서 감춘다.
 *
 * 온전한 이름을 비교하는 대신 앞선 붙임표 다음 문자로 갈라 보낸다. 그것이 명령행에 대한 한 번의 훑기가 알려진 모든 인자와의 비교가 되지 않게 하는 것이다.
 *
 * 이 인자 중 몇은 돌아오는 대신 프로세스를 끝낸다. 판본, 사용법, 점수, 또는 데이터 출력 중 하나를 요청하는 것은 무언가를 보고하고 멈추라는 요청이며, 게임을 시작하는 방법이 아니다.
 *
 * @param argc_p 인자 개수. 인자가 감춰지며 줄어든다
 * @param argv_p 인자들. 감춰지며 재배치된다
 * @param hackdir_p 게임 디렉터리. 디렉터리 인자가 그것을 대신한다
 * @note 인자가 소비되었을 때 되돌기가 나아가지 않는다. 하나를 감추는 것이 나머지를 그 자리로 내려보내기 때문이다. 그것이 색인이 되돌기 자신에 의해서가 아니라 조건에 따라 나아가는 이유다.
 * @note 겹친 붙임표가 하나로 받아들여지지만, 한 글자보다 긴 이름에 대해서만이다. 그래서 "-windowtype"과 "--windowtype"은 모두 되고 "--w"는 안 된다. 그 검사는 파서가 아는 규칙이 아니라 풀어 쓰여 있다.
 * @note 맨 물음표가 사용법 요청으로 다뤄지는데, 플레이어가 시도해 볼 것이 그것이기 때문이다. 기존 주석이 인정하듯 셸이 보통 그것을 먹어 버리고, 디렉터리 인자를 뒤따르면 동작하지 않는다.
 * @note 프로그램 이름은 두 번째 요소에서 시작함으로써 건너뛰어진다. 기존 주석이 기록하듯 인자 검사 도우미는 어느 쪽이든 개의치 않지만 점수 인쇄기는 인자를 세며, 그것이 점수 경우가 넘겨주기 전에 포인터를 하나 되돌리는 이유다.
 * @note 창 체계와 설정 파일은 가리켜지는 대신 복사된다. 명령행의 메모리는 게임이 지닐 것이 아니기 때문이다.
 * @note 경로를 보이는 것은 여기서 이루어지는 대신 미뤄진다. 그것은 요청을 기록하고 돌아온다. 이 시점에 경로가 아직 모두 알려지지 않았으므로, 지금 그것을 보고하는 것은 그것을 틀리게 보고하는 일일 것이다.
 * @note 디렉터리 인자는 특정 글자로 시작하는 값을 거부해, 그 첫 글자를 공유하는 다른 두 인자와 맞는 것을 피한다. 그것은 한 문자로 갈라 보내는 것의 결과이며, 그 지점의 주석이 그 둘을 이름 짓는다.
 * @note 오류는 하나씩 보고되는 대신 모아져 세어지므로, 실수가 여럿인 명령행이 하나의 보고를 낸다.
 */
void
early_options(int *argc_p, char ***argv_p, char **hackdir_p)
{
    char **argv, *arg, *origarg;
    int argc, oldargc, ndx = 0, consumed = 0;

#ifdef ENHANCED_SYMBOLS
    if (argcheck(*argc_p, *argv_p, ARG_DUMPGLYPHIDS) == 2)
        opt_terminate();
#endif

    config_error_init(FALSE, "command line", FALSE);

    /* treat "nethack ?" as a request for usage info; due to shell
       processing, player likely has to use "nethack \?" or "nethack '?'"
       [won't work if used as "nethack -dpath ?" or "nethack -d path ?"] */
    if (*argc_p > 1 && !strcmp((*argv_p)[1], "?"))
        opt_usage(*hackdir_p); /* doesn't return */

    /*
     * Both *argc_p and *argv_p account for the program name as (*argv_p)[0];
     * local argc and argv implicitly discard that (by starting 'ndx' at 1).
     * argcheck() doesn't mind, prscore() (via scores_only()) does (for the
     * number of args it gets passed, not for the value of argv[0]).
     */
    for (ndx = 1; ndx < *argc_p; ndx += (consumed ? 0 : 1)) {
        consumed = 0;
        argc = *argc_p - ndx;
        argv = *argv_p + ndx;

        arg = origarg = argv[0];
        /* skip any args intended for deferred options */
        if (*arg != '-')
            continue;
        /* allow second dash if arg name is longer than one character */
        if (arg[0] == '-' && arg[1] == '-' && arg[2] != '\0'
            && (arg[3] != '\0' && arg[3] != '=' && arg[3] != ':'))
            ++arg;

        switch (arg[1]) { /* char after leading dash */
        case 'b':
#ifdef CRASHREPORT
            // --bidshow
            if (argcheck(argc, argv, ARG_BIDSHOW) == 2) {
                opt_terminate();
                /*NOTREACHED*/
            }
#endif
            break;
        case 'd':
            if (argcheck(argc, argv, ARG_DEBUG) == 1) {
                consume_arg(ndx, argc_p, argv_p), consumed = 1;
#ifndef NODUMPENUMS
            } else if (argcheck(argc, argv, ARG_DUMPENUMS) == 2) {
                opt_terminate();
                /*NOTREACHED*/
#endif
            } else if (argcheck(argc, argv, ARG_DUMPMONGEN) == 2) {
                opt_terminate();
                /*NOTREACHED*/
            } else if (argcheck(argc, argv, ARG_DUMPWEIGHTS) == 2) {
                opt_terminate();
                /*NOTREACHED*/
            } else {
#ifdef CHDIR
                oldargc = argc;
                arg = lopt(arg,
                           (ArgValRequired | ArgNamOneLetter | ArgErrSilent),
                           "-directory", origarg, &argc, &argv);
                if (!arg)
                    error("Flag -d must be followed by a directory name.");
                if (*arg != 'e') { /* avoid matching -decgraphics or -debug */
                    *hackdir_p = arg;
                    if (oldargc == argc)
                        consume_arg(ndx, argc_p, argv_p), consumed = 1;
                    else
                        consume_two_args(ndx, argc_p, argv_p), consumed = 2;
                }
#endif /* CHDIR */
            }
            break;
        case 'h':
        case '?':
            if (lopt(arg, ArgValDisallowed, "-help", origarg, &argc, &argv)
                || lopt(arg, ArgValDisallowed | ArgNamOneLetter, "-?",
                        origarg, &argc, &argv))
                opt_usage(*hackdir_p); /* doesn't return */
            break;
        case 'n':
            oldargc = argc;
            if (!strcmp(arg, "-no-nethackrc")) /* no abbreviation allowed */
                arg = nhStr("/dev/null");
            else
                arg = lopt(arg, (ArgValRequired | ArgErrComplain),
                           "-nethackrc", origarg, &argc, &argv);
            if (arg) {
                gc.cmdline_rcfile = dupstr(arg);
                if (oldargc == argc)
                    consume_arg(ndx, argc_p, argv_p), consumed = 1;
                else
                    consume_two_args(ndx, argc_p, argv_p), consumed = 2;
            }
            break;
        case 's':
            if (argcheck(argc, argv, ARG_SHOWPATHS) == 2) {
                gd.deferred_showpaths = TRUE;
                gd.deferred_showpaths_dir = *hackdir_p;
                config_error_done();
                return;
            }
            /* check for "-s" request to show scores */
            if (lopt(arg,
                     ((ArgValDisallowed | ArgErrComplain)
                      /* only accept one-letter if there is just one
                         dash; reject "--s" because prscore() via
                         scores_only() doesn't understand it */
                      | ((origarg[1] != '-') ? ArgNamOneLetter : 0)),
                     /* [ought to omit val-disallowed and accept
                        --scores=foo since -s foo and -sfoo are
                        allowed, but -s form can take more than one
                        space-separated argument and --scores=foo
                        isn't suited for that] */
                     "-scores", origarg, &argc, &argv)) {
                /* at this point, argv[0] contains "-scores" or a leading
                   substring of it; prscore() (via scores_only()) expects
                   that to be in argv[1] so we adjust the pointer to make
                   that be the case; if there are any non-early args waiting
                   to be passed along to process_options(), the resulting
                   argv[0] will be one of those rather than the program
                   name but prscore() doesn't care */
                scores_only(argc + 1, argv - 1, *hackdir_p);
                /*NOTREACHED*/
            }
            break;
        case 'u':
#if defined(UNIX)
            if (lopt(arg, ArgValDisallowed, "-usage", origarg, &argc, &argv))
                opt_usage(*hackdir_p);
#elif defined(WIN32) || defined(MSDOS) || defined(AMIGA)
            if (arg[2]) {
                (void) strncpy(svp.plname, arg + 2, sizeof(svp.plname) - 1);
            } else if (ndx + 1 < *argc_p) {
                const char *nextarg = (*argv_p)[ndx + 1];

                if (nextarg[0] != '-') {
                    (void) strncpy(svp.plname, nextarg, sizeof(svp.plname) - 1);
                } else {
                    raw_print("Player name expected after -u\n");
                }
            }
#endif
            break;
        case 'v':
            if (argcheck(argc, argv, ARG_VERSION) == 2) {
                opt_terminate();
                /*NOTREACHED*/
            }
            break;
        case 'w': /* windowtype: "-wfoo" or "-w[indowtype]=foo"
                   * or "-w[indowtype]:foo" or "-w[indowtype] foo" */
            arg =
                lopt(arg, (ArgValRequired | ArgNamOneLetter | ArgErrComplain),
                     "-windowtype", origarg, &argc, &argv);
            if (gc.cmdline_windowsys)
                free((genericptr_t) gc.cmdline_windowsys);
            gc.cmdline_windowsys = arg ? dupstr(arg) : NULL;
            break;
 #if !defined(UNIX) && !defined(VMS)
        case 'D':
            wizard = TRUE, discover = FALSE;
            break;
        case 'X':
            discover = TRUE, wizard = FALSE;
            break;
#endif
        default:
            break;
        }
    }
    /* empty or "N errors on command line" */
    config_error_done();
    return;
}

/**
 * @brief Read the character-choosing arguments: role, race, alignment and gender.
 *
 * Separate from the pass above because these do not need to be early -- they only set what the character will be, which is not consulted until a game starts. They are here because they are still command-line parsing.
 *
 * @param argc the argument count
 * @param argv the arguments
 * @note Each of the four accepts its value either joined to the argument or as the following token, the same two forms the earlier parser supports -- but written out four times rather than shared, so each is independent of the others.
 * @note An unrecognised argument is not an error. As the existing comment records, it is taken as naming a role by its first letter, which is why "-w" starts a Wizard. That makes a mistyped argument silently choose a character rather than
 *       being reported.
 * @note A value that does not name anything is ignored and the choice is left unset, so the game asks. Nothing distinguishes "not specified" from "specified wrongly" at this point.
 * @note Does not hide the arguments it consumed, unlike the early pass, because nothing parses the command line after this.
 * @warning Advances past the program name before reading anything, so it must be given the list including it. Passing an already-advanced list silently loses the first argument.
 */
/**
 * @brief 인물을 고르는 인자를 읽는다. 직업, 종족, 성향, 성별.
 *
 * 위의 훑기와 따로인 것은 이것들이 이를 필요가 없기 때문이다. 그것들은 인물이 무엇이 될지만 정하며, 그것은 게임이 시작되기 전까지 참조되지 않는다. 그럼에도 여기 있는 것은 그것들이 여전히 명령행 해석이기 때문이다.
 *
 * @param argc 인자 개수
 * @param argv 인자들
 * @note 넷 각각이 자기 값을 인자에 이어서든 뒤따르는 토큰으로든 받아들이며, 앞선 파서가 뒷받침하는 것과 같은 두 형태다. 그러나 공유되는 대신 네 번 풀어 쓰여 있으므로, 각각이 다른 것들과 독립적이다.
 * @note 인식되지 않는 인자는 오류가 아니다. 기존 주석이 기록하듯 그것은 첫 글자로 직업을 이름 짓는 것으로 여겨지며, 그것이 "-w"가 마법사를 시작하는 이유다. 그것은 잘못 입력된 인자가 보고되는 대신 조용히 인물을 고르게 만든다.
 * @note 아무것도 이름 짓지 않는 값은 무시되고 그 선택은 설정되지 않은 채로 남으므로, 게임이 물어본다. 이 시점에 "명시되지 않음"과 "틀리게 명시됨"을 구별하는 것은 없다.
 * @note 이른 훑기와 달리 자기가 소비한 인자를 감추지 않는다. 이 뒤로 명령행을 해석하는 것이 없기 때문이다.
 * @warning 무엇이든 읽기 전에 프로그램 이름을 지나 나아가므로, 그것을 포함한 목록이 주어져야 한다. 이미 나아간 목록을 넘기면 조용히 첫 인자를 잃는다.
 */
void
genl_prag(int argc, char *argv[])
{
    char *arg;
    int i;

    config_error_init(FALSE, "command line", FALSE);

    while (argc > 1 && argv[1][0] == '-') {
        argv++;
        argc--;
        arg = argv[0];
        /* allow second dash if arg is longer than one character */
        if (arg[0] == '-' && arg[1] == '-'
            && arg[2] != '\0'
            /* "--a=b" violates the "--" ok when at least 2 chars long rule */
            && (arg[3] != '\0' && arg[3] != '=' && arg[3] != ':'))
            ++arg;
        if (strchr("prag@", arg[1])) {
            switch (arg[1]) {
            case 'a':
                if (arg[2]) {
                    if ((i = str2align(&arg[2])) >= 0)
                        flags.initalign = i;
                } else if (argc > 1) {
                    argc--;
                    argv++;
                    if ((i = str2align(argv[0])) >= 0)
                        flags.initalign = i;
                }
                break;
            case 'g':
                if (arg[2]) {
                    if ((i = str2gend(&arg[2])) >= 0)
                        flags.initgend = i;
                } else if (argc > 1) {
                    argc--;
                    argv++;
                    if ((i = str2gend(argv[0])) >= 0)
                        flags.initgend = i;
                }
                break;
            case 'p': /* profession (role) */
                if (arg[2]) {
                    if ((i = str2role(&arg[2])) >= 0)
                        flags.initrole = i;
                } else if (argc > 1) {
                    argc--;
                    argv++;
                    if ((i = str2role(argv[0])) >= 0)
                        flags.initrole = i;
                }
                break;
            case 'r': /* race */
                if (arg[2]) {
                    if ((i = str2race(&arg[2])) >= 0)
                        flags.initrace = i;
                } else if (argc > 1) {
                    argc--;
                    argv++;
                    if ((i = str2race(argv[0])) >= 0)
                        flags.initrace = i;
                }
                break;
            case '@':
                flags.randomall = 1;
                break;
            }
        } else {
            /* default for "-x" is to play as the role that starts with "x" */
            if ((i = str2role(&argv[1][0])) >= 0) {
                flags.initrole = i;
            }
        }
    }

    /* empty or "N errors on command line" */
    config_error_done();
    return;
}

/**
 * @brief End the program successfully after an argument that reported something instead of starting a game.
 *
 * The common exit for arguments like asking for the version. It is not merely a call to exit: it clears the flag saying early options are still being processed, and releases the error-collecting machinery -- which also emits the summary of
 * any errors gathered.
 *
 * @note Exits with success even though errors may have been reported. The argument did what it was asked to do; a mistyped second argument does not make reporting the version a failure.
 * @note Clearing the early-options flag matters because the shutdown path behaves differently while it is set, and shutting down as though still parsing would skip work that must happen.
 */
/**
 * @brief 게임을 시작하는 대신 무언가를 보고한 인자 뒤에 프로그램을 성공으로 끝낸다.
 *
 * 판본을 묻는 것 같은 인자를 위한 공통된 나가기다. 그것은 단순한 나가기 호출이 아니다. 그것은 이른 옵션이 아직 처리되고 있다고 말하는 표시를 지우고, 오류를 모으는 기계를 해제한다. 그것은 또한 모아진 오류의 요약을 내놓는다.
 *
 * @note 오류가 보고되었을 수 있음에도 성공으로 나간다. 그 인자는 자기가 요청받은 것을 했다. 잘못 입력된 두 번째 인자가 판본을 보고하는 것을 실패로 만들지 않는다.
 * @note 이른 옵션 표시를 지우는 것이 중요한 것은, 그것이 설정된 동안 종료 경로가 다르게 행동하고, 아직 해석하고 있는 것처럼 종료하는 것은 반드시 일어나야 하는 일을 건너뛸 것이기 때문이다.
 */
ATTRNORETURN staticfn void
opt_terminate(void)
{
    program_state.early_options = 0;
    config_error_done(); /* free memory allocated by config_error_init() */

    nh_terminate(EXIT_SUCCESS);
    /*NOTREACHED*/
}

/**
 * @brief Show the usage help and stop.
 * @param hackdir the game directory
 * @note The help is a data file rather than text in the program, so the directory must be entered and the data library opened before it can be read. That is why asking for usage does more setup than asking for the version.
 * @note Does not return.
 */
/**
 * @brief 사용법 도움을 보이고 멈춘다.
 * @param hackdir 게임 디렉터리
 * @note 그 도움은 프로그램 안의 글이 아니라 데이터 파일이므로, 그것이 읽힐 수 있기 전에 디렉터리로 들어가고 데이터 라이브러리가 열려야 한다. 그것이 사용법을 묻는 것이 판본을 묻는 것보다 더 많은 준비를 하는 이유다.
 * @note 돌아오지 않는다.
 */
ATTRNORETURN staticfn void
opt_usage(const char *hackdir)
{
#ifdef CHDIR
    chdirx(hackdir, TRUE);
#else
    nhUse(hackdir);
#endif
    dlb_init();

    genl_display_file(USAGEHELP, TRUE);
    opt_terminate();
}
/**
 * @brief Finish the deferred paths report and stop.
 *
 * The second half of showing the paths. The request is recognised in the early pass but cannot be answered there, because the paths depend on configuration that has not been read yet -- so the reporting itself happens elsewhere and this is
 * only the ending.
 *
 * @param dir the game directory
 * @note Enters the game directory without complaining if it cannot, unlike the usage path which insists. By this point the paths have already been reported, and failing to change directory afterwards would not change what was said.
 * @note Does not return.
 */
/**
 * @brief 미뤄진 경로 보고를 마치고 멈춘다.
 *
 * 경로를 보이는 일의 후반이다. 그 요청은 이른 훑기에서 인식되지만 거기서 답해질 수 없다. 경로가 아직 읽히지 않은 설정에 달려 있기 때문이다. 그래서 보고 자체가 다른 곳에서 일어나며 이것은 그 마무리일 뿐이다.
 *
 * @param dir 게임 디렉터리
 * @note 고집하는 사용법 경로와 달리, 게임 디렉터리로 들어갈 수 없어도 불평하지 않는다. 이 시점에 경로는 이미 보고되었고, 뒤에 디렉터리를 바꾸는 데 실패하는 것이 말해진 것을 바꾸지 않을 것이다.
 * @note 돌아오지 않는다.
 */
ATTRNORETURN void
after_opt_showpaths(const char *dir)
{
#ifdef CHDIR
    chdirx(dir, FALSE);
#else
    nhUse(dir);
#endif
    opt_terminate();
    /*NOTREACHED*/
}

/**
 * @brief Show high-score entries and stop.
 *
 * Reads more configuration than the other reporting arguments, and for a specific reason: the system configuration decides whether crash tracing is enabled, and printing scores must be able to produce a trace if it fails. So the settings are
 * read even though no game will start.
 *
 * @param argc the argument count, as the score printer expects it
 * @param argv the arguments, the score request being the second element
 * @param dir the game directory
 * @note The error summary is emitted first rather than at the end, as the existing comment records, so that it appears before the scores instead of after them.
 * @note Configuration is read with termination suppressed, so a faulty configuration file does not prevent the scores being shown. Reporting scores should not depend on the settings being correct.
 * @note The default player name is worked out on systems that can, so that "-s" with no name shows this player's own scores.
 * @note Waits for the graphical window to be dismissed on builds where the scores appear in one, since otherwise the window would vanish with the process.
 * @note Ends the process directly rather than through the common exit, as the code notes. The cleanup that exit performs has already been done here.
 * @note Does not return.
 */
/**
 * @brief 최고 점수 항목을 보이고 멈춘다.
 *
 * 다른 보고 인자들보다 더 많은 설정을 읽으며, 구체적인 이유가 있다. 시스템 설정이 충돌 추적이 켜지는지를 정하고, 점수 인쇄는 그것이 실패하면 추적을 낼 수 있어야 한다. 그래서 게임이 시작되지 않을 것임에도 설정이 읽힌다.
 *
 * @param argc 점수 인쇄기가 예상하는 대로의 인자 개수
 * @param argv 인자들. 점수 요청이 두 번째 요소다
 * @param dir 게임 디렉터리
 * @note 기존 주석이 기록하듯 오류 요약이 끝이 아니라 먼저 내놓아져서, 점수 뒤가 아니라 앞에 나타난다.
 * @note 설정이 종료가 억제된 채로 읽히므로, 결함 있는 설정 파일이 점수가 보이는 것을 막지 않는다. 점수를 보고하는 것이 설정이 옳은지에 달려서는 안 된다.
 * @note 할 수 있는 시스템에서는 기본 플레이어 이름이 알아내어지므로, 이름 없는 "-s"가 이 플레이어 자신의 점수를 보인다.
 * @note 점수가 창에 나타나는 빌드에서는 그래픽 창이 닫히기를 기다린다. 그러지 않으면 창이 프로세스와 함께 사라질 것이기 때문이다.
 * @note 코드가 적듯 공통된 나가기를 통하는 대신 프로세스를 곧바로 끝낸다. 그 나가기가 하는 정리는 이미 여기서 이루어졌다.
 * @note 돌아오지 않는다.
 */
ATTRNORETURN staticfn void
scores_only(int argc, char **argv, const char *dir)
{
    /* do this now rather than waiting for final termination, in case there
       is an error summary coming */
    config_error_done();

#ifdef CHDIR
    chdirx(dir, FALSE);
#else
    nhUse(dir);
#endif
#ifdef SYSCF
    iflags.initoptions_noterminate = TRUE;
    initoptions(); /* sysconf options affect whether panictrace is enabled */
    iflags.initoptions_noterminate = FALSE;
#endif
#ifdef PANICTRACE
    ARGV0 = gh.hname; /* save for possible stack trace */
#ifndef NO_SIGNAL
    panictrace_setsignals(TRUE);
#endif
#endif
#ifdef UNIX
    (void) whoami(); /* set up default plname[] */
#endif
    prscore(argc, argv);
#ifdef MSWIN_GRAPHICS
    /* NetHackW can also support WINDOWPORT(curses) now, so check */
    if (WINDOWPORT(mswin)) {
        wait_synch();
    }
#endif

    nh_terminate(EXIT_SUCCESS); /* bypass opt_terminate() */
    /*NOTREACHED*/
}

/**
 * @brief Look for one particular early argument on the command line and act on it if found.
 *
 * Asked about one argument at a time rather than being told to parse everything, so a caller can enquire about whichever arguments matter to it. The action is performed here, not by the caller -- so this both finds and does.
 *
 * @param argc the argument count
 * @param argv the arguments
 * @param e_arg which argument to look for
 * @return 0 if not present, 1 if present and the caller should step past it, 2 if the program should now stop, as the existing comment records
 * @note The three-way return is the interface's whole subtlety. A 1 means the argument was handled and play may continue; a 2 means the program has done what was asked and must not start a game. A caller that treated 2 as 1 would report the
 *       version and then play.
 * @note Searches the whole remaining command line rather than only the current position, so an argument is found wherever it appears.
 * @note An argument may carry an extra part after a colon or an equals, which changes what it does. Asking for the version can instead copy it to the clipboard or dump the compatibility values, and an unrecognised extra part is reported and
 *       stops the program rather than being ignored -- a mistyped extra part should not silently give the plain behaviour.
 * @note The doubled-dash form is remembered so that a message about a bad extra part uses the same spelling the player typed.
 * @note The debug argument's extra part is copied before being parsed, because parsing it modifies the text and the command line is not this function's to alter.
 * @note An argument not in the table, or an empty command line, yields no match rather than being an error. Callers ask about arguments their build may not have.
 */
/**
 * @brief 명령행에서 특정한 이른 인자 하나를 찾고, 발견되면 그것에 대해 행동한다.
 *
 * 모든 것을 해석하라고 지시받는 대신 한 번에 한 인자에 대해 물어지므로, 호출자가 자기에게 중요한 인자에 대해 물을 수 있다. 그 행동은 호출자가 아니라 여기서 수행된다. 그래서 이것은 찾기도 하고 하기도 한다.
 *
 * @param argc 인자 개수
 * @param argv 인자들
 * @param e_arg 어느 인자를 찾을지
 * @return 기존 주석이 기록하듯, 없으면 0, 있고 호출자가 그것을 지나 나아가야 하면 1, 프로그램이 이제 멈춰야 하면 2
 * @note 세 갈래 반환이 이 인터페이스의 미묘함 전부다. 1은 그 인자가 처리되었고 놀이가 이어질 수 있다는 뜻이고, 2는 프로그램이 요청받은 것을 했으며 게임을 시작해서는 안 된다는 뜻이다. 2를 1로 다룬 호출자는 판본을 보고하고 그다음 놀 것이다.
 * @note 현재 위치만이 아니라 남은 명령행 전체를 찾으므로, 인자가 어디 나타나든 발견된다.
 * @note 인자는 쌍점이나 등호 뒤에 여분의 부분을 지닐 수 있고, 그것이 그 인자가 하는 일을 바꾼다. 판본을 묻는 것이 대신 그것을 클립보드로 복사하거나 호환성 값을 출력할 수 있으며, 인식되지 않는 여분 부분은 무시되는 대신 보고되고 프로그램을 멈춘다. 잘못 입력된 여분 부분이 조용히 맨 동작을
 *       주어서는 안 된다.
 * @note 겹친 붙임표 형태가 기억되므로, 잘못된 여분 부분에 대한 메시지가 플레이어가 입력한 것과 같은 표기를 쓴다.
 * @note 디버그 인자의 여분 부분은 해석되기 전에 복사된다. 그것을 해석하는 것이 그 글을 수정하고, 명령행은 이 함수가 바꿀 것이 아니기 때문이다.
 * @note 표에 없는 인자나 빈 명령행은 오류가 되는 대신 맞지 않음을 낸다. 호출자들은 자기 빌드가 갖지 않을 수도 있는 인자에 대해 묻는다.
 */
int
argcheck(int argc, char *argv[], enum earlyarg e_arg)
{
    int i, idx;
    boolean match = FALSE;
    char *userea = (char *) 0;
    const char *dashdash = "";

    for (idx = 0; idx < SIZE(earlyopts); idx++) {
        if (earlyopts[idx].e == e_arg){
            break;
        }
    }
    if (idx >= SIZE(earlyopts) || argc < 1)
        return 0;

    for (i = 0; i < argc; ++i) {
        if (argv[i][0] != '-')
            continue;
        if (argv[i][1] == '-') {
            userea = &argv[i][2];
            dashdash = "-";
        } else {
            userea = &argv[i][1];
        }
        match = match_optname(userea, earlyopts[idx].name,
                              earlyopts[idx].minlength,
                              earlyopts[idx].valallowed);
        if (match)
            break;
    }

    if (match) {
        const char *extended_opt = strchr(userea, ':');

        if (!extended_opt)
            extended_opt = strchr(userea, '=');
        switch(e_arg) {
        case ARG_DEBUG:
            if (extended_opt) {
                char *cpy_extended_opt;

                cpy_extended_opt = dupstr(extended_opt);
                debug_fields(cpy_extended_opt + 1);
                free((genericptr_t) cpy_extended_opt);
            }
            return 1;
        case ARG_VERSION: {
            boolean insert_into_pastebuf = FALSE;

            if (extended_opt) {
                extended_opt++;
                if (match_optname(extended_opt, "copy", 4, FALSE)) {
                    insert_into_pastebuf = TRUE;
                } else if (match_optname(extended_opt, "dump", 4, FALSE)) {
                    /* version number plus enabled features and sanity
                       values that the program compares against the same
                       thing recorded in save and bones files to check
                       whether they're being used compatibly */
                    dump_version_info();
                    return 2; /* done */
                } else if (!match_optname(extended_opt, "show", 4, FALSE)) {
                    raw_printf("-%sversion can only be extended with"
                               " -%sversion:copy or :dump or :show.\n",
                               dashdash, dashdash);
                    /* exit after we've reported bad command line argument */
                    return 2;
                }
            }
            early_version_info(insert_into_pastebuf);
            return 2;
        }
        case ARG_SHOWPATHS:
            return 2;
#ifndef NODUMPENUMS
        case ARG_DUMPENUMS:
            dump_enums();
            return 2;
#endif
        case ARG_DUMPGLYPHIDS:
            dump_glyphnames();
            return 2;
        case ARG_DUMPMONGEN:
            dump_mongen();
            return 2;
        case ARG_DUMPWEIGHTS:
            dump_weights();
            return 2;
#ifdef CRASHREPORT
        case ARG_BIDSHOW:
            crashreport_bidshow();
            return 2;
#endif
#ifdef WIN32
        case ARG_WINDOWS:
            if (extended_opt) {
                extended_opt++;
                return windows_early_options(extended_opt);
            }
        FALLTHROUGH;
        /*FALLTHRU*/
#endif
        default:
            break;
        }
    };
    return 0;
}

/*
 * These are internal controls to aid developers with
 * testing and debugging particular aspects of the code.
 * They are not player options and the only place they
 * are documented is right here. No gameplay is altered.
 *
 * test             - test whether this parser is working
 * ttystatus        - TTY:
 * immediateflips   - WIN32: turn off display performance
 *                    optimization so that display output
 *                    can be debugged without buffering.
 * fuzzer           - enable fuzzer without debugger intervention.
 */
/**
 * @brief Parse the comma-separated list of internal debugging controls.
 *
 * These are not player options. As the existing comment states, they exist to help a developer test one aspect of the code, the comment above is the only documentation of them, and none of them alters how the game plays.
 *
 * @param opts the list, modified during parsing
 * @note Each name may be negated by a leading exclamation mark or "no", and negations stack -- so "nonotest" turns the control on. That is a consequence of the loop rather than a designed feature, but it is harmless here.
 * @note Handles the list by splitting at the first comma and recursing on everything after it, then falling through to parse the entry before it. So the whole tail is parsed before the head, and the entries end up applied back to front --
 *       which does not matter because they are independent. The loop runs only once despite being a loop, because overwriting the comma leaves nothing for the next search to find.
 * @note Several controls exist only in the builds that have the thing they debug, so a name accepted on one system is silently ignored on another. That is deliberate: a developer's shared debugging arguments should not fail on the wrong
 *       platform.
 * @note The fuzzer control cannot be negated in effect -- it is only ever turned on -- so "nofuzzer" is accepted and does nothing.
 * @warning An unrecognised name is silently ignored, which for a mistyped control means the developer sees no debugging output and no explanation.
 * @warning The whole call abandons the rest of its work if the entry it is looking at is longer than half a buffer, rather than skipping just that entry. Since the check happens after the recursion, an over-long entry discards the entries
 *          before it in the list while the ones after it have already been applied.
 */
/**
 * @brief 쉼표로 나뉜 내부 디버깅 제어 목록을 해석한다.
 *
 * 이것들은 플레이어 옵션이 아니다. 기존 주석이 밝히듯 그것들은 개발자가 코드의 한 측면을 시험하는 것을 돕기 위해 존재하며, 위의 주석이 그것들에 대한 유일한 문서이고, 그중 무엇도 게임이 어떻게 진행되는지를 바꾸지 않는다.
 *
 * @param opts 그 목록. 해석 중에 수정된다
 * @note 각 이름은 앞선 느낌표나 "no"로 부정될 수 있고, 부정이 쌓인다. 그래서 "nonotest"는 그 제어를 켠다. 그것은 설계된 기능이 아니라 되돌기의 결과지만, 여기서는 무해하다.
 * @note 첫 쉼표에서 쪼개고 그 뒤의 모든 것에 재귀함으로써 목록을 다루며, 그다음 그 앞의 항목을 해석하러 떨어진다. 그래서 뒷부분 전체가 앞부분보다 먼저 해석되고, 항목들이 뒤에서 앞으로 적용되기에 이른다. 그것들이 서로 독립적이므로 문제가 되지 않는다. 되돌기이면서도 한 번만 도는 것은, 쉼표를
 *       덮어쓰는 것이 다음 찾기가 발견할 것을 남기지 않기 때문이다.
 * @note 몇몇 제어는 자기가 디버그하는 것을 가진 빌드에만 존재하므로, 한 시스템에서 받아들여지는 이름이 다른 시스템에서 조용히 무시된다. 그것은 의도된 것이다. 개발자가 공유하는 디버깅 인자가 틀린 플랫폼에서 실패해서는 안 된다.
 * @note 퍼저 제어는 사실상 부정될 수 없다. 그것은 켜지기만 한다. 그래서 "nofuzzer"는 받아들여지고 아무것도 하지 않는다.
 * @warning 인식되지 않는 이름은 조용히 무시되는데, 잘못 입력된 제어에 대해 그것은 개발자가 어떤 디버깅 출력도 어떤 설명도 보지 못한다는 뜻이다.
 * @warning 자기가 보고 있는 항목이 버퍼 절반보다 길면, 그 항목만 건너뛰는 대신 호출 전체가 남은 일을 포기한다. 그 검사가 재귀 뒤에 일어나므로, 지나치게 긴 항목은 목록에서 자기 앞의 항목들을 버리는 반면 자기 뒤의 것들은 이미 적용되어 있다.
 */
staticfn void
debug_fields(char *opts)
{
    char *op;
    boolean negated = FALSE;

    while ((op = strchr(opts, ',')) != 0) {
        *op++ = 0;
        /* recurse */
        debug_fields(op);
    }
    if (strlen(opts) > BUFSZ / 2)
        return;


    /* strip leading and trailing white space */
    while (isspace((uchar) *opts))
        opts++;
    op = eos((char *) opts);
    while (--op >= opts && isspace((uchar) *op))
        *op = '\0';

    if (!*opts) {
        /* empty */
        return;
    }
    while ((*opts == '!') || !strncmpi(opts, "no", 2)) {
        if (*opts == '!')
            opts++;
        else
            opts += 2;
        negated = !negated;
    }
    if (match_optname(opts, "test", 4, FALSE))
        iflags.debug.test = negated ? FALSE : TRUE;
#ifdef TTY_GRAPHICS
    if (match_optname(opts, "ttystatus", 9, FALSE))
        iflags.debug.ttystatus = negated ? FALSE : TRUE;
#endif
#ifdef WIN32
    if (match_optname(opts, "immediateflips", 14, FALSE))
        iflags.debug.immediateflips = negated ? FALSE : TRUE;
#endif
    if (match_optname(opts, "fuzzer", 4, FALSE))
        iflags.fuzzerpending = TRUE;
    return;
}

/**
 * @name Enumeration dump tables
 *
 * Tables pairing each internal numbering with the name it was given in the source, built so the program can report its own numbering to an outside tool.
 *
 * They are produced by including the same headers the game itself is built from, with a macro set that makes those headers expand to name-and-value pairs instead of declarations. That is the whole point: the numbers cannot drift from the ones
 * the program uses, because they are the same text read a second way. A table written out by hand would be a second copy to keep in step.
 *
 * @note Each table brackets its include with defining and undefining one selector macro, because a header expands differently depending on which is set and several tables come from the same header.
 * @note Two of these are shared with the code that maps glyphs to display characters, as the existing comment records, so they are not private to the dumping feature.
 * @note One header's enumeration is not part of the usual included set, as the comment there notes, so it is declared here as well as dumped -- the dump and the declaration come from the same file read twice.
 * @note Entries beyond the generated ones are added by hand for the boundary values: the counts and the first and last markers. Those are not part of the generated sequence but are what a reader of the dump needs to interpret it.
 * @{
 */

/**
 * @name 열거 출력 표
 *
 * 각 내부 번호 매김을 그것이 소스에서 받은 이름과 짝지은 표들. 프로그램이 자기 번호 매김을 밖의 도구에게 보고할 수 있도록 만들어졌다.
 *
 * 그것들은 게임 자신이 빌드되는 것과 같은 헤더를 포함함으로써 만들어지는데, 그 헤더가 선언이 아니라 이름과 값의 짝으로 펼쳐지게 하는 매크로 묶음과 함께다. 그것이 요점 전부다. 그 숫자는 프로그램이 쓰는 것에서 어긋날 수 없다. 그것들이 두 번째 방식으로 읽힌 같은 글이기 때문이다. 손으로 쓰인 표는 발을 맞춰 두어야 할 두 번째 사본일 것이다.
 *
 * @note 각 표가 자기 포함을 선택자 매크로 하나를 정의하고 해제하는 것으로 감싼다. 헤더가 어느 것이 설정되었는지에 따라 다르게 펼쳐지고 여러 표가 같은 헤더에서 오기 때문이다.
 * @note 기존 주석이 기록하듯 이 가운데 둘이 글리프를 표시 문자로 대응시키는 코드와 공유되므로, 그것들은 출력 기능의 사적인 것이 아니다.
 * @note 한 헤더의 열거는 보통 포함되는 묶음의 일부가 아니며, 거기의 주석이 그것을 적는다. 그래서 그것은 출력되기도 하고 여기서 선언되기도 한다. 출력과 선언이 두 번 읽힌 같은 파일에서 온다.
 * @note 생성된 것들 너머의 항목은 경계 값을 위해 손으로 더해진다. 개수, 그리고 첫과 마지막 표시. 그것들은 생성된 수열의 일부가 아니지만 출력을 읽는 사람이 그것을 해석하는 데 필요한 것이다.
 * @{
 */

#if !defined(NODUMPENUMS)
/* monsdump[] and objdump[] are also used in utf8map.c */

#define DUMP_ENUMS
#define UNPREFIXED_COUNT (5)
struct enum_dump monsdump[] = {
#include "monsters.h"
    { NUMMONS, "NUMMONS" },
    { NON_PM, "NON_PM" },
    { LOW_PM, "LOW_PM" },
    { HIGH_PM, "HIGH_PM" },
    { SPECIAL_PM, "SPECIAL_PM" }
};
struct enum_dump objdump[] = {
#include "objects.h"
    { NUM_OBJECTS, "NUM_OBJECTS" },
};

#define DUMP_ENUMS_PCHAR
static struct enum_dump defsym_cmap_dump[] = {
#include "defsym.h"
    { MAXPCHARS, "MAXPCHARS" },
};
#undef DUMP_ENUMS_PCHAR

#define DUMP_ENUMS_MONSYMS
static struct enum_dump defsym_mon_syms_dump[] = {
#include "defsym.h"
    { MAXMCLASSES, "MAXMCLASSES" },
};
#undef DUMP_ENUMS_MONSYMS

#define DUMP_ENUMS_MONSYMS_DEFCHAR
static struct enum_dump defsym_mon_defchars_dump[] = {
#include "defsym.h"
};
#undef DUMP_ENUMS_MONSYMS_DEFCHAR

#define DUMP_ENUMS_OBJCLASS_DEFCHARS
static struct enum_dump objclass_defchars_dump[] = {
#include "defsym.h"
};
#undef DUMP_ENUMS_OBJCLASS_DEFCHARS

#define DUMP_ENUMS_OBJCLASS_CLASSES
static struct enum_dump objclass_classes_dump[] = {
#include "defsym.h"
    { MAXOCLASSES, "MAXOCLASSES" },
};
#undef DUMP_ENUMS_OBJCLASS_CLASSES

#define DUMP_ENUMS_OBJCLASS_SYMS
static struct enum_dump objclass_syms_dump[] = {
#include "defsym.h"
};
#undef DUMP_ENUMS_OBJCLASS_SYMS

#define DUMP_ARTI_ENUM
static struct enum_dump arti_enum_dump[] = {
#include "artilist.h"
    { AFTER_LAST_ARTIFACT, "AFTER_LAST_ARTIFACT" }
};
#undef DUMP_ARTI_ENUM

/* the enums are not part of hack.h for this one */
#define DUMP_MCASTU_ENUM1
enum mcast_dumpenum_spells {
    #include "mcastu.h"
};
#undef DUMP_MCASTU_ENUM1

#define DUMP_MCASTU_ENUM2
static struct enum_dump mcastu_enum_dump[] = {
#include "mcastu.h"
};
#undef DUMP_MCASTU_ENUM2

#undef DUMP_ENUMS

/** @} */

#ifndef NODUMPENUMS

/**
 * @brief Write out every internal numbering with its source name.
 *
 * Exists so that tools built alongside the game -- tile editors, symbol-set builders -- can learn the numbering from the program itself rather than being told it. A tool that hard-coded these numbers would break silently the next time a monster
 * was added.
 *
 * @note The tables are gathered into one list indexed by an enumeration, so the dumping loop is written once rather than per table. The enumeration and the list must stay in the same order, since nothing connects an entry to its name except
 *       position.
 * @note Two of the tables are built here rather than from a header, because their contents are individual named boundary values rather than a generated sequence. The local macro that builds each entry from a single name is what keeps a name
 *       and the text of that name from disagreeing.
 * @note The glyph-offset table is the one worth reading. The glyph numbering is a series of contiguous ranges rather than a lookup, so the offsets where each range begins are what a tool needs in order to classify a glyph at all.
 * @note Absent entirely from builds that switch it off, which is why every use of it is guarded. It is a developer facility and costs a table of every monster and object name in the binary.
 */
/**
 * @brief 모든 내부 번호 매김을 그 소스 이름과 함께 써낸다.
 *
 * 게임과 나란히 빌드되는 도구들, 타일 편집기나 기호 묶음 생성기가 그 번호 매김을 지시받는 대신 프로그램 자신에게서 배울 수 있도록 존재한다. 이 숫자를 코드에 박아 넣은 도구는 다음에 몬스터가 더해질 때 조용히 망가질 것이다.
 *
 * @note 표들이 열거로 색인되는 하나의 목록으로 모아지므로, 출력 되돌기가 표마다가 아니라 한 번 쓰인다. 그 열거와 목록은 같은 순서로 머물러야 한다. 위치 말고는 항목을 그 이름과 잇는 것이 없기 때문이다.
 * @note 표 가운데 둘이 헤더에서가 아니라 여기서 만들어진다. 그 내용이 생성된 수열이 아니라 개별적으로 이름 지어진 경계 값이기 때문이다. 각 항목을 하나의 이름에서 만드는 그 국지적 매크로가 이름과 그 이름의 글이 어긋나지 않게 하는 것이다.
 * @note 글리프 위치 표가 읽을 만한 것이다. 글리프 번호 매김은 찾기가 아니라 이어진 범위의 연속이므로, 각 범위가 시작하는 위치가 도구가 글리프를 분류하기라도 하기 위해 필요한 것이다.
 * @note 그것을 끄는 빌드에는 전혀 없으며, 그것이 그것의 모든 쓰임이 방벽에 싸인 이유다. 그것은 개발자 편의이며 실행 파일에 모든 몬스터와 물건 이름의 표를 들인다.
 */
staticfn void
dump_enums(void)
{
    enum enum_dumps {
        monsters_enum,
        objects_enum,
        objects_misc_enum,
        glyph_offsets_enum,
        defsym_cmap_enum,
        defsym_mon_syms_enum,
        defsym_mon_defchars_enum,
        objclass_defchars_enum,
        objclass_classes_enum,
        objclass_syms_enum,
        arti_enum,
        mcastu_enum,
        NUM_ENUM_DUMPS
    };

#define dump_go(go) { GLYPH_##go, #go }
static const struct enum_dump glyph_offsets_dump[] = {
        dump_go(MON_OFF),
        dump_go(MON_MALE_OFF),
        dump_go(MON_FEM_OFF),
        dump_go(PET_OFF),
        dump_go(PET_MALE_OFF),
        dump_go(PET_FEM_OFF),
        dump_go(INVIS_OFF),
        dump_go(DETECT_OFF),
        dump_go(DETECT_MALE_OFF),
        dump_go(DETECT_FEM_OFF),
        dump_go(BODY_OFF),
        dump_go(RIDDEN_OFF),
        dump_go(RIDDEN_MALE_OFF),
        dump_go(RIDDEN_FEM_OFF),
        dump_go(OBJ_OFF),
        dump_go(CMAP_OFF),
        dump_go(CMAP_STONE_OFF),
        dump_go(CMAP_MAIN_OFF),
        dump_go(CMAP_MINES_OFF),
        dump_go(CMAP_GEH_OFF),
        dump_go(CMAP_KNOX_OFF),
        dump_go(CMAP_SOKO_OFF),
        dump_go(CMAP_A_OFF),
        dump_go(ALTAR_OFF),
        dump_go(CMAP_B_OFF),
        dump_go(ZAP_OFF),
        dump_go(CMAP_C_OFF),
        dump_go(SWALLOW_OFF),
        dump_go(EXPLODE_OFF),
        dump_go(EXPLODE_DARK_OFF),
        dump_go(EXPLODE_NOXIOUS_OFF),
        dump_go(EXPLODE_MUDDY_OFF),
        dump_go(EXPLODE_WET_OFF),
        dump_go(EXPLODE_MAGICAL_OFF),
        dump_go(EXPLODE_FIERY_OFF),
        dump_go(EXPLODE_FROSTY_OFF),
        dump_go(WARNING_OFF),
        dump_go(STATUE_OFF),
        dump_go(STATUE_MALE_OFF),
        dump_go(STATUE_FEM_OFF),
        dump_go(PILETOP_OFF),
        dump_go(OBJ_PILETOP_OFF),
        dump_go(BODY_PILETOP_OFF),
        dump_go(STATUE_MALE_PILETOP_OFF),
        dump_go(STATUE_FEM_PILETOP_OFF),
        dump_go(UNEXPLORED_OFF),
        dump_go(NOTHING_OFF),
        { MAX_GLYPH, "MAX_GLYPH" }
    };
#undef dump_go

#define dump_om(om) { om, #om }
    static const struct enum_dump omdump[] = {
        dump_om(LAST_GENERIC),
        dump_om(OBJCLASS_HACK),
        dump_om(FIRST_OBJECT),
        dump_om(FIRST_AMULET),
        dump_om(LAST_AMULET),
        dump_om(FIRST_SPELL),
        dump_om(LAST_SPELL),
        dump_om(MAXSPELL),
        dump_om(FIRST_REAL_GEM),
        dump_om(LAST_REAL_GEM),
        dump_om(FIRST_GLASS_GEM),
        dump_om(LAST_GLASS_GEM),
        dump_om(NUM_REAL_GEMS),
        dump_om(NUM_GLASS_GEMS),
        dump_om(MAXEXPCHARS),
        dump_om(MAXTCHARS),
        dump_om(WARNCOUNT),
    };
#undef dump_om

    static const struct enum_dump *const ed[NUM_ENUM_DUMPS] = {
        monsdump, objdump, omdump, glyph_offsets_dump,
        defsym_cmap_dump, defsym_mon_syms_dump,
        defsym_mon_defchars_dump,
        objclass_defchars_dump,
        objclass_classes_dump,
        objclass_syms_dump,
        arti_enum_dump,
        mcastu_enum_dump,
    };

    static const struct de_params {
        const char *const title;
        const char *const pfx;
        int unprefixed_count;
        int dumpflgs;  /* 0 = dump numerically only, 1 = add 'char' comment */
        int szd;
    } edmp[NUM_ENUM_DUMPS] = {
        { "monnums", "PM_", UNPREFIXED_COUNT, 0, SIZE(monsdump) },
        { "objects_nums", "", 1, 0, SIZE(objdump) },
        { "misc_object_nums", "", 1, 0, SIZE(omdump) },
        { "glyph_offsets", "GLYPH_", 1, 0, SIZE(glyph_offsets_dump) },
        { "cmap_symbols", "", 1, 0, SIZE(defsym_cmap_dump) },
        { "mon_syms", "", 1, 0, SIZE(defsym_mon_syms_dump) },
        { "mon_defchars", "", 1, 1, SIZE(defsym_mon_defchars_dump) },
        { "objclass_defchars", "", 1, 1, SIZE(objclass_defchars_dump) },
        { "objclass_classes", "", 1, 0, SIZE(objclass_classes_dump) },
        { "objclass_syms", "", 1, 0, SIZE(objclass_syms_dump) },
        { "artifacts_nums", "", 1, 0, SIZE(arti_enum_dump) },
        { "mcast_spells", "MCAST_", 0, 0, SIZE(mcastu_enum_dump) },
    };

    const char *nmprefix;
    int i, j, nmwidth;
    char comment[BUFSZ];

    for (i = 0; i < NUM_ENUM_DUMPS; ++ i) {
        raw_printf("enum %s = {", edmp[i].title);
        for (j = 0; j < edmp[i].szd; ++j) {
            nmprefix = (j >= edmp[i].szd - edmp[i].unprefixed_count)
                           ? "" : edmp[i].pfx; /* "" or "PM_" */
            nmwidth = 27 - (int) strlen(nmprefix); /* 27 or 24 */
            if (edmp[i].dumpflgs > 0) {
                Snprintf(comment, sizeof comment,
                         "    /* '%c' */",
                         (ed[i][j].val >= 32 && ed[i][j].val <= 126)
                         ? ed[i][j].val : ' ');
            } else {
                comment[0] = '\0';
            }
            raw_printf("    %s%*s = %3d,%s",
                       nmprefix, -nmwidth,
                       ed[i][j].nm, ed[i][j].val,
                       comment);
       }
        raw_print("};");
        raw_print("");
    }
    raw_print("");
}
#undef UNPREFIXED_COUNT
#endif /* NODUMPENUMS */

/**
 * @brief Write out every glyph's name.
 *
 * A separate request from the enumeration dump because it answers a different question. The enumerations give the offsets where each range of glyphs begins; this gives a name for every individual glyph, which is what a tool assembling a tile
 * set needs in order to label its images.
 *
 * @note Writes to standard output rather than through the game's own output, since this runs before there is any window to write to.
 * @note A thin wrapper over the routine that does the work. It exists so that the argument handling here does not have to know where glyph names are kept.
 */
/**
 * @brief 모든 글리프의 이름을 써낸다.
 *
 * 열거 출력과 따로 된 요청인 것은 그것이 다른 질문에 답하기 때문이다. 열거는 각 글리프 범위가 시작하는 위치를 주고, 이것은 개별 글리프마다 이름을 준다. 그것이 타일 묶음을 조립하는 도구가 자기 그림에 이름표를 붙이기 위해 필요한 것이다.
 *
 * @note 게임 자신의 출력을 통하는 대신 표준 출력에 쓴다. 이것이 쓸 창이 있기 전에 실행되기 때문이다.
 * @note 그 일을 하는 함수 위의 얇은 감싸기다. 그것이 존재하는 것은 여기의 인자 다루기가 글리프 이름이 어디 지켜지는지 알 필요가 없게 하기 위해서다.
 */
void
dump_glyphnames(void)
{
    dump_all_glyphnames(stdout);
}
#endif /* !NODUMPENUMS */

/*allmain.c*/
