/* NetHack 5.0	cfgfiles.c	$NHDT-Date: 1781973042 2026/06/20 16:30:42 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.23 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Derek S. Ray, 2015. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file cfgfiles.c
 * @brief Reading the configuration files, and reporting what is wrong with them.
 *
 * A configuration file is text a person wrote, which makes this different from every other file the game reads. It will contain mistakes -- a misspelled option, a value out of range, a line that means
 * nothing -- and the game has to keep going and say what it found rather than refusing to start.
 *
 * That obligation shapes the file. Roughly half of it is the parser and roughly half is error reporting, and the reporting is the more careful part: errors are collected rather than printed, because
 * they arrive before the display exists and because a player would rather see all of them at once than one at a time.
 *
 * Three things beyond plain parsing are worth knowing about.
 *
 * A file may be divided into sections named for a platform or a display, so one file can serve several installations. Only the matching sections are obeyed, and a section for something else is skipped
 * without complaint -- so an unrecognised section name is not an error.
 *
 * A value may offer several alternatives separated by a character, one of which is chosen at random. That is why some settings differ between runs from the same file.
 *
 * And the same parsing serves more than files. It reads the system-wide configuration, the player's own, an environment variable and the command line, and which of those a line came from decides whether
 * it is permitted -- so the parser is told its source and refuses settings that source may not make.
 *
 * @note There is no undo. Parsing applies each line as it is read, so a file whose later lines are rejected has still applied its earlier ones.
 * @warning Where the game is a shared installation, part of what is read here is written by an administrator and part by a player. The distinction is enforced by the source, and a setting reaching the
 *          wrong side of it is a security matter rather than a cosmetic one.
 */

/**
 * @file cfgfiles.c
 * @brief 설정 파일을 읽고, 그것에 무엇이 잘못되었는지 알리기.
 *
 * 설정 파일은 사람이 쓴 글이며, 그것이 이 파일을 게임이 읽는 다른 모든 파일과 다르게 만든다. 그 안에는 잘못이 있을 것이다. 철자가 틀린 옵션, 범위를 벗어난 값, 아무 뜻도 없는 줄. 그리고 게임은 시작하기를 거부하는 대신 계속 나아가며 자신이 발견한 것을 말해야 한다.
 *
 * 그 의무가 이 파일의 모양을 만든다. 대략 절반이 파서이고 대략 절반이 오류 보고이며, 그 보고가 더 조심스러운 부분이다. 오류는 인쇄되는 대신 모아진다. 그것이 표시부가 존재하기 전에 도착하기 때문이고, 플레이어가 하나씩 보기보다 한꺼번에 전부 보고 싶어 하기 때문이다.
 *
 * 평범한 파싱 말고 알아 둘 만한 세 가지가 있다.
 *
 * 파일은 플랫폼이나 표시부의 이름을 딴 절로 나뉘어 있을 수 있으며, 그래서 하나의 파일이 여러 설치본을 맡을 수 있다. 일치하는 절만이 따라지고, 다른 것을 위한 절은 불평 없이 건너뛰어진다. 그래서 알 수 없는 절 이름은 오류가 아니다.
 *
 * 값은 어떤 문자로 구분된 여러 대안을 제시할 수 있고, 그 중 하나가 무작위로 골라진다. 그것이 같은 파일에서도 어떤 설정이 실행마다 달라지는 이유다.
 *
 * 그리고 같은 파싱이 파일 이상을 맡는다. 시스템 전체 설정, 플레이어 자신의 것, 환경 변수, 명령줄을 읽으며, 어떤 줄이 그 중 어디서 왔는지가 그것이 허용되는지를 정한다. 그래서 파서는 자기 출처를 듣고 그 출처가 해서는 안 되는 설정을 거부한다.
 *
 * @note 되돌리기는 없다. 파싱은 읽히는 각 줄을 그때 적용하므로, 뒤쪽 줄이 거부된 파일도 앞쪽 줄은 이미 적용한 것이다.
 * @warning 게임이 공유 설치본인 곳에서는 여기서 읽히는 것의 일부가 관리자가 쓴 것이고 일부가 플레이어가 쓴 것이다. 그 구별은 출처로 강제되며, 설정이 그 경계의 잘못된 쪽에 닿는 것은 미관의 문제가 아니라 보안의 문제다.
 */

#define NEED_VARARGS

#include "hack.h"
#include "dlb.h"
#include <errno.h>

#if (!defined(MAC68K) && !defined(O_WRONLY) && !defined(AZTEC_C)) \
    || defined(USE_FCNTL)
#include <fcntl.h>
#endif

#define BIGBUFSZ (5 * BUFSZ) /* big enough to format a 4*BUFSZ string (from
                              * config file parsing) with modest decoration;
                              * result will then be truncated to BUFSZ-1 */

#ifdef USER_SOUNDS
extern char *sounddir; /* defined in sounds.c */
#endif

staticfn void vconfig_error_add(const char *, va_list);
staticfn FILE *fopen_config_file(const char *, int);
staticfn int get_uchars(char *, uchar *, boolean, int, const char *);
#ifdef NOCWD_ASSUMPTIONS
staticfn void adjust_prefix(char *, int);
#endif
staticfn char *choose_random_part(char *, char);
staticfn boolean config_error_nextline(const char *);
staticfn void free_config_sections(void);
staticfn char *is_config_section(char *);
staticfn boolean handle_config_section(char *);
boolean parse_config_line(char *);
staticfn char *find_optparam(char *);
#ifdef WIN32
staticfn boolean portable_sysconf_only_this_statement(int);
#endif
#ifndef SFCTOOL
staticfn boolean cnf_line_OPTIONS(char *);
staticfn boolean cnf_line_AUTOPICKUP_EXCEPTION(char *);
staticfn boolean cnf_line_BINDINGS(char *);
staticfn boolean cnf_line_AUTOCOMPLETE(char *);
staticfn boolean cnf_line_MSGTYPE(char *);
staticfn boolean cnf_line_HACKDIR(char *);
staticfn boolean cnf_line_LEVELDIR(char *);
staticfn boolean cnf_line_SAVEDIR(char *);
staticfn boolean cnf_line_BONESDIR(char *);
staticfn boolean cnf_line_DATADIR(char *);
staticfn boolean cnf_line_SCOREDIR(char *);
staticfn boolean cnf_line_LOCKDIR(char *);
staticfn boolean cnf_line_CONFIGDIR(char *);
staticfn boolean cnf_line_TROUBLEDIR(char *);
staticfn boolean cnf_line_NAME(char *);
staticfn boolean cnf_line_ROLE(char *);
staticfn boolean cnf_line_dogname(char *);
staticfn boolean cnf_line_catname(char *);
#endif /* SFCTOOL */
#ifdef SYSCF
staticfn boolean cnf_line_WIZARDS(char *);
staticfn boolean cnf_line_SHELLERS(char *);
staticfn boolean cnf_line_MSGHANDLER(char *);
staticfn boolean cnf_line_EXPLORERS(char *);
staticfn boolean cnf_line_DEBUGFILES(char *);
staticfn boolean cnf_line_DUMPLOGFILE(char *);
staticfn boolean cnf_line_GENERICUSERS(char *);
staticfn boolean cnf_line_BONES_POOLS(char *);
staticfn boolean cnf_line_SUPPORT(char *);
staticfn boolean cnf_line_RECOVER(char *);
staticfn boolean cnf_line_CHECK_SAVE_UID(char *);
staticfn boolean cnf_line_CHECK_PLNAME(char *);
staticfn boolean cnf_line_SEDUCE(char *);
staticfn boolean cnf_line_HIDEUSAGE(char *);
staticfn boolean cnf_line_MAXPLAYERS(char *);
staticfn boolean cnf_line_MAX_REROLL_RATE(char *);
staticfn boolean cnf_line_PERSMAX(char *);
staticfn boolean cnf_line_PERS_IS_UID(char *);
staticfn boolean cnf_line_ENTRYMAX(char *);
staticfn boolean cnf_line_POINTSMIN(char *);
staticfn boolean cnf_line_MAX_STATUENAME_RANK(char *);
staticfn boolean cnf_line_LIVELOG(char *);
staticfn boolean cnf_line_PANICTRACE_LIBC(char *);
staticfn boolean cnf_line_PANICTRACE_GDB(char *);
staticfn boolean cnf_line_GDBPATH(char *);
staticfn boolean cnf_line_GREPPATH(char *);
staticfn boolean cnf_line_CRASHREPORTURL(char *);
staticfn boolean cnf_line_ACCESSIBILITY(char *);

staticfn boolean cnf_line_PORTABLE_DEVICE_PATHS(char *);
#endif /* SYSCF */
#ifndef SFCTOOL
staticfn boolean cnf_line_BOULDER(char *);
staticfn boolean cnf_line_MENUCOLOR(char *);
staticfn boolean cnf_line_HILITE_STATUS(char *);
staticfn boolean cnf_line_WARNINGS(char *);
staticfn boolean cnf_line_ROGUESYMBOLS(char *);
staticfn boolean cnf_line_SYMBOLS(char *);
staticfn boolean cnf_line_WIZKIT(char *);
#ifdef USER_SOUNDS
staticfn boolean cnf_line_SOUNDDIR(char *);
staticfn boolean cnf_line_SOUND(char *);
#endif
staticfn boolean cnf_line_QT_TILEWIDTH(char *);
staticfn boolean cnf_line_QT_TILEHEIGHT(char *);
staticfn boolean cnf_line_QT_FONTSIZE(char *);
staticfn boolean cnf_line_QT_COMPACT(char *);
#endif /* SFCTOOL */
struct _cnf_parser_state; /* defined below (far below...) */
staticfn void cnf_parser_init(struct _cnf_parser_state *parser);
staticfn void cnf_parser_done(struct _cnf_parser_state *parser);
staticfn void parse_conf_buf(struct _cnf_parser_state *parser,
                           boolean (*proc)(char *arg));
    /* next one is in extern.h; why here too? */
boolean parse_conf_str(const char *str, boolean (*proc)(char *arg));
static boolean ignore_errors_on_unmatched = FALSE,
               ignore_statement_errors = FALSE;

#ifdef SFCTOOL
#ifdef wait_synch
#undef wait_synch
#endif
#define wait_synch()
#endif /* SFCTOOL */

/* ----------  BEGIN CONFIG FILE HANDLING ----------- */

/* used for messaging. Also used in options.c */
static const char *default_configfile =
#ifdef UNIX
    ".nethackrc";
#else
#if defined(MAC68K) || defined(__BEOS__)
    "NetHack Defaults";
#else
#if defined(MSDOS) || defined(WIN32)
    CONFIG_FILE;
#else
    "NetHack.cnf";
#endif
#endif
#endif
static char configfile[BUFSZ];

/**
 * @brief The configuration file actually being used, whichever it turned out to be.
 * @return the remembered name, which is empty until a file has been opened
 * @note The name is remembered rather than recomputed because it appears in error messages, and an error about a rejected line has to name the file the line came from -- which may not be the default one.
 */
/**
 * @brief 실제로 쓰이고 있는 설정 파일. 그것이 무엇으로 정해졌든.
 * @return 기억된 이름. 파일이 열리기 전까지는 비어 있다
 * @note 이름이 다시 계산되지 않고 기억되는 것은 그것이 오류 메시지에 나타나기 때문이다. 거부된 줄에 대한 오류는 그 줄이 온 파일의 이름을 밝혀야 하고, 그것이 기본 파일이 아닐 수 있다.
 */
char *
get_configfile(void)
{
    return configfile;
}

/**
 * @brief The name a configuration file would have if nobody said otherwise.
 * @note Differs by platform, which is why it is asked for rather than written into the code that needs it.
 */
/**
 * @brief 아무도 달리 말하지 않았다면 설정 파일이 가질 이름.
 * @note 플랫폼마다 다르며, 그래서 그것을 필요로 하는 코드에 적혀 있는 대신 물어진다.
 */
const char *
get_default_configfile(void)
{
    return default_configfile;
}

#ifdef MSDOS
/* conflict with speed-dial under windows
 * for XXX.cnf file so support of NetHack.cnf
 * is for backward compatibility only.
 * Preferred name (and first tried) is now defaults.nh but
 * the game will try the old name if there
 * is no defaults.nh.
 */
const char *backward_compat_configfile = "nethack.cnf";
#endif

#ifndef SFCTOOL

/**
 * @brief Write the player's current settings back out as a configuration file.
 *
 * The reverse of everything else here, and the file is honest about how well it works: it warns the player that not every setting is saved and that the file's comments and hand-written arrangement will be
 * lost. That is why it asks before overwriting rather than simply doing it.
 *
 * @return the command result, which is the same whether it wrote or the player declined
 * @warning Destroys the existing file's structure. What a player wrote by hand -- their comments, their ordering, their sections -- does not survive, because what is written is generated from the settings
 *          rather than edited into the old text.
 * @note A partial write is reported rather than being silently accepted, because a truncated configuration file is worse than none: it would load and be wrong.
 */
/**
 * @brief 플레이어의 현재 설정을 설정 파일로 다시 써 낸다.
 *
 * 여기의 다른 모든 것의 역이며, 이 파일은 그것이 얼마나 잘 작동하는지에 대해 정직하다. 모든 설정이 저장되지는 않으며 그 파일의 주석과 손으로 정돈한 배치가 사라질 것이라고 플레이어에게 경고한다. 그것이 그냥 덮어쓰는 대신 먼저 묻는 이유다.
 *
 * @return 명령 결과. 썼든 플레이어가 물렸든 같다
 * @warning 기존 파일의 구조를 파괴한다. 플레이어가 손으로 쓴 것 -- 주석, 순서, 절 -- 은 살아남지 못한다. 기록되는 것이 예전 글을 편집한 것이 아니라 설정에서 생성된 것이기 때문이다.
 * @note 부분적인 기록은 조용히 받아들여지는 대신 알려진다. 잘린 설정 파일은 없는 것보다 나쁘기 때문이다. 그것은 읽히고 틀리다.
 */
