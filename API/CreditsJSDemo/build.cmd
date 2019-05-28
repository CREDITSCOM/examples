rmdir /S /Q build64
rmdir /S /Q CS-API
rmdir /S /Q thrift
rmdir /S /Q api
rmdir /S /Q general

git clone https://github.com/CREDITSCOM/thrift-interface-definitions
mkdir api
mkdir general
thrift.exe -gen js -out .\api .\thrift-interface-definitions\api.thrift
thrift.exe -gen js -out .\general .\thrift-interface-definitions\general.thrift
