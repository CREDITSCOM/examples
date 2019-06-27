import hashlib
import math
from struct import pack

import ed25519
from thrift.protocol.TBinaryProtocol import TBinaryProtocol
from thrift.transport.TSocket import TSocket

from api.API import Client, Transaction, AmountCommission, SmartContractInvocation, SmartContractDeploy
from general.ttypes import Amount, ByteCodeObject


class ClientEx:

    def __init__(self, address):
        self.tr = TSocket(address[0], address[1])
        self.protocol = TBinaryProtocol(self.tr)
        self.client = Client(self.protocol)
        self.tr.open()

    def close(self):
        self.tr.close()

    def wallet_balance_get(self, pub_key_bytes):
        return self.client.WalletBalanceGet(pub_key_bytes)

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

    def transfer_coins(self, integral, fraction, fee, keys):
        res = self.client.TransactionFlow(self.create_transaction(integral, fraction, fee, keys))
        print(res)

    def create_transaction(self, integral, fraction, fee, keys):
        tr = Transaction()
        tr.id = self.client.WalletTransactionsCountGet(keys.public_key_bytes).lastTransactionInnerId + 1
        tr.source = keys.public_key_bytes
        tr.target = keys.target_public_key_bytes
        tr.amount = Amount()
        tr.amount.integral = integral
        tr.amount.fraction = fraction
        tr.currency = 1

        tr.fee = AmountCommission()
        tr.fee.commission = self.__fee(fee)

        serial_transaction = pack('=6s32s32slqhbb',                       # '=' - without alignment'
                                  bytearray(tr.id.to_bytes(6, 'little')), # 6s - 6 byte InnerID (char[] C Type)
                                  tr.source,                              # 32s - 32 byte source public key (char[] C Type)
                                  tr.target,                              # 32s - 32 byte target pyblic key (char[] C Type)
                                  tr.amount.integral,                     # i - 4 byte integer(int C Type)
                                  tr.amount.fraction,                     # q - 8 byte integer(long long C Type)
                                  tr.fee.commission,                      # h - 2 byte integer (short C Type)
                                  tr.currency,                            # b - 1 byte integer (signed char C Type)
                                  0                                       # b - 1 byte userfield_num
        )

        signing_key = ed25519.SigningKey(keys.private_key_bytes)
        sign = signing_key.sign(serial_transaction)
        tr.signature = sign

        return tr

    def deploy_smart_contract(self, code, fee, keys):
        res = self.client.TransactionFlow(self.create_transaction_with_smart_contract(code, fee, keys))
        print(res)

    def create_transaction_with_smart_contract(self, code, fee, keys):

        if code == "":
            code = 'import com.credits.scapi.annotations.*; import com.credits.scapi.v0.*; public class ' \
                   'MySmartContract extends SmartContract { public MySmartContract() {} public String hello2(String ' \
                   'say) { return \"Hello\" + say; } }';

        tr = Transaction()
        tr.id = self.client.WalletTransactionsCountGet(keys.public_key_bytes).lastTransactionInnerId + 1
        tr.source = keys.public_key_bytes
        tr.target = keys.target_public_key_bytes
        tr.amount = Amount()
        tr.amount.integral = 0
        tr.amount.fraction = 0
        tr.currency = 1

        tr.fee = AmountCommission()
        tr.fee.commission = self.__fee(fee)

        serial_transaction = pack('=6s32s32slqhbb',                       # '=' - without alignment'
                                  bytearray(tr.id.to_bytes(6, 'little')), # 6s - 6 byte InnerID (char[] C Type)
                                  tr.source,                              # 32s - 32 byte source public key (char[] C Type)
                                  tr.target,                              # 32s - 32 byte target pyblic key (char[] C Type)
                                  tr.amount.integral,                     # i - 4 byte integer(int C Type)
                                  tr.amount.fraction,                     # q - 8 byte integer(long long C Type)
                                  tr.fee.commission,                      # h - 2 byte integer (short C Type)
                                  tr.currency,                            # b - 1 byte integer (signed char C Type)
                                  1                                       # b - 1 byte userfield_num
        )

        target = pack('=6s', bytearray(tr.id.to_bytes(6, 'little')))
        byte_code = self.client.SmartContractCompile(code)
        if byte_code.status.code == 0:
            for bco in byte_code.byteCodeObjects:
                target = target + bco.byteCode
        else:
            print(byte_code.Status.Message)
            return 'compile error'

        tr.smartContract = SmartContractInvocation()
        tr.smartContract.smartContractDeploy = SmartContractDeploy()
        tr.smartContract.smartContractDeploy.sourceCode = code

        tr.smartContract.ForgetNewState = False
        tr.target = hashlib.blake2s(target).hexdigest()

        uf = bytearray(b'\x11\x00\x01\x00\x00\x00\x00\x015\x00\x02\x12\x00\x00\x00\x00\x15\x00\x03\x11\x00\x00\x00\x00\x02\x00\x04\x00\x12\x00\x05\x11\x00\x01')

        uf = uf + pack('=6s', self.reverse(len(code)))
        uf = uf + bytearray(code.encode())
        uf = uf + bytearray(b'\x15\x00\x02\x12')
        uf = uf + self.reverse(len(byte_code.byteCodeObjects))

        for bco in byte_code.byteCodeObjects:
            uf = uf + b'1101'
            uf = uf + self.reverse(len(bco.name))
            uf = uf + bytearray(bco.name.encode())
            uf = uf + b'1102'
            uf = uf + self.reverse(len(bco.byteCode))
            uf = uf + bco.byteCode

            nbco = ByteCodeObject()
            nbco.name = bco.name
            nbco.byteCode = bco.byteCode

            tr.smartContract.smartContractDeploy.byteCodeObjects = [nbco]

            uf = uf + b'\x00'

        uf = uf + b'\x11\x00\x03\x00\x00\x00\x00\x08\x00\x04\x00\x00\x00\x00\x00'
        uf = uf + b'\x00'

        serial_transaction = serial_transaction + self.reverse(len(uf))
        serial_transaction = serial_transaction + uf

        signing_key = ed25519.SigningKey(keys.private_key_bytes)
        sign = signing_key.sign(serial_transaction)
        tr.signature = sign

        return tr

    def reverse(self, a):
        a = a.to_bytes(6, 'little')
        a = bytearray(a)
        a.reverse()
        return a

