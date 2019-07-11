rmdir /S /Q build64
rmdir /S /Q thrift-interface-definitions
rmdir /S /Q thrift
rmdir /S /Q api
rmdir /S /Q libsodium
rmdir /S /Q blake2

git clone https://github.com/jedisct1/libsodium.git
git clone https://github.com/CREDITSCOM/blake2.git

git clone https://github.com/CREDITSCOM/thrift-interface-definitions
mkdir api
thrift -r -gen cpp:no_skeleton,pure_enums,moveable_types -out .\api .\thrift-interface-definitions\api.thrift

git clone https://github.com/CREDITSCOM/thrift
cd thrift

cd ..
mkdir build64
cd build64

cmake .. -DBOOST_ROOT=d:\boost -DCMAKE_BUILD_TYPE=Debug -A x64 ..
cmake  --build . --config Debug

rem cmake -DBOOST_ROOT=d:\boost -DCMAKE_BUILD_TYPE=Release -A x64 ..
rem cmake  --build . --config Release

pause