/* NetHack 5.0	nh_progstate.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Extracted 2026-07 from hack.h for the real-time fork (see MODIFICATIONS.md).
   Content is unchanged; hack.h includes this header at the point the
   definitions used to occupy, so preprocessing order is identical. */

/**
 * @file nh_progstate.h
 * @brief Where the program currently is: interpreter phase, level phase, and
 *        what the next keystroke means.
 *
 * These flags say what the process is *doing right now* -- saving, restoring,
 * panicking, building a level, sitting in a menu -- so that code reached from
 * many directions can behave appropriately, most importantly by refusing to
 * recurse or to report problems while already handling one.
 *
 * @note None of this is saved or restored; it describes the running process,
 *       not the game being played.
 * @warning @c panicking, @c exiting, and @c in_impossible guard error paths.
 *          Code that runs during teardown must consult them rather than assume
 *          a healthy game state.
 * @note Extracted verbatim from @c hack.h; include @c hack.h rather than this
 *       header directly, which is what every existing source file does.
 */

/**
 * @file nh_progstate.h
 * @brief 프로그램의 현재 위치: 인터프리터 단계, 레벨 단계, 다음 키 입력의 의미.
 *
 * 이 플래그들은 프로세스가 *지금 무엇을 하는 중인지* -- 저장, 복원, 패닉, 레벨
 * 생성, 메뉴 대기 -- 를 알려 준다. 여러 경로에서 도달하는 코드가 그에 맞게
 * 행동하도록 하며, 특히 이미 처리 중인 상황에서 재귀하거나 문제를 다시
 * 보고하는 일을 막는 데 쓰인다.
 *
 * @note 이 값들은 저장·복원되지 않는다. 진행 중인 게임이 아니라 실행 중인
 *       프로세스를 서술한다.
 * @warning @c panicking, @c exiting, @c in_impossible 은 오류 경로를 보호한다.
 *          종료 처리 중 실행되는 코드는 게임 상태가 정상이라고 가정하지 말고
 *          이 값들을 확인해야 한다.
 * @note @c hack.h 에서 그대로 추출했다. 기존 모든 소스 파일이 그렇듯 이 헤더를
 *       직접 포함하지 말고 @c hack.h 를 포함할 것.
 */

#ifndef NH_PROGSTATE_H
#define NH_PROGSTATE_H

/* structure for 'program_state'; not saved and restored */

/**
 * @brief What the process is doing at this instant.
 *
 * Most members are re-entrancy guards. Saving, restoring, level building, and
 * error reporting are all reachable from many directions, and several of them
 * can be provoked *by* the code they would run; the flags let such a path
 * detect that it is already inside one of these phases and step aside.
 *
 * @note Not saved or restored -- this describes the running program, not the
 *       game being played.
 * @warning During teardown (@c panicking, @c exiting, @c gameover) the game
 *          state may be partially destroyed; code that can run then must check
 *          rather than assume a consistent world.
 */
/**
 * @brief 이 순간 프로세스가 하고 있는 일.
 *
 * 대부분의 멤버는 재진입 방지 장치다. 저장, 복원, 레벨 생성, 오류 보고는 모두
 * 여러 경로에서 도달할 수 있고, 그중 일부는 자신이 실행할 코드 *때문에*
 * 유발되기도 한다. 이 플래그들은 그런 경로가 이미 해당 단계 안에 있음을 알아채고
 * 물러날 수 있게 한다.
 *
 * @note 저장·복원되지 않는다. 진행 중인 게임이 아니라 실행 중인 프로그램을
 *       서술한다.
 * @warning 종료 처리 중(@c panicking, @c exiting, @c gameover)에는 게임 상태가
 *          일부 파괴되어 있을 수 있다. 그때 실행될 수 있는 코드는 일관된 세계를
 *          가정하지 말고 확인해야 한다.
 */
