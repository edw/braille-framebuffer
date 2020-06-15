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

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "bfb.h"

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

  i->width = atol((const char *)ptr);
  for(; *ptr != 0x20; ptr++)
    ;
  ptr++;
  fputc(*ptr, stdout);
  i->height = atol((const char *)ptr);
  ptr++;
  for(; *ptr != 0x0a; ptr++)
    ;
  ptr++;
  for(; *ptr != 0x0a; ptr++)
    ;
  ptr++;
  i->pixels = ptr;
}

void rgb_to_luma_hue_saturation(
  const unsigned char *pixel,
  double *luma, double *hue, double *saturation,
  double r_coeff, double g_coeff, double b_coeff)
{
  double r = ((double)*pixel)/255.0 * r_coeff;
  double g = ((double)*(pixel+1))/255.0 * g_coeff;
  double b = ((double)*(pixel+2))/255.0 * b_coeff;

  *luma =  0.2126*r + 0.715*g + 0.722*b;

  double m, M, C, Hp;
  m = r < g ? r : g;
  m = m < b ? m : b;
  M = r < g ? g : r;
  M = M < b ? b : M;
  C = M - m;

  if (C == 0.0) {
    Hp = 0.0;
  } else if (M == r) {
    Hp = remainder(60.0 * ((g-b)/C) + 360.0, 360.0);
  } else if (M == g) {
    Hp = remainder(60.0 * ((b-r)/C) + 120.0, 360.0);
  } else {
    Hp = remainder(60.0 * ((r-g)/C) + 240.0, 360.0);
  }

  if (Hp < 0.0)
    *hue = Hp + 360.0;
  else
    *hue = Hp;

  if ((*luma <= 0.0) || (*luma >= 1.0))
    *saturation = 0.0;
  else
    *saturation = C/(1.0 - fabs(2.0*(*luma) - 1.0));
}

int hue_to_ansi_color(double hue_degs)
{
  if (hue_degs < 30.0)
    return 31;
  else if (hue_degs < 90.0)
    return 33;
  else if (hue_degs < 150.0)
    return 32;
  else if (hue_degs < 210.0)
    return 36;
  else if (hue_degs < 270.0)
    return 35;
  else
    return 31;
}

typedef struct pnm_xfer_options {
  double r_coeff, g_coeff, b_coeff;
  double luma_coeff, sat_thresh;
} pnm_xfer_options;

void pnm_xfer_fn(
  bfb *dest, const void *src,
  int x, int y, bfb_pt *pt, void *refcon)
{
  pnm_xfer_options *opts = (pnm_xfer_options *)refcon;
  const pnm_image *i = (const pnm_image*)src;
  const unsigned char *pixels = i->pixels;

  if (pt->block == NULL)
    return;

  int offset = 3 * (y * i->width + x);
  double luma, hue, sat;

  rgb_to_luma_hue_saturation(
    &pixels[offset], &luma, &hue, &sat,
    opts->r_coeff, opts->g_coeff, opts->b_coeff);

  if (sat >= opts->sat_thresh)
    pt->block->sgr1 = hue_to_ansi_color(hue);

  if (opts->luma_coeff * (rand() % 100)/100.0 > (1.0 - luma))
    bfb_pt_set(pt);
  else
    bfb_pt_reset(pt);
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
  int w2 = fb->width * 2, h4 = fb->height * 4;
  int xm1 = (x+w2-1) % w2;
  int xp1 = (x+1) % w2;
  int ym1 = (y+h4-1) % h4;
  int yp1 = (y+1) % h4;

  return (
    bfb_isset(fb, xm1, ym1)
    + bfb_isset(fb, xm1, y)
    + bfb_isset(fb, xm1, yp1)
    + bfb_isset(fb, x, ym1)
    + bfb_isset(fb, x, yp1)
    + bfb_isset(fb, xp1, ym1)
    + bfb_isset(fb, xp1, y)
    + bfb_isset(fb, xp1, yp1)
    );
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

  while(!done) {
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

  int pnm_fd = open("demo.pnm", O_RDONLY);
  if (pnm_fd < 0)
    return EXIT_FAILURE;

  struct stat pnm_stat_buf;
  if (fstat(pnm_fd, &pnm_stat_buf) < 0)
    return EXIT_FAILURE;

  unsigned char *pnm_bytes;
  if ((pnm_bytes = mmap(0, pnm_stat_buf.st_size,
                        PROT_READ, MAP_SHARED,
                        pnm_fd, 0))
      == MAP_FAILED)
    return EXIT_FAILURE;
  pnm_image image = {pnm_bytes};
  pnm_parse_header(&image);

  if (strcmp((const char *)image.type, "P6") != 0)
    return EXIT_FAILURE;

  double y_scale = (height * 0.9) / image.height;
  double x_scale = y_scale * 1.3;
  int image_y = (int)(height - image.height*y_scale) * 0.35;
  int image_x = (int)(width - image.width*x_scale) * 0.5;
  pnm_xfer_options opts = { 0.5, 0.8, 1.5, 1.0, 0.01};
  bfb_blit(
    current_fb, (const void *)&image,
    image_x, image_y,
    pnm_xfer_fn,
    image.width, image.height,
    x_scale, y_scale, &opts);

  munmap((void *)image.bytes, pnm_stat_buf.st_size);
  close(pnm_fd);

  bfb_home(current_fb, stdout);
  bfb_fput(current_fb, stdout);

  finalize_bfb(&fb);
  finalize_bfb(&fb2);

  fputs("\x1b[0m", stdout);

  return EXIT_SUCCESS;
}
