/* NetHack 5.0    tileset.h    $NHDT-Date: 1781973089 2026/06/20 16:31:29 $ $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.6 $ */
/* Copyright (c) Ray Chason, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file tileset.h
 * @brief Loading tile images, whatever format they arrived in.
 *
 * A tile set may be a BMP, a GIF or a PNG, and may be paletted or true colour.
 * The readers differ; what they produce does not -- so the interface a port uses
 * to ask for a tile is the same regardless.
 *
 * The distinction between paletted and direct colour survives loading, because a
 * port may want the indexes rather than the colours, so an image carries whichever
 * it has and the caller says which it wants.
 *
 * @note The reading functions are declared here rather than privately because
 *       which of them is compiled in depends on the build.
 * @warning Images are owned by this code; a tile obtained from it is borrowed and
 *          is invalidated when the set is freed.
 */

/**
 * @file tileset.h
 * @brief 어떤 형식으로 왔든 타일 이미지를 불러오기.
 *
 * 타일 집합은 BMP 나 GIF, PNG 일 수 있고 팔레트 방식이거나 트루컬러일 수 있다. 읽는
 * 쪽은 서로 다르지만 만들어 내는 것은 다르지 않다. 그래서 포팅이 타일을 요청하는
 * 인터페이스는 어느 쪽이든 동일하다.
 *
 * 팔레트와 직접 색의 구분은 불러온 뒤에도 남는다. 포팅이 색이 아니라 색인을 원할 수도
 * 있기 때문이며, 이미지는 자기가 가진 쪽을 지니고 호출자가 원하는 쪽을 말한다.
 *
 * @note 읽기 함수들을 비공개로 두지 않고 여기 선언한 것은, 그중 무엇이 컴파일되어
 *       들어가는지가 빌드에 달려 있기 때문이다.
 * @warning 이미지는 이 코드가 소유한다. 여기서 얻은 타일은 빌려 온 것이며, 집합을
 *          해제하면 무효가 된다.
 */

#ifndef TILESET_H
#define TILESET_H

/** @brief One pixel, as four eight-bit channels. */
/** @brief 한 픽셀. 8비트 채널 네 개로 표현한다. */
struct Pixel {
    unsigned char r, g, b, a;
};

struct TileImage {
    /* Image data */
    unsigned width, height;
    struct Pixel *pixels; /* for direct color */
    unsigned char *indexes; /* for paletted images */
};

boolean read_tiles(const char *filename, boolean true_color);
const struct Pixel *get_palette(void);
void set_tile_type(boolean true_color);
void free_tiles(void);
const struct TileImage *get_tile(unsigned tile_index);

/* For resizing tiles */
struct TileImage *stretch_tile(const struct TileImage *, unsigned,
                               unsigned);
void free_tile(struct TileImage *);

/* Used internally by the tile set code */
struct TileSetImage {
    /* Image data */
    unsigned width, height;
    struct Pixel *pixels; /* for direct color */
    unsigned char *indexes; /* for paletted images */
    struct Pixel palette[256];

    /* Image description from the file */
    char *image_desc;

    /* Tile dimensions */
    unsigned tile_width, tile_height;
};

boolean read_bmp_tiles(const char *filename, struct TileSetImage *image);
boolean read_gif_tiles(const char *filename, struct TileSetImage *image);
boolean read_png_tiles(const char *filename, struct TileSetImage *image);

#endif
