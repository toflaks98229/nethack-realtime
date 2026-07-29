/* NetHack 5.0	mhmap.c	$NHDT-Date: 1781973103 2026/06/20 16:31:43 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.115 $ */
/* Copyright (C) 2001 by Alex Kompel      */
/* NetHack may be freely redistributed.  See license for details. */
/* MODIFIED 2026-07 (real-time fork): added smooth-camera pan interpolation
   (REALTIME_PROTO) so the tile view slides instead of snapping when the hero
   moves; see MODIFICATIONS.md.  This file differs from the upstream NetHack. */

#include "win10.h"
#include "winMS.h"
#include "winos.h"

#include "mhfont.h"
#include "mhinput.h"
#include "mhmap.h"
#include "mhmsg.h"
#include "resource.h"

#include "color.h"
#if !defined(PATCHLEVEL_H)
#include "patchlevel.h"
#endif

#define NHMAP_FONT_NAME TEXT("Terminal")
#define NHMAP_TTFONT_NAME TEXT("Consolas")
#define MAXWINDOWTEXT 255

#define CURSOR_BLINK_INTERVAL 1000 // milliseconds
#define CURSOR_HEIGHT 2 // pixels

#ifdef REALTIME_PROTO
/* smooth-camera pan (stage 3).  The cursor-blink timer uses id 0; the pan
   animation uses a distinct id and fires every RT_CAM_FRAME_MS while active.
   A pan completes in about RT_TURN_MS (from config.h), matching game pace. */
#define RT_CAM_TIMER_ID 1
#define RT_CAM_FRAME_MS 16 /* ~60 fps while panning */
#define RT_CAM_MAX_TILES 2 /* clamp pan lag to this many tiles */
#endif

#define TILEBMP_X(ntile) \
    ((ntile % GetNHApp()->mapTilesPerLine) * GetNHApp()->mapTile_X)
#define TILEBMP_Y(ntile) \
    ((ntile / GetNHApp()->mapTilesPerLine) * GetNHApp()->mapTile_Y)

/* map window data */
typedef struct mswin_nethack_map_window {
    HWND hWnd;                  /* window */

    glyph_info map[COLNO][ROWNO];
    glyph_info bkmap[COLNO][ROWNO];
    boolean locDirty[COLNO][ROWNO]; /* dirty flag for map location */
    boolean mapDirty;           /* one or more map locations are dirty */
    int mapMode;                /* current map mode */
    boolean bAsciiMode;         /* switch ASCII/tiled mode */
    boolean bFitToScreenMode;   /* switch Fit map to screen mode on/off */
    int xPos, yPos;             /* scroll position */
    int xPageSize, yPageSize;   /* scroll page size */
    int xMin, xMax, yMin, yMax; /* scroll range */
    int xCur, yCur;             /* position of the cursor */
    int xFrontTile, yFrontTile; /* size of tile in front buffer in pixels */
    int xBackTile, yBackTile;   /* size of tile in back buffer in pixels */
    POINT map_orig;             /* map origin point */

    HFONT hMapFont;             /* font for ASCII mode */
    HFONT hMapFontUnicode;      /* font for Unicode mode */
    boolean bUnicodeFont;       /* font supports unicode page 437 */

    int tileWidth;              /* width of tile in pixels at 96 dpi */
    int tileHeight;             /* height of tile in pixels at 96 dpi */
    double backScale;           /* scaling from source to back buffer */
    double frontScale;          /* scaling from back to front */
    double monitorScale;        /* from 96dpi to monitor dpi*/

    boolean cursorOn;
    int yNoBlinkCursor;         /* non-blinking cursor height inback buffer
                                   in pixels */
    int yBlinkCursor;           /* blinking cursor height inback buffer
                                   in pixels */

    int backWidth;              /* back buffer width */
    int backHeight;             /* back buffer height */
    HBITMAP hBackBuffer;        /* back buffe bitmap */
    HDC backBufferDC;          /* back buffer drawing context */

    HDC tileDC;                /* tile drawing context */

#ifdef REALTIME_PROTO
    /* smooth-camera state (stage 3): when the hero moves, the scroll origin
       jumps by whole tiles; we render at a fractional pixel offset that
       decays to zero so the view pans smoothly instead of snapping */
    int camDX, camDY;           /* current pan offset, front-buffer pixels */
    int camLastXPos, camLastYPos; /* scroll pos observed at last pan seed */
    boolean camInit;            /* camLast{X,Y}Pos have been initialized */
    boolean camAnimating;       /* pan timer is currently running */

    /* hero sprite slide (stage 3): when the view does NOT scroll, the hero
       tile itself glides between cells.  We keep a decaying overlay offset
       and, while it is active, redraw the terrain over the hero's cell and
       draw the hero as a moving overlay in onPaint. */
    int heroDX, heroDY;         /* hero overlay offset, front-buffer pixels */
    int heroLastX, heroLastY;   /* hero cell observed at last slide seed */
    boolean heroInit;           /* heroLast{X,Y} initialized */
    boolean heroSlideActive;    /* hero is mid-glide */
    boolean monSliding;         /* at least one monster was mid-glide at the
                                   last paint; keeps the frame timer alive
                                   even though monsters carry no offset here
                                   (their motion lives in the core records) */
#endif
} NHMapWindow, *PNHMapWindow;

static TCHAR szNHMapWindowClass[] = TEXT("MSNethackMapWndClass");
LRESULT CALLBACK MapWndProc(HWND, UINT, WPARAM, LPARAM);
static void register_map_window_class(void);
static void onMSNHCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void onMSNH_VScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void onMSNH_HScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void onPaint(HWND hWnd);
static void onCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void nhcoord2display(PNHMapWindow data, int x, int y, LPRECT lpOut);
static void paint(PNHMapWindow data, int i, int j);
static void dirtyAll(PNHMapWindow data);
static void dirty(PNHMapWindow data, int i, int j);
static void setGlyph(PNHMapWindow data, int i, int j,
                     const glyph_info *fg, const glyph_info *bg);
static void clearAll(PNHMapWindow data);
#ifdef REALTIME_PROTO
static void rt_drive_hero(HWND hWnd);
#endif

#if (VERSION_MAJOR < 4) && (VERSION_MINOR < 4) && (PATCHLEVEL < 2)
static void nhglyph2charcolor(short glyph, uchar *ch, int *color);
#endif
extern boolean win32_cursorblink;       /* from sys\windows\windsys.c */
#ifdef REALTIME_PROTO
extern boolean mswin_rt_awaiting_cmd;   /* from win\win32\mswproc.c */
#endif

HWND
mswin_init_map_window(void)
{
    static int run_once = 0;
    HWND hWnd;
    RECT rt;

    if (!run_once) {
        register_map_window_class();
        run_once = 1;
    }

    /* get window position */
    if (GetNHApp()->bAutoLayout) {
        SetRect(&rt, 0, 0, 0, 0);
    } else {
        mswin_get_window_placement(NHW_MAP, &rt);
    }

    /* create map window object */
    hWnd = CreateWindow(
        szNHMapWindowClass, /* registered class name */
        NULL,               /* window name */
        WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_CLIPSIBLINGS
            | WS_SIZEBOX,     /* window style */
        rt.left,              /* horizontal position of window */
        rt.top,               /* vertical position of window */
        rt.right - rt.left,   /* window width */
        rt.bottom - rt.top,   /* window height */
        GetNHApp()->hMainWnd, /* handle to parent or owner window */
        NULL,                 /* menu handle or child identifier */
        GetNHApp()->hApp,     /* handle to application instance */
        NULL);                /* window-creation data */
    if (!hWnd) {
        panic("Cannot create map window");
    }

    /* Set window caption */
    SetWindowText(hWnd, "Map");

    mswin_apply_window_style(hWnd);

    /* set cursor blink timer */
    SetTimer(hWnd, 0, CURSOR_BLINK_INTERVAL, NULL);
#ifdef REALTIME_PROTO
    /* In real-time play the map has a frame rate, not just occasional redraws:
       held input has to be sampled and continuous positions advanced whether or
       not anything happened to dirty the map.  So the frame timer runs for the
       lifetime of the window rather than being started and stopped around
       individual animations. */
    SetTimer(hWnd, RT_CAM_TIMER_ID, RT_CAM_FRAME_MS, NULL);
#endif

    return hWnd;
}

