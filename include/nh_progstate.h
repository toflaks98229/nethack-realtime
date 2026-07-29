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
struct levelstatus {
    int making;                 /* makelevel has begun */
    int loading;                /* level loading has begun */
    int shkready;               /* shops ready */
    int ready;                  /* level is ready */
};

/* value of program_state.input_state, significant during readchar();
   get_count() expects digits then a command so sets it to commandInp */
enum InputState {
    otherInp   = 0, /* 'other' */
    commandInp = 1, /* readchar() */
    getposInp  = 2, /* getpos() */
    getdirInp  = 3, /* getdir() */
};

#endif /* NH_PROGSTATE_H */
