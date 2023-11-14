# Building on OpenBSD

## Install dependencies
```sh
$ doas pkg_add git gmake sdl2-gfx sdl2-image sdl2-mixer sdl2-ttf
```

## Get a copy of the code
```sh
$ git clone https://github.com/davidjoffe/dave_gnukem
$ cd dave_gnukem
$ git clone https://github.com/davidjoffe/gnukem_data
```

## Build
```sh
$ gmake -f Makefile.bsd
```

## Running
```sh
$ ./davegnukem
```

## As an alternative to building yourself, install the package
```sh
$ doas pkg_add gnukem
```