void
mswin_map_layout(HWND hWnd, LPSIZE map_size)
{
    /* check arguments */
    if (!IsWindow(hWnd) || !map_size || map_size->cx <= 0
        || map_size->cy <= 0)
        return;

    PNHMapWindow data = (PNHMapWindow) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    /* calculate window size */
    RECT client_rt;
    GetClientRect(hWnd, &client_rt);

    SIZE wnd_size;
    wnd_size.cx = client_rt.right - client_rt.left;
    wnd_size.cy = client_rt.bottom - client_rt.top;

    // calculate back buffer scale
    data->monitorScale = win10_monitor_scale(hWnd);

    boolean bText = data->bAsciiMode ||
                    (u.uz.dlevel != 0 && Is_rogue_level(&u.uz));

    if (bText && !data->bFitToScreenMode)
        data->backScale = data->monitorScale;
    else
        data->backScale = 1.0;

    /* set back buffer tile size */
    if (bText && data->bFitToScreenMode) {
        data->xBackTile = wnd_size.cx / COLNO;
        data->yBackTile = wnd_size.cy / ROWNO;
        data->yBackTile = max(data->yBackTile, 12);
    } else {
        data->xBackTile = (int)(data->tileWidth * data->backScale);
        data->yBackTile = (int)(data->tileHeight * data->backScale);
    }

    if (bText) {
        LOGFONT lgfnt;

        ZeroMemory(&lgfnt, sizeof(lgfnt));
        if (data->bFitToScreenMode) {
            lgfnt.lfHeight = -data->yBackTile;     // height of font
            lgfnt.lfWidth = 0;                     // average character width
        } else {
            lgfnt.lfHeight = -data->yBackTile;     // height of font
            lgfnt.lfWidth = -data->xBackTile;      // average character width
        }
        lgfnt.lfEscapement = 0;                    // angle of escapement
        lgfnt.lfOrientation = 0;                   // base-line orientation angle
        lgfnt.lfWeight = FW_NORMAL;                // font weight
        lgfnt.lfItalic = FALSE;                    // italic attribute option
        lgfnt.lfUnderline = FALSE;                 // underline attribute option
        lgfnt.lfStrikeOut = FALSE;                 // strikeout attribute option
        lgfnt.lfCharSet = mswin_charset();         // character set identifier
        lgfnt.lfOutPrecision = OUT_DEFAULT_PRECIS; // output precision
        lgfnt.lfClipPrecision = CLIP_DEFAULT_PRECIS; // clipping precision
        if (data->bFitToScreenMode) {
            lgfnt.lfQuality = ANTIALIASED_QUALITY; // output quality
        } else {
            lgfnt.lfQuality = NONANTIALIASED_QUALITY; // output quality
        }
        if (iflags.wc_font_map && *iflags.wc_font_map) {
            lgfnt.lfPitchAndFamily = DEFAULT_PITCH; // pitch and family
            NH_A2W(iflags.wc_font_map, lgfnt.lfFaceName, LF_FACESIZE);
        } else {
            if (!data->bFitToScreenMode) {
                lgfnt.lfPitchAndFamily = FIXED_PITCH; // pitch and family
                NH_A2W(NHMAP_FONT_NAME, lgfnt.lfFaceName, LF_FACESIZE);
            } else {
                lgfnt.lfPitchAndFamily = DEFAULT_PITCH; // pitch and family
                NH_A2W(NHMAP_TTFONT_NAME, lgfnt.lfFaceName, LF_FACESIZE);
            }
        }

        TEXTMETRIC textMetrics;
        HFONT font = NULL;

        while (1) {

            if (font != NULL)
                DeleteObject(font);

            font = CreateFontIndirect(&lgfnt);

            SelectObject(data->backBufferDC, font);

            GetTextMetrics(data->backBufferDC, &textMetrics);

            if (!data->bFitToScreenMode)
                break;

            if ((textMetrics.tmHeight > data->yBackTile ||
                 textMetrics.tmAveCharWidth > data->xBackTile) &&
                lgfnt.lfHeight < -MIN_FONT_HEIGHT) {
                lgfnt.lfHeight++;
                continue;
            }

            break;
        }

        if (data->hMapFont)
            DeleteObject(data->hMapFont);

        data->hMapFont = font;

        data->bUnicodeFont = winos_font_support_cp437(data->hMapFont);

        // Same as above, but with ANSI_CHARSET for IBM and Unicode modes
        lgfnt.lfCharSet = ANSI_CHARSET;
        if (data->hMapFontUnicode)
            DeleteObject(data->hMapFontUnicode);
        data->hMapFontUnicode = CreateFontIndirect(&lgfnt);

        // set tile size to match font metrics

        data->xBackTile = textMetrics.tmAveCharWidth;
        data->yBackTile = textMetrics.tmHeight;

    }

    int backWidth = COLNO * data->xBackTile;
    int backHeight = ROWNO * data->yBackTile;

    /* create back buffer */

    if (data->backWidth != backWidth || data->backHeight != backHeight) {

        HDC frontBufferDC = GetDC(hWnd);
        HBITMAP hBackBuffer = CreateCompatibleBitmap(frontBufferDC, backWidth, backHeight);
        ReleaseDC(hWnd, frontBufferDC);

        if (data->hBackBuffer != NULL) {
            SelectBitmap(data->backBufferDC, hBackBuffer);
            DeleteObject(data->hBackBuffer);
        }

        data->backWidth = backWidth;
        data->backHeight = backHeight;

        SelectBitmap(data->backBufferDC, hBackBuffer);
        data->hBackBuffer = hBackBuffer;
    }

    /* calculate front buffer tile size */

    if (wnd_size.cx > 0 && wnd_size.cy > 0 && !bText && data->bFitToScreenMode) {
        double windowAspectRatio =
            (double) wnd_size.cx / (double) wnd_size.cy;

        double backAspectRatio =
            (double) data->backWidth / (double) data->backHeight;

        if (windowAspectRatio > backAspectRatio)
            data->frontScale = (double) wnd_size.cy / (double) data->backHeight;
        else
            data->frontScale = (double) wnd_size.cx / (double) data->backWidth;

    } else {

        if (bText) {
            data->frontScale = 1.0;
        } else {
            data->frontScale = data->monitorScale;
        }

    }

    /* TODO: Should we round instead of clamping? */
    data->xFrontTile = (int) ((double) data->xBackTile * data->frontScale);
    data->yFrontTile = (int) ((double) data->yBackTile * data->frontScale);

    /* ensure tile is at least one pixel in size */
    if (data->xFrontTile < 1) data->xFrontTile = 1;
    if (data->yFrontTile < 1) data->yFrontTile = 1;

    /* ensure front tile is non-zero in size */
    data->xFrontTile = max(data->xFrontTile, 1);
    data->yFrontTile = max(data->yFrontTile, 1);

    /* calculate ASCII cursor height */
    data->yBlinkCursor = (int) ((double) CURSOR_HEIGHT * data->backScale);
    data->yNoBlinkCursor = data->yBackTile;

    /* set map origin point */
    data->map_orig.x =
        max(0, client_rt.left + (wnd_size.cx - data->xFrontTile * COLNO) / 2);
    data->map_orig.y =
        max(0, client_rt.top + (wnd_size.cy - data->yFrontTile * ROWNO) / 2);

    data->map_orig.x -= data->map_orig.x % data->xFrontTile;
    data->map_orig.y -= data->map_orig.y % data->yFrontTile;

    // Set horizontal scroll

    data->xPageSize = min(COLNO, wnd_size.cx / data->xFrontTile);

    GetNHApp()->bNoHScroll = (data->xPageSize == COLNO);

    data->xMin = 0;
    data->xMax = COLNO - data->xPageSize;
    data->xPos = max(0, min(data->xMax, u.ux - (data->xPageSize / 2)));

    SCROLLINFO si;

    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = data->xMin;
    si.nMax = data->xMax;
    si.nPage = 1;
    si.nPos = data->xPos;
    SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

    data->yPageSize = min(ROWNO, wnd_size.cy / data->yFrontTile);

    GetNHApp()->bNoVScroll = (data->yPageSize == ROWNO);

    data->yMin = 0;
    data->yMax = ROWNO - data->yPageSize;
    data->yPos = max(0, min(data->yMax, u.uy - (data->yPageSize / 2)));

    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = data->yMin;
    si.nMax = data->yMax;
    si.nPage = 1;
    si.nPos = data->yPos;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

    mswin_cliparound(data->xCur, data->yCur);

    // redraw all map locations
    dirtyAll(data);

    // invalidate entire map window
    InvalidateRect(hWnd, NULL, TRUE);
}

