/* NetHack 5.0	hacklib.c	$NHDT-Date: 1781973051 2026/06/20 16:30:51 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.133 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2007. */
/* Copyright (c) Robert Patrick Rankin, 1991                      */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file hacklib.c
 * @brief Small utilities that happen to live in NetHack but do not belong to it.
 *
 * String tidying, character classification, a few numeric helpers, and the
 * random-number conveniences. Nothing here knows about the dungeon, the hero,
 * or the map, which is deliberate: these are used from the build tools as well
 * as the game, and a dependency on game state would make that impossible.
 *
 * @note Several routines modify the string they are given and return it, so
 *       the result is the argument rather than a copy; the index below records
 *       which.
 * @warning Callers commonly chain these, and a routine that trims or strips
 *          can shorten a buffer another still holds a pointer into.
 */

/**
 * @file hacklib.c
 * @brief NetHack 안에 있지만 NetHack 의 것은 아닌 작은 유틸리티들.
 *
 * 문자열 정돈, 문자 분류, 몇 가지 수치 도우미, 그리고 난수 편의 함수들이다.
 * 여기의 어떤 것도 던전이나 영웅, 지도를 알지 못하며 이는 의도된 것이다. 이
 * 함수들은 게임뿐 아니라 빌드 도구에서도 쓰이는데, 게임 상태에 의존한다면
 * 그것이 불가능해지기 때문이다.
 *
 * @note 여러 루틴이 받은 문자열을 직접 고치고 그것을 반환한다. 결과는 사본이
 *       아니라 인자 자신이며, 아래 목록이 어느 것이 그런지 알려 준다.
 * @warning 호출자들은 이 함수들을 이어 쓰는 일이 잦고, 다듬거나 걷어내는 루틴은
 *          다른 쪽이 아직 포인터를 쥐고 있는 버퍼를 짧게 만들 수 있다.
 */

#include "hack.h" /* for config.h+extern.h */

/*=
    Assorted 'small' utility routines.  They're virtually independent of
    NetHack.

      return type     routine name    argument type(s)
        boolean         digit           (char)
        boolean         letter          (char)
        char            highc           (char)
        char            lowc            (char)
        char *          lcase           (char *)
        char *          ucase           (char *)
        char *          upstart         (char *)
        char *          upwords         (char *)
        char *          mungspaces      (char *)
        char *          trimspaces      (char *)
        char *          strip_newline   (char *)
        char *          stripchars      (char *, const char *, const char *)
        char *          stripdigits     (char *)
        char *          eos             (char *)
        const char *    c_eos           (const char *)
        boolean         str_start_is    (const char *, const char *, boolean)
        boolean         str_end_is      (const char *, const char *)
        int             str_lines_maxlen (const char *)
        char *          strkitten       (char *,char)
        void            copynchars      (char *,const char *,int)
        char            chrcasecpy      (int,int)
        char *          strcasecpy      (char *,const char *)
        char *          s_suffix        (const char *)
        char *          ing_suffix      (const char *)
        char *          xcrypt          (const char *, char *)
        boolean         onlyspace       (const char *)
        char *          tabexpand       (char *)
        char *          visctrl         (char)
        char *          strsubst        (char *, const char *, const char *)
        int             strNsubst       (char *,const char *,const char *,int)
        const char *    findword        (const char *,const char *,int,boolean)
        const char *    ordin           (int)
        char *          sitoa           (int)
        int             sgn             (int)
        int             distmin         (coordxy, coordxy, coordxy, coordxy)
        int             dist2           (coordxy, coordxy, coordxy, coordxy)
        boolean         online2         (coordxy, coordxy)
        int             strncmpi        (const char *, const char *, int)
        char *          strstri         (const char *, const char *)
        boolean         fuzzymatch      (const char *, const char *,
                                         const char *, boolean)
        int             swapbits        (int, int, int)
        void            nh_snprintf     (const char *, int, char *, size_t,
                                         const char *, ...)
=*/

/**
 * @name Character classification and case
 *
 * Written here rather than taken from the standard library because these must give the same answer everywhere. The standard versions depend on the locale, so a player running with a different one would get different results from the same
 * game -- a name capitalised on one machine and not on another.
 *
 * @warning All four assume the plain character ordering: that the letters are contiguous and that the two cases differ by a single bit. That is true of the encoding every supported system uses, and these would be wrong on one where it is not.
 * @{
 */

/**
 * @brief Is this character a digit?
 * @param c the character
 * @return whether it is
 */
/**
 * @brief 이 문자가 숫자인가?
 * @param c 그 문자
 * @return 그런지
 */
boolean
digit(char c)
{
    return (boolean) ('0' <= c && c <= '9');
}

/**
 * @brief Is this character a letter?
 * @param c the character
 * @return whether it is
 * @warning Counts the at sign as a letter, as the existing comment records. That is not an oversight but a consequence of testing a range that begins one character early, and code that relies on this to strip a non-letter prefix will leave an
 *          at sign in place.
 */
/**
 * @brief 이 문자가 글자인가?
 * @param c 그 문자
 * @return 그런지
 * @warning 기존 주석이 기록하듯 골뱅이 기호를 글자로 센다. 그것은 빠뜨림이 아니라 한 문자 앞에서 시작하는 범위를 검사하는 것의 결과이며, 글자가 아닌 접두 문자를 떼어내려고 이것에 의존하는 코드는 골뱅이 기호를 그대로 남길 것이다.
 */
