/* NetHack 5.0	xwindowp.h	$NHDT-Date: 1781973093 2026/06/20 16:31:33 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.12 $ */
/* Copyright (c) Dean Luick, 1992                                 */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file xwindowp.h
 * @brief The private half of the X11 map widget, as Xt requires it.
 *
 * Xt widgets are declared in two headers: a public one describing what callers
 * may use, and this one describing the instance and class records, which only the
 * widget's own implementation should see. Including it elsewhere would expose
 * fields that are not part of the interface.
 *
 * @note The dance around @c SYSV exists because X11's headers define it for their
 *       own purposes, and defining it changes how the game's own headers compile;
 *       it is saved and restored rather than left changed.
 * @warning Include order is load-bearing here -- the superclass private header
 *          must arrive between those two steps -- so this file is not safe to
 *          reorder.
 */

/**
 * @file xwindowp.h
 * @brief X11 맵 위젯의 비공개 절반. Xt 가 요구하는 형태다.
 *
 * Xt 위젯은 두 헤더로 선언된다. 호출자가 써도 되는 것을 기술하는 공개 헤더와, 인스턴스와
 * 클래스 레코드를 기술하는 이 헤더다. 후자는 그 위젯의 구현만이 보아야 한다. 다른 곳에서
 * 포함하면 인터페이스에 속하지 않는 필드가 드러난다.
 *
 * @note @c SYSV 를 두고 벌이는 조작은 X11 헤더가 자기 목적으로 그것을 정의하기 때문이며,
 *       그것이 정의되면 게임 자신의 헤더가 컴파일되는 방식이 달라진다. 그래서 바뀐 채로
 *       두지 않고 저장했다가 되돌린다.
 * @warning 여기서는 포함 순서가 동작을 좌우한다. 상위 클래스의 비공개 헤더가 그 두 단계
 *          사이에 와야 하므로, 이 파일은 재배치해서는 안 된다.
 */

#ifndef _xwindowp_h
#define _xwindowp_h

#include "xwindow.h"

#ifndef SYSV
#define PRESERVE_NO_SYSV /* X11 include files may define SYSV */
#endif

/* include superclass private header file */
#include <X11/CoreP.h>

#ifdef PRESERVE_NO_SYSV
#ifdef SYSV
#undef SYSV
#endif
#undef PRESERVE_NO_SYSV
#endif

/* define unique representation types not found in <X11/StringDefs.h> */

#define XtRWindowResource "WindowResource"

typedef struct {
    int empty;
} WindowClassPart;

typedef struct _WindowClassRec {
    CoreClassPart core_class;
    WindowClassPart window_class;
} WindowClassRec;

extern WindowClassRec windowClassRec;

typedef struct {
    /* resources */
    Dimension rows;
    Dimension columns;
    Pixel foreground;
    Pixel black;
    Pixel red;
    Pixel green;
    Pixel brown;
    Pixel blue;
    Pixel magenta;
    Pixel cyan;
    Pixel gray;
    Pixel orange;
    Pixel bright_green;
    Pixel yellow;
    Pixel bright_blue;
    Pixel bright_magenta;
    Pixel bright_cyan;
    Pixel white;
    XFontStruct *font;
    XtCallbackList expose_callback;
    XtCallbackList input_callback;
    XtCallbackList resize_callback;
    /* private state */
    /* (none) */
} WindowPart;

typedef struct _WindowRec {
    CorePart core;
    WindowPart window;
} WindowRec;

#endif /* _xwindowp_h */
