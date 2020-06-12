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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "bfb.h"
#include "lenna.H"

/* PNM stuff */

typedef struct pnm_image {
  const unsigned char *bytes;
  const unsigned char *pixels;
  int width;
  int height;
  unsigned char type[3];
} pnm_image;

void pnm_parse_header(pnm_image *i)

  /* const unsigned char *pnm, unsigned char *type, */
  /* int *w, int *h, const unsigned char **pixels) */
{
  const unsigned char *ptr = i->bytes;
  i->type[0] = *ptr++;
  i->type[1] = *ptr++;
  i->type[2] = '\0';

  ptr++;

  i->width = atol(ptr);
  for(; *ptr != 0x20; ptr++)
    ;
  ptr++;
  fputc(*ptr, stdout);
  i->height = atol(ptr);
  ptr++;
  for(; *ptr != 0x0a; ptr++)
    ;
  ptr++;
  for(; *ptr != 0x0a; ptr++)
    ;
  ptr++;
  i->pixels = ptr;
}

double pnm_rgb_to_luma(const unsigned char *pixel)
{
  /* Y'= 0.212 * R + 0.701 * G+0.087 * B */
  return (0.212 * ((double)*pixel/255.0)
          + 0.701 * ((double)*(pixel+1)/255.0)
          + 1.000 * ((double)*(pixel+2)/255.0));
}

void pnm_xfer_fn(
  bfb *dest, const void *src,
  int x, int y, bfb_pt *pt)
{
  const pnm_image *i = (const pnm_image*)src;
  double luma = pnm_rgb_to_luma(src+(i->width*x+y)*3);
  if ((double)(rand() % 100)/100.0 > (1.0 - luma))
    bfb_pt_set(*pt);
  else
    bfb_pt_reset(*pt);
}

/* END PNM stuff */


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
  int width = fb->width;
  int height = fb->height;
  int w2 = width * 2, h4 = height * 4;
  int xm1 = (x+w2-1) % w2;
  int xp1 = (x+1) % w2;
  int ym1 = (y+h4-1) % h4;
  int yp1 = (y+1) % h4;

  return (bfb_isset(fb, xm1, ym1)
          + bfb_isset(fb, xm1, y)
          + bfb_isset(fb, xm1, yp1)
          + bfb_isset(fb, x, ym1)
          + bfb_isset(fb, x, yp1)
          + bfb_isset(fb, xp1, ym1)
          + bfb_isset(fb, xp1, y)
          + bfb_isset(fb, xp1, yp1));
}

static double get_density(int argc, char **argv) {
  double percentage = 0;

  if (argc == 1) return 0.1;

  percentage = atof(argv[1]) / 100.0;

  if (percentage < 0.0) return 0.1;

  if (percentage >= 100.0) return 1.0;

  return percentage;
}

int done = 0;

static void handle_sigint(int _) {
  done = 1;
}

extern int main(int argc, char **argv) {
  struct winsize w = get_winsize();
  int width = (w.ws_col-1)*2, height = (w.ws_row-1)*4;
  bfb fb, fb2;
  bfb *current_fb = &fb, *next_fb = &fb2, *temp_fb;
  int x, y;
  unsigned int sgr1 = 0, sgr2 = 0, sgr3 = 0;

  signal(SIGINT, handle_sigint);

  init_bfb(&fb, width, height, 0x0);
  init_bfb(&fb2, width, height, 0x0);

  for(x=0; x<w.ws_col-1; x++) {
    for(y=0; y<w.ws_row-1; y++) {
      bfb_set_chunk_attrs(&fb, x*2, y*4, sgr1, sgr2, sgr3);
      bfb_set_chunk_attrs(&fb2, x*2, y*4, sgr1, sgr2, sgr3);
    }
  }

  seed(current_fb, get_density(argc, argv));

  bfb_fput(current_fb, stdout);

  /* while(!done) { */
  /*   for(x=0; x<width; x++) { */
  /*     for(y=0; y<height; y++) { */
  /*       int n = neighbors(current_fb, x, y); */
  /*       if(bfb_isset(current_fb, x, y)) */
  /*         bfb_plot(next_fb, x, y, (n == 2) || (n == 3)); */
  /*       else */
  /*         bfb_plot(next_fb, x, y, (n == 3)); */
  /*     } */
  /*   } */
  /*   bfb_home(current_fb, stdout); */
  /*   bfb_fput(current_fb, stdout); */
  /*   fflush(stdout); */

  /*   temp_fb = current_fb; */
  /*   current_fb = next_fb; */
  /*   next_fb = temp_fb; */
  /* } */

  pnm_image image = { MagickImage };
  pnm_parse_header(&image);

  if (strcmp(image.type, "P6") != 0)
    return EXIT_FAILURE;

  bfb_blit(
    current_fb, (const void *)&image,
    0, 0,
    pnm_xfer_fn,
    24, image.width, image.height,
    0.25, 0.25);

  bfb_home(current_fb, stdout);
  bfb_fput(current_fb, stdout);

  free_bfb(&fb);
  free_bfb(&fb2);

  fputs("\x1b[0m", stdout);

  fprintf(
    stdout,
    "pnm type %s width %d height %d pnm %x pixels %x\n",
    image.type, image.width, image.height,
    image.bytes, image.pixels);

  return EXIT_SUCCESS;
}