/* #saveoptions - save config options into file */
int
do_write_config_file(void)
{
    FILE *fp;
    char tmp[BUFSZ];

    if (!configfile[0]) {
        pline("Strange, could not figure out config file name.");
        return ECMD_OK;
    }
    if (flags.suppress_alert < FEATURE_NOTICE_VER(3,7,0)) {
        pline("Warning: saveoptions is highly experimental!");
        wait_synch();
        pline("Some settings are not saved!");
        wait_synch();
        pline("All manual customization and comments are removed"
              " from the file!");
        wait_synch();
    }
#define overwrite_prompt "Overwrite config file %.*s?"
    Sprintf(tmp, overwrite_prompt,
            (int) (BUFSZ - sizeof overwrite_prompt - 2), configfile);
#undef overwrite_prompt
    if (!paranoid_query(TRUE, tmp))
        return ECMD_OK;

    fp = fopen(configfile, "w");
    if (fp) {
        size_t len, wrote;
        strbuf_t buf;

        strbuf_init(&buf);
        all_options_strbuf(&buf);
        len = strlen(buf.str);
        wrote = fwrite(buf.str, 1, len, fp);
        fclose(fp);
        strbuf_empty(&buf);
        if (wrote != len)
            pline("An error occurred, wrote only partial data (%zu/%zu).",
                  wrote, len);
    }
    return ECMD_OK;
}
#endif /* SFCTOOL */

/**
 * @brief Remember which file is being read, for the sake of error messages.
 * @note Called by whatever opens a file rather than by the parser, because the parser is given lines and does not know where they came from -- and an error about a line has to say which file.
 * @warning Truncates silently if the name is too long. That is acceptable here because the name is only ever shown, never used to open anything.
 */
/**
 * @brief 오류 메시지를 위해 어느 파일이 읽히고 있는지 기억한다.
 * @note 파서가 아니라 파일을 여는 쪽이 호출한다. 파서는 줄을 받으며 그것이 어디서 왔는지 모르기 때문이다. 그리고 어떤 줄에 대한 오류는 어느 파일인지 말해야 한다.
 * @warning 이름이 너무 길면 조용히 잘라 낸다. 그 이름이 오직 보여지기만 하고 무엇을 여는 데 쓰이지는 않으므로 여기서는 받아들일 만하다.
 */
/* remember the name of the file we're accessing;
   if may be used in option reject messages */
void
set_configfile_name(const char *fname)
{
    (void) strncpy(configfile, fname, sizeof configfile - 1);
    configfile[sizeof configfile - 1] = '\0';
}

/**
 * @brief Open a configuration file, looking wherever that kind of file is kept.
 *
 * Where to look depends entirely on which kind is being opened, which is why the source is an argument rather than the caller supplying a path. The system-wide file lives where an administrator put it; the
 * player's own may be named by an environment variable, may be in their home directory, may be beside the game.
 *
 * @param filename the name to look for, which may be empty to mean the default
 * @param src which kind of configuration this is
 * @return the open file, or null if none was found
 * @note Not finding a file is usually not an error. A player with no configuration file is playing with the defaults, and only the system-wide file's absence is worth complaining about -- so the caller
 *       decides, and this only reports what happened.
 */
/**
 * @brief 설정 파일을 연다. 그 종류의 파일이 보관되는 곳이 어디든 찾아서.
 *
 * 어디를 볼지는 어느 종류가 열리고 있는지에 전적으로 달려 있으며, 그래서 호출자가 경로를 제공하는 대신 출처가 인자다. 시스템 전체 파일은 관리자가 둔 곳에 산다. 플레이어 자신의 것은 환경 변수로 지칭될 수도, 홈 디렉토리에 있을 수도, 게임 곁에 있을 수도 있다.
 *
 * @param filename 찾을 이름. 기본값을 뜻하도록 비어 있을 수 있다
 * @param src 이것이 어느 종류의 설정인지
 * @return 열린 파일. 아무것도 발견되지 않았으면 널
 * @note 파일을 찾지 못하는 것은 보통 오류가 아니다. 설정 파일이 없는 플레이어는 기본값으로 플레이하는 것이며, 시스템 전체 파일의 부재만이 불평할 가치가 있다. 그래서 호출자가 정하고, 이것은 무슨 일이 있었는지만 알린다.
 */
staticfn FILE *
fopen_config_file(const char *filename, int src)
{
    FILE *fp;
#if defined(UNIX) || defined(VMS)
    char tmp_config[BUFSZ];
    char *envp;
#endif

    if (src == set_in_sysconf) {
        /* SYSCF_FILE; if we can't open it, caller will bail */
        if (filename && *filename) {
            set_configfile_name(fqname(filename, SYSCONFPREFIX, 0));
            fp = fopen(configfile, "r");
        } else
            fp = (FILE *) 0;
        return  fp;
    }
    /* If src != set_in_sysconf, "filename" is an environment variable, so it
     * should hang around. If set, it is expected to be a full path name
     * (if relevant)
     */
    if (filename && *filename) {
        set_configfile_name(filename);
#ifdef UNIX
        if (!strncmp(configfile, "~/", 2) && (envp = nh_getenv("HOME")) != 0) {
            /* support for command line '--nethackrc=~/path' (or for
               NETHACKOPTIONS='@~/path'; we don't support ~user/path) */
            Snprintf(tmp_config, sizeof tmp_config, "%s/%s",
                     envp, configfile + 2); /* insert $HOME/ and remove ~/ */
            set_configfile_name(tmp_config);
        }
        if (access(configfile, 4) == -1) { /* 4 is R_OK on newer systems */
            /* nasty sneaky attempt to read file through
             * NetHack's setuid permissions -- this is the only
             * place a file name may be wholly under the player's
             * control (but SYSCF_FILE is not under the player's
             * control so it's OK).
             */
            raw_printf("Access to %s denied (%d).", configfile, errno);
            wait_synch();
            /* fall through to standard names */
        } else
#endif
        if ((fp = fopen(configfile, "r")) != (FILE *) 0) {
            return  fp;
#if defined(UNIX) || defined(VMS)
        } else {
            /* access() above probably caught most problems for UNIX */
            raw_printf("Couldn't open requested config file %s (%d).",
                       configfile, errno);
            wait_synch();
#endif
        }
    }
    /* fall through to standard names */

#if defined(MICRO) || defined(MAC68K) || defined(__BEOS__) || defined(WIN32)
    set_configfile_name(fqname(default_configfile, CONFIGPREFIX, 0));
    if ((fp = fopen(configfile, "r")) != (FILE *) 0) {
        return fp;
    } else if (strcmp(default_configfile, configfile)) {
        set_configfile_name(default_configfile);
        if ((fp = fopen(configfile, "r")) != (FILE *) 0)
            return fp;
    }
#ifdef MSDOS
    set_configfile_name(fqname(backward_compat_configfile, CONFIGPREFIX, 0));
    if ((fp = fopen(configfile, "r")) != (FILE *) 0) {
        return fp;
    } else if (strcmp(backward_compat_configfile, configfile)) {
        set_configfile_name(backward_compat_configfile);
        if ((fp = fopen(configfile, "r")) != (FILE *) 0)
            return fp;
    }
#endif
#else
/* constructed full path names don't need fqname() */
#ifdef VMS
    /* no punctuation, so might be a logical name */
    set_configfile_name("nethackini");
    if ((fp = fopen(configfile, "r")) != (FILE *) 0)
        return fp;
    set_configfile_name("sys$login:nethack.ini");
    if ((fp = fopen(configfile, "r")) != (FILE *) 0)
        return fp;

    envp = nh_getenv("HOME");
    if (!envp || !*envp)
        Strcpy(tmp_config, "NetHack.cnf");
    else
        Sprintf(tmp_config, "%s%s%s", envp,
                !strchr(":]>/", envp[strlen(envp) - 1]) ? "/" : "",
                "NetHack.cnf");
    set_configfile_name(tmp_config);
    if ((fp = fopen(configfile, "r")) != (FILE *) 0)
        return fp;
#else /* should be only UNIX left */
    envp = nh_getenv("HOME");
    if (!envp)
        Strcpy(tmp_config, ".nethackrc");
    else
        Sprintf(tmp_config, "%s/%s", envp, ".nethackrc");

    set_configfile_name(tmp_config);
    if ((fp = fopen(configfile, "r")) != (FILE *) 0)
        return fp;
#if defined(__APPLE__) /* UNIX+__APPLE__ => OSX || MacOS */
    /* try an alternative */
    if (envp) {
        /* keep 'tmp_config' intact here; if alternates fail, use it to
           restore configfile[] to its preferred setting (".nethackrc") */
        char alt_config[sizeof tmp_config];

        /* OSX-style configuration settings */
        Snprintf(alt_config, sizeof alt_config, "%s/%s", envp,
                 "Library/Preferences/NetHack Defaults");
        set_configfile_name(alt_config);
        if ((fp = fopen(configfile, "r")) != (FILE *) 0)
            return fp;
        /* may be easier for user to edit if filename has '.txt' suffix */
        Snprintf(alt_config, sizeof alt_config, "%s/%s", envp,
                 "Library/Preferences/NetHack Defaults.txt");
        set_configfile_name(alt_config);
        if ((fp = fopen(configfile, "r")) != (FILE *) 0)
            return fp;
        /* couldn't open either of the alternate names; for use in
           messages, put 'configfile' back to the normal value rather than
           leaving it set to last alternate; retry open() to reset 'errno' */
        set_configfile_name(tmp_config);
        if ((fp = fopen(configfile, "r")) != (FILE *) 0)
            return fp;
    }
#endif /*__APPLE__*/
    if (errno != ENOENT) {
        const char *details;

        /* e.g., problems when setuid NetHack can't search home
           directory restricted to user */
#if defined(NHSTDC) && !defined(NOTSTDC)
        if ((details = strerror(errno)) == 0)
#endif
            details = "";
        raw_printf("Couldn't open default config file %s %s(%d).",
                   configfile, details, errno);
        wait_synch();
    }
#endif /* !VMS => Unix */
#endif /* !(MICRO || MAC68K || __BEOS__ || WIN32) */
    return (FILE *) 0;
}

/**
 * @brief Read a list of small numbers written as text into an array.
 *
 * Used for settings whose value is a sequence rather than a single thing -- a set of symbol codes, a list of colours. It accepts the several notations a person might reasonably write, so a player is not
 * required to know which the parser prefers.
 *
 * @param bufp the text to read, advanced as it goes
 * @param list where to put the values
 * @param modlist whether the list already holds values that are being altered
 * @param size how many values there is room for
 * @param name the setting's name, for an error message
 * @return how many values were read
 * @warning The modifying mode changes what happens to a position whose value is zero. Modifying in place leaves such a position as it was, so a zero means "leave this one alone"; otherwise a zero is
 *          written. The existing note states that a caller not modifying in place must handle those zeros itself.
 * @note So the same call means two different things depending on that flag, and a caller that has the flag wrong gets a list that is plausible and wrong rather than an error.
 */
/**
 * @brief 글로 적힌 작은 숫자의 목록을 배열로 읽어 들인다.
 *
 * 값이 하나가 아니라 나열인 설정에 쓰인다. 심볼 코드의 묶음, 색의 목록. 사람이 합당하게 적을 만한 여러 표기를 받아들이므로, 플레이어가 파서가 어느 것을 선호하는지 알아야 하지는 않다.
 *
 * @param bufp 읽을 글. 진행하며 앞으로 나아간다
 * @param list 값을 넣을 곳
 * @param modlist 그 목록이 이미 바뀌고 있는 값을 담고 있는지
 * @param size 값이 들어갈 자리가 몇 개인지
 * @param name 그 설정의 이름. 오류 메시지를 위한 것
 * @return 값이 몇 개 읽혔는지
 * @warning 수정 모드는 값이 0인 자리에 무슨 일이 일어나는지를 바꾼다. 제자리에서 수정하는 것은 그런 자리를 그대로 두므로, 0은 "이것은 건드리지 말라"를 뜻한다. 그렇지 않으면 0이 기록된다. 기존 메모는 제자리에서 수정하지 않는 호출자가 그 0들을 스스로 처리해야 한다고
 *          밝히고 있다.
 * @note 그래서 같은 호출이 그 플래그에 따라 서로 다른 두 가지를 뜻하며, 그 플래그를 틀린 호출자는 오류가 아니라 그럴듯하고 틀린 목록을 받는다.
 */
/*
 * Retrieve a list of integers from buf into a uchar array.
 *
 * NOTE: zeros are inserted unless modlist is TRUE, in which case the list
 *  location is unchanged.  Callers must handle zeros if modlist is FALSE.
 */
staticfn int
get_uchars(char *bufp,       /* current pointer */
           uchar *list,      /* return list */
           boolean modlist,  /* TRUE: list is being modified in place */
           int size,         /* return list size */
           const char *name) /* name of option for error message */
{
    unsigned int num = 0;
    int count = 0;
    boolean havenum = FALSE;

    while (1) {
        switch (*bufp) {
        case ' ':
        case '\0':
        case '\t':
        case '\n':
            if (havenum) {
                /* if modifying in place, don't insert zeros */
                if (num || !modlist)
                    list[count] = num;
                count++;
                num = 0;
                havenum = FALSE;
            }
            if (count == size || !*bufp)
                return count;
            bufp++;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            havenum = TRUE;
            num = num * 10 + (*bufp - '0');
            bufp++;
            break;

        case '\\':
            goto gi_error;
            break;

        default:
 gi_error:
            raw_printf("Syntax error in %s", name);
            wait_synch();
            return count;
        }
    }
    /*NOTREACHED*/
}

/**
 * @brief Set one of the directories the game looks in, from a configuration line.
 *
 * @param bufp the directory as the player wrote it
 * @param prefixid which directory this is
 * @warning On one platform a directory may be locked, and this silently declines rather than reporting a refusal. That is deliberate: the lock exists because the installation decided the directory, and a
 *          player's attempt to change it is not a mistake worth complaining about.
 * @note Trailing text after a semicolon is discarded for compatibility with an older format, so a line written for an earlier version still works rather than being rejected.
 * @note A separator is appended if the player omitted one, because the rest of the game joins a directory to a filename without checking.
 */
