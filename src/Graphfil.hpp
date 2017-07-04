/*
**  wt -- a 3d game engine
**
**  Copyright (C) 1994 by Chris Laurel
**  email:  claurel@mr.net
**  snail mail:  Chris Laurel, 5700 W Lake St #208,  St. Louis Park, MN  55416
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "AmpHead.hpp"
#include "System.hpp"

#ifndef __GRAPHFIL__
#define __GRAPHFIL__

typedef enum { formatGIF87, formatGIF89, formatPPM, formatPICTR, formatUnknown } 
   Graphic_file_format;

typedef enum { gfTrueColor, gfPaletted } Graphic_file_type;

typedef struct {
     Graphic_file_type type;
     int height, width;
     int palette_entries;
     long transparent_entry;
     RGBcolor *palette;
     unsigned char *bitmap;
} Graphic_file;

void *wtmalloc(size_t size);
void *wtrealloc(void *v, size_t size);
void wtfree(void *v);
void FatalerFehler(char *errorStr, long error);


extern Graphic_file *new_graphic_file(void);
extern void free_graphic_file(Graphic_file *gfile);
extern short graphic_file_pixel(Graphic_file *gfile, int x, int y, RGBcolor *rgb);
extern Graphic_file *read_graphic_file(char *filename);

Graphic_file *LoadPPM(FILE *fp, char *filename);
Graphic_file *LoadGIF(FILE *fp, char *fname );
Graphic_file *LoadPICTR(FILE *fp, char *fname );

#endif