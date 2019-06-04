## CreditsJSDemo
https://developers.credits.com/en/Articles/a_Using_of_Credits_API_in_JS_(demo)

A simple console JS application<br>
Using the public key of the wallet displays the balance.

## Requirements
Git is a free and open source distributed version of control system designed to handle everything from small to very large projects with speed and efficiency.<br>
https://git-scm.com/

## Transaction field(Length in bytes)
Id (6 bytes)<br>
Source (32 bytes)<br>
Target (32 bytes)<br>
Amount.Integral (4 bytes)<br>
Amount.Fraction (8 bytes)<br>
Fee.Commission (2 bytes)<br>
Currency (1 bytes)<br>

## Which transaction fields must be filled
```shell
var tran = new Transaction();

//Internal transaction number (id)
let res = this.client().WalletTransactionsCountGet(this.publicKeyByte);
if (res.status.code === 0) {
    tran.id = res.lastTransactionInnerId + 1;
}
else {
    return null;
}

//Original/input/initial wallet byte array
tran.source = this.publicKeyByte;
//Target wallet byte array
tran.target = this.targetKeyByte;
//Quantity of transferable coins
tran.amount = new Amount({integral: amountVal, fraction: 0});

let F = this.fee(feeValue);
let FE = this.numbToBits(F.exp);
while (FE.length < 5){
    FE = "0" + FE;
}
let FM = this.numbToBits(F.man);
while (FM.length < 10) {
    FM = "0" + FM;
}

//Commission
tran.fee = new AmountCommission({
    commission: this.bitsToNumb("0" + FE + FM)
});

tran.currency = 1;
```

Field completion for transaction signature<br>
It is necessary create a byte array with a size of 85 bytes<br>
Last element of the array is filled out with zero (0).<br>

## Concern
It is necessary to convert commission value from double to short. For example:
```shell

fee(v) {
    let s = v > 0 ? 0 : 1;
    v = Math.abs(v);
    let exp = v === 0 ? 0 : Math.log10(v);
    exp = Math.floor(exp >= 0 ? exp + 0.5 : exp - 0.5);
    v /= Math.pow(10, exp);
    if (v >= 1) {
        v *= 0.1;
        ++exp;
    }
    v = Number((v * 1024).toFixed(0));
    return { exp: exp + 18, man: v === 1024 ? 1023 : v };
}
```

## Using:
### Build for Windows
```shell
build.cmd
```

### The content of build.cmd
```shell
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
```

### Run
Opem the index.html int the favorite browser

### Build for Linux:
```shell
./build.sh
```

### The content of build.sh
```shell
rm -r -f CS-API
rm -r -f api
rm -r -f general

git clone https://github.com/CREDITSCOM/thrift-interface-definitions
mkdir api
mkdir general
thrift -gen js -out .\api .\thrift-interface-definitions\api.thrift
thrift -gen js -out .\general .\thrift-interface-definitions\general.thrift
```

### Run
Open the index.html in the your favorite browser
