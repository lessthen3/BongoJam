# Bongo-Jam
 BONGO JAM

BongoJam is an open-source compiler + interpreter.

If you want to build the compiler + runtime for yourself:

0. This project is built using __C++20__, and you will need __CMake 3.20+__ and __conan2__ (scroll down to the resources section for links if you are unfamiliar)

1. Clone the repo

2. Run: __python init.py --debug or --release or --both__ in your terminal and your done!

__Note:__ BongoJam can only print hello world at the moment, however I am adding more functionality in the hopes of embedding this scripting language into Peach-E

### Conan Profile Settings

```ini
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=20
compiler.runtime=dynamic
compiler.version=193
os=Windows
```

### Resources:

[Latest CMake Download](https://cmake.org/download/)

[Latest Conan Download](https://conan.io/downloads)