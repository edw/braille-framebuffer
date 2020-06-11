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

extern void unicode_fput_codepoint(unsigned codepoint, FILE *fp) {
  unsigned b1, b2, b3, b4, cp = codepoint;

  if (cp < 0x80) {
    b1 = codepoint;
    fputc(b1, fp);

  } else if (cp < 0x800) {
    b2 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b1 = cp | 0xc0;
    fputc(b1, fp);
    fputc(b2, fp);

  } else if (cp < 0x10000) {
    b3 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b2 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b1 = cp | 0xe0;
    fputc(b1, fp);
    fputc(b2, fp);
    fputc(b3, fp);

  } else {
    b4 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b3 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b2 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b1 = cp | 0xf0;
    fputc(b1, fp);
    fputc(b2, fp);
    fputc(b3, fp);
    fputc(b4, fp);
  }
}

extern void unicode_fput_codepoint_description(
  unsigned codepoint, FILE *fp) {

  unsigned b1, b2, b3, b4, cp = codepoint;

  if (cp < 0x80) {
    b1 = codepoint;
    fprintf(fp, "%x (%c): %x", codepoint, b1, b1);

  } else if (cp < 0x800) {
    b2 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b1 = cp | 0xc0;
    fprintf(fp, "%x (%c%c): %x %x", codepoint, b1, b2, b1, b2);

  } else if (cp < 0x10000) {
    b3 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b2 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b1 = cp | 0xe0;
    fprintf(fp, "%x (%c%c%c): %x %x %x",
            codepoint, b1, b2, b3, b1, b2, b3);

  } else {
    b4 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b3 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b2 = (cp & 0x3f) | 0x80;
    cp >>= 6;
    b1 = cp | 0xf0;
    fprintf(fp, "%x (%c%c%c%c): %x %x %x %x",
            codepoint, b1, b2, b3, b4,
            b1, b2, b3, b4);
  }
}
