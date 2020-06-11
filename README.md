# A Braille Unicode Framebuffer Library for the Terminal in C89

## Building

```
make demo
./demo
```

## To-do

* Allow a chunk to be colored or have another terminal effect applied
  to it e.g. inverse or bold.

* Sprites.

* Routines to place text overlays and lines atop the image.

## Using bfb.c

The interface is extremely straightforward. You should be able to do
whatever you want with the following routines:

`int init_bfb(bfb *b, int w_dots, int h_dots, unsigned short default_block)`

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

Output the framebuffer to `fp` using UTF-8 and ANSI escape sequences.

`void bfb_home(bfb *b, FILE *fp)`

Send ANSI escape code contortions to return the terminal cursor to a
location so that repeated calls to `bfb_put` will exactly overprint
each other.

`void bfb_plot(bfb *b, int x, int y, int is_on)`

Set the "pixel" at (x,y) to either "white" or "black" based on whether
`is_on` is true or not.

That's about it. Pull requests welcome.

## Motivation

The subject of text graphics have been coming up on Hacker News a lot
recently, and I thought it would be fun to create a little library for
drawing inside the terminal.
