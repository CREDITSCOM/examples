## CreditsCPlusPlusDemoCMake
https://developers.credits.com/en/Articles/a_Using_Credits_API_in_C_(demo)_Cmake_Cross_platform

A simple console C++ (CMake) application<br>
Using the public key of the wallet displays the balance.

## Requirements
Git is a free and open source distributed version control system designed to handle everything from small to very large projects with speed and efficiency.<br>
https://git-scm.com/

Boost is a set of libraries for the C++ programming language that provide support for tasks and structures such as linear algebra, pseudorandom number generation, multithreading, image processing, regular expressions, and unit testing. It contains over eighty individual libraries.<br>
https://www.boost.org/

CMake is an open-source, cross-platform family of tools designed to build, test and package software. CMake is used to control the software compilation process using simple platform and compiler independent configuration files, and generate native makefiles and workspaces that can be used in the compiler environment of your choice. The suite of CMake tools were created by Kitware in response to the need for a powerful, cross-platform build environment for open-source projects such as ITK and VTK.<br>
https://cmake.org/

The Apache Thrift software framework, for scalable cross-language services development, combines a software stack with a code generation engine to build services that work efficiently and seamlessly between C++, Java, Python, PHP, Ruby, Erlang, Perl, Haskell, C#, Cocoa, JavaScript, Node.js, Smalltalk, OCaml and Delphi and other languages.<br>
https://thrift.apache.org/

## Using:
### Build for Windows
```shell
build64.cmd
```

### The content of build64.cmd
```shell
rmdir /S /Q build64
rmdir /S /Q CS-API
rmdir /S /Q thrift
rmdir /S /Q api

git clone https://github.com/CREDITSCOM/thrift-interface-definitions
mkdir api
thrift -r -gen cpp:no_skeleton,pure_enums,moveable_types -out .\api .\thrift-interface-definitions\api.thrift

git clone https://github.com/CREDITSCOM/thrift
cd thrift

cd ..
mkdir build64
cd build64

cmake .. -A x64 
cmake  --build . --config Debug
cmake  --build . --config Release

pause
```

### Run
```shell
run.cmd
```

### The content of run.cmd
```shell
.\build64\release\main.exe
pause
```

### Build for Linux:
```shell
./build64.sh
```

### The content of build64.sh
```shell
rm -r -f build64
rm -r -f CS-API
rm -r -f thrift
rm -r -f api

git clone https://github.com/CREDITSCOM/thrift-interface-definitions
mkdir api
thrift -r -gen cpp:no_skeleton,pure_enums,moveable_types -out ./api ./thrift-interface-definitions/api.thrift

git clone https://github.com/CREDITSCOM/thrift
cd thrift

cd ..
mkdir build64
cd build64

cmake ..
cmake  --build . --config Debug
cmake  --build . --config Release
```

### Run
```shell
./runme.sh
```

### The content of run.sh
```shell
./build64/main
```
