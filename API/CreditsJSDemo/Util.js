
class CreditsUtils {

    constructor(publicKey, privateKey, targetKey, url) {
        this.publicKey = publicKey;
        this.privateKey = privateKey;
        this.targetKey = targetKey;
        this.url = url;

        this.publicKeyByte = from_b58(this.publicKey);
        this.privateKeyByte = from_b58(this.privateKey);
        this.targetKeyByte = from_b58(this.targetKey);
    }

    client() {
        var transport = new Thrift.Transport(this.url);
        var protocol = new Thrift.Protocol(transport);
        return new APIClient(protocol);
    }

    walletGetBalance() {
        var balance = this.client().WalletBalanceGet(this.publicKeyByte).balance;
        return balance;
    }

    publicKey() {
        return this.publicKey;
    }

    privateKey() {
        return this.privateKey;
    }

    executeTransactin(amountVal, feeValue) {
        var tran = this.createTransaction(amountVal, feeValue);
        if(tran === null)
            return null;
            
        var tranFlow = this.client().TransactionFlow(tran);
        console.log(tranFlow);
        return tranFlow;
    }

    createTransaction(amountVal, feeValue) {
        var tran = new Transaction();

        let res = this.client().WalletTransactionsCountGet(this.publicKeyByte);
        if (res.status.code === 0) {
            tran.id = res.lastTransactionInnerId + 1;
        }
        else {
            return null;
        }

        tran.source = this.publicKeyByte;
        tran.target = this.targetKeyByte;
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

        tran.fee = new AmountCommission({
            commission: this.bitsToNumb("0" + FE + FM)
        });

        tran.currency = 1;

        let PerStr = this.numbToByte(tran.id, 6);
        PerStr = this.concatTypedArrays(PerStr, tran.source);
        PerStr = this.concatTypedArrays(PerStr, tran.target);
        PerStr = this.concatTypedArrays(PerStr, this.numbToByte(tran.amount.integral, 4));
        PerStr = this.concatTypedArrays(PerStr, this.numbToByte(tran.amount.fraction, 8));
        PerStr = this.concatTypedArrays(PerStr, this.numbToByte(tran.fee.commission, 2));
        PerStr = this.concatTypedArrays(PerStr, new Uint8Array([1]));
        PerStr = this.concatTypedArrays(PerStr, new Uint8Array(1));
        tran.signature = nacl.sign.detached(PerStr, this.privateKeyByte);
        return tran;
    }

    concatTypedArrays(a, b) {
        var c = new (Uint8Array.prototype.constructor)(a.length + b.length);
        c.set(a, 0);
        c.set(b, a.length);
        return c;
    }

    bitsToNumb(Bits) {
        let numb = 0;
        let mnoj = 1;
        for (var i = Bits.length - 1; i > 0; i -= 1) {
            if (Bits[i] !== 0) {
                numb += mnoj * Bits[i];
            }
            mnoj *= 2;
        }
        return numb;
    }

    numbToBits(int) {
        let Bits = "";

        let numb = String(int);
        while (true) {
            Bits = (numb % 2) + Bits;
            numb = Math.floor(numb / 2);

            if (numb <= 1) {
                Bits = numb + Bits;
                break;
            }
        }

        return Bits;
    }

    numbToByte(numb, CountByte) {
        let InnerId = new Uint8Array(CountByte);
        numb = String(numb);
        let i = 1;
        let index = 0;
        while (true) {
            InnerId[index] += (numb % 2) * i;
            numb = Math.floor(numb / 2);
            if (numb === 0) {
                break;
            }
            if (numb === 1) {
                var b = (numb % 2) * i * 2;
                if (b === 256) {
                    ++InnerId[index + 1];
                } else {
                    InnerId[index] += (numb % 2) * i * 2;
                }
                break;
            }

            if (i === 128) {
                i = 1;
                index++;
            } else {
                i *= 2;
            }
        }
        return InnerId;
    }

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
}

