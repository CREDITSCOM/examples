## CreditsPythonApiDemo
https://developers.credits.com/en/Articles/a_Using_of_Credits_API_in_Python_3_(demo)

A simple console Python application<br>
Using the public key of the wallet displays the balance.

## Requirements
Git is a free and open source distributed version of control system designed to handle everything from small to very large projects with speed and efficiency.<br>
https://git-scm.com/

Python is a programming language that lets you work quickly and integrate systems more effectively.<br>
https://www.python.org/

The Apache Thrift software framework is designed for development of scalable cross-language services. It combines a software stack with a code generation engine in order to build services that interact efficiently and seamlessly with C++, Java, Python, PHP, Ruby, Erlang, Perl, Haskell, C#, Cocoa, JavaScript, Node.js, Smalltalk, OCaml and Delphi and other languages.<br>
https://thrift.apache.org/

## Transaction field(Length in bytes)
Id (6 bytes)<br>
Source (32 bytes)<br>
Target (32 bytes)<br>
Amount.Integral (4 bytes)<br>
Amount.Fraction (8 bytes)<br>
Fee.Commission (2 bytes)<br>
Currency (1 bytes)<br>

## Concern
It is necessary to convert commission value from double to short. For example:

```shell
// commission
tr.fee.commission = self.__fee(0.9)

def __fee(self, value):
    sign = 0
    if value < 0.0:
        value = 1
    value = abs(value)
    expf = 0
    if value != 0.0:
        expf = math.log10(value)
    if expf >= 0:
        expf = expf + .5
    else:
        expf = expf - .5
    expi = int(expf)
    value /= math.pow(10, expi)
    if value >= 1.0:
        value *= 0.1
        expi = expi + 1
    exp = expi + 18
    if exp < 0 or exp > 28:
        print('exponent value {0} out of range [0, 28]'.format(exp))
        return -1
    frac = round(value * 1024)
    return sign * 32768 + exp * 1024 + frac
```

## Using:
### Build for Windows
```shell
build-windows.cmd
```

### The content of build-windows.cmd
```shell
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
```

### Run
```shell
run.cmd
```

### The content of run.cmd
```shell
call env\Scripts\activate.bat
python app.py
call env\Scripts\deactivate.bat
pause
```

### Build for Linux:
```shell
./build-linux.sh
```
### The content of build-linux.sh
```shell
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
```

### Run
```shell
./runme.sh
```
### The content of runme.sh
```shell
env/bin/activate
python3 app.py
env/bin/deactivate
```