/* set map mode */
int
mswin_map_mode(HWND hWnd, int mode)
{
    PNHMapWindow data;
    int oldMode;
    SIZE mapSize;

    data = (PNHMapWindow) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (mode == data->mapMode)
        return mode;

    oldMode = data->mapMode;
    data->mapMode = mode;

    switch (data->mapMode) {
    case MAP_MODE_ASCII4x6:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 4;
        data->tileHeight = 6;
        break;

    case MAP_MODE_ASCII6x8:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 6;
        data->tileHeight = 8;
        break;

    case MAP_MODE_ASCII8x8:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 8;
        data->tileHeight = 8;
        break;

    case MAP_MODE_ASCII16x8:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 16;
        data->tileHeight = 8;
        break;

    case MAP_MODE_ASCII7x12:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 7;
        data->tileHeight = 12;
        break;

    case MAP_MODE_ASCII8x12:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 8;
        data->tileHeight = 12;
        break;

    case MAP_MODE_ASCII16x12:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 16;
        data->tileHeight = 12;
        break;

    case MAP_MODE_ASCII12x16:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 12;
        data->tileHeight = 16;
        break;

    case MAP_MODE_ASCII10x18:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = 10;
        data->tileHeight = 18;
        break;

    case MAP_MODE_ASCII_FIT_TO_SCREEN:
        data->bAsciiMode = TRUE;
        data->bFitToScreenMode = TRUE;
        data->tileWidth = 12;
        data->tileHeight = 16;
        break;

    case MAP_MODE_TILES_FIT_TO_SCREEN:
        data->bAsciiMode = FALSE;
        data->bFitToScreenMode = TRUE;
        data->tileWidth = GetNHApp()->mapTile_X;
        data->tileHeight = GetNHApp()->mapTile_Y;
        break;

    case MAP_MODE_TILES:
    default:
        data->bAsciiMode = FALSE;
        data->bFitToScreenMode = FALSE;
        data->tileWidth = GetNHApp()->mapTile_X;
        data->tileHeight = GetNHApp()->mapTile_Y;
        break;
    }

    mapSize.cx = data->tileWidth * COLNO;
    mapSize.cy = data->tileHeight * ROWNO;

    mswin_map_layout(hWnd, &mapSize);

    mswin_update_inventory(0); /* for perm_invent to hide/show tiles */

    return oldMode;
}

void mswin_map_update(HWND hWnd)
{
    PNHMapWindow data = (PNHMapWindow)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (data->mapDirty)
    {
        /* update back buffer */
        HBITMAP savedBitmap = SelectObject(data->tileDC, GetNHApp()->bmpMapTiles);

        for (int i = 0; i < COLNO; i++)
            for (int j = 0; j < ROWNO; j++)
                if (data->locDirty[i][j])
                {
                    paint(data, i, j);
                    RECT rect;
                    nhcoord2display(data, i, j, &rect);
                    InvalidateRect(data->hWnd, &rect, FALSE);
                }

        SelectObject(data->tileDC, savedBitmap);
        data->mapDirty = FALSE;
    }

}

/* register window class for map window */
void
register_map_window_class(void)
{
    WNDCLASS wcex;
    ZeroMemory(&wcex, sizeof(wcex));

    /* window class */
    wcex.style = CS_NOCLOSE | CS_DBLCLKS;
    wcex.lpfnWndProc = (WNDPROC) MapWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetNHApp()->hApp;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground =
        CreateSolidBrush(RGB(0, 0, 0)); /* set backgroup here */
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szNHMapWindowClass;

    if (!RegisterClass(&wcex)) {
        panic("cannot register Map window class");
    }
}

/* map window procedure */
LRESULT CALLBACK
MapWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PNHMapWindow data;

    data = (PNHMapWindow) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (message) {
    case WM_CREATE:
        onCreate(hWnd, wParam, lParam);
        break;

    case WM_MSNH_COMMAND:
        onMSNHCommand(hWnd, wParam, lParam);
        break;

    case WM_PAINT:
        onPaint(hWnd);
        break;

    case WM_SETFOCUS:
        /* transfer focus back to the main window */
        SetFocus(GetNHApp()->hMainWnd);
        break;

    case WM_HSCROLL:
        onMSNH_HScroll(hWnd, wParam, lParam);
        break;

    case WM_VSCROLL:
        onMSNH_VScroll(hWnd, wParam, lParam);
        break;

    case WM_SIZE: {
        RECT rt;
        SIZE size;

        if (data->bFitToScreenMode) {
            size.cx = LOWORD(lParam);
            size.cy = HIWORD(lParam);
        } else {
            /* mapping factor is unchanged we just need to adjust scroll bars
             */
            size.cx = data->xFrontTile * COLNO;
            size.cy = data->yFrontTile * ROWNO;
        }
        mswin_map_layout(hWnd, &size);

        /* update window placement */
        GetWindowRect(hWnd, &rt);
        ScreenToClient(GetNHApp()->hMainWnd, (LPPOINT) &rt);
        ScreenToClient(GetNHApp()->hMainWnd, ((LPPOINT) &rt) + 1);
        mswin_update_window_placement(NHW_MAP, &rt);
    } break;

    case WM_MOVE: {
        RECT rt;
        GetWindowRect(hWnd, &rt);
        ScreenToClient(GetNHApp()->hMainWnd, (LPPOINT) &rt);
        ScreenToClient(GetNHApp()->hMainWnd, ((LPPOINT) &rt) + 1);
        mswin_update_window_placement(NHW_MAP, &rt);
    }
        return FALSE;

    case WM_LBUTTONDOWN:
        NHEVENT_MS(CLICK_1,
                   max(0, min(COLNO, data->xPos
                                         + (LOWORD(lParam) - data->map_orig.x)
                                               / data->xFrontTile)),
                   max(0, min(ROWNO, data->yPos
                                         + (HIWORD(lParam) - data->map_orig.y)
                                               / data->yFrontTile)));
        return 0;

    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
        NHEVENT_MS(CLICK_2,
                   max(0, min(COLNO, data->xPos
                                         + (LOWORD(lParam) - data->map_orig.x)
                                               / data->xFrontTile)),
                   max(0, min(ROWNO, data->yPos
                                         + (HIWORD(lParam) - data->map_orig.y)
                                               / data->yFrontTile)));
        return 0;

    case WM_DESTROY:
        if (data->hMapFont)
            DeleteObject(data->hMapFont);
        if (data->hMapFontUnicode)
            DeleteObject(data->hMapFontUnicode);
        if (data->hBackBuffer)
            DeleteBitmap(data->hBackBuffer);
        if (data->backBufferDC)
            DeleteDC(data->backBufferDC);
        free(data);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) 0);
        windowdata[NHW_MAP].address = 0;
        break;

    case WM_TIMER:
