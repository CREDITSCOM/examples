rm -r -f CS-API
rm -r -f api
rm -r -f general

git clone https://github.com/CREDITSCOM/thrift-interface-definitions
mkdir api
mkdir general
thrift -gen js -out .\api .\thrift-interface-definitions\api.thrift
thrift -gen js -out .\general .\thrift-interface-definitions\general.thrift
