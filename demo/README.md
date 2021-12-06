# Info
The following project produces a 2.2kb (2239 bytes built on Arch Linux) application which loads up OpenGL and initialises a window. 

# Building
You require:

- OpenGL (`-lgl`)
- X11 (`-lX11`)
- sstrip ([can be downloaded here](https://github.com/aunali1/super-strip), install onto your system)

# What's BetterGZ?

It's a modified version of gzexe which cuts alot of bloat out of the generated file. While this may cause it to not work properly on some systems, the size reduction is slightly imporved compared to regular gzexe. Feel free to use it in your projects if you wish.