#ifdef REALTIME_PROTO
        if (wParam == RT_CAM_TIMER_ID) {
            /* Offsets are no longer decayed here: they are recomputed from the
               hero's continuous position on every paint, which is what makes
               motion smooth rather than stepped.  This timer's job is to keep
               the frame going -- sampling held input and repainting while
               anything is moving.

               The timer is never stopped, because it is what samples input, but
               an idle map is not repainted sixty times a second. */
            {
                double hx = 0.0, hy = 0.0;

                /* ask the layer directly as well as looking at the cached
                   offsets, so the first frame after a step repaints even
                   though the offsets were computed before it happened */
                if (data->camDX || data->camDY || data->heroDX || data->heroDY
                    || data->monSliding || rtv_hero_offset(&hx, &hy))
                    InvalidateRect(hWnd, NULL, FALSE);
                else
                    rt_drive_hero(hWnd); /* sample input even while at rest */
            }
            break;
        }
#endif
        data->cursorOn = !data->cursorOn;
        dirty(data, data->xCur, data->yCur);
        break;

    case WM_DPICHANGED: {
        RECT rt;
        GetWindowRect(hWnd, &rt);
        ScreenToClient(GetNHApp()->hMainWnd, (LPPOINT)&rt);
        ScreenToClient(GetNHApp()->hMainWnd, ((LPPOINT)&rt) + 1);
        mswin_update_window_placement(NHW_MAP, &rt);
    } break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/* on WM_COMMAND */
void
onMSNHCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PNHMapWindow data;

    data = (PNHMapWindow) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (wParam) {
    case MSNH_MSG_PRINT_GLYPH: {
        PMSNHMsgPrintGlyph msg_data = (PMSNHMsgPrintGlyph) lParam;
        setGlyph(data, msg_data->x, msg_data->y,
            &msg_data->glyphinfo, &msg_data->bkglyphinfo);
    } break;

    case MSNH_MSG_CLIPAROUND: {
        PMSNHMsgClipAround msg_data = (PMSNHMsgClipAround) lParam;
        int x, y;
        BOOL scroll_x, scroll_y;
        int mcam = iflags.wc_scroll_margin;

        /* calculate if you should clip around */
        scroll_x =
            !GetNHApp()->bNoHScroll
            && (msg_data->x < (data->xPos + mcam)
                || msg_data->x > (data->xPos + data->xPageSize - mcam));
        scroll_y =
            !GetNHApp()->bNoVScroll
            && (msg_data->y < (data->yPos + mcam)
                || msg_data->y > (data->yPos + data->yPageSize - mcam));

        mcam += iflags.wc_scroll_amount - 1;
        /* get page size and center horizontally on x-position */
        if (scroll_x) {
            if (data->xPageSize <= 2 * mcam) {
                x = max(0, min(COLNO, msg_data->x - data->xPageSize / 2));
            } else if (msg_data->x < data->xPos + data->xPageSize / 2) {
                x = max(0, min(COLNO, msg_data->x - mcam));
            } else {
                x = max(0, min(COLNO, msg_data->x - data->xPageSize + mcam));
            }
            SendMessage(hWnd, WM_HSCROLL, (WPARAM) MAKELONG(SB_THUMBTRACK, x),
                        (LPARAM) NULL);
        }

        /* get page size and center vertically on y-position */
        if (scroll_y) {
            if (data->yPageSize <= 2 * mcam) {
                y = max(0, min(ROWNO, msg_data->y - data->yPageSize / 2));
            } else if (msg_data->y < data->yPos + data->yPageSize / 2) {
                y = max(0, min(ROWNO, msg_data->y - mcam));
            } else {
                y = max(0, min(ROWNO, msg_data->y - data->yPageSize + mcam));
            }
            SendMessage(hWnd, WM_VSCROLL, (WPARAM) MAKELONG(SB_THUMBTRACK, y),
                        (LPARAM) NULL);
        }
    } break;

    case MSNH_MSG_CLEAR_WINDOW:
        clearAll(data);
        break;

    case MSNH_MSG_CURSOR: {
        PMSNHMsgCursor msg_data = (PMSNHMsgCursor) lParam;

        if (data->xCur != msg_data->x || data->yCur != msg_data->y) {

            dirty(data, data->xCur, data->yCur);
            dirty(data, msg_data->x, msg_data->y);

            data->xCur = msg_data->x;
            data->yCur = msg_data->y;
        }

    } break;

    case MSNH_MSG_GETTEXT: {
        PMSNHMsgGetText msg_data = (PMSNHMsgGetText) lParam;
        size_t index;
        int col, row;
#if 0
        int color;
        unsigned special = 0U;
#endif
        int mgch;

        index = 0;
        for (row = 0; row < ROWNO; row++) {
            for (col = 0; col < COLNO; col++) {
                if (index >= msg_data->max_size)
                    break;
                if (data->map[col][row].glyph == NO_GLYPH)
                    mgch = ' ';
                msg_data->buffer[index] = data->map[col][row].ttychar;
                index++;
            }
            if (index >= msg_data->max_size - 1)
                break;
            msg_data->buffer[index++] = '\r';
            msg_data->buffer[index++] = '\n';
        }
        nhUse(mgch);
    } break;

#ifdef ENHANCED_SYMBOLS
    case MSNH_MSG_GETWIDETEXT: {
        PMSNHMsgGetWideText msg_data = (PMSNHMsgGetWideText) lParam;
        size_t index;
        int col, row;

        index = 0;
        for (row = 0; row < ROWNO; row++) {
            for (col = 0; col < COLNO; col++) {
                glyph_info *glyphinfo;
                uint32 ch;
                if (index >= msg_data->max_size)
                    break;
                glyphinfo = &data->map[col][row];
                if (glyphinfo->gm.u && glyphinfo->gm.u->utf8str) {
                    ch = glyphinfo->gm.u->utf32ch;
                } else {
                    ch = glyphinfo->ttychar;
                }
                winos_ascii_to_wide(msg_data->buffer + index, ch);
                index += wcslen(msg_data->buffer + index);
            }
            if (index >= msg_data->max_size - 1)
                break;
            msg_data->buffer[index++] = '\r';
            msg_data->buffer[index++] = '\n';
        }
    } break;
#endif

    case MSNH_MSG_RANDOM_INPUT:
        nhassert(0); // unexpected
        break;

    } /* end switch(wParam) */
}

