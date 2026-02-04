# Bau 'ma

## What is it mainly?

What is Bau 'ma?

Bau 'ma is my fun/hobby project to build a simple, minimalistic build and packaging system for C and C++. Ideally, only a bunch of .c, .cpp and .h files is needed for a project and all can be built and packaged with a single command without having any script, CMake file or whatsoever. I mean, why not? Rust can also do that with cargo. So it is kind of an anti-approach to CMake, conan, vcpackage and all of that stuff. Bau' ma will call the compiler, linker and other tools directly, witout having another intermediate build tool such as `Make` involved. That reduces risk that something goes wrong to a minimum. When using transitive dependencies and downloading other code while building, external programs like `Git` might get called for doing that.

The project is in a very early stage, so don't expect anything yet.

"Bau 'ma" is german slang and translates to something like "Just go and build it".

This is possible by having conventions about how code is organised, so that the build system can detect that.

For example, if the source repository only has a folder named `include/` with only header files in it, it is considered a header only library. The name of
such a library is extracted from the first sub-folder in `include/`, so when something like `include/myproj` is found, it knows that the header-only-library should be named `myproj`.

## What it is as well

It is also a fun place to try things out, like minimalistic C header-only general purpose libs that have little or no dependencies. Those libs could
be used anywhere else since they can be just downloaded and used without package managing. Development of it should be fun since it is a hobby project anyways.

## What it is not

Bau 'ma should make life easier, so it isn't anything comparable to other build systems or package managers. If you need mega-configurability and invoke a thousand tools, it won't be the right thing. Or it only will be the right thing when combined with other stuff that handles the complexity. Ideally, it is a build system for very portable general purpose C and C++ code that was witten to be built with Bau 'ma and the described conventions.

# Conventions

Conventions help. Even when later transitioning away from Bau 'ma and using something else to build, the conventions can stay and help to have everything pretty and organized.

## General

Conventions are preferred over configuration. When configuration is needed, it should be as descriptive and short as possible. For instance, transitive dependencies can't be modeled with a simple source directory layout, so some kind of description where to pull those dependencies is needed.

TODO: Brief description about project configuration file. Idea: Some sort of declarative `bauma.yml` in the root of a source repo.

Also, some kind of toolchain profiles are needed so Bau 'ma knows what commands to invoke to build a project.

## Applications

An application has a `src/` directory in the source tree, containing a `main.c` or `main.cpp` file. The file extension determines if it is a C or a C++ project.
That would be an unnamed application; if it should have a name, the main file should be in a sub folder named as the application, so `src/myapp/main.cpp` is a C++ application named `myapp`.

## Libraries

A library is organised into a `src/` subfolder for private sources and a `include/` folder for public header files. By default, a statically linked library is assumed. When the `main.c`/`main.cpp` is present as described in "Application", the project is compiled and linked to be an executable. When `src/` is missing and only `include/` is there, it is assumed to be a header-only library for which no compilation/linking steps are done.

When a project should be compiled and linked as dynamically linked/shared library, a `bauma.yml` file is needed because that can't be auto-detected.

## Specifying the sources to be compiled

By default, all code found is compiled: `.c` files as C code, `.cpp` or `.cc` or `.cxx` as C++ code, but `.cpp` should be used as a standard. When code should only be compiled under a specific OS, compiler, architecture or build type (such as `debug` vs. `release`), it should be organized in a folder such as `src/<projectname>/only_<something>/`, for example `src/myproj/only_os_windows/mycode.cpp` will only be compiled when the operating system for which it is built is windows. That way, portable code can be organized. The `only_` approach will work for architectures as well, so code in `src/myproj/only_arch_x86/` will only be built when the architecture for what is built is `x86`. Same for `src/myproj/only_build_debug` which has code for debug builds.

# How Bau'ma itself is built and organized

The idea is to have a simple build system written in the common sub-set of both C++98 and C89, which I call "Eternal C" because it's basically a subset which will
work basically forever. C as language isn't really going anywhere but is available everywhere, which is a very good thing. Yes, it is an unsafe language, but we are experts and know how to deal with that *smile*. Common sub-set means that the incompatible regions between C and C++ are avoided or abstracted away.

For what is incompatible, read https://en.wikipedia.org/wiki/Compatibility_of_C_and_C%2B%2B.

When having only a single `bauma.c` file and all of the needed libraries are in the same project and organized as header-only libraries, nothing but the C standard library and OS syscalls or OS-dependent libraries are needed and everything is buildable with a single compiler call such as `gcc -o bauma main.c`

The supplementary libraries can be coded in "stb header-only library" style and are thus consumable without having an external build system. All of the implementations and unit testing code will be contained per-library in the same `.h` file as the declarations, so a single compiler call can also build the unit test of each library (treating the `.h` file like a `.c` file by running the compiler on it, adding preprocessor flags so the unit tests are compiled and linked as well)

## bauma.yml file format

TODO

## Toolchain profile file format

TODO

# License

Bau' ma is licensed under the MIT License.
