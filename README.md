frm2png
=======
Fallout .frm to .png converter.


Usage
=====

```bash
frm2png PATH_TO_FILE.frm
```

Converted file will be saved as PATH_TO_FILE.png

```bash
frm2png --generator apng PATH_TO_FILE.frm
```

Converted file will be saved as PATH_TO_FILE_0.png, PATH_TO_FILE_1.png, ..., PATH_TO_FILE_5.png


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
