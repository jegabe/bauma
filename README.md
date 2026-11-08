# Bau 'ma

## TL;DR

Bau 'ma is one of the most simplistic ways of building a C- or C++ project
and should make programming a joy without thinking about the build system too much.

Usage example:

```
# Create a file src/hello_world/main.c file
bauma
```

Bau'ma detects that as application and will create a `hello_world` executable.

## What is it mainly?

What is Bau 'ma?

Bau 'ma is a fun/hobby project to build a simple, minimalistic build and packaging system for C and C++. Ideally, only a bunch of .c, .cpp and .h files is needed for a project and all can be built and packaged with a single command without having any script, CMake file or whatsoever. So it is kind of an anti-approach to CMake, conan, vcpackage and all of that stuff. Bau' ma will call the compiler, linker and other tools directly, without having another intermediate build tool such as `Make` involved. That reduces risk that something goes wrong to a minimum. When using transitive dependencies and downloading other code while building, external programs like `Git` might get called for doing that.

Bau 'ma only consists of a bunch of header files and one C file and can be compiled and linked by any C compiler with one command. So,
additionally to bauma being simplistic, building of it is also simplistic. Goal is to make it easy for developers to change bauma and
build it without effort.

The project is in a very early stage, so don't expect anything yet.

"Bau 'ma" is german slang and translates to something like "Just go and build it".

This simplicity is possible by having conventions about how code is organised, so that the build system can detect that (convention over configuration
approach).

## What it is as well

It is also a fun place to try things out, like minimalistic C header-only general purpose libs that have little or no dependencies. Those libs (parts of bauma) can be used anywhere else since they can be just downloaded and used without package managing. Development of it should be fun since it is a hobby project anyways.

## What it is not

Bau 'ma should make life easier, so it isn't anything comparable to other build systems or package managers. If you need mega-configurability and invoke a thousand tools, it won't be the right thing. Or it only will be the right thing when combined with other stuff that handles the complexity. Ideally, it is a build system for very portable general purpose C and C++ code that was witten to be built with Bau 'ma and the described conventions.

# Conventions

Conventions help. Even when later transitioning away from Bau 'ma and using something else to build, the conventions can stay and help to have everything pretty and organized. Anyone who uses CMake or package managers like conan know what a pain it is to deal with code that doesn't
follow conventions

## General

Conventions are preferred over configuration. When configuration is needed, it should be as descriptive and short as possible. For instance, transitive dependencies can't be modeled with a simple source directory layout, so some kind of description where to pull those dependencies is needed.

The name of a library or application is inferred from the sub-directory name of `include/` or `src/`. So, if a project has a
`include/hello_world` sub directory and/or a `src/hello_world` sub directory, the name of the current application/library is inferred
as "hello_world"

For libraries, public header files are placed in `include/<project name>`. Private header files go to `src/<project_name>`

Header files should be named as:

- C projects: File extension `.h`
- C++ projects: File extensions `.h` or `.hpp` or `.hxx`

Source files should be named as:

- C projects: File extension `.c`
- C++ projects: File extensions `.cpp` or `.cc` or `.cxx`
- Assembler: File extensions `.s`, `.S` or `.asm`

Header file extensions aren't critical and could also deviate from that standard, but best practise is to
have everything consistent.

We prefer: `.h` for header files (both C and C++) and `.c` for C source files and `.cpp` for C++ source files
and `.S` or `.asm` for assembler files, depending on the conventions of the build environment

## Applications

An application has a `src/` directory in the source tree, inside of it a sub-directory containing a `main.c` or `main.cpp` file. The file extension determines if it is a C or a C++ project.
The main file should be in a sub folder named as the application, so `src/myapp/main.cpp` is a C++ application named `myapp`.

## Libraries

A library is organised into a `src/` subfolder for private sources and a `include/` folder for public header files. By default, a statically linked library is assumed. When the `main.c`/`main.cpp` is present as described in "Application", the project is compiled and linked to be an executable. When `src/` is missing and only `include/` is there, it is assumed to be a header-only library for which no compilation/linking steps are done.

When a project should be compiled and linked as dynamically linked/shared library, a `bauma.nj` configuration file is needed
in the root of the project because that can't be auto-detected.

## Specifying the sources to be compiled

By default, all code found is compiled: `.c` files as C code, `.cpp` or `.cc` or `.cxx` as C++ code, but `.cpp` should be used as a standard. When code should only be compiled under a specific OS, compiler, architecture or build type (such as `debug` vs. `release`), it should be organized in a folder such as `src/<projectname>/only_<something>/`, for example `src/myproj/only_os_windows/mycode.cpp` will only be compiled when the operating system for which it is built is windows. That way, portable code can be organized. The `only_` approach will work for architectures as well, so code in `src/myproj/only_arch_x86/` will only be built when the architecture for what is built is `x86`. Same for `src/myproj/only_build_debug` which has code for debug builds.

# How Bau'ma itself is built and organized

The idea is to have a simple build system written in the common sub-set of both C++98 and C89, which I call "Eternal C" because it's basically a subset which will basically compile forever. C as language isn't really going anywhere but is available everywhere, which is a very good thing. Yes, it is an old and unsafe language, but we are experts and know how to deal with that *smile*. Common sub-set means that the incompatible regions between C and C++ are avoided or abstracted away.

For what is incompatible, read https://en.wikipedia.org/wiki/Compatibility_of_C_and_C%2B%2B.

Bau 'ma only consists of a bunch of general purpose header-only libraries in form of some `.h` files and one `.c` file as entry point.
One single command line can build all of Bau 'ma in one go.

## bauma.nj file format

TODO

## Profiles

Bau 'ma somehow needs to know how to invoke compiler, assembler linker and maybe other tools. This is defined in a 
profile file.

The default profile is chosen when no profile is passed via command line. It is stored in the user's folder under the hidden path
`<user home folder>/.bauma/dflt_dbg_prfl.nj` and is used for debug builds (without optimizations and using debug C/C++ runtime).

That file, and its `release` pendant called `<user home folder>/.bauma/dflt_rls_prfl.nj` can be auto-detected by Bau'ma: Under Linux, the default GCC installation will be scanned; under Windows, the latest
version of Visual Studio will be scanned.

The command for that is:

```
bauma profile detect
```

That command will create those files

## Profile file format

# License

Bau' ma is licensed under the MIT License.
