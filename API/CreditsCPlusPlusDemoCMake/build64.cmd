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