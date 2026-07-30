/* NetHack 5.0	xwindow.h	$NHDT-Date: 1781973092 2026/06/20 16:31:32 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.13 $ */
/* Copyright (c) Dean Luick, 1992                                 */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file xwindow.h
 * @brief The public half of the X11 map widget.
 *
 * An Xt widget is declared in two headers, and this is the one callers may use:
 * the class pointer, the resource names, and the callback the widget invokes.
 * The instance and class records live in @c xwindowp.h, which only the widget's
 * own implementation should include.
 *
 * The resource table below is the widget's interface as X11 understands it: those
 * names are what an X resource file may set, so they are documentation of a
 * configuration surface as much as of the code.
 *
 * @note The map is measured in rows and columns rather than pixels, which is why
 *       those appear as resources alongside the inherited geometry ones.
 */

/**
 * @file xwindow.h
 * @brief X11 맵 위젯의 공개 절반.
 *
 * Xt 위젯은 두 헤더로 선언되며, 이것이 호출자가 써도 되는 쪽이다. 클래스 포인터, 리소스
 * 이름, 위젯이 호출하는 콜백이다. 인스턴스와 클래스 레코드는 @c xwindowp.h 에 있고, 그것은
 * 위젯 자신의 구현만 포함해야 한다.
 *
 * 아래 리소스 표는 X11 이 이해하는 이 위젯의 인터페이스다. 그 이름들은 X 리소스 파일이
 * 설정할 수 있는 것이므로, 코드에 대한 문서인 만큼 설정 표면에 대한 문서이기도 하다.
 *
 * @note 맵은 픽셀이 아니라 행과 열로 측정된다. 그래서 상속된 기하 리소스들과 나란히 그것들이
 *       리소스로 나타난다.
 */

#ifndef _xwindow_h
#define _xwindow_h

/****************************************************************
 *
 * Window widget
 *
 ****************************************************************/

/* Resources:

 Name                Class              RepType         Default Value
 ----                -----              -------         -------------
 background          Background         Pixel           XtDefaultBackground
 border              BorderColor        Pixel           XtDefaultForeground
 borderWidth         BorderWidth        Dimension       1
 destroyCallback     Callback           Pointer         NULL
 height              Height             Dimension       0
 mappedWhenManaged   MappedWhenManaged  Boolean         True
 sensitive           Sensitive          Boolean         True
 width               Width              Dimension       0
 x                   Position           Position        0
 y                   Position           Position        0

 rows                Width              Dimension       21
 columns             Height             Dimension       80
 foreground          Color              Pixel           XtDefaultForeground

 black               Color              Pixel           "black"
 red                 Color              Pixel           "red"
 green               Color              Pixel           "pale green"
 brown               Color              Pixel           "brown"
 blue                Color              Pixel           "blue"
 magenta             Color              Pixel           "magenta"
 cyan                Color              Pixel           "light cyan"
 gray                Color              Pixel           "gray"
 //no color//
 orange              Color              Pixel           "orange"
 bright_green        Color              Pixel           "green"
 yellow              Color              Pixel           "yellow"
 bright_blue         Color              Pixel           "royal blue"
 bright_magenta      Color              Pixel           "violet"
 bright_cyan         Color              Pixel           "cyan"
 white               Color              Pixel           "white"

 font                Font               XFontStruct*    XtDefaultFont
 exposeCallback      Callback           Callback        NULL
 callback            Callback           Callback        NULL
 resizeCallback      Callback           Callback        NULL
*/

/* define any special resource names here that are not in <X11/StringDefs.h>
 */

#define XtNrows "rows"
#define XtNcolumns "columns"
#define XtNblack "black"
#define XtNred "red"
#define XtNgreen "green"
#define XtNbrown "brown"
#define XtNblue "blue"
#define XtNmagenta "magenta"
#define XtNcyan "cyan"
#define XtNgray "gray"
#define XtNorange "orange"
#define XtNbright_green "bright_green"
#define XtNyellow "yellow"
#define XtNbright_blue "bright_blue"
#define XtNbright_magenta "bright_magenta"
#define XtNbright_cyan "bright_cyan"
#define XtNwhite "white"
#define XtNexposeCallback "exposeCallback"
#define XtNresizeCallback "resizeCallback"

extern XFontStruct *WindowFontStruct(Widget);
extern Font WindowFont(Widget);

#define XtCWindowResource "WindowResource"
#define XtCRows "Rows"
#define XtCColumns "Columns"

/* declare specific WindowWidget class and instance datatypes */

typedef struct _WindowClassRec *WindowWidgetClass;
typedef struct _WindowRec *WindowWidget;

/* declare the class constant */

extern WidgetClass windowWidgetClass;

#endif /* _xwindow_h */