struct sinfo {
    int gameover;               /* self-explanatory? */
    int stopprint;              /* inhibit further end of game disclosure */
#ifdef HANGUPHANDLING
    volatile int done_hup;      /* SIGHUP or moral equivalent received
                                 * -- no more screen output */
    int preserve_locks;         /* don't remove level files prior to exit */
#endif
    int something_worth_saving; /* in case of panic */
    int panicking;              /* `panic' is in progress */
    int exiting;                /* an exit handler is executing */
    int saving;                 /* creating a save file */
    int restoring;              /* reloading a save file */
    int freeingdata;            /* in saveobjchn(), mode FREEING */
    int in_getlev;              /* in getlev() */
    int in_moveloop;            /* normal gameplay in progress */
    int in_impossible;          /* reporting a warning */
    int in_docrt;               /* in docrt(): redrawing the whole screen */
    int in_self_recover;        /* processing orphaned level files */
    int in_checkpoint;          /* saving insurance checkpoint */
    int in_parseoptions;        /* in parseoptions */
    int in_role_selection;      /* role/race/&c selection menus in progress */
    int in_getlin;              /* inside interface getlin routine */
    int in_sanity_check;        /* for impossible() during sanity checking */
    int config_error_ready;     /* config_error_add is ready, available */
    int beyond_savefile_load;   /* set when past savefile loading */
    int savefile_completed;     /* savefile has completed writing */
    int reading_bonesfile;      /* in the midst of trying to read bones file */
#ifdef PANICLOG
    int in_paniclog;            /* writing a panicloc entry */
#endif
    int wizkit_wishing;         /* starting wizard mode game w/ WIZKIT file */
    /* input_state:  used in the core for the 'altmeta' option to process ESC;
       used in the curses interface to avoid arrow keys when user is doing
       something other than entering a command or direction and in the Qt
       interface to suppress menu commands in similar conditions;
       readchar() always resets it to 'otherInp' prior to returning */
    int input_state; /* whether next key pressed will be entering a command */
    int early_options; /* inside early_options processing */
#ifdef TTY_GRAPHICS
    /* resize_pending only matters when handling a SIGWINCH signal for tty;
       getting_char is used along with that and also separately for UNIX;
       we minimize #if conditionals for them to avoid unnecessary clutter */
    volatile int resize_pending; /* set by signal handler */
    volatile int getting_char;  /* referenced during signal handling */
#endif
};

/* structure for current 'level_status'; not saved and restored */

/**
 * @brief How far along the current level's construction has got.
 *
 * A level passes through generation, loading, and shop setup before it is fit
 * to play. Code that can be reached during those phases -- monster placement,
 * display updates -- uses these to tell a half-built level from a live one.
 *
 * @note Not saved or restored; rebuilt as each level is entered.
 */
/**
 * @brief 현재 레벨의 구축이 어디까지 진행되었는지.
 *
 * 레벨은 생성, 적재, 상점 설정을 거쳐야 플레이 가능한 상태가 된다. 그 단계
 * 중에 도달할 수 있는 코드 -- 몬스터 배치, 화면 갱신 -- 는 이 값들로 절반만
 * 지어진 레벨과 정상 동작 중인 레벨을 구분한다.
 *
 * @note 저장·복원되지 않으며, 레벨에 들어갈 때마다 다시 설정된다.
 */
struct levelstatus {
    int making;                 /* makelevel has begun */
    int loading;                /* level loading has begun */
    int shkready;               /* shops ready */
    int ready;                  /* level is ready */
};

/* value of program_state.input_state, significant during readchar();
   get_count() expects digits then a command so sets it to commandInp */

/**
 * @brief What the next keystroke will be interpreted as.
 *
 * Interfaces need this because the same physical key must mean different
 * things in different contexts: the curses port suppresses arrow keys unless a
 * command or direction is expected, Qt suppresses menu accelerators similarly,
 * and the core uses it to decide how to treat ESC under the @c altmeta option.
 *
 * @warning @c readchar() resets this to @c otherInp before returning, so it is
 *          only meaningful for the keystroke currently being read.
 */
/**
 * @brief 다음 키 입력이 무엇으로 해석될지.
 *
 * 인터페이스가 이 값을 필요로 하는 이유는 같은 물리 키가 맥락에 따라 다른 것을
 * 뜻해야 하기 때문이다. curses 포트는 명령이나 방향을 기대하는 때가 아니면
 * 화살표 키를 억제하고, Qt 도 메뉴 단축키를 비슷하게 억제하며, 코어는
 * @c altmeta 옵션에서 ESC 를 어떻게 다룰지 결정하는 데 쓴다.
 *
 * @warning @c readchar() 는 반환 직전 이 값을 @c otherInp 로 되돌린다. 따라서
 *          지금 읽고 있는 키 입력에 대해서만 의미가 있다.
 */
enum InputState {
    otherInp   = 0, /* 'other' */
    commandInp = 1, /* readchar() */
    getposInp  = 2, /* getpos() */
    getdirInp  = 3, /* getdir() */
};

#endif /* NH_PROGSTATE_H */
