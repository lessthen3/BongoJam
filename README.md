# BongoJam
BongoJam is a free open source compiler + interpreter primarily intended for use in game engines

Some might say BongoJam is severely unoptimized, however here at BongoJam we prefer to say it has blazingly fast compile times

>[!WARNING]
>BongoJam is still in early alpha and extensive testing is still required. Work is being done to add features constantly, and the API is subject to breaking changes at any moment while work is being done to get BongoJam to a complete 1.0 release.

## Overall Design and Features

BongoJam is statically typed, and uses manual memory management (no GC, sorry not sorry). The syntax is an unholy amalgamation of C++ and Python



## Philosophy

### __Debugging should be easy__

My primary concern when it comes to writing code is always stability. I've rarely come across situations where performance was absolutely critical, and for those cases there is already a healthy amount of language choices.

I want low level access to computer resources, not for speed but because I enjoy explicit control over what my program is doing. When functions become black boxes that obscure their inner workings, I find debugging and coding in general far more difficult.

The Bongo Standard Library is written with this philosophy in mind.

I'm also comfortable compiling source -> native code, but catching runtime errors becomes far more involved, linker errors are the worst thing I've ever seen in my entire life, and the prospect of write once, run anywhere is nice.

Any runtime or compile time error will always have an associated line number. So you will always know which statement or bytecode instruction caused the error, and the line of code that halted execution in the stack trace.

I'm planning on adding a profiler.

## Building BongoJam for Yourself

If you want to build the compiler + runtime for yourself:

0. This project is built using __C++20__, and you will need __CMake 3.20+__ and __conan2__ (scroll down to the resources section for links if you are unfamiliar)

1. Clone the repo

2. Run: __python init.py [--debug or --release or --both] -G [desired_generator] -P [conan_profile]__ in your terminal and your done!

>[!TIP]
>For the complete list of generators run __python init.py [-h or --help]__. Also -P isn't required, if no profile is specified init.py will use the default profile

> [!NOTE]
>BongoJam can only print hello world at the moment, however I am adding more functionality in the hopes of embedding this scripting language into Peach-E

>[!IMPORTANT] 
>When ran from shell, bongojam will auto-populate the working directory with a logs folder and bytecode .bongo file. (I'm going to make the flags more robust however im tired and i wanna work on the game engine)

## Why Another Scripting Language

BongoJam isn't trying to replace any language in particular. BongoJam is just supposed to be a nice language to use, and if it's the right tool for your use case then all the better.

I'm a big advocate for using the right tool for the job, and BongoJam isn't supposed to be a one size fits all solution. BongoJam was born out of my desire to have a statically typed interpreted language that allows me the same fine grain control C++ offers. BongoJam is designed for embedding within C++ programs, but it also works just fine standalone.

## Conan Profile Settings Successfully Tested

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