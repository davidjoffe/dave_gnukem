
# [dj2022-12] I used roughly the following quick n dirty steps to get it successfully building on OpenBSD ... someone who is more of a BSD expert could perhaps refine these instructions:

# su

If git not present:

# pkg_add git

If gmake not present:

# pkg_add gmake

# pkg_add gcc (?)

# pkg_add g++ (?)


Depedencies:

# pkg_add sdl2

# pkg_add sdl2-gfx

# pkg_add sdl2-mixer



# git clone https://github.com/davidjoffe/dave_gnukem

# cd dave_gnukem

# git clone https://github.com/davidjoffe/gnukem_data 

THEN: Build with this command:

# gmake -f Makefile.bsd

Then run:

# ./davegnukem

To get into X to start:

# startx


Possible future new dependencies for unicode/ttf stuff etc.:

pkg_add sdl2-ttf

pkg_add sdl2-image

etc.

