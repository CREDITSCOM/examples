import math
from struct import pack

import ed25519
from thrift.protocol.TBinaryProtocol import TBinaryProtocol
from thrift.transport.TSocket import TSocket

from api.API import Client, Transaction, Amount, AmountCommission


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

    def execute_transaction(self, keys):
        res = self.client.TransactionFlow(self.create_transaction(keys))
        print(res)

    def create_transaction(self, keys):
        tr = Transaction()
        tr.id = self.client.WalletTransactionsCountGet(keys.public_key_bytes).lastTransactionInnerId + 1
        tr.source = keys.public_key_bytes
        tr.target = keys.target_public_key_bytes
        tr.amount = Amount()
        tr.amount.integral = 1
        tr.amount.fraction = 0
        tr.currency = 1

        tr.fee = AmountCommission()
        tr.fee.commission = self.__fee(0.9)

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



