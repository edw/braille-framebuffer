# A Braille Unicode Framebuffer Library for the Terminal in C89

![A screenshot](https://poseur.com/the-game-of-life-beatriz-et-madeleine-in-braille.png)

Fun fact: if you set your terminal to 141 by 49, your terminal will,
compared to an Apple II+, have the same resolution, far more colors,
and a slightly different set of rules constraining their use.


## Prerequisites

* A C compiler.
* A POSIX-y operating system.
* A terminal that support ANSI positioning and color escape sequences.
* A UTF-8 locale.

## Building

```
make demo
./demo
```

## To-do

* Sprites.

* Routines to place text overlays and lines atop the image.

* Translate bitmap to braille codepoint values more efficiently via a
  lookup table.

## Using bfb.c

The interface is extremely straightforward. You should be able to do
whatever you want with the following routines:

`int init_bfb(bfb *b, int dot_width, int dot_height, unsigned short default_block)`

Pass a pointer to your `brb` value along with the width and height in
dots and the default braille codepoint minus 0x2800 to populate the
framebbuffer. 0x is "black," 0xff is "white," and you can look up any
other value you like on
[Wikipedia](https://en.wikipedia.org/wiki/Braille_Patterns#Identifying,_naming_and_ordering).

`void free_bfb(bfb *b)`

When you're done, call this. It frees the framebuffer data inside the
`brb` structure.

`void bfb_clear(bfb *b, unsigned short block_value)`

Sets all the blocks in the framebufer to the same braille value as
when the the framebuffer was initialized.

`void bfb_fput(bfb *b, FILE *fp)`

Output the framebuffer to `fp` using UTF-8 and ANSI escape
sequences. Resets terminal to normal (zero) SGR rendition.

`void bfb_home(bfb *b, FILE *fp)`

Send ANSI escape code contortions to return the terminal cursor to a
location so that repeated sequences of `bfb_fput()` and `bfb_home()`
will exactly overprint each other.

`void bfb_plot(bfb *b, int x, int y, int is_on)`

Set the "pixel" at (x,y) to either "white" or "black" based on whether
`is_on` is true or not.

`int bfb_isset(bfb *b, int x, int y)`

Returns true if the pixel at (x,y) is "white."

`bfb_set_chunk_attrs(bfb *b, int dot_x, int dot_y, unsigned int sgr1, unsigned int sgr2, unsigned int sgr3, )`

Set's the ANSI SGR ("select graphic rendition") codes for the block
that contains (dot_x, dot_y). Note `bfb_fput()` will keep track of the
precviously-set SGR codes for each provided code argument and will not
output a new escape sequence if 1) that particlar code (e.g. `sgr2`)
has not changed since the previously printed chunk or 2) the code is
the same any SGR code that preceded it in the current chunk. This
approach is intended to allow all SGR codes to be set to zero by
default and have the output appear as expected. The number of SGR
escape sequences sent will be the sum of the SGR code _transitions_
within a framebuffer.

`void bfb_blit(
  bfb *dest, const void *src,
  int at_dest_x, int at_dest_y,
  bfb_xfer_fn transfer_fn,
  unsigned int src_width,
  unsigned int src_height,
  double x_scale, double y_scale, void *refcon)`

Blit `src` into `dest` using `transfer_fn`. The `bfb_blit`
implemenetaton does not look inside `src`; it simply passes it to the
transfer function. Due to scaling, the transfer function may or may
not be called for each pixel in the image, and it is possible the
transfer function will be called multiple times on the same pixel. The
`refcon` value is passed to the transfer function and is intended for
passing options to it. Consult the header file and demo program for
details on the transfer function's signature.

## Contributions

Pull requests welcome.

## Motivation

The subject of text graphics have been coming up on Hacker News a lot
recently, and I thought it would be fun to create a little library for
drawing inside the terminal.

## Demo Image

The demo image is copyright 2000 Edwin Watkeys. Permission is granted
to use the image only in conjuction with execution of the `demo`
program. All other rights reserved. The rest of this project is
distributed under an MIT-style license.

## Refereneces

* [The PPM image format.](http://netpbm.sourceforge.net/doc/ppm.html)
* [ANSI escape codes.](https://en.wikipedia.org/wiki/ANSI_escape_code)
* [HSL and HSV color representations.](https://en.wikipedia.org/wiki/HSL_and_HSV)
* ["Code Signing for the Concerned."](https://eclecticlight.co/2019/01/17/code-signing-for-the-concerned-3-signing-an-app/)