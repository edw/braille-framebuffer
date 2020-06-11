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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "bfb.h"

static struct winsize get_winsize() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  return w;
}

extern int main(int argc, char **argv) {
  struct winsize w = get_winsize();
  int width = (w.ws_col-1)*2, height = (w.ws_row-1)*4;
  bfb bm;
  double theta;
  double r = (double)height * 0.45;
  double aspect = (double)width/(double)height;
  double xc = (double)width/2, yc = (double)height/2;
  int steps = 5000;
  int loops = 10;
  double decay = 99.95e-2;
  int i;
  int x, y;

  init_bfb(&bm, width, height, 0xff);

  bfb_fput(&bm, stdout);

  for (i = 0; i < steps; i++) {
    theta = 2.0 * M_PI * ((double)i/((double)steps/(double)loops));
    r *= decay;
    x = (int)(aspect*r*cos(theta) + xc);
    y = (int)(r*sin(theta) + yc);
    bfb_plot(&bm, x, y, 0);

    bfb_home(&bm, stdout); /* repainting on each interation is brutal */
    bfb_fput(&bm, stdout); /* performance-wise; thank your lucky stars */
    fflush(stdout);        /* you're not using a 110 baud modem. */
  }
  free_bfb(&bm);

  return EXIT_SUCCESS;
}