/**
 * @brief 설정 줄에서, 게임이 들여다보는 디렉토리 중 하나를 설정한다.
 *
 * @param bufp 플레이어가 적은 대로의 디렉토리
 * @param prefixid 이것이 어느 디렉토리인지
 * @warning 한 플랫폼에서는 디렉토리가 잠겨 있을 수 있고, 이것은 거부를 알리는 대신 조용히 물러난다. 의도적이다. 그 잠금은 설치본이 그 디렉토리를 정했기 때문에 존재하며, 플레이어가 그것을 바꾸려는 시도는 불평할 가치가 있는 잘못이 아니다.
 * @note 세미콜론 뒤의 글은 더 예전 형식과의 호환을 위해 버려진다. 그래서 이전 판본을 위해 쓰인 줄이 거부되는 대신 여전히 작동한다.
 * @note 플레이어가 구분자를 빠뜨렸으면 그것이 덧붙여진다. 게임의 나머지가 확인 없이 디렉토리와 파일 이름을 잇기 때문이다.
 */
#ifdef NOCWD_ASSUMPTIONS
staticfn void
adjust_prefix(char *bufp, int prefixid)
{
    char *ptr;

    if (!bufp)
        return;
#ifdef WIN32
    if (fqn_prefix_locked[prefixid])
        return;
#endif
    /* Backward compatibility, ignore trailing ;n */
    if ((ptr = strchr(bufp, ';')) != 0)
        *ptr = '\0';
    if (strlen(bufp) > 0) {
        gf.fqn_prefix[prefixid] = (char *) alloc(strlen(bufp) + 2);
        Strcpy(gf.fqn_prefix[prefixid], bufp);
        append_slash(gf.fqn_prefix[prefixid]);
    }
}
#endif

/**
 * @brief Pick one of several alternatives a setting's value offered.
 *
 * A player may write several values separated by a character and have one chosen at random. That is what makes a configuration file able to vary a game -- a randomly chosen character name, a randomly chosen
 * role -- without the player editing it between games.
 *
 * @param str the alternatives, which this modifies
 * @param sep the character separating them
 * @return the chosen alternative, pointing into the modified string
 * @warning Mangles the string it is given, as the existing comment says: it terminates the chosen part in place. The original is not recoverable, and the returned pointer is into the caller's own buffer.
 * @note In the tool built from these sources the choice is not random, since that tool converts files and must be repeatable. So the same input gives the same output there and not here.
 */
/**
 * @brief 설정의 값이 제시한 여러 대안 중 하나를 고른다.
 *
 * 플레이어는 어떤 문자로 구분된 여러 값을 적고 그 중 하나가 무작위로 골라지게 할 수 있다. 그것이 설정 파일이 게임을 다양하게 만들 수 있게 하는 것이다. 무작위로 골라진 캐릭터 이름, 무작위로 골라진 직업. 플레이어가 게임 사이에 그것을 편집하지 않고도.
 *
 * @param str 그 대안들. 이것이 그것을 바꾼다
 * @param sep 그것들을 구분하는 문자
 * @return 골라진 대안. 바뀐 문자열 안을 가리킨다
 * @warning 기존 주석이 말하듯 건네진 문자열을 망가뜨린다. 골라진 부분을 제자리에서 종결시킨다. 원래 것은 되찾을 수 없고, 반환된 포인터는 호출자 자신의 버퍼 안이다.
 * @note 이 소스로 빌드되는 도구에서는 그 선택이 무작위가 아니다. 그 도구는 파일을 변환하며 되풀이 가능해야 하기 때문이다. 그래서 같은 입력이 거기서는 같은 출력을 주고 여기서는 그렇지 않다.
 */
/* Choose at random one of the sep separated parts from str. Mangles str. */
staticfn char *
choose_random_part(char *str, char sep)
{
    int nsep = 1;
    int csep;
    int len = 0;
    char *begin = str;

    if (!str)
        return (char *) 0;

    while (*str) {
        if (*str == sep)
            nsep++;
        str++;
    }
#ifndef SFCTOOL
    csep = rn2(nsep);
#else
    nhUse(nsep);
    csep = 1;
#endif
    str = begin;
    while ((csep > 0) && *str) {
        str++;
        if (*str == sep)
            csep--;
    }
    if (*str) {
        if (*str == sep)
            str++;
        begin = str;
        while (*str && *str != sep) {
            str++;
            len++;
        }
        *str = '\0';
        if (len)
            return begin;
    }
    return (char *) 0;
}

/**
 * @brief Release the remembered section names after a file has been read.
 * @note Two names are held: the section the file asked for and the one currently being read. Both are freed together because they are only meaningful while a file is open.
 */
/**
 * @brief 파일이 읽힌 뒤 기억된 절 이름들을 놓아준다.
 * @note 두 이름이 보관된다. 파일이 요청한 절과 지금 읽히고 있는 절. 그것들이 파일이 열려 있는 동안만 의미가 있으므로 함께 해제된다.
 */
staticfn void
free_config_sections(void)
{
    if (gc.config_section_chosen) {
        free(gc.config_section_chosen);
        gc.config_section_chosen = NULL;
    }
    if (gc.config_section_current) {
        free(gc.config_section_current);
        gc.config_section_current = NULL;
    }
}

/**
 * @brief Whether a line is a section heading, and if so what the section is called.
 *
 * @param str the line, which this modifies if the answer is yes
 * @return the section's name, or null if the line is not a heading
 * @note Lenient about spacing on purpose, as the existing comment's pattern shows, and it accepts a comment after the heading. A configuration file is written by a person, so the parser accommodates how a
 *       person would write rather than requiring one exact form.
 * @warning Modifies the line when it succeeds -- it terminates the name in place, stripping the closing bracket and anything after it. So the caller's buffer is altered by asking the question.
 */
/**
 * @brief 어떤 줄이 절 제목인지, 그렇다면 그 절이 무엇이라 불리는지.
 *
 * @param str 그 줄. 답이 그렇다면 이것이 그것을 바꾼다
 * @return 그 절의 이름. 그 줄이 제목이 아니면 널
 * @note 기존 주석의 패턴이 보여 주듯 공백에 대해 일부러 관대하며, 제목 뒤의 주석도 받아들인다. 설정 파일은 사람이 쓰므로, 파서가 하나의 정확한 형태를 요구하는 대신 사람이 쓸 방식에 맞춘다.
 * @warning 성공할 때 그 줄을 바꾼다. 이름을 제자리에서 종결시키며 닫는 괄호와 그 뒤의 것을 잘라 낸다. 그래서 그 질문을 하는 것만으로 호출자의 버퍼가 바뀐다.
 */
/* check for " [ anything-except-bracket-or-empty ] # arbitrary-comment"
   with spaces optional; returns pointer to "anything-except..." (with
   trailing " ] #..." stripped) if ok, otherwise Null */
staticfn char *
is_config_section(
    char *str) /* trailing spaces are stripped, ']' too iff result is good */
{
    char *a, *c, *z;

    /* remove any spaces at start and end; won't significantly interfere
       with echoing the string in a config error message, if warranted */
    a = trimspaces(str);
    /* first character should be open square bracket; set pointer past it */
    if (*a++ != '[')
        return (char *) 0;
    /* last character should be close bracket, ignoring any comment */
    z = strchr(a, ']');
    if (!z)
        return (char *) 0;
    /* comment, if present, can be preceded by spaces */
    for (c = z + 1; *c == ' '; ++c)
        continue;
    if (*c && *c != '#')
        return (char *) 0;
    /* we now know that result is good; there won't be a config error
       message so we can modify the input string */
    *z = '\0';
    /* 'a' points past '[' and the string ends where ']' was; remove any
       spaces between '[' and choice-start and between choice-end and ']' */
    return trimspaces(a);
}

/**
 * @brief Deal with sections, and say whether the caller should ignore this line.
 *
 * The gate every line passes through before being parsed. It recognises a heading and remembers which section is now in force, and for an ordinary line it answers whether that section is one the file asked
 * for -- so a line in a section meant for another platform is skipped and not reported as an error.
 *
 * @param buf the line
 * @return true if the caller should ignore this line, whether because it was a heading or because it belongs to a section not chosen
 * @note A section with an empty name ends sectioning, so the rest of the file applies unconditionally. That is how a file can have platform-specific settings followed by common ones.
 * @warning A section heading with nothing having chosen a section is reported as an error, because such a file would be silently ignoring everything in it -- which is far more likely to be a mistake than an
 *          intention.
 */
/**
 * @brief 절을 처리하고, 호출자가 이 줄을 무시해야 하는지 말한다.
 *
 * 모든 줄이 파싱되기 전에 지나는 관문이다. 제목을 알아보고 이제 어느 절이 유효한지 기억하며, 평범한 줄에 대해서는 그 절이 파일이 요청한 것인지 답한다. 그래서 다른 플랫폼을 위한 절의 줄은 건너뛰어지고 오류로 알려지지 않는다.
 *
 * @param buf 그 줄
 * @return 호출자가 이 줄을 무시해야 하면 참. 그것이 제목이었기 때문이든 고르지 않은 절에 속하기 때문이든
 * @note 이름이 빈 절은 절 나누기를 끝내므로, 파일의 나머지가 조건 없이 적용된다. 그것이 파일이 플랫폼별 설정 뒤에 공통 설정을 둘 수 있는 방식이다.
 * @warning 절을 고른 것이 아무것도 없는 상태의 절 제목은 오류로 알려진다. 그런 파일은 그 안의 모든 것을 조용히 무시하고 있을 것이며, 그것은 의도보다 잘못일 가능성이 훨씬 크다.
 */
staticfn boolean
handle_config_section(char *buf)
{
    char *sect = is_config_section(buf);

    if (sect) {
        if (gc.config_section_current)
            free(gc.config_section_current), gc.config_section_current = 0;
        /* is_config_section() removed brackets from 'sect' */
        if (!gc.config_section_chosen) {
            config_error_add("Section \"[%s]\" without CHOOSE", sect);
            return TRUE;
        }
        if (*sect) { /* got a section name */
            gc.config_section_current = dupstr(sect);
            debugpline1("set config section: '%s'",
                        gc.config_section_current);
        } else { /* empty section name => end of sections */
            free_config_sections();
            debugpline0("unset config section");
        }
        return TRUE;
    }

    if (gc.config_section_current) {
        if (!gc.config_section_chosen)
            return TRUE;
        if (strcmp(gc.config_section_current, gc.config_section_chosen))
            return TRUE;
    }
    return FALSE;
}

#define match_varname(INP, NAM, LEN) match_optname(INP, NAM, LEN, TRUE)

/**
 * @brief Find where a configuration line's value begins.
 * @param buf the line
 * @return a pointer to the separator, or null if the line has no value
 * @note Two separators are accepted, and whichever comes first is the one -- so a value containing one of them is not split at the wrong place. That is why this searches for both rather than trying one and
 *       then the other.
 */
/**
 * @brief 설정 줄의 값이 어디서 시작하는지 찾는다.
 * @param buf 그 줄
 * @return 그 구분자를 가리키는 포인터. 그 줄에 값이 없으면 널
 * @note 두 구분자가 받아들여지며, 먼저 오는 것이 그것이다. 그래서 그 중 하나를 담은 값이 잘못된 자리에서 쪼개지지 않는다. 그것이 하나를 시도한 뒤 다른 것을 시도하는 대신 둘 다를 찾는 이유다.
 */
/* find the '=' or ':' */
staticfn char *
find_optparam(char *buf)
{
    char *bufp, *altp;

    bufp = strchr(buf, '=');
    altp = strchr(buf, ':');
    if (!bufp || (altp && altp < bufp))
        bufp = altp;

    return bufp;
}

#ifndef SFCTOOL

staticfn boolean
cnf_line_OPTIONS(char *origbuf)
{
    char *bufp = find_optparam(origbuf);

    ++bufp; /* skip '='; parseoptions() handles spaces */
    return parseoptions(bufp, TRUE, TRUE);
}

staticfn boolean
cnf_line_AUTOPICKUP_EXCEPTION(char *bufp)
{
    add_autopickup_exception(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_BINDINGS(char *bufp)
{
    return parsebindings(bufp);
}

staticfn boolean
cnf_line_AUTOCOMPLETE(char *bufp)
{
    parseautocomplete(bufp, TRUE);
    return TRUE;
}

staticfn boolean
cnf_line_MSGTYPE(char *bufp)
{
    return msgtype_parse_add(bufp);
}

staticfn boolean
cnf_line_HACKDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, HACKPREFIX);
#else /*NOCWD_ASSUMPTIONS*/
#ifdef MICRO
    (void) strncpy(gh.hackdir, bufp, PATHLEN - 1);
#else /* MICRO */
    nhUse(bufp);
#endif /* MICRO */
#endif /*NOCWD_ASSUMPTIONS*/
    return TRUE;
}

staticfn boolean
cnf_line_LEVELDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, LEVELPREFIX);
#else /*NOCWD_ASSUMPTIONS*/
#ifdef MICRO
    if (strlen(bufp) >= PATHLEN)
        bufp[PATHLEN - 1] = '\0';
    Strcpy(g.permbones, bufp);
    if (!ramdisk_specified || !*levels)
        Strcpy(levels, bufp);
    gr.ramdisk = (strcmp(g.permbones, levels) != 0);
#else /* MICRO */
    nhUse(bufp);
#endif /* MICRO */
#endif /*NOCWD_ASSUMPTIONS*/
    return TRUE;
}

staticfn boolean
cnf_line_SAVEDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, SAVEPREFIX);
#else /*NOCWD_ASSUMPTIONS*/
#ifdef MICRO
    char *ptr;

    if ((ptr = strchr(bufp, ';')) != 0) {
        *ptr = '\0';
    }

    (void) strncpy(gs.SAVEP, bufp, SAVESIZE - 1);
    append_slash(gs.SAVEP);
#else /* MICRO */
    nhUse(bufp);
