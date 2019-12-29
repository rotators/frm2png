frm2png
=======
Fallout .frm to .png converter.


Usage
=====

```
  frm2png [--help|--version]
  frm2png ([-p <PAL>] | [-P <name>]) [-g <name>] [-o <PNG>] [-V] <filename.frm>

General options
  --help, -h                  show help summary
  --version, -v               show program version

Input options
  -p, --pal <PAL>             Use specified PAL file
  -P, --palette <name>        Use embedded palette

Output options
  -g, --generator <name>      generator
  -o, --output <PNG>          output filename

Misc options
  -V, --verbose               prints various debug messages
```

Compilation
===========

Dependencies
------------
- CMake >= 3.7.2
- [clipp](https://github.com/muellan/clipp/) (included as git submoule)
- [libpng-apng](https://sourceforge.net/projects/libpng-apng/) (included as git submodule)
- [zlib](https://github.com/madler/zlib/) (included as git submodule)

Build
```bash
git clone https://github.com/rotators/frm2png.git
git submodule update --init

mkdir Build
cd Build

cmake ../Source
cmake --build . --config Release
```
