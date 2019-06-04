rmdir /S /Q thrift-interface-definitions
rmdir /S /Q api
rmdir /S /Q general
rmdir /S /Q env

git clone https://github.com/CREDITSCOM/thrift-interface-definitions
thrift -gen py -out . .\thrift-interface-definitions\general.thrift
thrift -gen py -out . .\thrift-interface-definitions\api.thrift
python -m venv env
call env\Scripts\activate.bat
pip install thrift
pip install base58
pip install ed25519
call env\Scripts\deactivate.bat
pause