boolean
letter(char c)
{
    return (boolean) ('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

/**
 * @brief Force a character to upper case.
 * @param c the character
 * @return the upper-case form, or the character unchanged if it is not a lower-case letter
 * @note Done by clearing one bit rather than by table lookup, which is why the range is tested first: clearing that bit in a character that is not a lower-case letter would produce a different character rather than the same one.
 */
/**
 * @brief 문자를 대문자로 강제한다.
 * @param c 그 문자
 * @return 대문자 형태, 또는 그것이 소문자 글자가 아니면 바뀌지 않은 문자
 * @note 표 찾기가 아니라 비트 하나를 지움으로써 이루어지며, 그것이 범위가 먼저 검사되는 이유다. 소문자 글자가 아닌 문자에서 그 비트를 지우는 것은 같은 문자가 아니라 다른 문자를 낼 것이다.
 */
char
highc(char c)
{
    return (char) (('a' <= c && c <= 'z') ? (c & ~040) : c);
}

/**
 * @brief Force a character to lower case.
 * @param c the character
 * @return the lower-case form, or the character unchanged if it is not an upper-case letter
 * @note The exact counterpart of forcing upper case, setting the same bit that one clears.
 */
/**
 * @brief 문자를 소문자로 강제한다.
 * @param c 그 문자
 * @return 소문자 형태, 또는 그것이 대문자 글자가 아니면 바뀌지 않은 문자
 * @note 대문자로 강제하는 것의 정확한 짝이며, 그것이 지우는 것과 같은 비트를 설정한다.
 */
char
lowc(char c)
{
    return (char) (('A' <= c && c <= 'Z') ? (c | 040) : c);
}

/** @} */

/**
 * @name Case conversion of whole strings
 *
 * All four change the string where it stands and return it, so the result is the argument rather than a copy. A caller must not pass a string it needs to keep in its original form.
 * @{
 */

/**
 * @brief Lower-case every letter in a string.
 * @param s the string, modified in place
 * @return the same string
 * @note Sets the bit directly rather than calling the single-character routine, which comes to the same thing.
 */
/**
 * @brief 문자열의 모든 글자를 소문자로 만든다.
 * @param s 그 문자열. 제자리에서 수정된다
 * @return 같은 문자열
 * @note 한 문자 함수를 호출하는 대신 비트를 곧바로 설정하며, 그것은 같은 것에 이른다.
 */
char *
lcase(char *s)
{
    char *p;

    for (p = s; *p; p++)
        if ('A' <= *p && *p <= 'Z')
            *p |= 040;
    return s;
}

/**
 * @brief Upper-case every letter in a string.
 * @param s the string, modified in place
 * @return the same string
 */
/**
 * @brief 문자열의 모든 글자를 대문자로 만든다.
 * @param s 그 문자열. 제자리에서 수정된다
 * @return 같은 문자열
 */
char *
ucase(char *s)
{
    char *p;

    for (p = s; *p; p++)
        if ('a' <= *p && *p <= 'z')
            *p &= ~040;
    return s;
}

/**
 * @brief Capitalise a string's first character, for the start of a sentence.
 * @param s the string, modified in place; may be null
 * @return the same string
 * @note Accepts null and does nothing, unlike its companions. Callers hand it the result of something that may not have produced a string, so refusing null would push the check onto every one of them.
 * @warning Capitalises the first character whatever it is, not the first letter. A string beginning with a quotation mark or a space is left alone, which is a real case for messages built by concatenation.
 */
/**
 * @brief 문장 시작을 위해 문자열의 첫 문자를 대문자로 만든다.
 * @param s 그 문자열. 제자리에서 수정된다. 널일 수 있다
 * @return 같은 문자열
 * @note 짝들과 달리 널을 받아들이고 아무것도 하지 않는다. 호출자들이 문자열을 내지 않았을 수도 있는 무엇의 결과를 그것에 넘기므로, 널을 거부하는 것은 그 검사를 그들 모두에게 밀 것이다.
 * @warning 첫 글자가 아니라 첫 문자를 그것이 무엇이든 대문자로 만든다. 인용 부호나 공백으로 시작하는 문자열은 그대로 남겨지며, 그것은 이어붙이기로 만들어지는 메시지에 실제로 있는 경우다.
 */
char *
upstart(char *s)
{
    if (s)
        *s = highc(*s);
    return s;
}

/**
 * @brief Capitalise the first letter of every word.
 * @param s the string, modified in place
 * @return the same string
 * @note Words are separated by spaces only. A hyphenated word is one word, so "long-worm" does not become "Long-Worm" -- which is what the naming conventions want.
 * @note Skips over a leading non-letter without treating the following letter as mid-word, so a quoted word is still capitalised.
 */
/**
 * @brief 모든 낱말의 첫 글자를 대문자로 만든다.
 * @param s 그 문자열. 제자리에서 수정된다
 * @return 같은 문자열
 * @note 낱말은 공백으로만 나뉜다. 붙임표로 이어진 낱말은 한 낱말이므로, "long-worm"은 "Long-Worm"이 되지 않는다. 그것이 이름 짓기 관례가 원하는 것이다.
 * @note 앞선 글자 아닌 문자를 뒤따르는 글자를 낱말 가운데로 여기지 않으면서 지나치므로, 인용된 낱말도 여전히 대문자로 만들어진다.
 */
char *
upwords(char *s)
{
    char *p;
    boolean space = TRUE;

    for (p = s; *p; p++)
        if (*p == ' ') {
            space = TRUE;
        } else if (space && letter(*p)) {
            *p = highc(*p);
            space = FALSE;
        } else {
            space = FALSE;
        }
    return s;
}

/** @} */

/**
 * @name Whitespace tidying
 * @{
 */

/**
 * @brief Collapse runs of whitespace to single spaces and drop leading and trailing ones.
 *
 * Used on text a person typed or a configuration file supplied, where the spacing is not meaningful and comparing such text against a fixed string would otherwise fail over a stray space.
 *
 * @param bp the buffer, shortened in place
 * @return the same buffer
 * @note Tabs become spaces, so a tab-indented line compares equal to a space-indented one.
 * @note Stops at a newline and treats it as the end, as the existing comment records. So this tidies one line, and text containing several lines is truncated to the first -- which is what the callers want, since they are handling one line at a
 *       time.
 * @note Unlike trimming, this returns the buffer it was given rather than a pointer into it, because leading spaces are removed by shifting the text down rather than by skipping past them.
 */
/**
 * @brief 이어진 공백을 하나의 공백으로 줄이고 앞뒤의 것을 버린다.
 *
 * 사람이 입력하거나 설정 파일이 내준 글에 쓰이며, 거기서 띄어쓰기는 뜻이 없고 그런 글을 고정된 문자열과 비교하는 것이 그러지 않으면 떠돌아다니는 공백 하나 때문에 실패할 것이다.
 *
 * @param bp 그 버퍼. 제자리에서 짧아진다
 * @return 같은 버퍼
 * @note 탭이 공백이 되므로, 탭으로 들여쓴 줄이 공백으로 들여쓴 줄과 같게 비교된다.
 * @note 기존 주석이 기록하듯 줄바꿈에서 멈추고 그것을 끝으로 다룬다. 그래서 이것은 한 줄을 정돈하며, 여러 줄을 담은 글은 첫 줄로 잘린다. 그것이 호출자들이 원하는 것이다. 그들은 한 번에 한 줄을 다루고 있다.
 * @note 다듬기와 달리 이것은 자기가 받은 버퍼를 그 안을 가리키는 포인터가 아니라 그대로 돌려준다. 앞선 공백이 그것을 지나쳐 감으로써가 아니라 글을 내려 옮김으로써 없어지기 때문이다.
 */
char *
mungspaces(char *bp)
{
    char c, *p, *p2;
    boolean was_space = TRUE;

    for (p = p2 = bp; (c = *p) != '\0'; p++) {
        if (c == '\n')
            break; /* treat newline the same as end-of-string */
        if (c == '\t')
            c = ' ';
        if (c != ' ' || !was_space)
            *p2++ = c;
        was_space = (c == ' ');
    }
    if (was_space && p2 > bp)
        p2--;
    *p2 = '\0';
    return bp;
}

/**
 * @brief Remove whitespace from both ends of a string.
 * @param txt the string; trailing whitespace is removed in place
 * @return a pointer past any leading whitespace, which is not necessarily the argument
 * @warning The two ends are handled differently, and this is the trap. Trailing whitespace is removed by shortening the string, but leading whitespace is skipped by returning a later pointer -- as the existing comment concedes, it remains in
 *          the buffer. So a caller that discards the return value and keeps using its own pointer gets the untrimmed string, and one that frees the returned pointer frees the middle of an allocation.
 */
/**
 * @brief 문자열 양 끝에서 공백을 없앤다.
 * @param txt 그 문자열. 뒤따르는 공백은 제자리에서 없어진다
 * @return 앞선 공백을 지난 곳을 가리키는 포인터. 반드시 인자와 같지는 않다
 * @warning 두 끝이 다르게 다뤄지며, 그것이 함정이다. 뒤따르는 공백은 문자열을 짧게 함으로써 없어지지만, 앞선 공백은 더 뒤의 포인터를 돌려줌으로써 지나쳐진다. 기존 주석이 인정하듯 그것은 버퍼에 남는다. 그래서 반환 값을 버리고 자기 포인터를 계속 쓰는 호출자는 다듬어지지 않은 문자열을 얻고,
 *          돌려받은 포인터를 해제하는 호출자는 할당의 가운데를 해제한다.
 */
char *
trimspaces(char *txt)
{
    char *end;

    /* leading whitespace will remain in the buffer */
    while (*txt == ' ' || *txt == '\t')
        txt++;
    end = eos(txt);
    while (--end >= txt && (*end == ' ' || *end == '\t'))
        *end = '\0';

    return txt;
}

/**
 * @brief Remove the line ending from a line read from a file.
 * @param str the string, shortened in place
 * @return the same string
 * @note Removes a preceding carriage return as well when there is one, as the existing comment records. That is what lets a file written on one kind of system be read on another without every line carrying a stray character.
 * @note Looks for the last newline rather than the first, so a buffer that somehow holds more than one line loses only the final ending.
 */
/**
 * @brief 파일에서 읽은 줄에서 줄 끝을 없앤다.
 * @param str 그 문자열. 제자리에서 짧아진다
 * @return 같은 문자열
 * @note 기존 주석이 기록하듯 앞선 캐리지 리턴이 있으면 그것도 없앤다. 그것이 한 종류의 시스템에서 쓰인 파일이 다른 시스템에서 모든 줄이 떠돌아다니는 문자를 지니지 않고 읽힐 수 있게 하는 것이다.
 * @note 첫 것이 아니라 마지막 줄바꿈을 찾으므로, 어쩌다 한 줄보다 많이 담은 버퍼는 마지막 끝만 잃는다.
 */
char *
strip_newline(char *str)
{
    char *p = strrchr(str, '\n');

    if (p) {
        if (p > str && *(p - 1) == '\r')
            --p;
        *p = '\0';
    }
    return str;
}

/** @} */

/**
 * @name Finding and comparing within strings
 * @{
 */

/**
 * @brief The end of a string -- a pointer at its terminator.
 * @param s the string
 * @return where its terminator is
 * @note Used constantly to append to a buffer, and it reads better at the call site than adding a length would: writing at the end of a string says what is meant, where adding a computed length says how.
 */
/**
 * @brief 문자열의 끝. 그 종료 문자를 가리키는 포인터.
 * @param s 그 문자열
 * @return 그 종료 문자가 있는 곳
 * @note 버퍼에 덧붙이기 위해 끊임없이 쓰이며, 호출하는 곳에서 길이를 더하는 것보다 잘 읽힌다. 문자열의 끝에 쓴다는 것은 뜻해진 것을 말하고, 계산된 길이를 더한다는 것은 방법을 말한다.
 */
char *
eos(char *s)
{
    while (*s)
        s++; /* s += strlen(s); */
    return s;
}

/**
 * @brief The end of a string that must not be modified.
 * @param s the string
 * @return where its terminator is
 * @note Identical in what it does to the ordinary version, as the existing comment records. It exists only so that a caller with a read-only string does not have to cast the constness away to find the end of it -- and casting it away would
 *       let a later slip write into a string literal.
 */
/**
 * @brief 수정되어서는 안 되는 문자열의 끝.
 * @param s 그 문자열
 * @return 그 종료 문자가 있는 곳
 * @note 기존 주석이 기록하듯 하는 일이 보통 판본과 똑같다. 그것은 읽기 전용 문자열을 가진 호출자가 그 끝을 찾기 위해 상수성을 벗겨내지 않아도 되게 하기 위해서만 존재한다. 그리고 그것을 벗겨내는 것은 나중의 실수가 문자열 상수에 써넣게 할 것이다.
 */
const char *
c_eos(const char *s)
{
    while (*s)
        s++; /* s += strlen(s); */
    return s;
}

/**
 * @brief Does a string begin with another?
 *
 * The comparison behind every abbreviated command and option name: a player types a few characters and this decides whether they name the thing.
 *
 * @param str the string to examine
 * @param chkstr the beginning to look for
 * @param caseblind whether to ignore case
 * @return whether @p str begins with @p chkstr
 * @note An empty beginning matches everything, which is what makes the shortest abbreviation ambiguous rather than unrecognised. Callers relying on a minimum length must enforce it themselves.
 * @note A beginning longer than the string does not match, so this is not symmetric -- it asks one question, not whether either is a prefix of the other.
 * @warning The header comment says it panics on huge strings, but the code that would has been disabled. The loop is bounded by the largest representable count, so an unterminated string is walked to that bound and returns a match rather than
 *          failing -- which is a wrong answer instead of a report.
 */
/**
 * @brief 어떤 문자열이 다른 문자열로 시작하는가?
 *
 * 모든 줄인 명령과 옵션 이름 뒤의 비교다. 플레이어가 몇 문자를 입력하고 이것이 그것이 그것을 이름 짓는지를 정한다.
 *
 * @param str 살펴볼 문자열
 * @param chkstr 찾을 시작 부분
 * @param caseblind 대소문자를 무시할지
 * @return @p str 이 @p chkstr 로 시작하는지
 * @note 빈 시작 부분은 모든 것과 맞으며, 그것이 가장 짧은 줄임말을 인식되지 않는 것이 아니라 애매한 것으로 만드는 것이다. 최소 길이에 의존하는 호출자는 스스로 그것을 강제해야 한다.
 * @note 문자열보다 긴 시작 부분은 맞지 않으므로, 이것은 대칭이 아니다. 그것은 어느 쪽이 다른 쪽의 앞부분인지가 아니라 하나의 질문을 묻는다.
 * @warning 머리글 주석이 거대한 문자열에서 멈춘다고 말하지만, 그럴 코드는 비활성화되어 있다. 되돌기가 나타낼 수 있는 가장 큰 개수로 제한되므로, 종료되지 않은 문자열은 그 한계까지 걸어가지고 실패하는 대신 맞음을 돌려준다. 그것은 보고가 아니라 틀린 답이다.
 */
boolean
str_start_is(
    const char *str,
    const char *chkstr,
    boolean caseblind)
{
    char t1, t2;
    int n = LARGEST_INT;

    while (--n) {
        if (!*str)
            return (*chkstr == 0); /* chkstr >= str */
        else if (!*chkstr)
            return TRUE; /* chkstr < str */
        t1 = caseblind ? lowc(*str) : *str;
        t2 = caseblind ? lowc(*chkstr) : *chkstr;
        str++, chkstr++;
        if (t1 != t2)
            return FALSE;
    }
#if 0
    if (n == 0)
        panic("string too long");
#endif
    return TRUE;
}

/**
 * @brief Does a string end with another?
 * @param str the string to examine
 * @param chkstr the ending to look for
 * @return whether @p str ends with @p chkstr
 * @note Case-sensitive, unlike its counterpart for beginnings. Endings are tested against text the program produced -- a suffix it appended -- rather than against what a player typed, so there is nothing to be lenient about.
 */
/**
 * @brief 어떤 문자열이 다른 문자열로 끝나는가?
 * @param str 살펴볼 문자열
 * @param chkstr 찾을 끝부분
 * @return @p str 이 @p chkstr 로 끝나는지
 * @note 시작 부분을 위한 짝과 달리 대소문자를 가린다. 끝부분은 플레이어가 입력한 것이 아니라 프로그램이 낸 글, 곧 자기가 덧붙인 접미사와 견주어 검사되므로, 너그러울 것이 없다.
 */
boolean
str_end_is(const char *str, const char *chkstr)
{
    int clen = (int) strlen(chkstr);

    if ((int) strlen(str) >= clen)
        return (boolean) (!strncmp(eos((char *) str) - clen, chkstr, clen));
    return FALSE;
}

/**
 * @brief The length of the longest line in a multi-line string.
 * @param str the string, its lines separated by newlines
 * @return the greatest line length
 * @note Exists to size a window before its contents are shown, so the window is made wide enough for its widest line rather than being resized as text arrives.
 * @note The final line need not end with a newline; text that does not is measured to its end.
 */
/**
 * @brief 여러 줄로 된 문자열에서 가장 긴 줄의 길이.
 * @param str 그 문자열. 그 줄들이 줄바꿈으로 나뉘어 있다
 * @return 가장 큰 줄 길이
 * @note 창의 내용이 보이기 전에 그 크기를 정하기 위해 존재하며, 그래서 창이 글이 도착하며 크기가 바뀌는 대신 자기 가장 넓은 줄에 넉넉하도록 넓게 만들어진다.
 * @note 마지막 줄이 줄바꿈으로 끝나야 하는 것은 아니다. 그러지 않는 글은 그 끝까지 재어진다.
 */
int
str_lines_maxlen(const char *str)
{
    const char *s1, *s2;
    int len, max_len = 0;

    s1 = str;
    while (s1 && *s1) {
        s2 = strchr(s1, '\n');
        if (s2) {
            len = (int) (s2 - s1);
            s1 = s2 + 1;
        } else {
            len = (int) strlen(s1);
            s1 = (char *) 0;
        }
        if (len > max_len)
            max_len = len;
    }

    return max_len;
}

/** @} */

/**
 * @name Copying and appending
 * @{
 */

/**
 * @brief Append one character to a string.
 * @param s the string, extended in place
 * @param c the character
 * @return the same string
 * @note Exists because appending a single character otherwise means building a two-character string to concatenate, which the existing comment shows. This says what is meant and does less.
 * @warning No bound. The caller must know the buffer has room for one more character and a terminator.
 */
/**
 * @brief 문자열에 한 문자를 덧붙인다.
 * @param s 그 문자열. 제자리에서 늘어난다
 * @param c 그 문자
 * @return 같은 문자열
 * @note 존재하는 이유는, 그러지 않으면 한 문자를 덧붙이는 것이 이어붙일 두 문자 문자열을 만드는 것을 뜻하기 때문이며, 기존 주석이 그것을 보인다. 이것은 뜻해진 것을 말하고 더 적게 한다.
 * @warning 한계가 없다. 호출자가 버퍼에 한 문자와 종료 문자를 위한 자리가 있음을 알아야 한다.
 */
char *
strkitten(char *s, char c)
{
    char *p = eos(s);

    *p++ = c;
    *p = '\0';
    return s;
}

/**
 * @brief Copy at most a given number of characters, always terminating.
 * @param dst where to copy to
 * @param src what to copy
 * @param n the most characters to copy, not counting the terminator
 * @note Differs from the standard bounded copy in two ways the existing comment sets out, and both are the reason it exists. It always writes a terminator, so the result is always a string; and it stops at a newline as well as at the end, so a
 *       line read from a file copies as one line.
 * @warning The buffer must hold one more character than the limit, for the terminator. The limit counts characters copied, not the space needed.
 */
/**
 * @brief 주어진 개수까지의 문자를 복사하며, 언제나 종료 문자를 쓴다.
 * @param dst 복사할 곳
 * @param src 복사할 것
 * @param n 복사할 최대 문자 수. 종료 문자는 세지 않는다
 * @note 기존 주석이 밝히는 두 가지 방식에서 표준 한정 복사와 다르며, 둘 다 그것이 존재하는 이유다. 그것은 언제나 종료 문자를 쓰므로 결과가 언제나 문자열이다. 그리고 끝뿐 아니라 줄바꿈에서도 멈추므로, 파일에서 읽은 줄이 한 줄로 복사된다.
 * @warning 버퍼는 종료 문자를 위해 그 한계보다 한 문자를 더 담아야 한다. 그 한계는 필요한 자리가 아니라 복사되는 문자를 센다.
 */
void
copynchars(char *dst, const char *src, int n)
{
    /* copies at most n characters, stopping sooner if terminator reached;
       treats newline as input terminator; unlike strncpy, always supplies
       '\0' terminator so dst must be able to hold at least n+1 characters */
    while (n > 0 && *src != '\0' && *src != '\n') {
        *dst++ = *src++;
        --n;
    }
    *dst = '\0';
}

/**
 * @brief Put one character into the case of another.
 * @param oc the character whose case is wanted
 * @param nc the character to convert
 * @return @p nc in @p oc 's case
 * @note Neither character need be a letter. If the one supplying the case is not a letter, the other is returned unchanged -- so a case pattern taken from punctuation leaves the replacement alone rather than forcing it either way.
 * @note Arguments are integers rather than characters so that the disabled widening at the top could be enabled if this ever used the standard classification routines, as its comment records.
 */
/**
 * @brief 한 문자를 다른 문자의 대소문자로 만든다.
 * @param oc 그 대소문자가 바라지는 문자
 * @param nc 변환할 문자
 * @return @p oc 의 대소문자로 된 @p nc
 * @note 어느 문자도 글자여야 하는 것은 아니다. 대소문자를 내주는 쪽이 글자가 아니면 다른 쪽이 바뀌지 않고 돌려주어진다. 그래서 기호에서 취해진 대소문자 본이 그 대신할 것을 어느 쪽으로 강제하는 대신 그대로 남긴다.
 * @note 인자가 문자가 아니라 정수인 것은, 이것이 언젠가 표준 분류 함수를 쓰게 되면 맨 위의 비활성화된 넓히기가 켜질 수 있게 하기 위해서다. 그 주석이 그것을 기록한다.
 */
char
chrcasecpy(int oc, int nc)
{
#if 0 /* this will be necessary if we switch to <ctype.h> */
    oc = (int) (unsigned char) oc;
    nc = (int) (unsigned char) nc;
#endif
    if ('a' <= oc && oc <= 'z') {
        /* old char is lower case; if new char is upper case, downcase it */
        if ('A' <= nc && nc <= 'Z')
            nc += 'a' - 'A'; /* lowc(nc) */
    } else if ('A' <= oc && oc <= 'Z') {
        /* old char is upper case; if new char is lower case, upcase it */
        if ('a' <= nc && nc <= 'z')
            nc += 'A' - 'a'; /* highc(nc) */
    }
    return (char) nc;
}

/**
 * @brief Overwrite a string with another, keeping the original's pattern of capitalisation.
 *
 * Exists so that pluralising and singularising words do not lose their case, as the existing comment records. Turning "Dagger" into daggers must give "Daggers", and doing that by replacing the text would give "daggers" -- so the case is taken
 * from what was there before, character by character.
 *
 * @param dst the string to overwrite, modified in place
 * @param src the replacement text
 * @return the same @p dst
 * @note The new text may be shorter, the same, or longer than the old, as the existing comment notes. When it is longer the old text runs out, and from then on the case of the old text's last character is applied to everything remaining. That
 *       is why "Dagger" becoming "Daggers" capitalises correctly and does not leave the added letter in a different case.
 * @warning When called with an already-empty destination it reads the character before it, as the existing comment concedes -- so such a call is only valid on a pointer into the middle or end of a longer string, never on the start of a buffer.
 *          Nothing checks this, and the read would be outside the allocation.
 */
/**
 * @brief 문자열을 다른 것으로 덮어쓰되, 원본의 대소문자 본을 지킨다.
 *
 * 기존 주석이 기록하듯 낱말을 복수형과 단수형으로 만드는 것이 그 대소문자를 잃지 않도록 존재한다. "Dagger"를 daggers로 바꾸는 것은 "Daggers"를 내어야 하고, 글을 갈아치움으로써 그것을 하면 "daggers"를 낼 것이다. 그래서 대소문자가 앞서 거기 있던 것에서 문자 하나하나 취해진다.
 *
 * @param dst 덮어쓸 문자열. 제자리에서 수정된다
 * @param src 대신할 글
 * @return 같은 @p dst
 * @note 기존 주석이 적듯 새 글은 옛 것보다 짧거나 같거나 길 수 있다. 그것이 더 길 때 옛 글이 다하고, 그때부터 옛 글의 마지막 문자의 대소문자가 남은 모든 것에 적용된다. 그것이 "Dagger"가 "Daggers"가 되는 것이 옳게 대문자가 되고 더해진 글자를 다른 대소문자로 남기지 않는 이유다.
 * @warning 기존 주석이 인정하듯 이미 빈 목적지와 함께 호출되면 그 앞의 문자를 읽는다. 그래서 그런 호출은 더 긴 문자열의 가운데나 끝을 가리키는 포인터에서만 유효하며, 버퍼의 시작에서는 결코 아니다. 무엇도 이것을 검사하지 않으며, 그 읽기는 할당 밖일 것이다.
 */
char *
strcasecpy(char *dst, const char *src)
{
    char *result = dst;
    int ic, oc, dst_exhausted = 0;

    /* while dst has characters, replace each one with corresponding
       character from src, converting case in the process if they differ;
       once dst runs out, propagate the case of its last character to any
       remaining src; if dst starts empty, it must be a pointer to the
       tail of some other string because we examine the char at dst[-1] */
    while ((ic = (int) *src++) != '\0') {
        if (!dst_exhausted && !*dst)
            dst_exhausted = 1;
        oc = (int) *(dst - dst_exhausted);
        *dst++ = chrcasecpy(oc, ic);
    }
    *dst = '\0';
    return result;
}

/** @} */

/**
 * @name English word forms
 * @{
 */

/**
 * @brief A name in the possessive.
 * @param s the name
 * @return the possessive form, in a shared buffer
 * @note Handles the two pronouns the game uses in messages as special cases, since "it's" and "you's" would both be wrong -- the first means something else and the second is not English.
 * @note A name already ending in an s takes only an apostrophe, which is the convention for a plural or a classical name.
 * @warning The result is in a buffer shared by every caller. Two calls in one message expression overwrite each other, so a message naming two possessors must copy the first result before asking for the second.
 */
/**
 * @brief 소유격으로 된 이름.
 * @param s 그 이름
 * @return 소유격 형태. 공유된 버퍼에
 * @note 게임이 메시지에서 쓰는 두 대명사를 특별한 경우로 다룬다. "it's"와 "you's"는 둘 다 틀릴 것이기 때문이다. 첫 번째는 다른 것을 뜻하고 두 번째는 영어가 아니다.
 * @note 이미 s로 끝나는 이름은 아포스트로피만 취하며, 그것이 복수형이나 고전적인 이름에 대한 관례다.
 * @warning 결과는 모든 호출자가 공유하는 버퍼에 있다. 한 메시지 식 안의 두 호출이 서로를 덮어쓰므로, 두 소유자를 이름 짓는 메시지는 두 번째를 요청하기 전에 첫 결과를 복사해야 한다.
 */
char *
s_suffix(const char *s)
{
    static char buf[BUFSZ];

    Strcpy(buf, s);
    if (!strcmpi(buf, "it")) /* it -> its */
        Strcat(buf, "s");
    else if (!strcmpi(buf, "you")) /* you -> your */
        Strcat(buf, "r");
    else if (*(eos(buf) - 1) == 's') /* Xs -> Xs' */
        Strcat(buf, "'");
    else /* X -> X's */
        Strcat(buf, "'s");
    return buf;
}

/**
 * @brief Turn a verb into its "-ing" form, applying the English spelling rules.
 *
 * Needed because the game describes what the hero is doing in progress -- "You stop digging" -- and the verb is stored in its plain form. Appending the ending naively would give "diging" and "greaseing".
 *
 * Four rules are applied, in the order tried: a verb already ending in "er" takes the ending as it is; a short verb ending consonant-vowel-consonant doubles its last letter; one ending in "ie" changes that to a y; and one ending in a silent e
 * drops it.
 *
 * @param s the verb, which may carry a trailing particle
 * @return the gerund, in a shared buffer
 * @note A trailing particle -- "on", "off", "with" -- is detached before the ending is applied and put back afterwards, so "put on" becomes "putting on" rather than "put oning".
 * @warning Shares the same hazard as the possessive: a single buffer, overwritten by the next call.
 */
/**
 * @brief 동사를 "-ing" 형태로 만들며, 영어 철자 규칙을 적용한다.
 *
 * 게임이 영웅이 진행 중에 하고 있는 일을 서술하기 때문에 필요하다. "You stop digging". 그리고 동사는 맨 형태로 저장된다. 그 어미를 순진하게 덧붙이면 "diging"과 "greaseing"을 낼 것이다.
 *
 * 네 규칙이 시도되는 순서대로 적용된다. 이미 "er"로 끝나는 동사는 어미를 그대로 취한다. 자음, 모음, 자음으로 끝나는 짧은 동사는 마지막 글자를 겹친다. "ie"로 끝나는 것은 그것을 y로 바꾼다. 그리고 묵음 e로 끝나는 것은 그것을 버린다.
 *
 * @param s 그 동사. 뒤따르는 불변화사를 지닐 수 있다
 * @return 그 동명사. 공유된 버퍼에
 * @note 뒤따르는 불변화사, "on", "off", "with"는 어미가 적용되기 전에 떼어지고 뒤에 되붙여지므로, "put on"이 "put oning"이 아니라 "putting on"이 된다.
 * @warning 소유격과 같은 위험을 나눈다. 하나의 버퍼이며, 다음 호출이 덮어쓴다.
 */
char *
ing_suffix(const char *s)
{
    static const char vowel[] = "aeiouwy";
    static char buf[BUFSZ];
    char onoff[10];
    char *p;

    Strcpy(buf, s);
    p = eos(buf);
    onoff[0] = *p = *(p + 1) = '\0';
    if ((p >= &buf[3] && !strcmpi(p - 3, " on"))
        || (p >= &buf[4] && !strcmpi(p - 4, " off"))
        || (p >= &buf[5] && !strcmpi(p - 5, " with"))) {
        p = strrchr(buf, ' ');
        Strcpy(onoff, p);
        *p = '\0';
    }
    if (p >= &buf[2] && !strcmpi(p - 2, "er")) { /* slither + ing */
        /* nothing here */
    } else if (p >= &buf[3] && !strchr(vowel, *(p - 1))
        && strchr(vowel, *(p - 2)) && !strchr(vowel, *(p - 3))) {
        /* tip -> tipp + ing */
        *p = *(p - 1);
        *(p + 1) = '\0';
    } else if (p >= &buf[2] && !strcmpi(p - 2, "ie")) { /* vie -> vy + ing */
        *(p - 2) = 'y';
        *(p - 1) = '\0';
    } else if (p >= &buf[1] && *(p - 1) == 'e') /* grease -> greas + ing */
        *(p - 1) = '\0';
    Strcat(buf, "ing");
    if (onoff[0])
        Strcat(buf, onoff);
    return buf;
}

/** @} */

/**
 * @brief Scramble or unscramble text with a trivial reversible cipher.
 *
 * Applied to the rumor, oracle and epitaph data files so that a player browsing them does not spoil the game by accident. It is not security and does not pretend to be -- the same routine both scrambles and unscrambles, and the method is
 * described here in the open.
 *
 * @param str the text
 * @param buf where to write the result
 * @return the buffer
 * @note Its own inverse, which is why the data-building tool and the game can share one routine.
 * @note Only characters with either of two particular bits set are altered, so digits and most punctuation pass through unchanged. That is deliberate: the file's structural characters must stay readable for the format to be parsed.
 * @warning The buffer must be at least as long as the text plus a terminator. Nothing is checked.
 */
/**
 * @brief 하찮은 되돌릴 수 있는 암호로 글을 뒤섞거나 되돌린다.
 *
 * 소문, 신탁, 묘비명 데이터 파일에 적용되어 그것을 뒤져 보는 플레이어가 우연히 게임을 망치지 않게 한다. 그것은 보안이 아니며 그런 척하지도 않는다. 같은 함수가 뒤섞기도 되돌리기도 하며, 그 방법이 여기 드러나 서술되어 있다.
 *
 * @param str 그 글
 * @param buf 결과를 쓸 곳
 * @return 그 버퍼
 * @note 자기 자신의 역이며, 그것이 데이터를 만드는 도구와 게임이 하나의 함수를 나눌 수 있는 이유다.
 * @note 두 특정 비트 중 어느 것이 설정된 문자만이 바뀌므로, 숫자와 대부분의 기호는 바뀌지 않고 지나간다. 그것은 의도된 것이다. 형식이 해석되기 위해 파일의 구조 문자가 읽을 수 있는 채로 머물러야 한다.
 * @warning 버퍼는 적어도 글의 길이에 종료 문자를 더한 만큼 길어야 한다. 무엇도 검사되지 않는다.
 */
char *
xcrypt(const char *str, char *buf)
{
    const char *p;
    char *q;
    int bitmask;

    for (bitmask = 1, p = str, q = buf; *p; q++) {
        *q = *p++;
        if (*q & (32 | 64))
            *q ^= bitmask;
        if ((bitmask <<= 1) >= 32)
            bitmask = 1;
    }
    *q = '\0';
    return buf;
}

/**
 * @brief Is a string entirely whitespace?
 * @param s the string
 * @return whether it holds nothing but spaces and tabs
 * @note An empty string counts as entirely whitespace, which is what the callers want: they are asking whether there is anything to act on.
 */
/**
 * @brief 문자열이 온통 공백인가?
 * @param s 그 문자열
 * @return 그것이 공백과 탭 말고 아무것도 담지 않는지
 * @note 빈 문자열이 온통 공백으로 셈되며, 그것이 호출자들이 원하는 것이다. 그들은 행동할 무엇이 있는지를 묻고 있다.
 */
boolean
onlyspace(const char *s)
{
    for (; *s; s++)
        if (*s != ' ' && *s != '\t')
            return FALSE;
    return TRUE;
}

/**
 * @brief Replace tabs with the spaces that reach the next tab stop.
 *
 * Needed because the game's display writes characters at positions and does not interpret a tab, so text from a data file containing tabs would come out misaligned.
 *
 * @param sbuf the string, replaced in place; expansion beyond the buffer's capacity is truncated, as the existing comment records
 * @return the same buffer
 * @note Tab stops are every eight columns, counted from the start of the string rather than from the tab, which is what makes columns line up between lines.
 * @note Built in a working buffer and copied back, because expansion makes the text longer and cannot be done in place.
 * @note There is a comment recording that one compiler version mis-compiled the expansion loop, causing the game to appear to hang while reading data entries. It names the versions affected and notes that whether the fault was fixed or merely
 *       disabled is unknown -- which is why the note is worth keeping rather than being assumed stale.
 */
/**
 * @brief 탭을 다음 탭 위치에 이르는 공백으로 바꾼다.
 *
 * 게임의 화면이 문자를 위치에 쓰고 탭을 해석하지 않기 때문에 필요하다. 그래서 탭을 담은 데이터 파일의 글은 어긋나게 나올 것이다.
 *
 * @param sbuf 그 문자열. 제자리에서 갈아치워진다. 기존 주석이 기록하듯 버퍼 용량을 넘는 펼침은 잘린다
 * @return 같은 버퍼
 * @note 탭 위치는 여덟 열마다이며, 탭에서가 아니라 문자열 시작에서 세어진다. 그것이 열이 줄 사이에서 맞춰지게 하는 것이다.
 * @note 작업 버퍼에 만들어져 되복사된다. 펼침이 글을 길게 만들고 제자리에서 이루어질 수 없기 때문이다.
 * @note 한 컴파일러 판본이 그 펼침 되돌기를 잘못 컴파일해 게임이 데이터 항목을 읽는 동안 멈춘 것처럼 보이게 했다는 것을 기록하는 주석이 있다. 그것은 영향받은 판본을 이름 짓고, 그 결함이 고쳐졌는지 그저 비활성화되었는지 알 수 없다고 적는다. 그것이 그 적바림이 낡은 것으로 여겨지는 대신 지녀 둘 만한
 *       이유다.
 */
char *
tabexpand(
    char *sbuf) /* assumed to be [BUFSZ] but can be smaller provided that
                 * expanded string fits; expansion bigger than BUFSZ-1
                 * will be truncated */
{
    char buf[BUFSZ + 10];
    char *bp, *s = sbuf;
    int idx;

    if (!*s)
        return sbuf;
    for (bp = buf, idx = 0; *s; s++) {
        if (*s == '\t') {
            /*
             * clang-8's optimizer at -Os has been observed to mis-compile
             * this code.  Symptom is nethack getting stuck in an apparent
             * infinite loop (or perhaps just an extremely long one) when
             * examining data.base entries.
             * clang-9 doesn't exhibit this problem.  [Was the incorrect
             * optimization fixed or just disabled?]
             */
            do
                *bp++ = ' ';
            while (++idx % 8);
        } else {
            *bp++ = *s;
            ++idx;
        }
        if (idx >= BUFSZ) {
            bp = &buf[BUFSZ - 1];
            break;
        }
    }
    *bp = 0;
    return strcpy(sbuf, buf);
}

/**
 * @def VISCTRL_NBUF
 * @brief How many results can be alive at once.
 * @note Chosen to be enough for the messages that show several keys in one line. It is not a bound anything checks -- exceeding it silently reuses the oldest.
 */
/**
 * @def VISCTRL_NBUF
 * @brief 한 번에 몇 개의 결과가 살아 있을 수 있는지.
 * @note 한 줄에 여러 키를 보이는 메시지에 넉넉하도록 골라졌다. 그것은 무엇이 검사하는 한계가 아니다. 그것을 넘으면 조용히 가장 오래된 것을 다시 쓴다.
 */
#define VISCTRL_NBUF 5

/**
 * @brief Render a character in a form that can be printed.
 *
 * A control character has no printable shape, so it is shown the way a terminal conventionally writes one -- a caret and a letter. Used wherever the game names a key to the player, since the key a player pressed may not be printable.
 *
 * @param c the character
 * @return a short printable string, in one of several rotating buffers
 * @note Rotates through a handful of buffers rather than using one, so a message naming two keys can render both before printing. That is a bounded fix rather than a real one -- a message naming more keys than there are buffers overwrites its
 *       earliest, and nothing warns.
 * @note A high-bit character is shown with a meta prefix and then its remaining bits rendered as usual, so an accented key and the control character it shares bits with are distinguishable.
 * @note The delete character is shown as a caret and a question mark, which does not follow from the same arithmetic as the other control characters and is handled separately.
 */
/**
 * @brief 문자를 인쇄할 수 있는 형태로 그린다.
 *
 * 제어 문자는 인쇄할 수 있는 모양을 갖지 않으므로, 터미널이 관례적으로 그것을 쓰는 방식으로 보인다. 삽 기호와 글자. 게임이 플레이어에게 키를 이름 짓는 어디서든 쓰이는데, 플레이어가 누른 키가 인쇄할 수 없는 것일 수 있기 때문이다.
 *
 * @param c 그 문자
 * @return 짧은 인쇄할 수 있는 문자열. 돌아가는 여러 버퍼 중 하나에
 * @note 하나를 쓰는 대신 몇 개의 버퍼를 돌려 쓰므로, 두 키를 이름 짓는 메시지가 인쇄하기 전에 둘 다 그릴 수 있다. 그것은 진짜 해결이 아니라 한정된 임시 조치다. 버퍼보다 많은 키를 이름 짓는 메시지는 자기 가장 이른 것을 덮어쓰며, 무엇도 경고하지 않는다.
 * @note 높은 비트가 설정된 문자는 메타 접두와 함께 보이고 그다음 남은 비트가 보통대로 그려지므로, 악센트가 있는 키와 그것과 비트를 나누는 제어 문자가 구별될 수 있다.
 * @note 삭제 문자는 삽 기호와 물음표로 보이며, 그것은 다른 제어 문자와 같은 산술에서 따라 나오지 않고 따로 다뤄진다.
 */
char *
visctrl(char c)
{
    static char visctrl_bufs[VISCTRL_NBUF][5];
    static int nbuf = 0;
    int i = 0;
    char *ccc = visctrl_bufs[nbuf];
    nbuf = (nbuf + 1) % VISCTRL_NBUF;

    if ((uchar) c & 0200) {
        ccc[i++] = 'M';
        ccc[i++] = '-';
    }
    c &= 0177;
    if (c < 040) {
        ccc[i++] = '^';
        ccc[i++] = c | 0100; /* letter */
    } else if (c == 0177) {
        ccc[i++] = '^';
        ccc[i++] = c & ~0100; /* '?' */
    } else {
        ccc[i++] = c; /* printable character */
    }
    ccc[i] = '\0';
    return ccc;
}

/**
 * @brief Copy a string, leaving out every character in a given set.
 * @param bp where to write the result
 * @param stuff_to_strip the characters to omit
 * @param orig the string to copy
 * @return the destination buffer
 * @warning The destination must be a full-sized buffer, as the existing comment insists, because that is the only bound the copy respects -- it truncates at that size regardless of how long the source is. A smaller buffer is overrun.
 * @note Copies rather than working in place, unlike most of the tidying routines here, which is why the source may be read-only.
 */
/**
 * @brief 주어진 묶음의 모든 문자를 빼고 문자열을 복사한다.
 * @param bp 결과를 쓸 곳
 * @param stuff_to_strip 빼놓을 문자들
 * @param orig 복사할 문자열
 * @return 그 목적지 버퍼
 * @warning 기존 주석이 고집하듯 목적지는 온전한 크기의 버퍼여야 한다. 그것이 그 복사가 존중하는 유일한 한계이기 때문이다. 그것은 원본이 얼마나 긴지와 무관하게 그 크기에서 자른다. 더 작은 버퍼는 넘쳐 흐른다.
 * @note 여기 대부분의 정돈 함수와 달리 제자리에서 일하는 대신 복사하며, 그것이 원본이 읽기 전용일 수 있는 이유다.
 */
char *
stripchars(
    char *bp,
    const char *stuff_to_strip,
    const char *orig)
{
    int i = 0;
    char *s = bp;

    while (*orig && i < (BUFSZ - 1)) {
        if (!strchr(stuff_to_strip, *orig)) {
            *s++ = *orig;
            i++;
        }
        orig++;
    }
    *s = '\0';

    return bp;
}

/**
 * @brief Remove every digit from a string.
 * @param s the string, shortened in place
 * @return the same string
 * @note Works in place rather than copying, unlike the general character-stripping routine, because removing characters only shortens the text.
 * @note Tests the digit range directly instead of calling the classification routine, which comes to the same answer.
 */
/**
 * @brief 문자열에서 모든 숫자를 없앤다.
 * @param s 그 문자열. 제자리에서 짧아진다
 * @return 같은 문자열
 * @note 일반 문자 걷어내기 함수와 달리 복사하는 대신 제자리에서 일한다. 문자를 없애는 것이 글을 짧게만 하기 때문이다.
 * @note 분류 함수를 호출하는 대신 숫자 범위를 곧바로 검사하며, 그것은 같은 답에 이른다.
 */
char *
stripdigits(char *s)
{
    char *s1, *s2;

    for (s1 = s2 = s; *s1; s1++)
        if (*s1 < '0' || *s1 > '9')
            *s2++ = *s1;
    *s2 = '\0';

    return s;
}

/**
 * @brief Replace the first occurrence of a substring.
 * @param bp the string, modified in place
 * @param orig the text to look for
 * @param replacement what to put in its place
 * @return the same string
 * @note Could be expressed as the general substitution routine asked for one occurrence, as the existing comment observes. It is kept separate because the simple case is the common one and reads more plainly at the call site.
 * @warning The caller must ensure the buffer can hold the result, as the existing comment insists. A replacement longer than what it replaces makes the string longer, and nothing here checks. Unlike the general routine, this has no internal
 *          bound at all.
 */
/**
 * @brief 어떤 부분 문자열의 첫 나타남을 갈아치운다.
 * @param bp 그 문자열. 제자리에서 수정된다
 * @param orig 찾을 글
 * @param replacement 그 자리에 놓을 것
 * @return 같은 문자열
 * @note 기존 주석이 살피듯 일반 갈아치우기 함수에 한 번을 요청한 것으로 나타낼 수 있다. 따로 지녀지는 것은 단순한 경우가 흔한 경우이고 호출하는 곳에서 더 담박하게 읽히기 때문이다.
 * @warning 기존 주석이 고집하듯 호출자가 버퍼가 결과를 담을 수 있음을 보장해야 한다. 자기가 갈아치우는 것보다 긴 대신할 것은 문자열을 길게 만들고, 여기의 무엇도 검사하지 않는다. 일반 함수와 달리 이것은 내부 한계가 아예 없다.
 */
char *
strsubst(
    char *bp,
    const char *orig,
    const char *replacement)
{
    char *found, buf[BUFSZ];
    /* [this could be replaced by strNsubst(bp, orig, replacement, 1)] */

    found = strstr(bp, orig);
    if (found) {
        Strcpy(buf, found + strlen(orig));
        Strcpy(found, replacement);
        Strcat(bp, buf);
    }
    return bp;
}

/**
 * @brief Replace a chosen occurrence of a substring, or every occurrence.
 *
 * Three operations in one, reached by giving the arguments unusual values, and the existing comments record each: an empty text to look for inserts rather than replaces, an empty replacement deletes, and a count of zero means all rather than
 * none.
 *
 * @param inoutbuf the string, and where the result goes
 * @param orig the text to look for; empty means insert before the Nth character
 * @param replacement what to put in its place; empty means delete
 * @param n which occurrence, or zero for all
 * @return how many substitutions were made
 * @note Builds the result in a working buffer and copies back only if something changed, so a string with no match is left untouched rather than being rewritten identically.
 * @note Bounded by the standard buffer size throughout, so an expansion that would overflow is truncated instead. That is why the count returned can be right while the text is short.
 * @note There is a special case for inserting after the last character. As the existing comment explains, with an empty search text the occurrence counter advances once per input character, so asking for the occurrence one past the length means
 *       appending -- which the main loop cannot reach because it stops at the terminator.
 * @warning The zero-means-all convention makes an off-by-one error silent in a particular way: a caller that computes an occurrence number and gets zero replaces everything rather than nothing.
 */
/**
 * @brief 부분 문자열의 골라진 나타남 하나, 또는 모든 나타남을 갈아치운다.
 *
 * 인자에 별난 값을 줌으로써 닿는 세 연산이 하나에 있으며, 기존 주석들이 각각을 기록한다. 찾을 글이 비어 있으면 갈아치우는 대신 끼워 넣고, 대신할 것이 비어 있으면 지우고, 개수가 영이면 하나도 아닌 것이 아니라 모두를 뜻한다.
 *
 * @param inoutbuf 그 문자열, 그리고 결과가 가는 곳
 * @param orig 찾을 글. 비어 있으면 N번째 문자 앞에 끼워 넣기를 뜻한다
 * @param replacement 그 자리에 놓을 것. 비어 있으면 지우기를 뜻한다
 * @param n 몇 번째 나타남인지, 또는 모두에 영
 * @return 몇 번의 갈아치우기가 이루어졌는지
 * @note 결과를 작업 버퍼에 만들고 무언가 바뀌었을 때만 되복사하므로, 맞음이 없는 문자열은 똑같이 다시 쓰이는 대신 건드려지지 않은 채로 남는다.
 * @note 처음부터 끝까지 표준 버퍼 크기로 제한되므로, 넘칠 펼침은 대신 잘린다. 그것이 돌려주는 개수는 옳으면서 글은 짧을 수 있는 이유다.
 * @note 마지막 문자 뒤에 끼워 넣기를 위한 특별한 경우가 있다. 기존 주석이 설명하듯 찾을 글이 비어 있으면 나타남 계수기가 입력 문자마다 한 번 나아가므로, 길이보다 하나 뒤의 나타남을 요청하는 것은 덧붙이기를 뜻한다. 주된 되돌기는 종료 문자에서 멈추므로 그것에 닿을 수 없다.
 * @warning 영이 모두를 뜻하는 관례는 하나 차이 오류를 특정한 방식으로 조용하게 만든다. 나타남 번호를 계산해 영을 얻은 호출자는 아무것도 아닌 것이 아니라 모든 것을 갈아치운다.
 */
int
strNsubst(
    char *inoutbuf,   /* current string, and result buffer */
    const char *orig, /* old substring; if "", insert in front of Nth char */
    const char *replacement, /* new substring; if "", delete old substring */
    int n) /* which occurrence to replace; 0 => all */
{
    char *bp, *op, workbuf[BUFSZ];
    const char *rp;
    unsigned len = (unsigned) strlen(orig);
    int ocount = 0, /* number of times 'orig' has been matched */
        rcount = 0; /* number of substitutions made */

    for (bp = inoutbuf, op = workbuf; *bp && op < &workbuf[BUFSZ - 1]; ) {
        if ((!len || !strncmp(bp, orig, len)) && (++ocount == n || n == 0)) {
            /* Nth match found */
            for (rp = replacement; *rp && op < &workbuf[BUFSZ - 1]; )
                *op++ = *rp++;
            ++rcount;
            if (len) {
                bp += len; /* skip 'orig' */
                continue;
            }
        }
        /* no match (or len==0) so retain current character */
        *op++ = *bp++;
    }
    if (!len && n == ocount + 1) {
        /* special case: orig=="" (!len) and n==strlen(inoutbuf)+1,
           insert in front of terminator (in other words, append);
           [when orig=="", ocount will have been incremented once for
           each input char] */
        for (rp = replacement; *rp && op < &workbuf[BUFSZ - 1]; )
            *op++ = *rp++;
        ++rcount;
    }
    if (rcount) {
        *op = '\0';
        Strcpy(inoutbuf, workbuf);
    }
    return rcount;
}

/**
 * @brief Find a whole word in a space-separated list.
 * @param list the words, separated by spaces
 * @param word the word to look for
 * @param wordlen its length, so it need not be terminated, as the existing comment records
 * @param ignorecase whether to compare case-blind
 * @return where the word begins in the list, or null
 * @note Matches whole words only: a word must be followed by a space or the end of the list, so looking for one word does not match the beginning of a longer one.
 * @note The length being passed separately is what lets a caller search for a word that is part of a larger string without copying it out first.
 * @note Returns a position rather than merely whether it was found, so a caller can read what follows the word.
 */
/**
 * @brief 공백으로 나뉜 목록에서 온전한 낱말을 찾는다.
 * @param list 공백으로 나뉜 낱말들
 * @param word 찾을 낱말
 * @param wordlen 그 길이. 기존 주석이 기록하듯 그것이 종료되어야 하지 않도록
 * @param ignorecase 대소문자를 가리지 않고 비교할지
 * @return 그 낱말이 목록에서 시작하는 곳, 또는 널
 * @note 온전한 낱말만 맞춘다. 낱말은 공백이나 목록의 끝이 뒤따라야 하므로, 한 낱말을 찾는 것이 더 긴 낱말의 시작과 맞지 않는다.
 * @note 길이가 따로 넘겨지는 것이, 호출자가 더 큰 문자열의 일부인 낱말을 먼저 꺼내 복사하지 않고 찾을 수 있게 하는 것이다.
 * @note 발견되었는지만이 아니라 위치를 돌려주므로, 호출자가 그 낱말 뒤에 오는 것을 읽을 수 있다.
 */
const char *
findword(
    const char *list,   /* string of space-separated words */
    const char *word,   /* word to try to find */
    int wordlen,        /* so that it isn't required to be \0 terminated */
    boolean ignorecase) /* T: case-blind, F: case-sensitive */
{
    const char *p = list;

    while (p) {
        while (*p == ' ')
            ++p;
        if (!*p)
            break;
        if ((ignorecase ? !strncmpi(p, word, wordlen)
                        : !strncmp(p, word, wordlen))
            && (p[wordlen] == '\0' || p[wordlen] == ' '))
            return p;
        p = strchr(p + 1, ' ');
    }
    return (const char *) 0;
}

/**
 * @name Numeric helpers
 * @{
 */

/**
 * @brief The English ordinal ending for a number.
 * @param n the number, which should not be negative as the existing comment records
 * @return "st", "nd", "rd" or "th"
 * @note The teens are handled by testing the tens digit, because eleven through thirteen take "th" despite ending in one, two and three. That single condition is the whole reason this is not a lookup on the last digit.
 * @note A negative number is not refused but gives a nonsense ending, since the remainder of a negative is itself negative and matches nothing.
 */
/**
 * @brief 수에 대한 영어 서수 어미.
 * @param n 그 수. 기존 주석이 기록하듯 음수여서는 안 된다
 * @return "st", "nd", "rd" 또는 "th"
 * @note 십몇 대는 십의 자리를 검사함으로써 다뤄진다. 열하나부터 열셋이 하나, 둘, 셋으로 끝남에도 "th"를 취하기 때문이다. 그 하나의 조건이 이것이 마지막 숫자에 대한 찾기가 아닌 이유 전부다.
 * @note 음수는 거부되지 않지만 뜻 없는 어미를 낸다. 음수의 나머지가 그 자체로 음수이고 아무것과도 맞지 않기 때문이다.
 */
const char *
ordin(int n)               /* note: should be non-negative */
{
    int dd = n % 10;

    return (dd == 0 || dd > 3 || (n % 100) / 10 == 1) ? "th"
               : (dd == 1) ? "st" : (dd == 2) ? "nd" : "rd";
}

DISABLE_WARNING_FORMAT_NONLITERAL  /* one compiler complains about
                                      result of ?: for format string */

/**
 * @brief Render a number always carrying its sign.
 * @param n the number
 * @return the text, in a shared buffer
 * @note A plus is added only for a number that is not negative, since a negative one already prints its own minus. So the condition reads backwards from what one expects -- the negative case is the one that needs nothing done.
 * @note Zero comes out with a plus, which is what an enchantment display wants: a weapon known to be unenchanted shows plus zero rather than a bare zero, so that "known to be zero" and "unknown" look different.
 * @note The format is chosen by an expression, which one compiler objects to on principle. The surrounding directives silence that rather than restructuring the call.
 * @warning Shared buffer; the next call overwrites it.
 */
/**
 * @brief 수를 언제나 그 부호를 지닌 채로 그린다.
 * @param n 그 수
 * @return 그 글. 공유된 버퍼에
 * @note 음수가 아닌 수에만 플러스가 더해진다. 음수는 이미 자기 마이너스를 인쇄하기 때문이다. 그래서 그 조건이 기대하는 것과 거꾸로 읽힌다. 음수인 경우가 아무것도 할 필요가 없는 쪽이다.
 * @note 영이 플러스와 함께 나오며, 그것이 강화 값 표시가 원하는 것이다. 강화되지 않은 것으로 알려진 무기는 맨 영이 아니라 플러스 영을 보이므로, "영으로 알려짐"과 "알 수 없음"이 다르게 보인다.
 * @note 형식이 식으로 골라지는데, 한 컴파일러가 원칙적으로 그것에 반대한다. 둘러싼 지시문이 호출을 다시 짜는 대신 그것을 잠잠하게 한다.
 * @warning 공유된 버퍼다. 다음 호출이 그것을 덮어쓴다.
 */
char *
sitoa(int n)
{
    static char buf[13];

    Sprintf(buf, (n < 0) ? "%d" : "+%d", n);
    return buf;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/**
 * @brief The sign of a number.
 * @param n the number
 * @return -1, 0 or 1
 * @note Used to turn a difference between two positions into a direction, which is why it must give zero for zero rather than folding that into one of the other two.
 */
/**
 * @brief 수의 부호.
 * @param n 그 수
 * @return -1, 0 또는 1
 * @note 두 위치 사이의 차이를 방향으로 바꾸는 데 쓰이며, 그것이 이것이 영을 다른 둘 중 하나에 접어 넣는 대신 영에 대해 영을 주어야 하는 이유다.
 */
int
sgn(int n)
{
    return (n < 0) ? -1 : (n != 0);
}

/** @} */

/**
 * @name Distance on the map
 * @{
 */

/**
 * @brief How many moves apart two squares are.
 *
 * Not the straight-line distance. Because a move may go diagonally, crossing both a column and a row costs the same as crossing one -- so the number of moves is the larger of the two differences, as the existing comment states.
 *
 * @param x0 first column
 * @param y0 first row
 * @param x1 second column
 * @param y1 second row
 * @return the number of moves
 * @note This is the distance the game means almost everywhere: whether a monster is adjacent, whether something is in reach, how far a thrown object travels. Substituting the straight-line distance would change the shape of every range in the
 *       game from a square to a circle.
 */
/**
 * @brief 두 칸이 몇 이동만큼 떨어져 있는지.
 *
 * 직선 거리가 아니다. 이동이 대각선으로 갈 수 있으므로, 열과 행을 모두 건너는 것이 하나를 건너는 것과 같은 값이 든다. 그래서 기존 주석이 밝히듯 이동의 수는 두 차이 중 더 큰 것이다.
 *
 * @param x0 첫 열
 * @param y0 첫 행
 * @param x1 두 번째 열
 * @param y1 두 번째 행
 * @return 이동의 수
 * @note 이것이 게임이 거의 어디서나 뜻하는 거리다. 몬스터가 인접한지, 무언가가 닿는 곳에 있는지, 던진 물건이 얼마나 멀리 가는지. 직선 거리를 대신 쓰는 것은 게임의 모든 사거리 모양을 정사각형에서 원으로 바꿀 것이다.
 */
int
distmin(coordxy x0, coordxy y0, coordxy x1, coordxy y1)
{
    coordxy dx = x0 - x1, dy = y0 - y1;

    if (dx < 0)
        dx = -dx;
    if (dy < 0)
        dy = -dy;
    /*  The minimum number of moves to get from (x0,y0) to (x1,y1) is the
     *  larger of the [absolute value of the] two deltas.
     */
    return (dx < dy) ? dy : dx;
}

/**
 * @brief The square of the straight-line distance between two squares.
 * @param x0 first column
 * @param y0 first row
 * @param x1 second column
 * @param y1 second row
 * @return the squared distance
 * @note Squared deliberately, not as an intermediate step. Comparing squared distances against squared limits answers the same question as comparing distances, without a square root -- so a circular range is expressed as a squared limit and no
 *       approximation enters.
 * @warning Not the same measure as the move distance, and the two are not interchangeable. Using this where moves are meant makes a range round where the game elsewhere makes it square.
 */
/**
 * @brief 두 칸 사이 직선 거리의 제곱.
 * @param x0 첫 열
 * @param y0 첫 행
 * @param x1 두 번째 열
 * @param y1 두 번째 행
 * @return 제곱된 거리
 * @note 중간 단계가 아니라 의도적으로 제곱된 것이다. 제곱된 거리를 제곱된 한계와 비교하는 것은 거리를 비교하는 것과 같은 질문에 답하며, 제곱근 없이 그렇게 한다. 그래서 둥근 사거리가 제곱된 한계로 나타내어지고 어떤 근사도 들어오지 않는다.
 * @warning 이동 거리와 같은 잣대가 아니며, 그 둘은 서로 바꿔 쓸 수 없다. 이동이 뜻해진 곳에 이것을 쓰는 것은 게임이 다른 곳에서 정사각형으로 만드는 사거리를 둥글게 만든다.
 */
int
dist2(coordxy x0, coordxy y0, coordxy x1, coordxy y1)
{
    coordxy dx = x0 - x1, dy = y0 - y1;

    return dx * dx + dy * dy;
}

/**
 * @brief Integer square root, without floating point.
 * @param val the number
 * @return the largest integer whose square does not exceed it
 * @note Avoids floating point deliberately. The game must behave identically on every machine, and floating point does not guarantee that -- a rounding difference could change a range by a square.
 * @note Computed by subtracting successive odd numbers, which is the simplest method rather than the fastest. The existing comment sets out the four reasons that was judged the right trade: it reads plainly, it needs no wide arithmetic, the values
 *       passed are small, and the few callers are not in hot paths.
 */
/**
 * @brief 부동소수점 없는 정수 제곱근.
 * @param val 그 수
 * @return 제곱이 그것을 넘지 않는 가장 큰 정수
 * @note 부동소수점을 의도적으로 피한다. 게임은 모든 기계에서 똑같이 행동해야 하고, 부동소수점은 그것을 보장하지 않는다. 반올림 차이가 사거리를 한 칸 바꿀 수 있다.
 * @note 잇따르는 홀수를 빼서 계산되며, 그것은 가장 빠른 방법이 아니라 가장 단순한 방법이다. 기존 주석이 그것이 옳은 맞바꿈으로 판단된 네 이유를 밝힌다. 담박하게 읽히고, 넓은 산술을 필요로 하지 않고, 넘겨지는 값이 작고, 몇 안 되는 호출자가 뜨거운 경로에 없다.
 */
int
isqrt(int val)
{
    int rt = 0;
    int odd = 1;
    /*
     * This could be replaced by a faster algorithm, but has not been because:
     * + the simple algorithm is easy to read;
     * + this algorithm does not require 64-bit support;
     * + in current usage, the values passed to isqrt() are not really that
     *   large, so the performance difference is negligible;
     * + isqrt() is used in only few places, which are not bottle-necks.
     */
    while (val >= odd) {
        val = val - odd;
        odd = odd + 2;
        rt = rt + 1;
    }
    return rt;
}

/**
 * @brief Are two squares lined up along a direction something can travel?
 *
 * Not any straight line -- only the eight directions. Two squares a knight's move apart lie on a straight line geometrically but nothing can move or fire between them, so this reports them as not lined up.
 *
 * @param x0 first column
 * @param y0 first row
 * @param x1 second column
 * @param y1 second row
 * @return whether they are on one of the eight lines
 * @note Decided by four comparisons, as the existing comment explains: either difference being zero means an orthogonal line, and the differences matching in size regardless of sign means a diagonal.
 * @note A square is lined up with itself, since both differences are zero. Callers that mean "lined up and distinct" must check that separately.
 */
/**
 * @brief 두 칸이 무언가가 지나갈 수 있는 방향으로 나란히 놓여 있는가?
 *
 * 아무 직선이 아니라 여덟 방향만이다. 나이트의 이동만큼 떨어진 두 칸은 기하학적으로 직선 위에 있지만 무엇도 그 사이를 움직이거나 쏠 수 없으므로, 이것은 그것들을 나란하지 않은 것으로 보고한다.
 *
 * @param x0 첫 열
 * @param y0 첫 행
 * @param x1 두 번째 열
 * @param y1 두 번째 행
 * @return 그것들이 여덟 선 중 하나에 있는지
 * @note 기존 주석이 설명하듯 네 비교로 정해진다. 어느 차이든 영인 것은 직교하는 선을 뜻하고, 부호와 무관하게 차이의 크기가 맞는 것은 대각선을 뜻한다.
 * @note 한 칸은 자기 자신과 나란하다. 두 차이가 모두 영이기 때문이다. "나란하고 서로 다른"을 뜻하는 호출자는 그것을 따로 검사해야 한다.
 */
boolean
online2(coordxy x0, coordxy y0, coordxy x1, coordxy y1)
{
    int dx = x0 - x1, dy = y0 - y1;
    /*  If either delta is zero then they're on an orthogonal line,
     *  else if the deltas are equal (signs ignored) they're on a diagonal.
     */
    return (boolean) (!dy || !dx || dy == dx || dy == -dx);
}

/** @} */

/**
 * @name Case-insensitive comparison
 *
 * Provided here only for systems whose libraries lack them, which is why each is wrapped in a test for the corresponding setting. On a system that has them the library version is used instead.
 * @{
 */

/**
 * @brief Compare two strings up to a length, ignoring case.
 * @param s1 first string
 * @param s2 second string
 * @param n how many characters to compare
 * @return negative, zero or positive as the first sorts before, equal to, or after the second
 * @note Named for the standard function it stands in for, as the existing comment records.
 * @note The count is a signed integer where it should be unsigned, as the existing comment concedes. A negative count would decrement toward the far end of the range rather than stopping immediately.
 */
/**
 * @brief 어떤 길이까지 두 문자열을 대소문자를 무시하고 비교한다.
 * @param s1 첫 문자열
 * @param s2 두 번째 문자열
 * @param n 몇 문자를 비교할지
 * @return 첫 것이 앞서, 같게, 뒤에 놓이는지에 따라 음수, 영, 양수
 * @note 기존 주석이 기록하듯 자기가 대신하는 표준 함수의 이름을 따랐다.
 * @note 기존 주석이 인정하듯 개수가 부호 없어야 할 곳에서 부호 있는 정수다. 음수 개수는 곧바로 멈추는 대신 범위의 먼 끝을 향해 줄어들 것이다.
 */
#ifndef STRNCMPI
int
strncmpi(
    const char *s1, const char *s2,
    int n) /*(should probably be size_t, which is unsigned)*/
{
    char t1, t2;

    while (n--) {
        if (!*s2)
            return (*s1 != 0); /* s1 >= s2 */
        else if (!*s1)
            return -1; /* s1  < s2 */
        t1 = lowc(*s1++);
        t2 = lowc(*s2++);
        if (t1 != t2)
            return (t1 > t2) ? 1 : -1;
    }
    return 0; /* s1 == s2 */
}
#endif /* STRNCMPI */

/**
 * @brief Find a substring within a string, ignoring case.
 *
 * Does more than the obvious repeated comparison, and the extra work is a filter that rules out a match cheaply. Before comparing anything it counts, for each group of characters, how many the string holds and how many the substring holds. If the
 * substring needs more of some group than the string has, no match is possible and the search stops without a single comparison.
 *
 * @param str the string to search
 * @param sub the substring to find
 * @return where it begins, or null
 * @note The counting also establishes the relative lengths as a side effect, so a substring longer than the string is rejected by the same pass. The existing comment notes this doing useful work while measuring.
 * @note The group size is chosen so that the two cases of a letter fall into one group, which is what makes the filter valid for a case-blind search. The accompanying comment records that doubling it would make the search case-sensitive instead --
 *       so the constant is not a tuning parameter but part of the meaning.
 * @note There are disabled assertions recording the two things the group size and the case routine must satisfy: the size must be an exact power of two for the masking to work, and lowering case must be a real function rather than a macro that
 *       evaluates its argument twice, since it is applied to an expression with a side effect.
 * @note An empty substring is found at the start, matching the convention of the standard search.
 */
/**
 * @brief 대소문자를 무시하고 문자열 안에서 부분 문자열을 찾는다.
 *
 * 뻔한 되풀이 비교보다 많은 일을 하며, 그 여분의 일은 맞음을 값싸게 배제하는 걸름이다. 무엇을 비교하기 전에 그것은 문자 묶음마다 문자열이 몇 개를 담고 부분 문자열이 몇 개를 담는지 센다. 부분 문자열이 어떤 묶음을 문자열이 가진 것보다 많이 필요로 하면 맞음이 있을 수 없고, 찾기가 단 한 번의 비교 없이 멈춘다.
 *
 * @param str 찾을 문자열
 * @param sub 찾을 부분 문자열
 * @return 그것이 시작하는 곳, 또는 널
 * @note 세기가 딸린 결과로 상대적 길이도 세우므로, 문자열보다 긴 부분 문자열이 같은 지나가기로 거부된다. 기존 주석이 재면서 쓸모 있는 일을 한다고 적는다.
 * @note 묶음 크기는 글자의 두 대소문자가 한 묶음에 들도록 골라지며, 그것이 그 걸름을 대소문자를 가리지 않는 찾기에 유효하게 만드는 것이다. 딸린 주석이 그것을 두 배로 하면 대신 찾기가 대소문자를 가리게 된다고 기록한다. 그러니 그 상수는 조절 값이 아니라 뜻의 일부다.
 * @note 묶음 크기와 대소문자 함수가 만족해야 하는 두 가지를 기록하는 비활성화된 단정문이 있다. 가리기가 동작하려면 크기가 정확한 이의 거듭제곱이어야 하고, 소문자로 만드는 것은 자기 인자를 두 번 평가하는 매크로가 아니라 진짜 함수여야 한다. 딸린 결과가 있는 식에 적용되기 때문이다.
 * @note 빈 부분 문자열은 시작에서 발견되며, 표준 찾기의 관례와 맞는다.
 */
#ifndef STRSTRI
char *
strstri(const char *str, const char *sub)
{
    const char *s1, *s2;
    int i, k;
#define TABSIZ 0x20                  /* 0x40 would be case-sensitive */
    char tstr[TABSIZ], tsub[TABSIZ]; /* nibble count tables */
#if 0
    assert( (TABSIZ & ~(TABSIZ-1)) == TABSIZ ); /* must be exact power of 2 */
    assert( &lowc != 0 );                       /* can't be unsafe macro */
#endif

    /* special case: empty substring */
    if (!*sub)
        return (char *) str;

    /* do some useful work while determining relative lengths */
    for (i = 0; i < TABSIZ; i++)
        tstr[i] = tsub[i] = 0; /* init */
    for (k = 0, s1 = str; *s1; k++)
        tstr[*s1++ & (TABSIZ - 1)]++;
    for (s2 = sub; *s2; --k)
        tsub[*s2++ & (TABSIZ - 1)]++;

    /* evaluate the info we've collected */
    if (k < 0)
        return (char *) 0;       /* sub longer than str, so can't match */
    for (i = 0; i < TABSIZ; i++) /* does sub have more 'x's than str? */
        if (tsub[i] > tstr[i])
            return (char *) 0; /* match not possible */

    /* now actually compare the substring repeatedly to parts of the string */
    for (i = 0; i <= k; i++) {
        s1 = &str[i];
        s2 = sub;
        while (lowc(*s1++) == lowc(*s2++))
            if (!*s2)
                return (char *) &str[i]; /* full match */
    }
    return (char *) 0; /* not found */
}
#endif /* STRSTRI */

/** @} */

/**
 * @brief Compare two strings for equality while disregarding certain characters entirely.
 *
 * Used to match what a player typed against a name the game knows, where the two may differ in spacing or punctuation without differing in meaning. As the existing comment records, the disregarded set is typically whitespace.
 *
 * @param s1 first string
 * @param s2 second string
 * @param ignore_chars the characters to disregard in both
 * @param caseblind whether to ignore case as well
 * @return whether they match
 * @note The disregarded characters are skipped in both strings independently, so they need not appear in the same places -- which is the point: "elven cloak" matches "elvencloak".
 * @note Equality only; there is no ordering. A comparison that skips characters cannot give a consistent order, so nothing here pretends to.
 * @note A match requires both strings to have ended, as the existing comment states. One string being a prefix of the other is not a match, unlike the beginning comparison above.
 */
/**
 * @brief 어떤 문자를 아예 무시하면서 두 문자열이 같은지 비교한다.
 *
 * 플레이어가 입력한 것을 게임이 아는 이름과 맞추는 데 쓰이며, 거기서 그 둘은 뜻에서 다르지 않으면서 띄어쓰기나 기호에서 다를 수 있다. 기존 주석이 기록하듯 무시되는 묶음은 보통 공백이다.
 *
 * @param s1 첫 문자열
 * @param s2 두 번째 문자열
 * @param ignore_chars 둘 모두에서 무시할 문자들
 * @param caseblind 대소문자도 무시할지
 * @return 그것들이 맞는지
 * @note 무시되는 문자가 두 문자열에서 독립적으로 건너뛰어지므로, 그것들이 같은 자리에 나타나야 하지 않는다. 그것이 요점이다. "elven cloak"이 "elvencloak"과 맞는다.
 * @note 같음뿐이며 순서는 없다. 문자를 건너뛰는 비교는 일관된 순서를 줄 수 없으므로, 여기의 무엇도 그런 척하지 않는다.
 * @note 기존 주석이 밝히듯 맞음은 두 문자열 모두가 끝났을 것을 요구한다. 위의 시작 비교와 달리, 한 문자열이 다른 것의 앞부분인 것은 맞음이 아니다.
 */
boolean
fuzzymatch(
    const char *s1, const char *s2,
    const char *ignore_chars,
    boolean caseblind)
{
    char c1, c2;

    do {
        while ((c1 = *s1++) != '\0' && strchr(ignore_chars, c1) != 0)
            continue;
        while ((c2 = *s2++) != '\0' && strchr(ignore_chars, c2) != 0)
            continue;
        if (!c1 || !c2)
            break; /* stop when end of either string is reached */

        if (caseblind) {
            c1 = lowc(c1);
            c2 = lowc(c2);
        }
    } while (c1 == c2);

    /* match occurs only when the end of both strings has been reached */
    return (boolean) (!c1 && !c2);
}

/*
 * Time routines
 *
 * The time is used for:
 *  - seed for rand()
 *  - year on tombstone and yyyymmdd in record file
 *  - phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *  - night and midnight (the undead are dangerous at midnight)
 *  - determination of what files are "very old"
 */

/**
 * @name Types for the system clock routines
 *
 * Defined here with the ordinary type but overridable, and the existing comment records why: some systems declare these routines as taking a plain long rather than the standard time type, so a port may need to say so in its own configuration
 * header.
 *
 * @note Overridable rather than conditional on a platform test, because the set of systems needing it is not knowable in advance -- a port declares its own need.
 * @{
 */
/**
 * @name 시스템 시계 함수를 위한 형
 *
 * 보통의 형으로 여기서 정의되지만 덮어쓸 수 있으며, 기존 주석이 이유를 기록한다. 어떤 시스템은 이 함수들을 표준 시간 형이 아니라 맨 long을 취하는 것으로 선언하므로, 이식판이 자기 설정 헤더에서 그렇다고 말해야 할 수 있다.
 *
 * @note 플랫폼 검사에 조건 지어지는 대신 덮어쓸 수 있는 것은, 그것을 필요로 하는 시스템의 묶음이 미리 알 수 있는 것이 아니기 때문이다. 이식판이 자기 필요를 스스로 밝힌다.
 * @{
 */

/* TIME_type: type of the argument to time(); we actually use &(time_t);
   you might need to define either or both of these to 'long *' in *conf.h */
#ifndef TIME_type
#define TIME_type time_t *
#endif
#ifndef LOCALTIME_type
#define LOCALTIME_type time_t *
#endif

/** @} */

/**
 * @brief Exchange two bits within a value.
 * @param val the value
 * @param bita one bit's position
 * @param bitb the other's
 * @return the value with those two bits exchanged
 * @note Done without a branch, by computing whether the two bits differ and flipping both if they do. When they are the same there is nothing to exchange and the computed difference is zero, so the same expression handles both cases.
 */
/**
 * @brief 값 안의 두 비트를 맞바꾼다.
 * @param val 그 값
 * @param bita 한 비트의 자리
 * @param bitb 다른 것의 자리
 * @return 그 두 비트가 맞바뀐 값
 * @note 가지 나눔 없이 이루어지며, 두 비트가 다른지를 계산하고 다르면 둘 다 뒤집는다. 그것들이 같을 때는 맞바꿀 것이 없고 계산된 차이가 영이므로, 같은 식이 두 경우 모두를 다룬다.
 */
int
swapbits(int val, int bita, int bitb)
{
    int tmp = ((val >> bita) & 1) ^ ((val >> bitb) & 1);

    return (val ^ ((tmp << bita) | (tmp << bitb)));
}

DISABLE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Format into a bounded buffer, guaranteeing termination.
 *
 * A wrapper rather than direct use of the standard routine, for the three reasons the existing comment sets out and each of them is a real one: the standard routine has been badly implemented on enough systems that having a single place to work
 * around it is worth the indirection; the checking that follows a formatting call is tedious enough that nobody would write it at every call site; and one compiler warns when the result is discarded and will not be quieted by casting it away.
 *
 * @param func the calling function's name, for a message that is not currently produced
 * @param line the calling line, likewise
 * @param str the buffer
 * @param size its size
 * @param fmt the format, followed by its arguments
 * @note Forces a terminator when the text did not fit, so the result is always a string. That is the guarantee the callers rely on, and it is what makes truncation survivable rather than fatal -- as the existing comment puts it, trouble with text
 *       need not end the game.
 * @warning Truncation is silent. The code that would report it is disabled, and the note there records what is missing: a way to reach the complaint machinery from a routine this low down, which cannot depend on the game. So the caller's name and
 *          line are accepted and unused, and a message quietly cut short gives no indication.
 */
/**
 * @brief 한정된 버퍼에 형식화하며, 종료를 보장한다.
 *
 * 표준 함수를 곧바로 쓰는 대신의 감싸기이며, 기존 주석이 밝히는 세 이유 때문이다. 그리고 그 각각이 진짜 이유다. 표준 함수가 충분히 많은 시스템에서 나쁘게 구현되어서 그것을 우회할 단 하나의 자리를 갖는 것이 그 우회를 들일 만하다. 형식화 호출 뒤에 오는 검사가 아무도 모든 호출 자리에 쓰지 않을 만큼 지루하다. 그리고 한 컴파일러가 결과가 버려질 때 경고하며 그것을 벗겨내는 것으로 잠잠해지지 않는다.
 *
 * @param func 호출하는 함수의 이름. 현재 내어지지 않는 메시지를 위해
 * @param line 호출하는 줄. 마찬가지로
 * @param str 그 버퍼
 * @param size 그 크기
 * @param fmt 그 형식. 뒤에 그 인자들이 따른다
 * @note 글이 들어가지 않았을 때 종료 문자를 강제하므로, 결과가 언제나 문자열이다. 그것이 호출자들이 의존하는 보장이며, 잘림을 치명적인 것이 아니라 견딜 수 있는 것으로 만드는 것이다. 기존 주석이 표현하듯 글을 다루는 데서의 곤란이 게임을 끝내야 하지 않는다.
 * @warning 잘림이 조용하다. 그것을 보고할 코드가 비활성화되어 있고, 거기의 적바림이 무엇이 없는지 기록한다. 게임에 의존할 수 없는 이렇게 낮은 곳의 함수에서 불평 기계에 닿을 방법. 그래서 호출자의 이름과 줄이 받아들여지고 쓰이지 않으며, 조용히 짧아진 메시지가 아무 표시도 주지 않는다.
 */
void
nh_snprintf(
    const char *func UNUSED, int line UNUSED,
    char *str, size_t size,
    const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(str, size, fmt, ap);
    va_end(ap);
    if (n < 0 || (size_t) n >= size) { /* is there a problem? */
#if 0
TODO: add set_impossible(), impossible -> func pointer,
 test funcpointer before call
        impossible("snprintf %s: func %s, file line %d",
                   (n < 0) ? "format error" : "overflow",
                   func, line);
#endif
        str[size - 1] = '\0'; /* make sure it is nul terminated */
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

/**
 * @brief Encode a Unicode character value as its byte sequence.
 *
 * Needed because the display may be told to draw a character by its Unicode value while the interface it writes through takes bytes. The encoding table is set out in the comment inside, which is worth reading rather than deriving.
 *
 * @param uval the character value
 * @param buffer where to write the bytes
 * @param bufsz its size
 * @return whether the value could be encoded
 * @note Refuses the range reserved for surrogate pairs, which are not characters in their own right -- a value there is a sign that something was decoded wrongly rather than a character to draw.
 * @note Refuses anything above the highest valid value, so an out-of-range number produces a refusal rather than a plausible-looking sequence of bytes.
 * @note Terminates the result, so it may be used as a string. It is not a string in the ordinary sense -- the bytes are not characters -- but every caller passes it to something expecting one.
 * @warning Requires room for five bytes even to begin, refusing a smaller buffer outright rather than encoding a short value into it. That is a deliberate simplification: the caller need not know how long the encoding will be.
 */
/**
 * @brief 유니코드 문자 값을 그 바이트 수열로 부호화한다.
 *
 * 화면이 문자를 유니코드 값으로 그리라고 들을 수 있는 반면 그것이 쓰는 인터페이스는 바이트를 취하기 때문에 필요하다. 그 부호화 표가 안의 주석에 밝혀져 있으며, 그것은 끌어내는 대신 읽을 만하다.
 *
 * @param uval 그 문자 값
 * @param buffer 바이트를 쓸 곳
 * @param bufsz 그 크기
 * @return 그 값이 부호화될 수 있었는지
 * @note 대체 짝을 위해 남겨진 범위를 거부한다. 그것들은 그 자체로 문자가 아니다. 거기의 값은 그릴 문자라기보다 무언가가 틀리게 해독되었다는 표시다.
 * @note 가장 높은 유효 값보다 위의 무엇이든 거부하므로, 범위를 벗어난 수가 그럴듯해 보이는 바이트 수열이 아니라 거부를 낸다.
 * @note 결과를 종료하므로 문자열로 쓰일 수 있다. 그것은 보통의 뜻에서 문자열이 아니다. 그 바이트가 문자가 아니다. 그러나 모든 호출자가 그것을 문자열을 기대하는 무엇에 넘긴다.
 * @warning 시작하기라도 하려면 다섯 바이트의 자리를 요구하며, 더 작은 버퍼는 그 안에 짧은 값을 부호화하는 대신 곧바로 거부한다. 그것은 의도된 단순화다. 호출자가 그 부호화가 얼마나 길지 알 필요가 없다.
 */
int
unicodeval_to_utf8str(int uval, uint8 *buffer, size_t bufsz)
{
    //    static uint8 buffer[7];
    uint8 *b = buffer;

    if (bufsz < 5)
        return 0;
    /*
     *   Binary   Hex        Comments
     *   0xxxxxxx 0x00..0x7F Only byte of a 1-byte character encoding
     *   10xxxxxx 0x80..0xBF Continuation byte : one of 1-3 bytes following
     * first 110xxxxx 0xC0..0xDF First byte of a 2-byte character encoding
     *   1110xxxx 0xE0..0xEF First byte of a 3-byte character encoding
     *   11110xxx 0xF0..0xF7 First byte of a 4-byte character encoding
     */
    *b = '\0';
    if (uval < 0x80) {
        *b++ = uval;
    } else if (uval < 0x800) {
        *b++ = 192 + uval / 64;
        *b++ = 128 + uval % 64;
    } else if (uval - 0xd800u < 0x800) {
        return 0;
    } else if (uval < 0x10000) {
        *b++ = 224 + uval / 4096;
        *b++ = 128 + uval / 64 % 64;
        *b++ = 128 + uval % 64;
    } else if (uval < 0x110000) {
        *b++ = 240 + uval / 262144;
        *b++ = 128 + uval / 4096 % 64;
        *b++ = 128 + uval / 64 % 64;
        *b++ = 128 + uval % 64;
    } else {
        return 0;
    }
    *b = '\0'; /* NUL terminate */
    return 1;
}

/**
 * @brief Compare two whole strings, ignoring case.
 * @param s1 first string
 * @param s2 second string
 * @return negative, zero or positive as the first sorts before, equal to, or after the second
 * @warning Uses the standard library's case routines rather than this file's own, so unlike everything else here its answer can depend on the player's locale. That is a real difference from the neighbouring comparison functions, which were written
 *          locally precisely to avoid it.
 * @note Unbounded, unlike the counted comparison above -- it runs to the end of the shorter string. Both must be terminated.
 */
/**
 * @brief 두 문자열 전체를 대소문자를 무시하고 비교한다.
 * @param s1 첫 문자열
 * @param s2 두 번째 문자열
 * @return 첫 것이 앞서, 같게, 뒤에 놓이는지에 따라 음수, 영, 양수
 * @warning 이 파일 자신의 것이 아니라 표준 라이브러리의 대소문자 함수를 쓰므로, 여기의 다른 모든 것과 달리 그 답이 플레이어의 로케일에 좌우될 수 있다. 그것은 바로 그것을 피하기 위해 국지적으로 쓰인 이웃 비교 함수들과의 진짜 차이다.
 * @note 위의 세는 비교와 달리 한계가 없다. 그것은 더 짧은 문자열의 끝까지 달린다. 둘 다 종료되어 있어야 한다.
 */
int
case_insensitive_comp(const char *s1, const char *s2)
{
    uchar u1, u2;

    for (;; s1++, s2++) {
        u1 = (uchar) *s1;
        if (isupper(u1))
            u1 = (uchar) tolower(u1);
        u2 = (uchar) *s2;
        if (isupper(u2))
            u2 = (uchar) tolower(u2);
        if (u1 == '\0' || u1 != u2)
            break;
    }
    return u1 - u2;
}

#if defined(MACOS)
#define RETTYPE ssize_t
#else
#define RETTYPE int
#endif

/**
 * @brief Copy the whole of one open file to another.
 * @param ifd the descriptor to read from
 * @param ofd the descriptor to write to
 * @return whether the copy completed
 * @note Stops when a read returns less than a full buffer, which is how the end of the file is recognised -- there is no separate end test.
 * @note Fails if a write is short as well as if a read errors, so a full disk is reported rather than producing a truncated copy that looks like a success.
 * @note The return type of the read and write calls differs between systems, which is why it is named by a local definition undefined immediately after. Comparing a count against a differently-signed type would make a failed read compare as a large
 *       positive count.
 */
/**
 * @brief 열린 파일 하나의 전체를 다른 것으로 복사한다.
 * @param ifd 읽을 서술자
 * @param ofd 쓸 서술자
 * @return 복사가 마쳐졌는지
 * @note 읽기가 온전한 버퍼보다 적게 돌려줄 때 멈추며, 그것이 파일의 끝을 알아보는 방법이다. 따로 된 끝 검사는 없다.
 * @note 읽기가 잘못될 때뿐 아니라 쓰기가 짧을 때도 실패하므로, 꽉 찬 디스크가 성공처럼 보이는 잘린 사본을 내는 대신 보고된다.
 * @note 읽기와 쓰기 호출의 반환 형이 시스템 사이에서 다르며, 그것이 그것이 바로 뒤에 정의 해제되는 국지적 정의로 이름 지어지는 이유다. 개수를 다르게 부호 지어진 형과 비교하는 것은 실패한 읽기가 큰 양수 개수로 비교되게 만들 것이다.
 */
boolean
copy_bytes(int ifd, int ofd)
{
    char buf[BUFSIZ];
    RETTYPE nfrom, nto;

    do {
        nto = 0;
        nfrom = read(ifd, buf, BUFSIZ);
        /* read can return -1 */
        if (nfrom >= 0 && nfrom <= BUFSIZ)
            nto = write(ofd, buf, nfrom);
        if (nto != nfrom || nfrom < 0)
            return FALSE;
    } while (nfrom == (RETTYPE) BUFSIZ);
    return TRUE;
}
#undef RETTYPE

#define MAX_D 5
struct datamodel_information {
    int sz[MAX_D];
    const char *datamodel;
    const char *dmplatform;
};

/**
 * @var dm
 * @brief Known combinations of integer and pointer widths, with the names they go by.
 *
 * Used to turn this build's integer sizes into something a person can act on. "Your save file is from a 64-bit Windows build" tells a player what to do; a list of five numbers does not.
 *
 * @note The first entry is not a known combination but this build's own sizes, filled in by the compiler from the actual types. That is what makes the table self-describing: the search compares every other entry against the first rather than against
 *       values written by hand.
 * @warning Because the first entry is the one being identified, it must stay first. A combination added before it would be compared against itself and reported as a match.
 * @note Each entry names a platform as well as a data model, because the model name is precise and the platform name is the one a player recognises.
 */
/**
 * @var dm
 * @brief 알려진 정수와 포인터 너비의 조합들, 그리고 그것들이 불리는 이름.
 *
 * 이 빌드의 정수 크기를 사람이 행동할 수 있는 무엇으로 바꾸는 데 쓰인다. "당신의 저장 파일은 64비트 윈도우 빌드에서 왔다"는 플레이어에게 무엇을 할지 알려주고, 다섯 숫자의 목록은 그러지 않는다.
 *
 * @note 첫 항목은 알려진 조합이 아니라 이 빌드 자신의 크기이며, 실제 형으로부터 컴파일러가 채운다. 그것이 이 표를 스스로를 서술하게 만드는 것이다. 찾기가 손으로 쓰인 값이 아니라 첫 항목과 견주어 다른 모든 항목을 비교한다.
 * @warning 첫 항목이 식별되고 있는 것이기 때문에, 그것이 첫째로 머물러야 한다. 그 앞에 더해진 조합은 자기 자신과 비교되어 맞음으로 보고될 것이다.
 * @note 각 항목이 데이터 모형뿐 아니라 플랫폼도 이름 짓는다. 모형 이름은 정확하고 플랫폼 이름은 플레이어가 알아보는 것이기 때문이다.
 */
static struct datamodel_information dm[] = {
    { { (int) sizeof(short), (int) sizeof(int), (int) sizeof(long),
        (int) sizeof(long long), (int) sizeof(genericptr_t) },
      "", "" },
    { { 2, 4, 4, 8, 4 }, "ILP32LL64", "x86 32-bit" }, /* Windows or Unix */
    { { 2, 4, 4, 8, 8 }, "IL32LLP64", "Windows x64 64-bit" },
    { { 2, 4, 8, 8, 8 }, "I32LP64", "Unix 64-bit"},
    { { 2, 8, 8, 8, 8 }, "ILP64", "Unix ILP64"},      /* HAL, SPARC64 */
};

/**
 * @brief Name the data model this build was compiled for.
 * @param retidx 0 for the model's technical name, anything else for the platform description
 * @return the name, or a placeholder if the combination is not in the table
 * @note Reads the sizes from the table's first entry rather than from the types directly, so this and the routine that identifies an arbitrary combination share one table and cannot disagree.
 * @note Reports the placeholder rather than guessing at the closest match. A wrong platform name would be worse than none, since it would be acted on.
 */
/**
 * @brief 이 빌드가 어느 데이터 모형으로 컴파일되었는지 이름 짓는다.
 * @param retidx 모형의 기술적 이름에 0, 플랫폼 서술에 그 밖의 무엇이든
 * @return 그 이름, 또는 그 조합이 표에 없으면 자리표
 * @note 형에서 곧바로가 아니라 표의 첫 항목에서 크기를 읽으므로, 이것과 임의의 조합을 식별하는 함수가 하나의 표를 나누고 어긋날 수 없다.
 * @note 가장 가까운 맞음을 짐작하는 대신 자리표를 보고한다. 틀린 플랫폼 이름은 없는 것보다 나쁠 것이다. 그것에 따라 행동될 것이기 때문이다.
 */
const char *
datamodel(int retidx)
{
    int i, j, matchcount;
    static const char *unknown = "Unknown";

    for (i = 1; i < SIZE(dm); ++i) {
        matchcount = 0;
        for (j = 0; j < MAX_D; ++j) {
            if (dm[0].sz[j] == dm[i].sz[j])
                ++matchcount;
        }
        if (matchcount == MAX_D)
            return (retidx == 0) ? dm[i].datamodel : dm[i].dmplatform;
    }
    return unknown;
}

/**
 * @brief Name the data model a given set of widths belongs to.
 *
 * The counterpart for widths that came from somewhere else -- a save file recording the sizes of the build that wrote it. That is what lets the compatibility check say which kind of build a rejected file came from rather than only that it was
 * incompatible.
 *
 * @param retidx 0 for the model's technical name, anything else for the platform description
 * @param szshort the short integer's size
 * @param szint the plain integer's size
 * @param szlong the long integer's size
 * @param szll the long long integer's size
 * @param szptr a pointer's size
 * @return the name, or a placeholder if the combination is not in the table
 * @note Starts at the second entry, skipping this build's own sizes. Comparing against those would report the local model for any file that happens to match it, which is true but not what was asked.
 */
/**
 * @brief 주어진 너비 묶음이 어느 데이터 모형에 속하는지 이름 짓는다.
 *
 * 다른 곳에서 온 너비를 위한 짝이다. 자기를 쓴 빌드의 크기를 기록한 저장 파일. 그것이 호환성 검사가 거부된 파일이 호환되지 않는다는 것만이 아니라 어떤 종류의 빌드에서 왔는지 말할 수 있게 하는 것이다.
 *
 * @param retidx 모형의 기술적 이름에 0, 플랫폼 서술에 그 밖의 무엇이든
 * @param szshort 짧은 정수의 크기
 * @param szint 맨 정수의 크기
 * @param szlong 긴 정수의 크기
 * @param szll 아주 긴 정수의 크기
 * @param szptr 포인터의 크기
 * @return 그 이름, 또는 그 조합이 표에 없으면 자리표
 * @note 이 빌드 자신의 크기를 건너뛰고 두 번째 항목에서 시작한다. 그것과 견주어 비교하는 것은 마침 그것과 맞는 어떤 파일에든 국지적 모형을 보고할 것이고, 그것은 참이지만 물어진 것이 아니다.
 */
const char *
what_datamodel_is_this(int retidx, int szshort, int szint, int szlong, int szll,
                       int szptr)
{
    int i;
    static const char *unknown = "Unknown";

    for (i = 1; i < SIZE(dm); ++i) {
        if (szshort == dm[i].sz[0] && szint == dm[i].sz[1]
            && szlong == dm[i].sz[2] && szll == dm[i].sz[3]
            && szptr == dm[i].sz[4])
            return (retidx == 0) ? dm[i].datamodel : dm[i].dmplatform;
    }
    return unknown;
}
#undef MAX_D
/*hacklib.c*/
