
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

    executeTransactinWithSmartContract(feeValue, smCode) {
        var tran = this.createTransactionWithSmartContract(feeValue, smCode);
        if(tran === null)
            return null;
            
        var tranFlow = this.client().TransactionFlow(tran);
        console.log(tranFlow);
        return tranFlow;
    }

    createTransactionWithSmartContract(feeValue, smCode) {

        if(smCode == '')
            smCode = 
            "import com.credits.scapi.annotations.*; import com.credits.scapi.v0.*; public class MySmartContract extends SmartContract { public MySmartContract() {} public String hello() { return \"Hello\"; } }";

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
        tran.amount = new Amount({integral: 0, fraction: 0});

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

        let target = tran.source;
        target = this.concatTypedArrays(target, this.numbToByte(tran.id, 6));
        let byteCode = this.client().SmartContractCompile(smCode);
        if (byteCode.status.code === 0) {
            for (let i in byteCode.byteCodeObjects) {
                target = this.concatTypedArrays(target, this.convertCharToByte(byteCode.byteCodeObjects[i].byteCode));
            }
        }
        else {
            ResObj.Message = ByteCode.status.message;
            return ResObj;
        }

        tran.target = this.blake2s(target);

        let PerStr = this.numbToByte(tran.id, 6);
        PerStr = this.concatTypedArrays(PerStr, tran.source);
        PerStr = this.concatTypedArrays(PerStr, tran.target);
        PerStr = this.concatTypedArrays(PerStr, this.numbToByte(tran.amount.integral, 4));
        PerStr = this.concatTypedArrays(PerStr, this.numbToByte(tran.amount.fraction, 8));
        PerStr = this.concatTypedArrays(PerStr, this.numbToByte(tran.fee.commission, 2));
        PerStr = this.concatTypedArrays(PerStr, new Uint8Array([1]));
        PerStr = this.concatTypedArrays(PerStr, new Uint8Array(1));

        let UserField = new Uint8Array();
        tran.smartContract = new SmartContractInvocation();
        UserField = this.concatTypedArrays(UserField, new Uint8Array([11, 0, 1]));
        UserField = this.concatTypedArrays(UserField, new Uint8Array(4));

        UserField = this.concatTypedArrays(UserField, new Uint8Array([15, 0, 2, 12]));
        UserField = this.concatTypedArrays(UserField, new Uint8Array(4));

        UserField = this.concatTypedArrays(UserField, new Uint8Array([15, 0, 3, 11, 0, 0, 0, 0]));

        tran.smartContract.forgetNewState = false;
        UserField = this.concatTypedArrays(UserField, new Uint8Array([2, 0, 4, 0]));

        if (smCode !== undefined) {
            UserField = this.concatTypedArrays(UserField, new Uint8Array([12, 0, 5, 11, 0, 1]));

            tran.smartContract.smartContractDeploy = new SmartContractDeploy({
                sourceCode: smCode
            });

            UserField = this.concatTypedArrays(UserField, this.numbToByte(smCode.length, 4).reverse());
            UserField = this.concatTypedArrays(UserField, this.convertCharToByte(smCode));

            UserField = this.concatTypedArrays(UserField, new Uint8Array([15, 0, 2, 12]));
            let ByteCode = this.client().SmartContractCompile(smCode);

            if (ByteCode.status.code === 0) {
                tran.smartContract.smartContractDeploy.byteCodeObjects = [];
                UserField = this.concatTypedArrays(UserField, this.numbToByte(ByteCode.byteCodeObjects.length, 4).reverse());

                for (let i in ByteCode.byteCodeObjects) {
                    
                    let val = ByteCode.byteCodeObjects[i];
                    UserField = this.concatTypedArrays(UserField, new Uint8Array([11, 0, 1]));
                    UserField = this.concatTypedArrays(UserField, this.numbToByte(val.name.length, 4).reverse());
                    UserField = this.concatTypedArrays(UserField, this.convertCharToByte(val.name));

                    UserField = this.concatTypedArrays(UserField, new Uint8Array([11, 0, 2]));
                    UserField = this.concatTypedArrays(UserField, this.numbToByte(val.byteCode.length, 4).reverse());
                    UserField = this.concatTypedArrays(UserField, this.convertCharToByte(val.byteCode));
                    tran.smartContract.smartContractDeploy.byteCodeObjects.push(new ByteCodeObject({
                        name: val.name,
                        byteCode: val.byteCode
                    }));
                    UserField = this.concatTypedArrays(UserField, new Uint8Array(1));
                }
            }
            else {
                ResObj.Message = ByteCode.Status.Message;
                return ResObj;
            }

            UserField = this.concatTypedArrays(UserField, new Uint8Array([11, 0, 3, 0, 0, 0, 0, 8, 0, 4, 0, 0, 0, 0, 0]));
        }

        UserField = this.concatTypedArrays(UserField, new Uint8Array(1));
        PerStr = this.concatTypedArrays(PerStr, this.numbToByte(UserField.length, 4));
        PerStr = this.concatTypedArrays(PerStr, UserField);

        tran.signature = nacl.sign.detached(PerStr, this.privateKeyByte);
        return tran;
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

    BLAKE2S_IV = new Uint32Array([
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
        0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19])

    convertCharToByte(Str) {
        let B = new Uint8Array(Str.length);
        for (let i in Str) {
            B[i] = Str[i].charCodeAt();
        }
        return B;
    }

    blake2sInit(outlen, key) {
        if (!(outlen > 0 && outlen <= 32)) {
            throw new Error('Incorrect output length, should be in [1, 32]')
        }
        var keylen = key ? key.length : 0
        if (key && !(keylen > 0 && keylen <= 32)) {
            throw new Error('Incorrect key length, should be in [1, 32]')
        }

        var ctx = {
            h: new Uint32Array(this.BLAKE2S_IV),
            b: new Uint32Array(64),
            c: 0,
            t: 0,
            outlen: outlen
        }
        ctx.h[0] ^= 0x01010000 ^ (keylen << 8) ^ outlen

        if (keylen > 0) {
            this.blake2sUpdate(ctx, key)
            ctx.c = 64
        }

        return ctx
    }

    blake2sUpdate(ctx, input) {
        for (var i = 0; i < input.length; i++) {
            if (ctx.c === 64) {
                ctx.t += ctx.c
                this.blake2sCompress(ctx, false)
                ctx.c = 0
            }
            ctx.b[ctx.c++] = input[i]
        }
    }

    blake2sFinal(ctx) {
        ctx.t += ctx.c
        while (ctx.c < 64) {
            ctx.b[ctx.c++] = 0
        }
        this.blake2sCompress(ctx, true)

        var out = new Uint8Array(ctx.outlen)
        for (var i = 0; i < ctx.outlen; i++) {
            out[i] = (ctx.h[i >> 2] >> (8 * (i & 3))) & 0xFF
        }
        return out
    }

    blake2s(input, key, outlen) {
        outlen = outlen || 32

        var ctx = this.blake2sInit(outlen, key)
        this.blake2sUpdate(ctx, input)
        return this.blake2sFinal(ctx)
    }

    ROTR32(x, y) {
        return (x >>> y) ^ (x << (32 - y))
    }

    SIGMA = new Uint8Array([
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3,
        11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4,
        7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8,
        9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13,
        2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9,
        12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11,
        13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10,
        6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5,
        10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0])

    v = new Uint32Array(16)
    m = new Uint32Array(16)

    B2S_GET32(v, i) {
        return v[i] ^ (v[i + 1] << 8) ^ (v[i + 2] << 16) ^ (v[i + 3] << 24)
    }

    B2S_G(a, b, c, d, x, y) {
        this.v[a] = this.v[a] + this.v[b] + x
        this.v[d] = this.ROTR32(this.v[d] ^ this.v[a], 16)
        this.v[c] = this.v[c] + this.v[d]
        this.v[b] = this.ROTR32(this.v[b] ^ this.v[c], 12)
        this.v[a] = this.v[a] + this.v[b] + y
        this.v[d] = this.ROTR32(this.v[d] ^ this.v[a], 8)
        this.v[c] = this.v[c] + this.v[d]
        this.v[b] = this.ROTR32(this.v[b] ^ this.v[c], 7)
    }

    blake2sCompress(ctx, last) {
        var i = 0
        for (i = 0; i < 8; i++) {
            this.v[i] = ctx.h[i]
            this.v[i + 8] = this.BLAKE2S_IV[i]
        }

        this.v[12] ^= ctx.t
        this.v[13] ^= (ctx.t / 0x100000000)
        if (last) {
            this.v[14] = ~this.v[14]
        }

        for (i = 0; i < 16; i++) {
            this.m[i] = this.B2S_GET32(ctx.b, 4 * i)
        }

        for (i = 0; i < 10; i++) {

            this.B2S_G(0, 4, 8, 12, this.m[this.SIGMA[i * 16 + 0]], this.m[this.SIGMA[i * 16 + 1]])
            this.B2S_G(1, 5, 9, 13, this.m[this.SIGMA[i * 16 + 2]], this.m[this.SIGMA[i * 16 + 3]])
            this.B2S_G(2, 6, 10, 14, this.m[this.SIGMA[i * 16 + 4]], this.m[this.SIGMA[i * 16 + 5]])
            this.B2S_G(3, 7, 11, 15, this.m[this.SIGMA[i * 16 + 6]], this.m[this.SIGMA[i * 16 + 7]])
            this.B2S_G(0, 5, 10, 15, this.m[this.SIGMA[i * 16 + 8]], this.m[this.SIGMA[i * 16 + 9]])
            this.B2S_G(1, 6, 11, 12, this.m[this.SIGMA[i * 16 + 10]], this.m[this.SIGMA[i * 16 + 11]])
            this.B2S_G(2, 7, 8, 13, this.m[this.SIGMA[i * 16 + 12]], this.m[this.SIGMA[i * 16 + 13]])
            this.B2S_G(3, 4, 9, 14, this.m[this.SIGMA[i * 16 + 14]], this.m[this.SIGMA[i * 16 + 15]])
        }

        for (i = 0; i < 8; i++) {
            ctx.h[i] ^= this.v[i] ^ this.v[i + 8]
        }
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

