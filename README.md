# BongoJam
BongoJam is a free open source compiler + interpreter primarily intended for use in game engines

Some might say BongoJam is severely unoptimized, however here at BongoJam we prefer to say it has blazingly fast compile times

>[!WARNING]
>BongoJam is still in early alpha and extensive testing is still required. Work is being done to add features constantly, and the API is subject to breaking changes at any moment while work is being done to get BongoJam to a complete 1.0 release.

## Overall Design and Features

BongoJam is statically typed, and uses a GC. The syntax is an unholy amalgamation of C++ and Python

## Philosophy

### __Debugging should be easy__

My primary concern when it comes to writing code is always stability. I've rarely come across situations where performance was absolutely critical, and for those cases there is already a healthy amount of language choices.

I want low level access to computer resources, not for speed but because I enjoy explicit control over what my program is doing. When functions become black boxes that obscure their inner workings, I find debugging and coding in general far more difficult. The Bongo Standard Library is written with this philosophy in mind.

I'm also comfortable compiling source -> native code, but catching runtime errors becomes far more involved and linker errors are the worst thing I've ever seen in my entire life.

I'm planning on adding a profiler.

## Building BongoJam for Yourself

If you want to build the compiler + runtime for yourself:

0. This project is built using __C++20__, and you will need [__CMake 3.20+__](https://cmake.org/download/)

1. Clone the repo

2. Run: __python init.py [--debug | --release | --both] -G [desired_generator]__ in your terminal and your done!

>[!TIP]
>For the complete list of generators and commands run __python init.py [-h | --help]__

> [!NOTE]
>Build output:
> * bongoC (static lib)— BongoJam compiler
> * bongo_runtime (static lib)— BongoJam Interpreter/Runtime
> * bongo (executable)— BongoJam CLI

## Why Another Scripting Language

BongoJam isn't trying to replace any language in particular. BongoJam is just supposed to be a nice language to use, it isn't supposed to be a one size fits all solution. BongoJam was born out of my desire to have a statically typed python. BongoJam is designed for embedding within C++ programs, but it also works just fine standalone.

## Platforms Successfully Tested

```ini
os=Windows
arch=x86_64
compiler=msvc
compiler.runtime=static
compiler.version=193
```