/* on WM_CREATE */
void
onCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PNHMapWindow data;

    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    /* set window data */
    data = (PNHMapWindow) malloc(sizeof(NHMapWindow));
    if (!data)
        panic("out of memory");

    ZeroMemory(data, sizeof(NHMapWindow));

    data->hWnd = hWnd;

    data->bAsciiMode = FALSE;
    data->cursorOn = TRUE;

    data->xFrontTile = GetNHApp()->mapTile_X;
    data->yFrontTile = GetNHApp()->mapTile_Y;
    data->tileWidth = GetNHApp()->mapTile_X;
    data->tileHeight = GetNHApp()->mapTile_Y;

    HDC hDC = GetDC(hWnd);
    data->backBufferDC = CreateCompatibleDC(hDC);
    data->tileDC = CreateCompatibleDC(hDC);
    ReleaseDC(hWnd, hDC);

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) data);
    windowdata[NHW_MAP].address = (genericptr_t) data;

    clearAll(data);

}

static void
paintTile(PNHMapWindow data, int i, int j, RECT * rect)
{
    short ntile;
    int t_x, t_y;
    int glyph, bkglyph;
    int layer;
#ifdef USE_PILEMARK
//    int color;
//    unsigned special = 0U;
//    int mgch;
#endif
    layer = 0;
    glyph = data->map[i][j].glyph;
    bkglyph = data->bkmap[i][j].glyph;

    if (glyph == NO_GLYPH && bkglyph == NO_GLYPH) {
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(data->backBufferDC, rect, blackBrush);
        DeleteObject(blackBrush);
    }


    if (glyph != NO_GLYPH) {
        if (bkglyph != NO_GLYPH) {
            ntile = data->bkmap[i][j].gm.tileidx;
            t_x = TILEBMP_X(ntile);
            t_y = TILEBMP_Y(ntile);

            StretchBlt(data->backBufferDC, rect->left, rect->top,
                       data->xBackTile, data->yBackTile, data->tileDC, t_x,
                       t_y, GetNHApp()->mapTile_X, GetNHApp()->mapTile_Y,
                       SRCCOPY);
            layer++;
        }
        if (glyph != bkglyph) {
            /* rely on tileidx provided by NetHack core */
            ntile = data->map[i][j].gm.tileidx;
            t_x = TILEBMP_X(ntile);
            t_y = TILEBMP_Y(ntile);

            /* Don't use all black GLYPH_UNEXPLORED tile as a background */
            if (layer > 0 && bkglyph != GLYPH_UNEXPLORED) {
                (*GetNHApp()->lpfnTransparentBlt)(
                    data->backBufferDC, rect->left, rect->top,
                    data->xBackTile, data->yBackTile, data->tileDC, t_x,
                    t_y, GetNHApp()->mapTile_X,
                    GetNHApp()->mapTile_Y, TILE_BK_COLOR);
            } else {
                StretchBlt(data->backBufferDC, rect->left, rect->top,
                            data->xBackTile, data->yBackTile, data->tileDC,
                            t_x, t_y, GetNHApp()->mapTile_X,
                            GetNHApp()->mapTile_Y, SRCCOPY);
            }
            layer++;
        }

#ifdef USE_PILEMARK
        if ((data->map[i][j].gm.glyphflags & MG_PET) != 0
#else
        if (glyph_is_pet(glyph)
#endif
            && iflags.wc_hilite_pet) {
            /* apply pet mark transparently over
                pet image */
            HDC hdcPetMark;
            HBITMAP bmPetMarkOld;

            /* this is DC for petmark bitmap */
            hdcPetMark = CreateCompatibleDC(data->backBufferDC);
            bmPetMarkOld =
                SelectObject(hdcPetMark, GetNHApp()->bmpPetMark);

            (*GetNHApp()->lpfnTransparentBlt)(
                data->backBufferDC, rect->left, rect->top,
                data->xBackTile, data->yBackTile, hdcPetMark, 0, 0,
                TILE_X, TILE_Y, TILE_BK_COLOR);
            SelectObject(hdcPetMark, bmPetMarkOld);
            DeleteDC(hdcPetMark);
        }
#ifdef USE_PILEMARK
        if ((data->map[i][j].gm.glyphflags & MG_OBJPILE) != 0
            && iflags.hilite_pile) {
            /* apply pilemark transparently over other image */
            HDC hdcPileMark;
            HBITMAP bmPileMarkOld;

            /* this is DC for pilemark bitmap */
            hdcPileMark = CreateCompatibleDC(data->backBufferDC);
            bmPileMarkOld = SelectObject(hdcPileMark,
                                            GetNHApp()->bmpPileMark);

            (*GetNHApp()->lpfnTransparentBlt)(
                data->backBufferDC, rect->left, rect->top,
                data->xBackTile, data->yBackTile, hdcPileMark, 0, 0,
                TILE_X, TILE_Y, TILE_BK_COLOR);
            SelectObject(hdcPileMark, bmPileMarkOld);
            DeleteDC(hdcPileMark);
        }
#endif
    }       /* glyph != NO_GLYPH */

    if (i == data->xCur && j == data->yCur &&
        (data->cursorOn || !win32_cursorblink))
        DrawFocusRect(data->backBufferDC, rect);
}


static void
paintGlyph(PNHMapWindow data, int i, int j, RECT * rect)
{
    if (data->map[i][j].glyph >= 0) {

        glyph_info *glyphinfo;
        uint32 ch;
        WCHAR wch[3];
        int color;
        uint32 rgbcolor;
//        unsigned special;
//        int mgch;
        HBRUSH back_brush;
        COLORREF OldFg;

        SetBkMode(data->backBufferDC, TRANSPARENT);

        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(data->backBufferDC, rect, blackBrush);
        DeleteObject(blackBrush);

        glyphinfo = &data->map[i][j];
        ch = glyphinfo->ttychar;
        color = (int) glyphinfo->gm.sym.color;
        rgbcolor = nhcolor_to_RGB(color);
#ifdef ENHANCED_SYMBOLS
        if (SYMHANDLING(H_UTF8)
            && glyphinfo->gm.u
            && glyphinfo->gm.u->utf8str) {
            ch = glyphinfo->gm.u->utf32ch;
        }
#endif
        if (glyphinfo->gm.customcolor != 0
            && (mswin_procs.wincap2 & WC2_EXTRACOLORS) != 0) {
            if ((glyphinfo->gm.customcolor & NH_BASIC_COLOR) != 0) {
                color = (int) COLORVAL(glyphinfo->gm.customcolor);
                rgbcolor = nhcolor_to_RGB(color);
            }
        }
        if ((((data->map[i][j].gm.glyphflags & MG_PET) && iflags.hilite_pet)
             || ((data->map[i][j].gm.glyphflags & MG_OBJPILE) && iflags.hilite_pile))
            || ((data->map[i][j].gm.glyphflags & (MG_DETECT | MG_BW_LAVA
                                                  | MG_BW_ICE | MG_BW_SINK
                                                  | MG_BW_ENGR)) != 0)) {
            back_brush =
                CreateSolidBrush(nhcolor_to_RGB(CLR_GRAY));
            FillRect(data->backBufferDC, rect, back_brush);
            DeleteObject(back_brush);
            switch (color) {
            case CLR_GRAY:
            case CLR_WHITE:
                OldFg = SetTextColor(
                    data->backBufferDC, nhcolor_to_RGB(CLR_BLACK));
                break;
            default:
                OldFg =
                    SetTextColor(data->backBufferDC, rgbcolor);
            }
        } else {
            OldFg = SetTextColor(data->backBufferDC, rgbcolor);
        }
        if (data->bUnicodeFont || SYMHANDLING(H_UTF8)) {
            winos_ascii_to_wide(wch, ch);
            if (wch[0] == 0x2591 || wch[0] == 0x2592) {
                int intensity = 80;
                HBRUSH brush = CreateSolidBrush(RGB(intensity, intensity, intensity));
                FillRect(data->backBufferDC, rect, brush);
                DeleteObject(brush);
                intensity = (wch[0] == 0x2591 ? 1 : 2);
                brush = CreateSolidBrush(RGB(
                            GetRValue(rgbcolor)*intensity/2,
                            GetGValue(rgbcolor)*intensity/2,
                            GetBValue(rgbcolor)*intensity/2));
                RECT smallRect = {0};
                smallRect.left = rect->left + 1;
                smallRect.top = rect->top + 1;
                smallRect.right = rect->right - 1;
                smallRect.bottom = rect->bottom - 1;
                FillRect(data->backBufferDC, &smallRect, brush);
                DeleteObject(brush);
            } else {
                SelectObject(data->backBufferDC, data->hMapFontUnicode);
                DrawTextW(data->backBufferDC, wch, -1, rect,
                    DT_CENTER | DT_VCENTER | DT_NOPREFIX
                    | DT_SINGLELINE);
            }
        } else {
            char ch8 = (char) ch;
            SelectObject(data->backBufferDC, data->hMapFont);
            DrawTextA(data->backBufferDC, &ch8, 1, rect,
                        DT_CENTER | DT_VCENTER | DT_NOPREFIX
                            | DT_SINGLELINE);
        }

        SetTextColor(data->backBufferDC, OldFg);
    }

    if (i == data->xCur && j == data->yCur &&
        (data->cursorOn || !win32_cursorblink)) {
        int yCursor = (win32_cursorblink ? data->yBlinkCursor :
                                           data->yNoBlinkCursor);
        PatBlt(data->backBufferDC,
                rect->left, rect->bottom - yCursor,
                rect->right - rect->left,
                yCursor,
                DSTINVERT);
    }
}

static void setGlyph(PNHMapWindow data, int i, int j,
                     const glyph_info *fg, const glyph_info *bg)
{
    if ((data->map[i][j].glyph != fg->glyph)
            || (data->bkmap[i][j].glyph != bg->glyph)
        || data->map[i][j].ttychar != fg->ttychar
        || data->map[i][j].gm.sym.color != fg->gm.sym.color
        || data->map[i][j].gm.customcolor != fg->gm.customcolor
        || data->map[i][j].gm.glyphflags != fg->gm.glyphflags
        || data->map[i][j].gm.tileidx != fg->gm.tileidx) {
        data->map[i][j] = *fg;
        data->bkmap[i][j] = *bg;
        data->locDirty[i][j] = TRUE;
        data->mapDirty = TRUE;
    }
}

static void clearAll(PNHMapWindow data)
{
    for (int x = 0; x < COLNO; x++)
        for (int y = 0; y < ROWNO; y++) {
            data->map[x][y] = nul_glyphinfo;
            data->bkmap[x][y] = nul_glyphinfo;
            data->locDirty[x][y] = TRUE;
        }
    data->mapDirty = TRUE;
}

static void dirtyAll(PNHMapWindow data)
{
    for (int i = 0; i < COLNO; i++)
        for (int j = 0; j < ROWNO; j++)
            data->locDirty[i][j] = TRUE;
    data->mapDirty = TRUE;
}

static void dirty(PNHMapWindow data, int x, int y)
{
    data->locDirty[x][y] = TRUE;
    data->mapDirty = TRUE;
}

static void
paint(PNHMapWindow data, int i, int j)
{
    RECT rect;

    rect.left = i * data->xBackTile;
    rect.top = j * data->yBackTile;
    rect.right = rect.left + data->xBackTile;
    rect.bottom = rect.top + data->yBackTile;

    if (data->bAsciiMode || Is_rogue_level(&u.uz)) {
        paintGlyph(data, i, j, &rect);
    } else {
        paintTile(data, i, j, &rect);
    }

    data->locDirty[i][j] = FALSE;
}


#ifdef REALTIME_PROTO
/* Detect a whole-tile jump of the scroll origin (which happens when the hero
   walks and the view re-centers) and seed a fractional pan offset so the view
   starts at the OLD position and slides to the new one.  The WM_TIMER handler
   decays camD{X,Y} back to zero over ~RT_TURN_MS.  Contained to this window;
   if the offset is zero the render is pixel-identical to stock. */
/* The frame timer runs for the window's lifetime (see onCreate), so this only
   records that something is moving; it no longer has to start anything. */
static void
rt_anim_start_timer(PNHMapWindow data)
{
    data->camAnimating = TRUE;
}

/*
 * Sample the arrow keys and let the hero's continuous intent decide when to
 * step, rather than turning each key event into a queued command.  This is the
 * half of the interpolation layer that leads the grid instead of following it;
 * see rtv_hero_free_move().
 *
 * Held keys are read as *state* rather than as events, because a sustained
 * direction is a continuous quantity and the message queue only reports edges.
 * A step is handed to the game as an ordinary movement key, so it goes through
 * exactly the same command path as one the player typed.
 */
static void
rt_drive_hero(HWND hWnd)
{
    int dx = 0, dy = 0, i;
    coordxy sx = 0, sy = 0;

    /* Only while the game is waiting for a command, and only while this
       application is the one being typed at.

       The flag is used rather than program_state.input_state because a step we
       inject is taken from the command queue without parse() running, which is
       what sets input_state -- so after the first injected step input_state
       would never read as commandInp again and movement would stop dead.

       The window tested is the top-level one, not this map.  Keystrokes are
       handled by the main window, so the map child never holds the keyboard
       focus and testing for it here would refuse to move the hero at all. */
    if (!mswin_rt_awaiting_cmd
        || GetForegroundWindow() != GetNHApp()->hMainWnd) {
        /* keep time accounted for, but stand still */
        (void) rtv_hero_free_move(0.0, 0.0, &sx, &sy);
        return;
    }
    nhUse(hWnd);

#define RT_HELD(vk) ((GetAsyncKeyState(vk) & 0x8000) != 0)
    if (RT_HELD(VK_LEFT))
        dx = -1;
    else if (RT_HELD(VK_RIGHT))
        dx = 1;
    if (RT_HELD(VK_UP))
        dy = -1;
    else if (RT_HELD(VK_DOWN))
        dy = 1;
#undef RT_HELD

    /* the layer moves the hero's real position and reports a step whenever it
       crosses into another square */
    if (!rtv_hero_free_move((double) dx, (double) dy, &sx, &sy))
        return;

    /* Deliver it as input rather than on the command queue.  The game is
       waiting for input right now, so this is consumed by that wait and paced
       by it; a command-queue entry would instead be taken on the next pass
       without waiting, which cost the pacing and spent a turn resting first. */
    for (i = 0; i < 8; i++) {
        if (xdir[i] == sx && ydir[i] == sy) {
            const char *dc = gc.Cmd.dirchars;

            if (dc && dc[i])
                NHEVENT_KBD(dc[i]);
            return;
        }
    }
}

/*
 * The tile to paint over an entity that is being drawn somewhere else.
 *
 * The background glyph the core sends with each square is not usable for this:
 * where a monster or the hero stands it is frequently the all-black unexplored
 * tile, so erasing with it left a black hole under everything that moved. Ask
 * the core what the *terrain* is instead -- back_to_glyph() answers ignoring
 * whoever is standing on it, which is exactly the question being asked.
 *
 * Returns FALSE when no usable terrain tile can be had, in which case the
 * caller should leave the entity drawn on its square rather than erase it with
 * something wrong.
 */
static boolean
rt_terrain_tile(coordxy x, coordxy y, short *tileidx)
{
    glyph_info gi;
    int tglyph;

    if (!isok(x, y))
        return FALSE;
    tglyph = back_to_glyph(x, y);
    if (tglyph == NO_GLYPH || glyph_is_unexplored(tglyph))
        return FALSE;
    map_glyphinfo(x, y, tglyph, 0U, &gi);
    if (gi.glyph == NO_GLYPH)
        return FALSE;
    *tileidx = gi.gm.tileidx;
    return TRUE;
}

/*
 * Derive this frame's hero and camera offsets from the hero's continuous
 * position.
 *
 * There used to be two mechanisms here -- one that detected a whole-tile jump
 * of the scroll origin and decayed a camera offset, and another that did the
 * same for the hero tile -- which is why motion still looked stepped: the
 * offsets were seeded a square at a time and then decayed, rather than being
 * read from where the hero actually is. Both are now the same quantity, taken
 * straight from the interpolation layer.
 *
 * Whether that offset moves the world or the hero depends on the view: when the
 * map is larger than the window it scrolls to follow the hero, so the offset is
 * applied to the world and the hero stays put on screen; when the level fits,
 * there is nothing to scroll and the hero moves within a fixed view.
 */
static void
rt_frame_offsets(PNHMapWindow data)
{
    double hx = 0.0, hy = 0.0;
    boolean moving = rtv_hero_offset(&hx, &hy);
    boolean scrolls = (data->xMax > 0 || data->yMax > 0);

    if (!moving) {
        data->camDX = data->camDY = 0;
        data->heroDX = data->heroDY = 0;
        data->heroSlideActive = FALSE;
        return;
    }

    if (scrolls) {
        /* the view follows the hero, so slide the world under a fixed hero */
        data->camDX = -(int) (hx * (double) data->xFrontTile);
        data->camDY = -(int) (hy * (double) data->yFrontTile);
        data->heroDX = data->heroDY = 0;
        data->heroSlideActive = FALSE;
    } else {
        /* nothing to scroll: move the hero within the view instead */
        data->camDX = data->camDY = 0;
        data->heroDX = (int) (hx * (double) data->xFrontTile);
        data->heroDY = (int) (hy * (double) data->yFrontTile);
        /* only glide where there is terrain to erase the static hero with */
        {
            short unused;

            data->heroSlideActive = rt_terrain_tile(u.ux, u.uy, &unused);
        }
    }
    rt_anim_start_timer(data);
}
#endif /* REALTIME_PROTO */

/* on WM_PAINT */
void
onPaint(HWND hWnd)
{
    PNHMapWindow data = (PNHMapWindow) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    PAINTSTRUCT ps;
    HDC hFrontBufferDC = BeginPaint(hWnd, &ps);

    /* stretch back buffer onto front buffer window */
    int frontWidth = COLNO * data->xFrontTile;
    int frontHeight = ROWNO * data->yFrontTile;
    int originX = data->map_orig.x - (data->xPos * data->xFrontTile);
    int originY = data->map_orig.y - (data->yPos * data->yFrontTile);

#ifdef REALTIME_PROTO
    /* move the continuous positions on by however much real time has passed
       since the previous frame, before anything reads them below */
    rtv_advance();
    /* and let a held direction decide whether it is time to step */
    rt_drive_hero(hWnd);

    /* both offsets come from where the hero actually is, not from detecting
       that its square changed */
    rt_frame_offsets(data);
    originX += data->camDX;
    originY += data->camDY;
#endif

    StretchBlt(hFrontBufferDC, originX, originY, frontWidth, frontHeight,
                data->backBufferDC, 0, 0, data->backWidth, data->backHeight, SRCCOPY);

#ifdef REALTIME_PROTO
    if (data->heroSlideActive) {
        /* The back-buffer blit above drew the hero statically at its cell.
           Redraw the terrain over that cell, then draw the hero transparently
           at the interpolated position so it appears to glide.  Nothing in the
           back buffer is touched, so when the slide ends the static hero simply
           reappears with no gap. */
        RECT hr;
        short ntile;
        int bx, by;
        HBITMAP savedTile = SelectObject(data->tileDC, GetNHApp()->bmpMapTiles);

        nhcoord2display(data, (int) u.ux, (int) u.uy, &hr);
        bx = hr.left + data->camDX; /* align with the (possibly) panned world */
        by = hr.top + data->camDY;

        /* (a) erase the static hero by repainting the terrain it stands on */
        if (rt_terrain_tile(u.ux, u.uy, &ntile))
            StretchBlt(hFrontBufferDC, bx, by, data->xFrontTile,
                       data->yFrontTile, data->tileDC, TILEBMP_X(ntile),
                       TILEBMP_Y(ntile), GetNHApp()->mapTile_X,
                       GetNHApp()->mapTile_Y, SRCCOPY);

        /* (b) draw the hero at the interpolated position, transparently */
        ntile = data->map[u.ux][u.uy].gm.tileidx;
        (*GetNHApp()->lpfnTransparentBlt)(
            hFrontBufferDC, bx + data->heroDX, by + data->heroDY,
            data->xFrontTile, data->yFrontTile, data->tileDC,
            TILEBMP_X(ntile), TILEBMP_Y(ntile), GetNHApp()->mapTile_X,
            GetNHApp()->mapTile_Y, TILE_BK_COLOR);

        SelectObject(data->tileDC, savedTile);
    }

    /* Glide monsters using the core's continuous positions.  Those positions
       are per entity, so two monsters exchanging squares are each drawn from
       their own, and a monster crossing several squares glides continuously
       instead of restarting at every step.  We ask by square, which is all a
       renderer working from a grid of glyphs can name.

       Only squares the map is actually showing a monster on are animated, so
       this cannot reveal a monster the player is not entitled to see. */
    {
        int i, j, i0, i1, j0, j1;
        boolean anyMoving = FALSE;
        HBITMAP savedTile = SelectObject(data->tileDC, GetNHApp()->bmpMapTiles);

        /* restrict the scan to the visible page rather than the whole map */
        i0 = max(0, data->xPos);
        i1 = min(COLNO - 1, data->xPos + data->xPageSize);
        j0 = max(0, data->yPos);
        j1 = min(ROWNO - 1, data->yPos + data->yPageSize);

        for (i = i0; i <= i1; i++) {
            for (j = j0; j <= j1; j++) {
                double ox = 0.0, oy = 0.0;
                int glyph = data->map[i][j].glyph;
                RECT mr;
                short ntile;
                int bx, by, offx, offy;

                if (glyph == NO_GLYPH || !glyph_is_monster(glyph))
                    continue;
                if (i == (int) u.ux && j == (int) u.uy)
                    continue; /* the hero is handled above */
                if (!rtv_offset_at((coordxy) i, (coordxy) j, &ox, &oy))
                    continue;
                if (!rt_terrain_tile((coordxy) i, (coordxy) j, &ntile))
                    continue; /* no terrain to erase with; leave it static */

                anyMoving = TRUE;
                nhcoord2display(data, i, j, &mr);
                bx = mr.left + data->camDX;
                by = mr.top + data->camDY;

                /* offsets arrive in grid units; scale to this port's tiles */
                offx = (int) (ox * (double) data->xFrontTile);
                offy = (int) (oy * (double) data->yFrontTile);

                /* erase the statically drawn monster with its terrain ... */
                StretchBlt(hFrontBufferDC, bx, by, data->xFrontTile,
                           data->yFrontTile, data->tileDC, TILEBMP_X(ntile),
                           TILEBMP_Y(ntile), GetNHApp()->mapTile_X,
                           GetNHApp()->mapTile_Y, SRCCOPY);

                /* ... and redraw it at the interpolated position */
                ntile = data->map[i][j].gm.tileidx;
                (*GetNHApp()->lpfnTransparentBlt)(
                    hFrontBufferDC, bx + offx, by + offy, data->xFrontTile,
                    data->yFrontTile, data->tileDC, TILEBMP_X(ntile),
                    TILEBMP_Y(ntile), GetNHApp()->mapTile_X,
                    GetNHApp()->mapTile_Y, TILE_BK_COLOR);
            }
        }
        SelectObject(data->tileDC, savedTile);

        /* keep the frame timer running while anything is still in motion */
        data->monSliding = anyMoving;
        if (anyMoving)
            rt_anim_start_timer(data);
    }
#endif

    EndPaint(hWnd, &ps);
}

/* on WM_VSCROLL */
void
onMSNH_VScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PNHMapWindow data;
    SCROLLINFO si;
    int yNewPos;
    int yDelta;

    UNREFERENCED_PARAMETER(lParam);

    /* get window data */
    data = (PNHMapWindow) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (LOWORD(wParam)) {
    /* User clicked shaft left of the scroll box. */
    case SB_PAGEUP:
        yNewPos = data->yPos - data->yPageSize;
        break;

    /* User clicked shaft right of the scroll box. */
    case SB_PAGEDOWN:
        yNewPos = data->yPos + data->yPageSize;
        break;

    /* User clicked the left arrow. */
    case SB_LINEUP:
        yNewPos = data->yPos - 1;
        break;

    /* User clicked the right arrow. */
    case SB_LINEDOWN:
        yNewPos = data->yPos + 1;
        break;

    /* User dragged the scroll box. */
    case SB_THUMBTRACK:
        yNewPos = HIWORD(wParam);
        break;

    default:
        yNewPos = data->yPos;
    }

    yNewPos = max(0, min(data->yMax, yNewPos));
    if (yNewPos == data->yPos)
        return;

    yDelta = yNewPos - data->yPos;
    data->yPos = yNewPos;

    ScrollWindowEx(hWnd, 0, -data->yFrontTile * yDelta, (CONST RECT *) NULL,
                   (CONST RECT *) NULL, (HRGN) NULL, (LPRECT) NULL,
                   SW_INVALIDATE | SW_ERASE);

    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    si.nPos = data->yPos;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
}

