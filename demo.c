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

static void seed(bfb *fb, double density) {
  int n = (int)(fb->width * fb->height * 8 * density);

  do {
    int x = rand() % (fb->width * 2);
    int y = rand() % (fb->height * 4);
    if (bfb_isset(fb, x, y))
      continue;

    n--;
    bfb_plot(fb, x, y, 1);
  } while (n > 0);
}

int neighbors(bfb *fb, int x, int y) {
  return (bfb_isset(fb, x-1, y-1)
          + bfb_isset(fb, x-1, y)
          + bfb_isset(fb, x-1, y+1)
          + bfb_isset(fb, x, y-1)
          + bfb_isset(fb, x, y+1)
          + bfb_isset(fb, x+1, y-1)
          + bfb_isset(fb, x+1, y)
          + bfb_isset(fb, x+1, y+1));
}

static double get_density(int argc, char **argv) {
  double percentage = 0;

  if (argc == 1) return 0.1;

  percentage = atof(argv[1]) / 100.0;

  if (percentage < 0.0) return 0.1;

  if (percentage >= 100.0) return 1.0;

  return percentage;
}

extern int main(int argc, char **argv) {
  struct winsize w = get_winsize();
  int width = (w.ws_col-1)*2, height = (w.ws_row-1)*4;
  bfb fb, fb2;
  bfb *current_fb = &fb, *next_fb = &fb2, *temp_fb;
  int x, y;

  init_bfb(&fb, width, height, 0x0);
  init_bfb(&fb2, width, height, 0x0);

  seed(current_fb, get_density(argc, argv));

  bfb_fput(current_fb, stdout);

  while(1) {
    for(x=0; x<width; x++) {
      for(y=0; y<height; y++) {
        int n = neighbors(current_fb, x, y);
        if(bfb_isset(current_fb, x, y))
          bfb_plot(next_fb, x, y, (n == 2) || (n == 3));
        else
          bfb_plot(next_fb, x, y, (n == 3));
      }
    }
    bfb_home(current_fb, stdout);
    bfb_fput(current_fb, stdout);
    fflush(stdout);

    temp_fb = current_fb;
    current_fb = next_fb;
    next_fb = temp_fb;
  }

  free_bfb(&fb);
  free_bfb(&fb2);
  return EXIT_SUCCESS;
}
