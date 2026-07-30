/* NetHack 5.0	tile2x11.h	$NHDT-Date: 1781973089 2026/06/20 16:31:29 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.14 $ */
/*      Copyright (c) 2002 by David Cohrs              */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file tile2x11.h
 * @brief The layout of the X11 tile file, as both writer and readers see it.
 *
 * The tile image is one large bitmap holding every tile in rows, so a tile is
 * located by arithmetic on its index. The header records the dimensions needed
 * for that, which means a reader does not have to be built with the same
 * constants as the writer.
 *
 * @note Read by Qt as well as X11, since Qt falls back to this file when its own
 *       bitmap is unavailable -- which is why the format is described here
 *       rather than inside either port.
 */

/**
 * @file tile2x11.h
 * @brief X11 타일 파일의 배치. 쓰는 쪽과 읽는 쪽이 함께 보는 형태.
 *
 * 타일 이미지는 모든 타일을 행으로 담은 하나의 큰 비트맵이므로, 타일의 위치는 색인에
 * 대한 산술로 찾는다. 헤더는 그 계산에 필요한 크기 정보를 기록하며, 덕분에 읽는 쪽이
 * 쓰는 쪽과 같은 상수로 빌드되어 있지 않아도 된다.
 *
 * @note X11 뿐 아니라 Qt 도 이것을 읽는다. Qt 는 자기 비트맵을 쓸 수 없을 때 이 파일로
 *       물러난다. 형식을 어느 한 포팅 안이 아니라 여기에 기술해 둔 이유다.
 */

#ifndef TILE2X11_H
#define TILE2X11_H

/*
 * Header for the X11 tile map.
 *
 * dat/x11tiles is used by Qt for fallback if nhtiles.bmp is inaccessible,
 * so this header is used by Qt as well as by X11.
 */
typedef struct {
    unsigned long version;
    unsigned long ncolors;
    unsigned long tile_width;
    unsigned long tile_height;
    unsigned long ntiles;
    unsigned long per_row;
} x11_header;

/* how wide each row in the tile file is, in tiles */
#define TILES_PER_ROW (40)

#endif /* TILE2X11_H */