/* on WM_HSCROLL */
void
onMSNH_HScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PNHMapWindow data;
    SCROLLINFO si;
    int xNewPos;
    int xDelta;

    UNREFERENCED_PARAMETER(lParam);

    /* get window data */
    data = (PNHMapWindow) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (LOWORD(wParam)) {
    /* User clicked shaft left of the scroll box. */
    case SB_PAGEUP:
        xNewPos = data->xPos - data->xPageSize;
        break;

    /* User clicked shaft right of the scroll box. */
    case SB_PAGEDOWN:
        xNewPos = data->xPos + data->xPageSize;
        break;

    /* User clicked the left arrow. */
    case SB_LINEUP:
        xNewPos = data->xPos - 1;
        break;

    /* User clicked the right arrow. */
    case SB_LINEDOWN:
        xNewPos = data->xPos + 1;
        break;

    /* User dragged the scroll box. */
    case SB_THUMBTRACK:
        xNewPos = HIWORD(wParam);
        break;

    default:
        xNewPos = data->xPos;
    }

    xNewPos = max(0, min(data->xMax, xNewPos));
    if (xNewPos == data->xPos)
        return;

    xDelta = xNewPos - data->xPos;
    data->xPos = xNewPos;

    ScrollWindowEx(hWnd, -data->xFrontTile * xDelta, 0, (CONST RECT *) NULL,
                   (CONST RECT *) NULL, (HRGN) NULL, (LPRECT) NULL,
                   SW_INVALIDATE | SW_ERASE);

    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    si.nPos = data->xPos;
    SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
}

