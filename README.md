# BongoJam
BongoJam is an open-source compiler + interpreter primarily intended for use in game engines

BongoJam is statically typed, and uses manual memory management (no GC, sorry not sorry)

The syntax is an unholy amalgamation of Cpp and Python with a sprinkle of Rust, because I like Python and Rust

Some might say BongoJam is severely unoptimized, however here at BongoJam we prefer to say it has blazingly fast compile times

## Philosophy

### __Debugging should be easy.__

My priorites in order are:

1. Debugging
2. Readability
3. Co-operation
4. Performance

Any runtime or compile-time error will always have an associated line number. So you will always know which statement or byte-code instruction caused the error, and the line of code that halted execution in the stack-trace.

I'm planning on adding a profiler.

## Building BongoJam for Yourself

If you want to build the compiler + runtime for yourself:

0. This project is built using __C++20__, and you will need __CMake 3.20+__ and __conan2__ (scroll down to the resources section for links if you are unfamiliar)

1. Clone the repo

2. Run: __python init.py --debug or --release or --both__ in your terminal

3. You'll need LLVM 18.1.8 to build BongoJam, the current CMakeLists.txt expects to find the includes in:

### __"${PROJECT_SOURCE_DIR}/static_libs/llvm/include"__

__Note:__ BongoJam can only print hello world at the moment, however I am adding more functionality in the hopes of embedding this scripting language into Peach-E

__IMPORTANT:__ When ran from the cmd line, bongojam will auto-populate the working directory with a logs folder and bytecode .bongo file. (I'm going to make the flags more robust however im tired and i wanna work on the game engine)

## Conan Profile Settings

The following is the conan profile I use to build BongoJam on my Windows machine:

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

## Resources

[Latest CMake Download](https://cmake.org/download/)

[Latest Conan Download](https://conan.io/downloads)