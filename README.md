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
./frm2png PATH_TO_FILE.frm apng
```

Converted file will be saved as PATH_TO_FILE_0.png, PATH_TO_FILE_1.png, ..., PATH_TO_FILE_5.png


Compilation
===========

Dependencies
------------
- CMake >= 3.7.2

Build
```bash
git clone https://github.com/rotators/frm2png.git
git submodule update --init

mkdir Build
cd Build

cmake ../Source
cmake --build .
```
