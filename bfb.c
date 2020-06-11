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

#include <stdio.h>
#include <stdlib.h>
#include "unicode.h"
#include "bfb.h"

extern int init_bfb(
  bfb *b,
  int w_dots, int h_dots,
  unsigned short default_block) {
  b->width = (w_dots+1)>>1;
  b->height = (h_dots+3)>>2;
  b->blocks = malloc(b->width * b->height * sizeof(unsigned short));

  if (b->blocks == NULL)
    return -1;

  bfb_clear(b, default_block);
  return 0;
}

extern void free_bfb(bfb *b) {
  free(b->blocks);
}

extern void bfb_clear(bfb *b, unsigned short block_value) {
  int i;
  for (i = 0; i < b->width * b->height; i++)
    b->blocks[i] = block_value;
}

extern void bfb_home(bfb *b, FILE *fp) {
  fprintf(fp, "\x1b[%dA\x1b[G", b->height);
}

extern void bfb_fput(bfb *b, FILE *fp) {
  int row, col;

  for(row = 0; row < b->height; row++) {
    for(col = 0; col < b->width; col++) {
      unicode_fput_codepoint(0x2800 + b->blocks[row*b->width + col], fp);
    }
    fputc('\n', fp);
  }
}

static size_t mask(int x, int y) {
  static unsigned short offsets[] = {0, 3, 1, 4, 2, 5, 6, 7};
  return 1 << offsets[(y << 1) | x];
}

extern void bfb_resolve_pt(bfb_pt *pt) {
  pt->char_col = pt->x >> 1;
  pt->char_row = pt->y >> 2;
  pt->mask = mask(pt->x & 0x01, pt->y & 0x03);
}

extern void bfb_plot(bfb *b, int x, int y, int is_on) {
  bfb_pt pt = { x, y };
  bfb_resolve_pt(&pt);
  if ((pt.char_col >= 0)
      && (pt.char_col < b->width)
      && (pt.char_row >= 0)
      && (pt.char_row < b->height)) {

        if (is_on)
          b->blocks[pt.char_row * b->width + pt.char_col] |= pt.mask;
        else
          b->blocks[pt.char_row * b->width + pt.char_col] &= pt.mask ^ 0xff;
  }
}