#endif /* MICRO */
#endif /*NOCWD_ASSUMPTIONS*/
    return TRUE;
}

staticfn boolean
cnf_line_BONESDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, BONESPREFIX);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_DATADIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, DATAPREFIX);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_SCOREDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, SCOREPREFIX);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_LOCKDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, LOCKPREFIX);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_CONFIGDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, CONFIGPREFIX);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_TROUBLEDIR(char *bufp)
{
#ifdef NOCWD_ASSUMPTIONS
    adjust_prefix(bufp, TROUBLEPREFIX);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_NAME(char *bufp)
{
    (void) strncpy(svp.plname, bufp, PL_NSIZ - 1);
    return TRUE;
}

staticfn boolean
cnf_line_ROLE(char *bufp)
{
    int len;

    if ((len = str2role(bufp)) >= 0)
        flags.initrole = len;
    return TRUE;
}

staticfn boolean
cnf_line_dogname(char *bufp)
{
    (void) strncpy(gd.dogname, bufp, PL_PSIZ - 1);
    return TRUE;
}

staticfn boolean
cnf_line_catname(char *bufp)
{
    (void) strncpy(gc.catname, bufp, PL_PSIZ - 1);
    return TRUE;
}
#endif /* SFCTOOL */

#ifdef SYSCF

staticfn boolean
cnf_line_WIZARDS(char *bufp)
{
    if (sysopt.wizards)
        free((genericptr_t) sysopt.wizards);
    sysopt.wizards = dupstr(bufp);
    if (strlen(sysopt.wizards) && strcmp(sysopt.wizards, "*")) {
        /* pre-format WIZARDS list now; it's displayed during a panic
           and since that panic might be due to running out of memory,
           we don't want to risk attempting to allocate any memory then */
        if (sysopt.fmtd_wizard_list)
            free((genericptr_t) sysopt.fmtd_wizard_list);
        sysopt.fmtd_wizard_list = build_english_list(sysopt.wizards);
    }
    return TRUE;
}

staticfn boolean
cnf_line_SHELLERS(char *bufp)
{
    if (sysopt.shellers)
        free((genericptr_t) sysopt.shellers);
    sysopt.shellers = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_MSGHANDLER(char *bufp)
{
    if (sysopt.msghandler)
        free((genericptr_t) sysopt.msghandler);
    sysopt.msghandler = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_EXPLORERS(char *bufp)
{
    if (sysopt.explorers)
        free((genericptr_t) sysopt.explorers);
    sysopt.explorers = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_DEBUGFILES(char *bufp)
{
    /* might already have a vaule from getenv("DEBUGFILES");
       if so, ignore this value from SYSCF */
    if (!sysopt.env_dbgfl) {
        if (sysopt.debugfiles)
            free((genericptr_t) sysopt.debugfiles);
        sysopt.debugfiles = dupstr(bufp);
    }
    return TRUE;
}

staticfn boolean
cnf_line_DUMPLOGFILE(char *bufp)
{
#ifdef DUMPLOG
    if (sysopt.dumplogfile)
        free((genericptr_t) sysopt.dumplogfile);
    sysopt.dumplogfile = dupstr(bufp);
#else
    nhUse(bufp);
#endif /*DUMPLOG*/
    return TRUE;
}

staticfn boolean
cnf_line_GENERICUSERS(char *bufp)
{
    if (sysopt.genericusers)
        free((genericptr_t) sysopt.genericusers);
    sysopt.genericusers = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_BONES_POOLS(char *bufp)
{
    /* max value of 10 guarantees (N % bones.pools) will be one digit
       so we don't lose control of the length of bones file names */
    int n = atoi(bufp);

    sysopt.bones_pools = (n <= 0) ? 0 : min(n, 10);
    /* note: right now bones_pools==0 is the same as bones_pools==1,
       but we could change that and make bones_pools==0 become an
       indicator to suppress bones usage altogether */
    return TRUE;
}

staticfn boolean
cnf_line_SUPPORT(char *bufp)
{
    if (sysopt.support)
        free((genericptr_t) sysopt.support);
    sysopt.support = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_RECOVER(char *bufp)
{
    if (sysopt.recover)
        free((genericptr_t) sysopt.recover);
    sysopt.recover = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_CHECK_SAVE_UID(char *bufp)
{
    int n = atoi(bufp);

    sysopt.check_save_uid = n;
    return TRUE;
}

staticfn boolean
cnf_line_CHECK_PLNAME(char *bufp)
{
    int n = atoi(bufp);

    sysopt.check_plname = n;
    return TRUE;
}

staticfn boolean
cnf_line_SEDUCE(char *bufp)
{
    int n = !!atoi(bufp); /* XXX this could be tighter */
#ifdef SYSCF
    int src = iflags.parse_config_file_src;
    boolean in_sysconf = (src == set_in_sysconf);
#else
    boolean in_sysconf = FALSE;
#endif

    /* allow anyone to disable it but can only enable it in sysconf
       or as a no-op for the user when sysconf hasn't disabled it */
    if (!in_sysconf && !sysopt.seduce && n != 0) {
        config_error_add("Illegal value in SEDUCE");
        n = 0;
    }
    sysopt.seduce = n;
    sysopt_seduce_set(sysopt.seduce);
    return TRUE;
}

staticfn boolean
cnf_line_HIDEUSAGE(char *bufp)
{
    int n = !!atoi(bufp);

    sysopt.hideusage = n;
    return TRUE;
}

staticfn boolean
cnf_line_MAXPLAYERS(char *bufp)
{
    int n = atoi(bufp);

    /* XXX to get more than 25, need to rewrite all lock code */
    if (n < 0 || n > 25) {
        config_error_add("Illegal value in MAXPLAYERS (maximum is 25)");
        n = 5;
    }
    sysopt.maxplayers = n;
    return TRUE;
}

staticfn boolean
cnf_line_MAX_REROLL_RATE(char *bufp)
{
    int n = atoi(bufp);

    if (n < 0 || n > 255) {
        config_error_add("Illegal value in MAX_REROLL_RATE (maximum is 255)");
        n = 10;
    }
    sysopt.maxrerollrate = n;
    return TRUE;
}

staticfn boolean
cnf_line_PERSMAX(char *bufp)
{
    int n = atoi(bufp);

    if (n < 1) {
        config_error_add("Illegal value in PERSMAX (minimum is 1)");
        n = 0;
    }
    sysopt.persmax = n;
    return TRUE;
}

staticfn boolean
cnf_line_PERS_IS_UID(char *bufp)
{
    int n = atoi(bufp);

    if (n != 0 && n != 1) {
        config_error_add("Illegal value in PERS_IS_UID (must be 0 or 1)");
        n = 0;
    }
    sysopt.pers_is_uid = n;
    return TRUE;
}

staticfn boolean
cnf_line_ENTRYMAX(char *bufp)
{
    int n = atoi(bufp);

    if (n < 10) {
        config_error_add("Illegal value in ENTRYMAX (minimum is 10)");
        n = 10;
    }
    sysopt.entrymax = n;
    return TRUE;
}

staticfn boolean
cnf_line_POINTSMIN(char *bufp)
{
    int n = atoi(bufp);

    if (n < 1) {
        config_error_add("Illegal value in POINTSMIN (minimum is 1)");
        n = 100;
    }
    sysopt.pointsmin = n;
    return TRUE;
}

staticfn boolean
cnf_line_MAX_STATUENAME_RANK(char *bufp)
{
    int n = atoi(bufp);

    if (n < 1) {
        config_error_add("Illegal value in MAX_STATUENAME_RANK"
                         " (minimum is 1)");
        n = 10;
    }
    sysopt.tt_oname_maxrank = n;
    return TRUE;
}

staticfn boolean
cnf_line_LIVELOG(char *bufp)
{
    /* using 0 for base accepts "dddd" as decimal provided that first 'd'
       isn't '0', "0xhhhh" as hexadecimal, and "0oooo" as octal; ignores
       any trailing junk, including '8' or '9' for leading '0' octal */
    long L = strtol(bufp, NULL, 0);

    if (L < 0L || L > 0xffffL) {
        config_error_add("Illegal value for LIVELOG"
                         " (must be between 0 and 0xFFFF).");
        return 0;
    }
    sysopt.livelog = L;
    return TRUE;
}

staticfn boolean
cnf_line_PANICTRACE_LIBC(char *bufp)
{
    int n = atoi(bufp);

#if defined(PANICTRACE) && defined(PANICTRACE_LIBC)
    if (n < 0 || n > 2) {
        config_error_add("Illegal value in PANICTRACE_LIBC (not 0,1,2)");
        n = 0;
    }
#endif
    sysopt.panictrace_libc = n;
    return TRUE;
}

staticfn boolean
cnf_line_PANICTRACE_GDB(char *bufp)
{
    int n = atoi(bufp);

#if defined(PANICTRACE)
    if (n < 0 || n > 2) {
        config_error_add("Illegal value in PANICTRACE_GDB (not 0,1,2)");
        n = 0;
    }
#endif
    sysopt.panictrace_gdb = n;
    return TRUE;
}

staticfn boolean
cnf_line_GDBPATH(char *bufp)
{
#if defined(PANICTRACE) && !defined(VMS)
    if (!file_exists(bufp)) {
        config_error_add("File specified in GDBPATH does not exist");
        return FALSE;
    }
#endif
    if (sysopt.gdbpath)
        free((genericptr_t) sysopt.gdbpath);
    sysopt.gdbpath = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_GREPPATH(char *bufp)
{
#if defined(PANICTRACE) && !defined(VMS)
    if (!file_exists(bufp)) {
        config_error_add("File specified in GREPPATH does not exist");
        return FALSE;
    }
#endif
    if (sysopt.greppath)
        free((genericptr_t) sysopt.greppath);
    sysopt.greppath = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_CRASHREPORTURL(char *bufp)
{
    if (sysopt.crashreporturl)
        free((genericptr_t) sysopt.crashreporturl);
    sysopt.crashreporturl = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_ACCESSIBILITY(char *bufp)
{
    int n = atoi(bufp);

    if (n < 0 || n > 1) {
        config_error_add("Illegal value in ACCESSIBILITY (not 0,1)");
        n = 0;
    }
    sysopt.accessibility = n;
    return TRUE;
}

staticfn boolean
cnf_line_PORTABLE_DEVICE_PATHS(char *bufp)
{
#ifdef WIN32
    int n = atoi(bufp);

    if (n < 0 || n > 1) {
        config_error_add("Illegal value in PORTABLE_DEVICE_PATHS"
                         " (not 0 or 1)");
        n = 0;
    }
    sysopt.portable_device_paths = n;
#else   /* Windows-only directive encountered by non-Windows config */
    nhUse(bufp);
    config_error_add("PORTABLE_DEVICE_PATHS is not supported");
#endif
    return TRUE;
}
#endif  /* SYSCF */

#ifndef SFCTOOL

staticfn boolean
cnf_line_BOULDER(char *bufp)
{
    (void) get_uchars(bufp, &go.ov_primary_syms[SYM_BOULDER + SYM_OFF_X],
                      TRUE, 1, "BOULDER");
    return TRUE;
}

staticfn boolean
cnf_line_MENUCOLOR(char *bufp)
{
    return add_menu_coloring(bufp);
}

staticfn boolean
cnf_line_HILITE_STATUS(char *bufp)
{
#ifdef STATUS_HILITES
    return parse_status_hl1(bufp, TRUE);
#else
    nhUse(bufp);
    return TRUE;
#endif
}

staticfn boolean
cnf_line_WARNINGS(char *bufp)
{
    uchar translate[MAXPCHARS];

    (void) get_uchars(bufp, translate, FALSE, WARNCOUNT, "WARNINGS");
    assign_warnings(translate);
    return TRUE;
}

staticfn boolean
cnf_line_ROGUESYMBOLS(char *bufp)
{
    if (parsesymbols(bufp, ROGUESET)) {
        switch_symbols(TRUE);
        return TRUE;
    }
    config_error_add("Error in ROGUESYMBOLS definition '%s'", bufp);
    return FALSE;
}

staticfn boolean
cnf_line_SYMBOLS(char *bufp)
{
    if (parsesymbols(bufp, PRIMARYSET)) {
        switch_symbols(TRUE);
        return TRUE;
    }
    if (!config_unmatched_ignored())
        config_error_add("Error in SYMBOLS definition '%s'", bufp);
    return FALSE;
}

staticfn boolean
cnf_line_WIZKIT(char *bufp)
{
    (void) strncpy(gw.wizkit, bufp, WIZKIT_MAX - 1);
    return TRUE;
}

#ifdef USER_SOUNDS
staticfn boolean
cnf_line_SOUNDDIR(char *bufp)
{
    if (sounddir)
        free((genericptr_t) sounddir);
    sounddir = dupstr(bufp);
    return TRUE;
}

staticfn boolean
cnf_line_SOUND(char *bufp)
{
    add_sound_mapping(bufp);
    return TRUE;
}
#endif /*USER_SOUNDS*/

staticfn boolean
cnf_line_QT_TILEWIDTH(char *bufp)
{
#ifdef QT_GRAPHICS
    extern char *qt_tilewidth;

    if (qt_tilewidth == NULL)
        qt_tilewidth = dupstr(bufp);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_QT_TILEHEIGHT(char *bufp)
{
#ifdef QT_GRAPHICS
    extern char *qt_tileheight;

    if (qt_tileheight == NULL)
        qt_tileheight = dupstr(bufp);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_QT_FONTSIZE(char *bufp)
{
#ifdef QT_GRAPHICS
    extern char *qt_fontsize;

    if (qt_fontsize == NULL)
        qt_fontsize = dupstr(bufp);
#else
    nhUse(bufp);
#endif
    return TRUE;
}

staticfn boolean
cnf_line_QT_COMPACT(char *bufp)
{
#ifdef QT_GRAPHICS
    extern int qt_compact_mode;

    qt_compact_mode = atoi(bufp);
#else
    nhUse(bufp);
#endif
    return TRUE;
}
#endif /* SFCTOOL */

typedef boolean (*config_line_stmt_func)(char *);

/* normal */
#define CNFL_N(n, l) { #n, l, FALSE, FALSE, cnf_line_##n }
/* normal, alias */
#define CNFL_NA(n, l, f) { #n, l, FALSE, FALSE, cnf_line_##f }
/* sysconf only */
#define CNFL_S(n, l) { #n, l, TRUE, FALSE, cnf_line_##n }

static const struct match_config_line_stmt {
    const char *name;
    int len;
    boolean syscnf_only;
    boolean origbuf;
    config_line_stmt_func fn;
} config_line_stmt[] = {
#ifndef SFCTOOL
    /* OPTIONS handled separately */
    { "OPTIONS", 4, FALSE, TRUE, cnf_line_OPTIONS },
    CNFL_N(AUTOPICKUP_EXCEPTION, 5),
    CNFL_N(BINDINGS, 4),
    CNFL_N(AUTOCOMPLETE, 5),
    CNFL_N(MSGTYPE, 7),
    CNFL_N(HACKDIR, 4),
    CNFL_N(LEVELDIR, 4),
    CNFL_NA(LEVELS, 4, LEVELDIR),
    CNFL_N(SAVEDIR, 4),
    CNFL_N(BONESDIR, 5),
    CNFL_N(DATADIR, 4),
    CNFL_N(SCOREDIR, 4),
    CNFL_N(LOCKDIR, 4),
    CNFL_N(CONFIGDIR, 4),
    CNFL_N(TROUBLEDIR, 4),
    CNFL_N(NAME, 4),
    CNFL_N(ROLE, 4),
    CNFL_NA(CHARACTER, 4, ROLE),
    CNFL_N(dogname, 3),
    CNFL_N(catname, 3),
#endif /* SFCTOOL */
#ifdef SYSCF
    CNFL_S(WIZARDS, 7),
    CNFL_S(SHELLERS, 8),
    CNFL_S(MSGHANDLER, 9),
    CNFL_S(EXPLORERS, 7),
    CNFL_S(DEBUGFILES, 5),
    CNFL_S(DUMPLOGFILE, 7),
    CNFL_S(GENERICUSERS, 12),
    CNFL_S(BONES_POOLS, 10),
    CNFL_S(SUPPORT, 7),
    CNFL_S(RECOVER, 7),
    CNFL_S(CHECK_SAVE_UID, 14),
    CNFL_S(CHECK_PLNAME, 12),
    CNFL_S(SEDUCE, 6),
    CNFL_S(HIDEUSAGE, 9),
    CNFL_S(MAXPLAYERS, 10),
    CNFL_S(MAX_REROLL_RATE, 10),
    CNFL_S(PERSMAX, 7),
    CNFL_S(PERS_IS_UID, 11),
    CNFL_S(ENTRYMAX, 8),
    CNFL_S(POINTSMIN, 9),
    CNFL_S(MAX_STATUENAME_RANK, 10),
    CNFL_S(LIVELOG, 7),
    CNFL_S(PANICTRACE_LIBC, 15),
    CNFL_S(PANICTRACE_GDB, 14),
    CNFL_S(CRASHREPORTURL, 13),
    CNFL_S(GDBPATH, 7),
    CNFL_S(GREPPATH, 7),
    CNFL_S(ACCESSIBILITY, 13),
    CNFL_S(PORTABLE_DEVICE_PATHS, 8),
#endif /*SYSCF*/
#ifndef SFCTOOL
    CNFL_N(BOULDER, 3),
    CNFL_N(MENUCOLOR, 9),
    CNFL_N(HILITE_STATUS, 6),
    CNFL_N(WARNINGS, 5),
    CNFL_N(ROGUESYMBOLS, 4),
    CNFL_N(SYMBOLS, 4),
    CNFL_N(WIZKIT, 6),
#ifdef USER_SOUNDS
    CNFL_N(SOUNDDIR, 8),
    CNFL_N(SOUND, 5),
#endif /*USER_SOUNDS*/
    CNFL_N(QT_TILEWIDTH, 12),
    CNFL_N(QT_TILEHEIGHT, 13),
    CNFL_N(QT_FONTSIZE, 11),
    CNFL_N(QT_COMPACT, 10)
#endif /* SFCTOOL */
};

#undef CNFL_N
#undef CNFL_NA
#undef CNFL_S

static boolean disregarded_config_lines[SIZE(config_line_stmt)];

/**
 * @brief Understand and apply one line of configuration.
 *
 * The single entry to the parser, and everything a configuration line can be goes through here: a section heading, a statement, an option, a comment, or nothing recognisable. Which of those it is decides what
 * happens, and a line that is none of them is reported rather than ignored.
 *
 * The line's source has already been established by the caller, and it governs what is permitted. A statement a player may not make is refused with a message about why rather than being obeyed or silently
 * dropped -- which is what makes a shared installation's restrictions visible to the player who ran into one.
 *
 * @param origbuf the line as read, which this may modify
 * @return whether the line was understood; false means an error has already been recorded
 * @note Applies as it goes and there is no undo. A file whose tenth line is rejected has already applied nine.
 * @warning Modifies the buffer. It splits the line at the separator and trims it, so a caller that needs the original text afterwards -- to echo it in an error message, for instance -- must have kept a copy,
 *          which is exactly what the error machinery does.
 */
/**
 * @brief 설정 한 줄을 이해하고 적용한다.
 *
 * 파서의 단 하나뿐인 입구이며, 설정 줄이 될 수 있는 모든 것이 여기를 지난다. 절 제목, 문장, 옵션, 주석, 또는 알아볼 수 없는 것. 그 중 무엇인지가 무슨 일이 일어날지 정하고, 그 중 어느 것도 아닌 줄은 무시되는 대신 알려진다.
 *
 * 그 줄의 출처는 호출자가 이미 확정해 두었으며, 그것이 무엇이 허용되는지를 지배한다. 플레이어가 해서는 안 되는 문장은 따라지거나 조용히 버려지는 대신 왜인지에 대한 메시지와 함께 거부된다. 그것이 공유 설치본의 제한을 그것에 부딪힌 플레이어에게 보이게 만드는 것이다.
 *
 * @param origbuf 읽힌 대로의 줄. 이것이 그것을 바꿀 수 있다
 * @return 그 줄이 이해되었는지. 거짓은 오류가 이미 기록되었다는 뜻이다
 * @note 진행하며 적용하고 되돌리기는 없다. 열 번째 줄이 거부된 파일은 이미 아홉 줄을 적용한 것이다.
 * @warning 버퍼를 바꾼다. 구분자에서 줄을 쪼개고 다듬으므로, 그 뒤에 원래 글이 필요한 호출자 -- 예컨대 오류 메시지에 그것을 되보이려는 -- 는 사본을 두고 있어야 한다. 오류 기제가 하는 일이 정확히 그것이다.
 */
boolean
parse_config_line(char *origbuf)
{
#if defined(MICRO) && !defined(NOCWD_ASSUMPTIONS)
    static boolean ramdisk_specified = FALSE;
#endif
#ifdef SYSCF
    int src = iflags.parse_config_file_src;
    boolean in_sysconf = (src == set_in_sysconf);
#endif
    char *bufp, buf[4 * BUFSZ];
    int i;

    while (*origbuf == ' ' || *origbuf == '\t') /* skip leading whitespace */
        ++origbuf;                   /* (caller probably already did this) */
    (void) strncpy(buf, origbuf, sizeof buf - 1);
    buf[sizeof buf - 1] = '\0'; /* strncpy not guaranteed to NUL terminate */
    /* convert any tab to space, condense consecutive spaces into one,
       remove leading and trailing spaces (exception: if there is nothing
       but spaces, one of them will be kept even though it leads/trails) */
    mungspaces(buf);

    /* find the '=' or ':' */
    bufp = find_optparam(buf);
    if (!bufp) {
        if (!ignore_statement_errors)
            config_error_add("Not a config statement, missing '='");
        return FALSE;
    }
    /* skip past '=', then space between it and value, if any */
    ++bufp;
    if (*bufp == ' ')
        ++bufp;

    for (i = 0; i < SIZE(config_line_stmt); i++) {
#ifdef SYSCF
        if (config_line_stmt[i].syscnf_only && !in_sysconf)
            continue;
#endif
        if (match_varname(buf, config_line_stmt[i].name,
                          config_line_stmt[i].len)) {
            char *parm = config_line_stmt[i].origbuf ? origbuf : bufp;

            if (!disregarded_config_lines[i])
                return config_line_stmt[i].fn(parm);
        }
    }

    if (!ignore_errors_on_unmatched)
        config_error_add("Unknown config statement");
    return FALSE;
}

#ifdef USER_SOUNDS
boolean
can_read_file(const char *filename)
{
    return (boolean) (access(filename, 4) == 0);
}
#endif /* USER_SOUNDS */

/**
 * @brief One error found in a configuration file, remembered rather than printed.
 * @note Errors are collected because they arrive before the display exists, and because a player would rather be shown all of them at once than interrupted for each. The line number is kept with the message
 *       since the message alone would not say where to look.
 */
/**
 * @brief 설정 파일에서 발견된 오류 하나. 인쇄되는 대신 기억된다.
 * @note 오류가 모아지는 것은 그것이 표시부가 존재하기 전에 도착하기 때문이고, 플레이어가 하나하나 방해받기보다 한꺼번에 전부 보여지기를 원하기 때문이다. 메시지만으로는 어디를 볼지 말해 주지 않으므로 줄 번호가 그것과 함께 보관된다.
 */
struct _config_error_errmsg {
    int line_num;
    char *errormsg;
    struct _config_error_errmsg *next;
};

/**
 * @brief What is known about the file currently being read, as a stack of frames.
 *
 * A stack rather than a single record because reading is nested: a configuration file may cause another to be read, and an error in the inner one must be reported against the inner file and line. So each file
 * being read pushes a frame and pops it when done.
 *
 * @note The original line is kept in full, at four times the ordinary buffer size, because an error message echoes the line that caused it -- and the parser has by then modified the copy it was given.
 * @note Whether the source is a file matters to the wording: a bad line in a file has a number to cite, and a bad setting from the command line does not.
 * @note The secure flag records that this file is one an ordinary player may not have written, which is what lets a message distinguish "your file is wrong" from "the installation's file is wrong".
 */
/**
 * @brief 지금 읽히고 있는 파일에 대해 알려진 것. 프레임의 스택으로.
 *
 * 하나의 기록이 아니라 스택인 것은 읽기가 중첩되기 때문이다. 설정 파일이 다른 파일을 읽게 할 수 있고, 안쪽 것의 오류는 안쪽 파일과 줄에 대해 알려져야 한다. 그래서 읽히는 각 파일이 프레임을 밀어 넣고 끝나면 꺼낸다.
 *
 * @note 원래 줄이 평범한 버퍼의 네 배 크기로 온전히 보관된다. 오류 메시지가 그것을 일으킨 줄을 되보이고, 그때쯤 파서는 자신이 받은 사본을 이미 바꾸어 놓았기 때문이다.
 * @note 출처가 파일인지가 표현에 중요하다. 파일 안의 잘못된 줄에는 인용할 번호가 있고, 명령줄에서 온 잘못된 설정에는 없다.
 * @note 보안 플래그는 이 파일이 평범한 플레이어가 쓰지 않았을 수 있는 것임을 기록한다. 그것이 메시지가 "당신의 파일이 잘못되었다"와 "설치본의 파일이 잘못되었다"를 구별할 수 있게 하는 것이다.
 */
struct _config_error_frame {
    int line_num;
    int num_errors;
    boolean origline_shown;
    boolean fromfile;
    boolean secure;
    char origline[4 * BUFSZ];
    char source[BUFSZ];
    struct _config_error_frame *next;
};

static struct _config_error_frame *config_error_data = 0;
static struct _config_error_errmsg *config_error_msg = 0;

/**
 * @brief Begin collecting errors for a source about to be read.
 * @param from_file whether the source is a file, which decides whether errors can cite a line number
 * @param sourcename what to call the source in a message
 * @param secure whether this source is one an ordinary player may not have written
 * @note Pushes a frame, so it pairs with finishing and the pair must balance. Reading nested sources works because of that pairing, and failing to finish one leaves later errors attributed to the wrong file.
 */
/**
 * @brief 곧 읽힐 출처에 대해 오류를 모으기 시작한다.
 * @param from_file 그 출처가 파일인지. 오류가 줄 번호를 인용할 수 있는지를 정한다
 * @param sourcename 메시지에서 그 출처를 무엇이라 부를지
 * @param secure 이 출처가 평범한 플레이어가 쓰지 않았을 수 있는 것인지
 * @note 프레임을 밀어 넣으므로 마치기와 짝을 이루며, 그 짝이 균형이 맞아야 한다. 중첩된 출처를 읽는 것이 그 짝지음 덕분에 작동하며, 하나를 마치지 못하면 이후의 오류가 잘못된 파일에 귀속된다.
 */
void
config_error_init(boolean from_file, const char *sourcename, boolean secure)
{
    struct _config_error_frame *tmp = (struct _config_error_frame *)
                                                           alloc(sizeof *tmp);

    tmp->line_num = 0;
    tmp->num_errors = 0;
    tmp->origline_shown = FALSE;
    tmp->fromfile = from_file;
    tmp->secure = secure;
    tmp->origline[0] = '\0';
    if (sourcename && sourcename[0]) {
        (void) strncpy(tmp->source, sourcename, sizeof (tmp->source) - 1);
        tmp->source[sizeof (tmp->source) - 1] = '\0';
    } else
        tmp->source[0] = '\0';

    tmp->next = config_error_data;
    config_error_data = tmp;
    program_state.config_error_ready = TRUE;
}

/**
 * @brief Advance to the next line, keeping a copy of it in case it turns out to be wrong.
 * @param line the line about to be parsed
 * @return whether reading should continue
 * @note The copy is the point. The parser modifies the line it is given, so by the time an error is discovered the original text is gone -- and an error message that could not quote the offending line would be
 *       much less useful.
 */
/**
 * @brief 다음 줄로 넘어가며, 그것이 잘못된 것으로 드러날 경우를 위해 사본을 둔다.
 * @param line 곧 파싱될 줄
 * @return 읽기를 계속해야 하는지
 * @note 그 사본이 요점이다. 파서는 자신이 받은 줄을 바꾸므로, 오류가 발견될 때쯤 원래 글은 사라져 있다. 그리고 문제가 된 줄을 인용할 수 없는 오류 메시지는 훨씬 덜 쓸모 있다.
 */
staticfn boolean
config_error_nextline(const char *line)
{
    struct _config_error_frame *ced = config_error_data;

    if (!ced)
        return FALSE;

    if (ced->num_errors && ced->secure)
        return FALSE;

    ced->line_num++;
    ced->origline_shown = FALSE;
    if (line && line[0]) {
        (void) strncpy(ced->origline, line, sizeof (ced->origline) - 1);
        ced->origline[sizeof (ced->origline) - 1] = '\0';
    } else
        ced->origline[0] = '\0';

    return TRUE;
}

#ifndef SFCTOOL
int
l_get_config_errors(lua_State *L)
{
    struct _config_error_errmsg *dat = config_error_msg;
    struct _config_error_errmsg *tmp;
    int idx = 1;

    lua_newtable(L);

    while (dat) {
        lua_pushinteger(L, idx++);
        lua_newtable(L);
        nhl_add_table_entry_int(L, "line", dat->line_num);
        nhl_add_table_entry_str(L, "error", dat->errormsg);
        lua_settable(L, -3);
        tmp = dat->next;
        free(dat->errormsg);
        dat->errormsg = (char *) 0;
        free(dat);
        dat = tmp;
    }
    config_error_msg = (struct _config_error_errmsg *) 0;

    return 1;
}
#endif /* SFCTOOL */

/**
 * @brief Record one error, attributing it to the line currently being read.
 *
 * @param buf the message, which may be empty
 * @note An empty message becomes a generic one rather than nothing, because an error that could not be described is still an error the player should be told about.
 * @note Punctuation is added if the caller omitted it, so a caller writes the substance and does not have to think about how the message will read in a list of them.
 * @note The variable-argument form callers usually use lives with the rest of the message machinery rather than here, as the existing comment records.
 */
/**
 * @brief 오류 하나를 기록하며, 지금 읽히고 있는 줄에 귀속시킨다.
 *
 * @param buf 그 메시지. 비어 있을 수 있다
 * @note 빈 메시지는 아무것도가 아니라 일반적인 메시지가 된다. 기술될 수 없었던 오류도 여전히 플레이어가 알아야 하는 오류이기 때문이다.
 * @note 호출자가 구두점을 빠뜨렸으면 그것이 더해진다. 그래서 호출자는 요지를 적고 그 메시지가 여러 개의 목록 안에서 어떻게 읽힐지 생각하지 않아도 된다.
 * @note 호출자가 보통 쓰는 가변 인자 형태는 기존 주석이 기록하듯 여기가 아니라 메시지 기제의 나머지와 함께 산다.
 */
/* varargs 'config_error_add()' moved to pline.c */
void
config_erradd(const char *buf)
{
    char lineno[QBUFSZ];
    const char *punct;

    if (!buf || !*buf)
        buf = "Unknown error";

    /* if buf[] doesn't end in a period, exclamation point, or question mark,
       we'll include a period (in the message, not appended to buf[]) */
    punct = c_eos((char *) buf) - 1; /* eos(buf)-1 is valid */
    punct = strchr(".!?", *punct) ? "" : ".";

    if (!program_state.config_error_ready) {
        /* either very early, where pline() will use raw_print(), or
           player gave bad value when prompted by interactive 'O' command */
        pline("%s%s%s", !iflags.window_inited ? "config_error_add: " : "",
              buf, punct);
        wait_synch();
        return;
    }

    if (iflags.in_lua) {
        struct _config_error_errmsg *dat
                         = (struct _config_error_errmsg *) alloc(sizeof *dat);

        dat->next = config_error_msg;
        dat->line_num = config_error_data->line_num;
        dat->errormsg = dupstr(buf);
        config_error_msg = dat;
        return;
    }

    config_error_data->num_errors++;
    if (!config_error_data->origline_shown && !config_error_data->secure) {
        pline("\n%s", config_error_data->origline);
        config_error_data->origline_shown = TRUE;
    }
    if (config_error_data->line_num > 0 && !config_error_data->secure) {
        Sprintf(lineno, "Line %d: ", config_error_data->line_num);
    } else
        lineno[0] = '\0';

    pline("%s %s%s%s", config_error_data->secure ? "Error:" : " *",
          lineno, buf, punct);
}

/**
 * @brief Finish with a source, tell the player how many errors it had, and pop the frame.
 *
 * @return how many errors were found, which the caller may act on
 * @note The count is reported rather than the errors themselves. The individual messages were shown as they were found; this is the summary, and a summary is what tells a player whether their file needs
 *       attention at all.
 * @note The wording distinguishes the command line from a file, because errors are "on" one and "in" the other -- a small thing, but the alternative reads as a mistake.
 * @note Where a whole class of setting was unavailable, the first occurrence was reported and the rest skipped; those skipped ones are added back into the count here so the number the player sees is the number
 *       of things wrong rather than the number of messages shown.
 */
/**
 * @brief 출처를 마치고, 그것에 오류가 몇 개 있었는지 플레이어에게 알리고, 프레임을 꺼낸다.
 *
 * @return 발견된 오류가 몇 개인지. 호출자가 그것에 따라 행동할 수 있다
 * @note 오류 자체가 아니라 개수가 알려진다. 개별 메시지는 발견될 때 보여졌다. 이것은 요약이며, 요약이 플레이어에게 자기 파일이 아예 손볼 필요가 있는지 말해 주는 것이다.
 * @note 표현이 명령줄과 파일을 구별한다. 오류가 한쪽에서는 "위에" 있고 다른 쪽에서는 "안에" 있기 때문이다. 작은 것이지만, 그 대안은 잘못처럼 읽힌다.
 * @note 어떤 종류의 설정 전체를 쓸 수 없던 곳에서는 첫 번째만 알려지고 나머지는 건너뛰어졌다. 건너뛰어진 것들이 여기서 개수에 되더해지므로, 플레이어가 보는 숫자가 보여진 메시지의 개수가 아니라 잘못된 것의 개수가 된다.
 */
int
config_error_done(void)
{
    int n;
    struct _config_error_frame *tmp = config_error_data;

    if (!config_error_data)
        return 0;
    n = config_error_data->num_errors;
#ifndef USER_SOUNDS
    if (gn.no_sound_notified > 0) {
        /* no USER_SOUNDS; config_error_add() was called once for first
           SOUND or SOUNDDIR entry seen, then skipped for any others;
           include those skipped ones in the total error count */
        n += (gn.no_sound_notified - 1);
        gn.no_sound_notified = 0;
    }
#endif
    if (n) {
        boolean cmdline = !strcmp(config_error_data->source, "command line");

        pline("\n%d error%s %s %s.\n", n, plur(n), cmdline ? "on" : "in",
              *config_error_data->source ? config_error_data->source
                                         : configfile);
        wait_synch();
    }
    config_error_data = tmp->next;
    free(tmp);
    program_state.config_error_ready = (config_error_data != 0);
    return n;
}

/**
 * @brief Read a configuration file from start to finish.
 *
 * Opens it, sets up error collection, parses every line, reports what was wrong and tidies up. The whole of reading one file, so a caller says which file and which kind and nothing else.
 *
 * @param filename the file to read, which may be empty to mean the default for its kind
 * @param src which kind of configuration this is, governing what its lines are permitted to set
 * @return whether the file was opened; a file full of errors still returns true
 * @warning The return value is about opening and not about correctness. A file that was read and rejected line by line succeeded here, which is deliberate -- the game should start with the defaults rather than
 *          refuse because a configuration file was faulty.
 * @note Duplicate-detection is started here, so an option set twice in one file can be reported. That is per file rather than overall, because setting something in the system file and again in a player's file is
 *       normal and not a mistake.
 */
/**
 * @brief 설정 파일을 처음부터 끝까지 읽는다.
 *
 * 그것을 열고, 오류 수집을 준비하고, 모든 줄을 파싱하고, 무엇이 잘못되었는지 알리고, 정리한다. 파일 하나를 읽는 일 전체이므로, 호출자는 어느 파일이고 어느 종류인지만 말하고 그 밖에는 아무것도 말하지 않는다.
 *
 * @param filename 읽을 파일. 그 종류의 기본값을 뜻하도록 비어 있을 수 있다
 * @param src 이것이 어느 종류의 설정인지. 그 줄들이 무엇을 설정해도 되는지를 지배한다
 * @return 그 파일이 열렸는지. 오류로 가득한 파일도 여전히 참을 반환한다
 * @warning 반환값은 열림에 관한 것이고 올바름에 관한 것이 아니다. 읽혔고 줄마다 거부된 파일도 여기서는 성공한 것이다. 의도적이다. 설정 파일에 결함이 있다고 거부하는 대신 게임이 기본값으로 시작해야 한다.
 * @note 중복 감지가 여기서 시작되므로, 한 파일에서 두 번 설정된 옵션이 알려질 수 있다. 그것은 전체가 아니라 파일마다다. 시스템 파일에서 설정하고 플레이어 파일에서 다시 설정하는 것은 정상이고 잘못이 아니기 때문이다.
 */
boolean
read_config_file(const char *filename, int src)
{
    FILE *fp;
    boolean rv = TRUE;

    if (!(fp = fopen_config_file(filename, src)))
        return FALSE;
#ifndef SFCTOOL
    /* begin detection of duplicate configfile options */
    reset_duplicate_opt_detection();
#endif /* SFCTOOL */
    free_config_sections();
    iflags.parse_config_file_src = src;

    rv = parse_conf_file(fp, parse_config_line);
    (void) fclose(fp);

    free_config_sections();
#ifndef SFCTOOL
    /* turn off detection of duplicate configfile options */
    reset_duplicate_opt_detection();
#endif /* SFCTOOL */
    return rv;
}

/**
 * @brief What the parser needs to remember between one line and the next.
 *
 * The parser is not line-by-line, and this is why it needs state. A configuration line may be continued onto the following one, so a line read may be incomplete and has to be held until the rest arrives -- and
 * the several flags are the situations that can be in progress when a line ends: a continuation is expected, a section is being skipped, the input ran out mid-line.
 *
 * @note The buffer is four times the ordinary size because it holds a merged line, and merging several continued lines produces something much longer than any one of them.
 * @note The result is kept here too so it survives the state being cleaned up, which is why the tidying routine deliberately leaves that one field alone.
 */
/**
 * @brief 파서가 한 줄과 다음 줄 사이에 기억해야 하는 것.
 *
 * 파서는 줄 단위가 아니며, 그것이 상태를 필요로 하는 이유다. 설정 줄이 다음 줄로 이어질 수 있으므로, 읽힌 줄이 불완전할 수 있고 나머지가 도착하기까지 보관되어야 한다. 그리고 여러 플래그는 줄이 끝날 때 진행 중일 수 있는 상황들이다. 이어짐이 기대되는 중, 절을 건너뛰는
 * 중, 입력이 줄 도중에 다한 것.
 *
 * @note 버퍼가 평범한 크기의 네 배인 것은 그것이 합쳐진 줄을 담기 때문이다. 이어진 여러 줄을 합치면 그 중 어느 하나보다 훨씬 긴 것이 나온다.
 * @note 결과도 여기에 보관되어 상태가 정리된 뒤에도 살아남는다. 그것이 정리 루틴이 의도적으로 그 한 필드를 건드리지 않는 이유다.
 */
struct _cnf_parser_state {
    char *inbuf;
    unsigned inbufsz;
    int rv;
    char *ep;
    char *buf;
    boolean skip, morelines;
    boolean cont;
    boolean pbreak;
};

/**
 * @brief Prepare the parser's state before reading begins.
 * @note Starts by assuming success, so a parse with no errors needs nothing to record that -- only a failure has to say so.
 */
/**
 * @brief 읽기가 시작되기 전에 파서의 상태를 준비한다.
 * @note 성공을 가정하며 시작하므로, 오류 없는 파싱은 그것을 기록할 무엇도 필요하지 않다. 실패만이 그렇다고 말해야 한다.
 */
/* Initialize config parser data */
staticfn void
cnf_parser_init(struct _cnf_parser_state *parser)
{
    parser->rv = TRUE; /* assume successful parse */
    parser->ep = parser->buf = (char *) 0;
    parser->skip = FALSE;
    parser->morelines = FALSE;
    parser->inbufsz = 4 * BUFSZ;
    parser->inbuf = (char *) alloc(parser->inbufsz);
    parser->cont = FALSE;
    parser->pbreak = FALSE;
    memset(parser->inbuf, 0, parser->inbufsz);
}

/**
 * @brief Release the parser's buffers once reading has finished.
 * @warning Deliberately leaves the result field alone, as the existing comment says, because the caller reads it after calling this. One of the pointers cleared here points into a buffer being freed, which is why
 *          it is nulled rather than simply left.
 */
/**
 * @brief 읽기가 끝난 뒤 파서의 버퍼를 놓아준다.
 * @warning 기존 주석이 말하듯 결과 필드는 의도적으로 건드리지 않는다. 호출자가 이것을 호출한 뒤 그것을 읽기 때문이다. 여기서 지워지는 포인터 중 하나는 해제되고 있는 버퍼 안을 가리키며, 그래서 그냥 남겨지는 대신 널로 만들어진다.
 */
/* caller has finished with 'parser' (except for 'rv' so leave that intact) */
staticfn void
cnf_parser_done(struct _cnf_parser_state *parser)
{
    parser->ep = 0; /* points into parser->inbuf, so becoming stale */
    if (parser->inbuf)
        free(parser->inbuf), parser->inbuf = 0;
    if (parser->buf)
        free(parser->buf), parser->buf = 0;
}

/**
 * @brief Break a buffer of configuration text into lines and hand each usable one on.
 *
 * Everything that is not a setting is dealt with here -- comments, blank lines, section headings, the statement that chooses a section, and continuation -- so the routine it hands lines to receives only lines that
 * are actually settings. That division is why the parser proper does not have to know about any of this.
 *
 * @param p the parser's state, carried between calls
 * @param proc what to do with each usable line
 * @note Continued lines are merged with a single space between them, as the existing comment records. So a setting split across three lines is indistinguishable from the same setting written on one, which is what
 *       makes continuation useful for a long list.
 * @note Called repeatedly with successive buffers rather than once with a whole file, which is what lets the same code read a file, a string and an environment variable.
 */
/**
 * @brief 설정 텍스트 버퍼를 줄로 나누고 쓸모 있는 줄을 넘겨준다.
 *
 * 설정이 아닌 모든 것이 여기서 처리된다. 주석, 빈 줄, 절 제목, 절을 고르는 문장, 그리고 이어짐. 그래서 줄을 넘겨받는 루틴은 실제로 설정인 줄만을 받는다. 그 분업이 파서 본체가 이 중 어느 것도 알 필요가 없는 이유다.
 *
 * @param p 파서의 상태. 호출 사이에 지녀진다
 * @param proc 각 쓸모 있는 줄에 대해 무엇을 할지
 * @note 기존 주석이 기록하듯 이어진 줄은 사이에 하나의 공백을 두고 합쳐진다. 그래서 세 줄에 걸쳐 쪼개진 설정이 한 줄에 적힌 같은 설정과 구별되지 않으며, 그것이 이어짐을 긴 목록에 쓸모 있게 만드는 것이다.
 * @note 파일 전체로 한 번이 아니라 잇따른 버퍼로 되풀이해 호출된다. 그것이 같은 코드가 파일, 문자열, 환경 변수를 읽을 수 있게 하는 것이다.
 */
/*
 * Parse config buffer, handling comments, empty lines, config sections,
 * CHOOSE, and line continuation, calling proc for every valid line.
 *
 * Continued lines are merged together with one space in between.
 */
staticfn void
parse_conf_buf(struct _cnf_parser_state *p, boolean (*proc)(char *arg))
{
    p->cont = FALSE;
    p->pbreak = FALSE;
    p->ep = strchr(p->inbuf, '\n');
    if (p->skip) { /* in case previous line was too long */
        if (p->ep)
            p->skip = FALSE; /* found newline; next line is normal */
    } else {
        if (!p->ep) {  /* newline missing */
            if (strlen(p->inbuf) < (p->inbufsz - 2)) {
                /* likely the last line of file is just
                   missing a newline; process it anyway  */
                p->ep = eos(p->inbuf);
            } else {
                config_error_add("Line too long, skipping");
                p->skip = TRUE; /* discard next fgets */
            }
        } else {
            *p->ep = '\0'; /* remove newline */
        }
        if (p->ep) {
            char *tmpbuf = (char *) 0;
            int len;
            boolean ignoreline = FALSE;
            boolean oldline = FALSE;

            /* line continuation (trailing '\') */
            p->morelines = (--p->ep >= p->inbuf && *p->ep == '\\');
            if (p->morelines)
                *p->ep = '\0';

            /* trim off spaces at end of line */
            while (p->ep >= p->inbuf
                   && (*p->ep == ' ' || *p->ep == '\t' || *p->ep == '\r'))
                *p->ep-- = '\0';

            if (!config_error_nextline(p->inbuf)) {
                p->rv = FALSE;
                if (p->buf)
                    free(p->buf), p->buf = (char *) 0;
                p->pbreak = TRUE;
                return;
            }

            p->ep = p->inbuf;
            while (*p->ep == ' ' || *p->ep == '\t')
                ++p->ep;

            /* ignore empty lines and full-line comment lines */
            if (!*p->ep || *p->ep == '#')
                ignoreline = TRUE;

            if (p->buf)
                oldline = TRUE;

            /* merge now read line with previous ones, if necessary */
            if (!ignoreline) {
                len = (int) strlen(p->ep) + 1; /* +1: final '\0' */
                if (p->buf)
                    len += (int) strlen(p->buf) + 1; /* +1: space */
                tmpbuf = (char *) alloc(len);
                *tmpbuf = '\0';
                if (p->buf) {
                    Strcat(strcpy(tmpbuf, p->buf), " ");
                    free(p->buf), p->buf = 0;
                }
                p->buf = strcat(tmpbuf, p->ep);
                if (strlen(p->buf) >= p->inbufsz)
                    p->buf[p->inbufsz - 1] = '\0';
            }

            if (p->morelines || (ignoreline && !oldline))
                return;

            if (handle_config_section(p->buf)) {
                free(p->buf), p->buf = (char *) 0;
                return;
            }

            /* from here onwards, we'll handle buf only */

            if (match_varname(p->buf, "CHOOSE", 6)) {
                char *section;
                char *bufp = find_optparam(p->buf);

                if (!bufp) {
                    config_error_add("Format is CHOOSE=section1"
                                     ",section2,...");
                    p->rv = FALSE;
                    free(p->buf), p->buf = (char *) 0;
                    return;
                }
                bufp++;
                if (gc.config_section_chosen)
                    free(gc.config_section_chosen),
                        gc.config_section_chosen = 0;
                section = choose_random_part(bufp, ',');
                if (section) {
                    gc.config_section_chosen = dupstr(section);
                } else {
                    config_error_add("No config section to choose");
                    p->rv = FALSE;
                }
                free(p->buf), p->buf = (char *) 0;
                return;
            }

            if (!(*proc)(p->buf))
                p->rv = FALSE;

            free(p->buf), p->buf = (char *) 0;
        }
    }
}

/**
 * @brief Read configuration from a string rather than a file.
 * @param str the text, which is not modified
 * @param proc what to do with each usable line
 * @note Exists so an environment variable and a command line can be parsed by the same code as a file. That is why the sectioning and continuation handling lives in the buffer routine and not in the file reader.
 * @note Takes a constant string and copies it, unlike the parsing of a line -- because an environment variable is not the game's to modify.
 */
/**
 * @brief 파일이 아니라 문자열에서 설정을 읽는다.
 * @param str 그 글. 바뀌지 않는다
 * @param proc 각 쓸모 있는 줄에 대해 무엇을 할지
 * @note 환경 변수와 명령줄이 파일과 같은 코드로 파싱될 수 있도록 존재한다. 그것이 절 나누기와 이어짐 처리가 파일 읽기가 아니라 버퍼 루틴에 사는 이유다.
 * @note 줄 파싱과 달리 상수 문자열을 받아 복사한다. 환경 변수는 게임이 바꿔도 되는 것이 아니기 때문이다.
 */
boolean
parse_conf_str(const char *str, boolean (*proc)(char *arg))
{
    size_t len;
    struct _cnf_parser_state parser;

    cnf_parser_init(&parser);
    free_config_sections();
    config_error_init(FALSE, "parse_conf_str", FALSE);
    while (str && *str) {
        len = 0;
        while (*str && len < (parser.inbufsz-1)) {
            parser.inbuf[len] = *str;
            len++;
            str++;
            if (parser.inbuf[len-1] == '\n')
                break;
        }
        parser.inbuf[len] = '\0';
        parse_conf_buf(&parser, proc);
        if (parser.pbreak)
            break;
    }
    cnf_parser_done(&parser);

    free_config_sections();
    config_error_done();
    return parser.rv;
}

/**
 * @brief Read configuration from an open file.
 * @param fp the file, already open
 * @param proc what to do with each usable line
 * @return whether the parse succeeded
 * @note Takes an already-open file rather than a name, so the caller decides where the file came from. That is what allows the same reading to serve the system file, the player's file and one named on the command
 *       line without this knowing which is which.
 */
/**
 * @brief 열린 파일에서 설정을 읽는다.
 * @param fp 이미 열린 그 파일
 * @param proc 각 쓸모 있는 줄에 대해 무엇을 할지
 * @return 파싱이 성공했는지
 * @note 이름이 아니라 이미 열린 파일을 받으므로, 호출자가 그 파일이 어디서 왔는지 정한다. 그것이 같은 읽기가 시스템 파일, 플레이어의 파일, 명령줄에서 지칭된 파일을 이것이 어느 것인지 모른 채로 맡을 수 있게 하는 것이다.
 */
/* parse_conf_file
 *
 * Read from file fp, calling parse_conf_buf for each line.
 */
boolean
parse_conf_file(FILE *fp, boolean (*proc)(char *arg))
{
    struct _cnf_parser_state parser;

    cnf_parser_init(&parser);
    free_config_sections();

    while (fgets(parser.inbuf, parser.inbufsz, fp)) {
        parse_conf_buf(&parser, proc);
        if (parser.pbreak)
            break;
    }
    cnf_parser_done(&parser);

    free_config_sections();
    return parser.rv;
}

DISABLE_WARNING_FORMAT_NONLITERAL

void
config_error_add(const char *str, ...)
{
    va_list the_args;

    va_start(the_args, str);
    vconfig_error_add(str, the_args);
    va_end(the_args);
}

staticfn void
vconfig_error_add(const char *str, va_list the_args)
{ /* start of vconf...() or of nested block in USE_OLDARG's conf...() */
    int vlen = 0;
    char buf[BIGBUFSZ]; /* will be chopped down to BUFSZ-1 if longer */

    vlen = vsnprintf(buf, sizeof buf, str, the_args);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED) && defined(DEBUG)
    if (vlen >= (int) sizeof buf)
        panic("%s: truncation of buffer at %zu of %d bytes",
              "config_error_add", sizeof buf, vlen);
#else
    nhUse(vlen);
#endif
    buf[BUFSZ - 1] = '\0';
    config_erradd(buf);
}

/**
 * @brief Read the player's own configuration, from wherever they put it.
 *
 * The player's settings may come from a file or from an environment variable, and both are read here in a fixed order so that later ones override earlier. Which order that is matters and is recorded with the
 * option phases rather than here.
 *
 * @note Reads the environment variable as well as the file, which is why this is not simply a call to the file reader. A player may set one option in their environment for a single session without editing their
 *       file, and that has to win.
 */
/**
 * @brief 플레이어 자신의 설정을 그가 둔 곳이 어디든 읽는다.
 *
 * 플레이어의 설정은 파일에서 올 수도 환경 변수에서 올 수도 있으며, 둘 다 정해진 순서로 여기서 읽힌다. 그래서 나중의 것이 앞의 것을 덮어쓴다. 그 순서가 무엇인지가 중요하고, 여기가 아니라 옵션 단계와 함께 기록되어 있다.
 *
 * @note 파일뿐 아니라 환경 변수도 읽으므로, 이것이 단순히 파일 읽기에 대한 호출이 아니다. 플레이어는 자기 파일을 편집하지 않고 한 세션만을 위해 환경에 옵션 하나를 설정할 수 있고, 그것이 이겨야 한다.
 */
#ifndef SFCTOOL
void
rcfile(void)
{
    char *opts = 0, *xtraopts = 0;
    const char *envname, *namesrc, *nameval;

    go.opt_phase = environ_opt;
    /* getenv() instead of nhgetenv(): let total length of options be long;
       parseoptions() will check each individually */
    envname = "NETHACKOPTIONS";
    opts = getenv(envname);
    if (!opts) {
        /* fall back to original name; discouraged */
        envname = "HACKOPTIONS";
        opts = getenv(envname);
    }

    if (gc.cmdline_rcfile) {
        namesrc = "command line";
        nameval = gc.cmdline_rcfile;
        xtraopts = opts;
        if (opts && (*opts == '/' || *opts == '\\' || *opts == '@'))
            xtraopts = 0; /* NETHACKOPTIONS is a file name; ignore it */
    } else if (opts && (*opts == '/' || *opts == '\\' || *opts == '@')) {
        /* NETHACKOPTIONS is a file name; use that instead of the default */
        if (*opts == '@')
            ++opts; /* @filename */
        namesrc = envname;
        nameval = opts;
        xtraopts = 0;
    } else {
        /* either no NETHACKOPTIONS or it wasn't a file name;
           read the default configuration file */
        nameval = namesrc = 0;
        xtraopts = opts;
    }

    go.opt_phase = rc_file_opt;
    /* seemingly arbitrary name length restriction is to prevent error
       messages, if any were to be delivered while accessing the file,
       from potentially overflowing buffers */
    if (nameval && (int) strlen(nameval) >= BUFSZ / 2) {
        config_error_init(TRUE, namesrc, FALSE);
        config_error_add(
            "nethackrc file name \"%.40s\"... too long; using default",
            nameval);
        config_error_done();
        nameval = namesrc = 0; /* revert to default nethackrc */
    }

    config_error_init(TRUE, nameval, nameval ? CONFIG_ERROR_SECURE : FALSE);
    (void) read_config_file(nameval, set_in_config);
    config_error_done();
    if (xtraopts) {
        /* NETHACKOPTIONS is present and not a file name */
        go.opt_phase = environ_opt;
        config_error_init(FALSE, envname, FALSE);
        (void) parseoptions(xtraopts, TRUE, FALSE);
        config_error_done();
    }

    if (gc.cmdline_rcfile)
        free((genericptr_t) gc.cmdline_rcfile), gc.cmdline_rcfile = 0;
    /*[end of nethackrc handling]*/
}


/**
 * @name Reading a configuration file more than once
 * @brief Narrow what the parser will act on, so a file can be read repeatedly for different purposes.
 *
 * A configuration file has to be read before the display exists, because it says which display to use. But most of what it contains cannot be applied until the display does exist. The resolution is to read the
 * same file more than once, attending to a different part each time.
 *
 * These are what make that possible. Everything can be disregarded and then one or two things heeded, so a pass that only wants to learn the display's name reads the file, finds it, and ignores the rest -- without
 * reporting the rest as errors, since they are not errors but simply not this pass's business.
 *
 * @note That is why suppressing unmatched-line errors belongs to this group. A pass that heeds one option would otherwise report every other line in the file as unrecognised.
 * @warning The narrowing is global and lasts until it is undone. A pass that narrows and does not widen again leaves the parser silently ignoring most of what it is given, which looks exactly like a configuration
 *          file having no effect.
 * @{
 */
/**
 * @name 설정 파일을 두 번 넘게 읽기
 * @brief 파서가 무엇에 따라 행동할지를 좁혀, 파일이 서로 다른 목적으로 되풀이해 읽힐 수 있게 한다.
 *
 * 설정 파일은 표시부가 존재하기 전에 읽혀야 한다. 그것이 어느 표시부를 쓸지 말하기 때문이다. 그런데 그것이 담은 것의 대부분은 표시부가 존재하기 전까지 적용될 수 없다. 그 해결은 같은 파일을 두 번 넘게 읽으며 매번 다른 부분에 주의하는 것이다.
 *
 * 이들이 그것을 가능하게 하는 것이다. 모든 것을 무시하게 한 뒤 하나나 둘만 따르게 할 수 있으므로, 표시부의 이름만 알고 싶은 통과가 그 파일을 읽고 그것을 찾고 나머지를 무시한다. 나머지를 오류로 알리지 않고. 그것들은 오류가 아니라 그저 이번 통과의 일이 아니기 때문이다.
 *
 * @note 그것이 일치하지 않는 줄에 대한 오류를 억제하는 것이 이 묶음에 속하는 이유다. 그러지 않으면 옵션 하나를 따르는 통과가 그 파일의 다른 모든 줄을 알 수 없는 것으로 알리게 된다.
 * @warning 그 좁히기는 전역이며 되돌려지기 전까지 지속된다. 좁히고 다시 넓히지 않은 통과는 파서가 자신이 받은 것의 대부분을 조용히 무시하는 상태로 남기며, 그것은 설정 파일이 아무 효과도 없는 것과 정확히 똑같아 보인다.
 * @{
 */
void
heed_all_config_statements(void)
{
    int i;

    for (i = 0; i < SIZE(disregarded_config_lines); i++) {
        disregarded_config_lines[i] = FALSE;
    }
}
void
disregard_all_config_statements(void)
{
    int i;

    for (i = 0; i < SIZE(disregarded_config_lines); i++) {
        disregarded_config_lines[i] = TRUE;
    }
}
void
heed_this_config_statement(int statement_idx)
{
    if (statement_idx >= 0 && statement_idx < SIZE(disregarded_config_lines))
        disregarded_config_lines[statement_idx] = FALSE;
}
void
disregard_this_config_statement(int statement_idx)
{
    if (statement_idx >= 0 && statement_idx < SIZE(disregarded_config_lines))
        disregarded_config_lines[statement_idx] = TRUE;
}

void
clear_ignore_errors_on_unmatched(void)
{
    ignore_errors_on_unmatched = FALSE;
}
void
set_ignore_errors_on_unmatched(void)
{
    ignore_errors_on_unmatched = TRUE;
}
boolean
config_unmatched_ignored(void)
{
    if (ignore_errors_on_unmatched)
        return TRUE;
    return FALSE;
}
/** @} */

/**
 * @brief Read the configuration for the sole purpose of learning which display and sound library to use.
 *
 * The first of the repeated passes, and the one that has to come before everything. It narrows the parser to exactly two options, reads the player's configuration, and widens again -- so the game learns what to
 * initialise from a file it cannot yet report errors about.
 *
 * @note It suppresses unmatched-line errors for the duration, because at this point almost every line in the file is one this pass is not looking at. Those lines will be read, and complained about if warranted, on
 *       a later pass.
 * @note It also disregards the two options afterwards, so the later pass does not act on them a second time -- the display has by then been chosen and changing it would be worse than ignoring the request.
 */
/**
 * @brief 어느 표시부와 사운드 라이브러리를 쓸지 알아내는 것만을 위해 설정을 읽는다.
 *
 * 되풀이되는 통과 중 첫 번째이며, 모든 것보다 먼저 와야 하는 것이다. 파서를 정확히 두 옵션으로 좁히고, 플레이어의 설정을 읽고, 다시 넓힌다. 그래서 게임이 아직 오류를 알릴 수 없는 파일에서 무엇을 초기화할지 배운다.
 *
 * @note 그 동안 일치하지 않는 줄에 대한 오류를 억제한다. 이 시점에서 그 파일의 거의 모든 줄이 이번 통과가 보고 있지 않은 것이기 때문이다. 그 줄들은 나중 통과에서 읽히고, 마땅하면 그때 불평받는다.
 * @note 그 뒤에 그 두 옵션을 무시하게도 한다. 그래서 나중 통과가 그것에 두 번 따르지 않는다. 그때쯤 표시부는 이미 골라져 있고 그것을 바꾸는 것은 그 요청을 무시하는 것보다 나쁘다.
 */
void
rcfile_interface_options(void)
{
    allopt_array_init();
    disregard_all_options();
    disregard_all_config_statements();
    heed_this_option(opt_windowtype);
    heed_this_option(opt_soundlib);
    set_ignore_errors_on_unmatched();
    ignore_statement_errors = TRUE;
    rcfile();
    heed_all_config_statements();
    heed_all_options();
    disregard_this_option(opt_windowtype);
    disregard_this_option(opt_soundlib);
    clear_ignore_errors_on_unmatched();
    ignore_statement_errors = FALSE;
}

void
rcfile_only_this_option(enum opt heeded_option)
{
    allopt_array_init();
    disregard_all_options();
    disregard_all_config_statements();
    heed_this_option(heeded_option);
    set_ignore_errors_on_unmatched();
    ignore_statement_errors = TRUE;
    rcfile();
    heed_all_config_statements();
    heed_all_options();
    clear_ignore_errors_on_unmatched();
    ignore_statement_errors = FALSE;
}

void
rcfile_only_this_statement(int statementid)
{
    allopt_array_init();
    disregard_all_options();
    disregard_all_config_statements();
    heed_this_config_statement(statementid);
    set_ignore_errors_on_unmatched();
    ignore_statement_errors = TRUE;
    rcfile();
    heed_all_config_statements();
    heed_all_options();
    clear_ignore_errors_on_unmatched();
    ignore_statement_errors = FALSE;
}

#ifdef WIN32
extern char portable_device_path[_MAX_PATH]; /* windsys.c */
extern boolean portable;

staticfn boolean
portable_sysconf_only_this_statement(int statementid)
{
    const char *exepath;
    char portable_sysconf[_MAX_PATH];

    exepath = windows_exepath();
    if (exepath) {
        Snprintf(portable_sysconf, sizeof portable_sysconf, "%s/sysconf",
                 exepath);
        if (portable_sysconf[0] && file_exists(portable_sysconf)) {
#ifdef SYSCF
#ifdef SYSCF_FILE
            allopt_array_init();
            disregard_all_options();
            disregard_all_config_statements();
            heed_this_config_statement(statementid);
            set_ignore_errors_on_unmatched();
            ignore_statement_errors = TRUE;

            config_error_init(TRUE, portable_sysconf, FALSE);
            go.opt_phase = syscf_opt;
            (void) read_config_file(portable_sysconf, set_in_sysconf);
            config_error_done();
            heed_all_config_statements();
            heed_all_options();
            clear_ignore_errors_on_unmatched();
            ignore_statement_errors = FALSE;
            if (sysopt.portable_device_paths) {
                Snprintf(portable_device_path, sizeof portable_device_path,
                         "%s\\", exepath);
                return TRUE;
            }
#endif
#endif /* SYSCF */
        }
    }
    return FALSE;
}

/**
 * @brief Whether this is a portable installation, by asking its own configuration.
 *
 * A portable installation keeps everything beside the program rather than in the system's usual places, and it says so in its own system configuration. But that configuration is in one of those places, so this is
 * a small circularity: the file has to be read to find out where files are kept.
 *
 * It is resolved by reading only the one statement that answers the question, from beside the program, before anything has decided where to look. Which is why this exists as its own pass rather than being part of
 * ordinary configuration reading.
 *
 * @return whether this installation is portable
 * @note Windows only, because that is where a program is commonly run from removable media.
 */
/**
 * @brief 이것이 휴대용 설치본인지, 그 자신의 설정에 물어서.
 *
 * 휴대용 설치본은 모든 것을 시스템의 통상적인 자리가 아니라 프로그램 곁에 보관하며, 자기 시스템 설정에 그렇다고 적어 둔다. 그런데 그 설정이 그 자리 중 하나에 있으므로, 이것은 작은 순환이다. 파일이 어디에 보관되는지 알아내려면 그 파일을 읽어야 한다.
 *
 * 그것은 그 질문에 답하는 단 하나의 문장만을, 프로그램 곁에서, 어디를 볼지 아무것도 정하기 전에 읽어서 해결된다. 그것이 이것이 평범한 설정 읽기의 일부가 아니라 자기 통과로 존재하는 이유다.
 *
 * @return 이 설치본이 휴대용인지
 * @note Windows 전용이다. 프로그램이 이동식 매체에서 흔히 실행되는 곳이 거기이기 때문이다.
 */
boolean
check_for_portable_config(void)
{
    int i, target_index = -1;

    for (i = 0; i < SIZE(config_line_stmt); i++) {
        if (!strcmp(config_line_stmt[i].name, "PORTABLE_DEVICE_PATHS")) {
            target_index = i;
            break;
        }
    }
    if (target_index >= 0) {
        return portable_sysconf_only_this_statement(target_index);
    }
    return FALSE;
}
#endif

#ifdef MSWIN_GRAPHICS
void
disregard_some_mswin_options(void)
{
    /* later for these */
    disregard_this_option(opt_map_mode);
    disregard_this_option(opt_font_map);
    disregard_this_option(opt_font_menu);
    disregard_this_option(opt_font_message);
    disregard_this_option(opt_font_status);

    disregard_this_option(opt_font_size_map);
    disregard_this_option(opt_font_size_menu);
    disregard_this_option(opt_font_size_message);
    disregard_this_option(opt_font_size_status);
}

void
rcfile_only_some_mswin_options(void)
{
    allopt_array_init();
    disregard_all_options();
    disregard_all_config_statements();
    heed_this_option(opt_map_mode);
    heed_this_option(opt_font_map);
    heed_this_option(opt_font_menu);
    heed_this_option(opt_font_message);
    heed_this_option(opt_font_status);

    heed_this_option(opt_font_size_map);
    heed_this_option(opt_font_size_menu);
    heed_this_option(opt_font_size_message);
    heed_this_option(opt_font_size_status);
    set_ignore_errors_on_unmatched();
    ignore_statement_errors = TRUE;
    rcfile();
    heed_all_config_statements();
    heed_all_options();
    clear_ignore_errors_on_unmatched();
    ignore_statement_errors = FALSE;
}
#endif /* MSWIN_GRAPHICS */

#endif /* SFCTOOL */

#ifdef SYSCF
#ifdef SYSCF_FILE
/**
 * @brief Verify that the system-wide configuration file can be read, and refuse to run if it cannot.
 *
 * The one configuration file whose absence is fatal. Everything else may be missing and the game plays with defaults, but a build that expects a system configuration has been told that an administrator decides
 * things -- who may debug, how many may play -- and running without those decisions would silently grant what the administrator withheld.
 *
 * @note It tests by opening rather than by asking about permissions, as the accompanying comment explains: what matters is the end result, and the several things that could prevent reading are not worth
 *       distinguishing when the outcome is the same.
 * @warning On one platform it locks the configuration directory as a side effect, because having found the file there it must not then be told to look elsewhere.
 */
/**
 * @brief 시스템 전체 설정 파일을 읽을 수 있는지 검증하고, 그럴 수 없으면 실행을 거부한다.
 *
 * 부재가 치명적인 유일한 설정 파일이다. 다른 모든 것은 없어도 되고 게임은 기본값으로 플레이된다. 그러나 시스템 설정을 기대하는 빌드는 관리자가 일을 정한다는 말을 들은 것이다. 누가 디버그해도 되는지, 몇 명이 플레이해도 되는지. 그리고 그 결정 없이 실행하는 것은 관리자가
 * 보류한 것을 조용히 허락하는 일이 된다.
 *
 * @note 권한을 묻는 것이 아니라 열어서 검사한다. 딸린 주석이 설명하듯 중요한 것은 최종 결과이고, 읽기를 막을 수 있는 여러 가지가 결과가 같을 때는 구별할 가치가 없다.
 * @warning 한 플랫폼에서는 부작용으로 설정 디렉토리를 잠근다. 그 파일을 거기서 찾았으므로 그 뒤에 다른 곳을 보라고 들어서는 안 되기 때문이다.
 */
void
assure_syscf_file(void)
{
    int fd;

#ifdef WIN32
    /* We are checking that the sysconf exists ... lock the path */
    fqn_prefix_locked[SYSCONFPREFIX] = TRUE;
#endif
    /*
     * All we really care about is the end result - can we read the file?
     * So just check that directly.
     *
     * Not tested on most of the old platforms (which don't attempt
     * to implement SYSCF).
     * Some ports don't like open()'s optional third argument;
     * VMS overrides open() usage with a macro which requires it.
     */
#ifndef VMS
#if defined(NOCWD_ASSUMPTIONS) && defined(WIN32)
    fd = open(fqname(SYSCF_FILE, SYSCONFPREFIX, 0), O_RDONLY);
#else
    fd = open(SYSCF_FILE, O_RDONLY);
#endif
#else   /* VMS */
    fd = open(SYSCF_FILE, O_RDONLY, 0);
#endif  /* VMS */
    if (fd >= 0) {
        /* readable */
        close(fd);
        return;
    }
#ifndef SFCTOOL
    if (gd.deferred_showpaths)
        do_deferred_showpaths(1); /* does not return */
#endif
    raw_printf("Unable to open SYSCF_FILE.\n");
    exit(EXIT_FAILURE);
}

#endif /* SYSCF_FILE */
#endif /* SYSCF */

/* ----------  END CONFIG FILE HANDLING ----------- */

/*cfgfiles.c*/

