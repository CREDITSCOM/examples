import hashlib
import math
from struct import pack

import ed25519
from thrift.protocol.TBinaryProtocol import TBinaryProtocol
from thrift.transport.TSocket import TSocket
from thrift.transport.TTransport import TMemoryBuffer
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

    def double_to_fee(self,value):
        fee_comission = 0
        a = True
        if value < 0.:
            fee_comission += 32768
        else:
            fee_comission += (32768 if value < 0. else 0)
            value = math.fabs(value)
            expf = (0. if value == 0. else math.log10(value))
            expi = int(expf + 0.5 if expf >= 0. else expf - 0.5)
            value /= math.pow(10, expi)
            if value >= 1.:
                value *= 0.1
                expi += 1
            fee_comission += int(1024*(expi + 18))
            fee_comission += int(value * 1024)
        return fee_comission

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

    def createContractAddress(self, source, tId, contract):
        tmpBytes = bytearray()
        tmpBytes.extend(source)
        tmpBytes.extend(tId)
        for a in contract.smartContractDeploy.byteCodeObjects:
            tmpBytes.extend(a.byteCode)
        res = hashlib.blake2s()
        res.update(tmpBytes)
        return res.digest()

    def normalizeCode(self,javaText):
        javaText = javaText.replace('\r', ' ').replace('\t', ' ').replace('{', ' {')
        while '  ' in javaText:
            javaText = javaText.replace('  ', ' ')
        return javaText    

    def compile_smart(self,contract_body):
        if self.client == None:
            return None
        res = self.client.SmartContractCompile(contract_body)
        return res

    def create_transaction_with_smart_contract(self, code, fee, keys):
        tr = Transaction()
        contract = SmartContractInvocation()
        contract.smartContractDeploy = SmartContractDeploy()
        if code == "":
            code = 'import com.credits.scapi.annotations.*; import com.credits.scapi.v0.*; public class ' \
                   'MySmartContract extends SmartContract { public MySmartContract() {} public String hello2(String ' \
                   'say) { return \"Hello\" + say; } }';
        contractText = self.normalizeCode(code)
        result = self.compile_smart(contractText)
        contract.smartContractDeploy.byteCodeObjects = result.byteCodeObjects
        tr.smartContract = contract
        tr.smartContract.smartContractDeploy.sourceCode = contractText
        tr.source = keys.public_key_bytes
        w = self.client.WalletTransactionsCountGet(tr.source)
        lastInnerId = bytearray((w.lastTransactionInnerId + 1).to_bytes(6,'little'))
        tr.id = int.from_bytes(lastInnerId,byteorder='little', signed=False)
        tr.target = self.createContractAddress(tr.source, lastInnerId, contract)
        tr.amount = Amount()
        tr.amount.integral = 0
        tr.amount.fraction = 0
        tr.balance = Amount()
        tr.balance.integral = 0
        tr.balance.fraction = 0
        tr.currency = 1
        tr.fee = AmountCommission()
        tr.fee.commission = self.double_to_fee(fee)
        tr.userFields = ""
        ufNum1 = bytearray(b'\x01')
        contract.smartContractDeploy.hashState = ""
        contract.smartContractDeploy.tokenStandard = 0
        contract.method = ""
        contract.params = []
        contract.usedContracts = []
        contract.forgetNewState = False
        transportOut = TMemoryBuffer()
        protocolOut = TBinaryProtocol(transportOut)
        contract.write(protocolOut)
        scBytes = transportOut.getvalue()
        sMap = '=6s32s32slqhb1s4s' + str(len(scBytes)) +'s' #4s' + str(scriptLength) + 's4s' + str(codeNameLength) + 's4s' + str(codeLength) + 's' #len(userField_bytes)
        serial_transaction_for_sign = pack(sMap,  #'=' - without alignment
                            lastInnerId,     #6s - 6 byte InnerID (char[] C Type)
                            tr.source,       #32s - 32 byte source public key (char[] C Type)
                            tr.target,       #32s - 32 byte target pyblic key (char[] C Type)
                            tr.amount.integral, #i - 4 byte integer(int C Type)
                            tr.amount.fraction, #q - 8 byte integer(long long C Type)
                            tr.fee.commission,  #h - 2 byte integer (short C Type)
                            tr.currency,        #b - 1 byte integer (signed char C Type)
                            ufNum1,
                            bytes(len(scBytes).to_bytes(4, byteorder="little")),
                            scBytes
                            )                     
        signing_key = ed25519.SigningKey(keys.private_key_bytes) # Create object for calulate signing
        tr.signature = signing_key.sign(serial_transaction_for_sign)
        return tr

    def reverse(self, a):
        a = a.to_bytes(6, 'little')
        a = bytearray(a)
        a.reverse()
        return a

