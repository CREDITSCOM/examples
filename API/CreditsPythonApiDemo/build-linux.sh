git clone https://github.com/CREDITSCOM/thrift-interface-definitions
thrift -gen py -out . ./thrift-interface-definitions/general.thrift
thrift -gen py -out . ./thrift-interface-definitions/api.thrift
python3 -m venv env
chmod +x env/bin/activate
env/bin/activate
pip3 install thrift
pip3 install base58
pip3 install ed25519
env/bin/deactivate