/* map nethack map coordinates to the screen location */
void
nhcoord2display(PNHMapWindow data, int x, int y, LPRECT lpOut)
{
    lpOut->left = (x - data->xPos) * data->xFrontTile + data->map_orig.x;
    lpOut->top = (y - data->yPos) * data->yFrontTile + data->map_orig.y;
    lpOut->right = lpOut->left + data->xFrontTile;
    lpOut->bottom = lpOut->top + data->yFrontTile;
}

#if (VERSION_MAJOR < 4) && (VERSION_MINOR < 4) && (PATCHLEVEL < 2)
/* map glyph to character/color combination */
void
nhglyph2charcolor(short g, uchar *ch, int *color)
{
    int offset;

#define zap_color(n) *color = iflags.use_color ? zapcolors[n] : NO_COLOR
#define cmap_color(n) *color = iflags.use_color ? defsyms[n].color : NO_COLOR
#define obj_color(n) \
    *color = iflags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n) *color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define pet_color(n) *color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define warn_color(n) \
    *color = iflags.use_color ? def_warnsyms[n].color : NO_COLOR

    if ((offset = (g - GLYPH_WARNING_OFF)) >= 0) { /* a warning flash */
        *ch = showsyms[offset + SYM_OFF_W];
        warn_color(offset);
    } else if ((offset = (g - GLYPH_SWALLOW_OFF)) >= 0) { /* swallow */
        /* see swallow_to_glyph() in display.c */
        *ch = (uchar) showsyms[(S_sw_tl + (offset & 0x7)) + SYM_OFF_P];
        mon_color(offset >> 3);
    } else if ((offset = (g - GLYPH_ZAP_OFF)) >= 0) { /* zap beam */
        /* see zapdir_to_glyph() in display.c */
        *ch = showsyms[(S_vbeam + (offset & 0x3)) + SYM_OFF_P];
        zap_color((offset >> 2));
    } else if ((offset = (g - GLYPH_CMAP_OFF)) >= 0) { /* cmap */
        *ch = showsyms[offset + SYM_OFF_P];
        cmap_color(offset);
    } else if ((offset = (g - GLYPH_OBJ_OFF)) >= 0) { /* object */
        *ch = showsyms[(int) objects[offset].oc_class + SYM_OFF_O];
        obj_color(offset);
    } else if ((offset = (g - GLYPH_BODY_OFF)) >= 0) { /* a corpse */
        *ch = showsyms[(int) objects[CORPSE].oc_class + SYM_OFF_O];
        mon_color(offset);
    } else if ((offset = (g - GLYPH_PET_OFF)) >= 0) { /* a pet */
        *ch = showsyms[(int) mons[offset].mlet + SYM_OFF_M];
        pet_color(offset);
    } else { /* a monster */
        *ch = showsyms[(int) mons[g].mlet + SYM_OFF_M];
        mon_color(g);
    }
    // end of wintty code
}
#endif

/* map nethack color to RGB */
COLORREF
nhcolor_to_RGB(int c)
{
    if (c >= 0 && c < CLR_MAX)
        return GetNHApp()->regMapColors[c];
    return RGB(0x00, 0x00, 0x00);
}
