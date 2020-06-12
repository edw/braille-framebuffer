/* Copyright 2020 Edwin Watkeys */

/* Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: */

/* The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef __BFB_H__
#define __BFB_H__

#include <stdio.h>

typedef struct bfb_block {
  unsigned int pattern : 8;
  unsigned int sgr : 8;
} bfb_block;

typedef struct bfb {
  bfb_block *blocks;
  int width, height;
} bfb;

typedef struct bfb_pt {
  int x;
  int y;
  int char_col;
  int char_row;
  unsigned int mask : 8;
} bfb_pt;

int init_bfb(bfb *b, int w_dots, int h_dots, unsigned short default_block);
void free_bfb(bfb *b);
void bfb_clear(bfb *b, unsigned short block_value);
void bfb_home(bfb *b, FILE *fp);
void bfb_fput(bfb *b, FILE *fp);
void bfb_resolve_pt(bfb_pt *pt);
void bfb_plot(bfb *b, int x, int y, int is_on);
int bfb_isset(bfb *b, int x, int y);
void bfb_set_attrs(bfb *b, int x, int y, unsigned int sgr);

#endif
