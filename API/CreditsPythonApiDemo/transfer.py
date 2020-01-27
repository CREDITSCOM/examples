#!/usr/bin/env python

import sys
import glob
import base58check
import json
import hashlib
from datetime import datetime, date, time, timedelta
import time
import random
import ed25519          #Signing transaction
from struct import *    #Serialize struct
import collections
import itertools

sys.path.append('api')

from api import API
# from executor import ContractExecutor
from general.ttypes import Variant, Amount
from api.ttypes import Transaction, TransactionType, TransactionFlowResult, AmountCommission, SmartContractInvocation, Pool, PoolListGetResult, StatsGetResult, PeriodStats

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol


src_pub='public wallet key (base58)'
src_priv='private wallet key (base58)'

smartpub='public key of a smart contract (base58)'

dst1_pub='public key (base58)'
dst2_pub='public key (base58)'

def node_setup():
   # Make socket
    transport = TSocket.TSocket('127.0.0.1', 9091)

    # Buffering is critical. Raw sockets are very slow
    transport = TTransport.TBufferedTransport(transport)

    # Wrap in a protocol'
    protocol = TBinaryProtocol.TBinaryProtocol(transport)

    # Create a client to use the protocol encoder
    client = API.Client(protocol)

    # Connect!
    transport.open()
    return client


# transfer money to wallet
def send_cs(api, src_priv, src_pub,  dst_pub):
    wallet = api.WalletTransactionsCountGet(base58check.b58decode(src_pub))
    lastInnerId = bytearray((wallet.lastTransactionInnerId + 1).to_bytes(6,'little'))

    tr = Transaction()
    tr.id = int.from_bytes(lastInnerId,byteorder='little', signed=False)
    tr.source = base58check.b58decode(src_pub)
    tr.target = base58check.b58decode(dst_pub)
    # Amount
    tr.amount = Amount()
    tr.amount.integral = 0
    tr.amount.fraction = random.randint(100000000000000000,750000000000000000)  # random number

    # Balance
    tr.balance = Amount()
    tr.balance.integral = 0
    tr.balance.fraction = 0
    tr.currency = 1
    # Fee
    tr.fee = AmountCommission()
    tr.fee.commission = 19128
    serial_transaction = pack('=6s32s32slqhbb',  #'=' - without alignment
                       lastInnerId,     #6s - 6 byte InnerID (char[] C Type)
                       tr.source,       #32s - 32 byte source public key (char[] C Type)
                       tr.target,       #32s - 32 byte target pyblic key (char[] C Type)
                       tr.amount.integral, #i - 4 byte integer(int C Type)
                       tr.amount.fraction, #q - 8 byte integer(long long C Type)
                       tr.fee.commission,  #h - 2 byte integer (short C Type)
                       tr.currency,        #b - 1 byte integer (signed char C Type)
                       0)                  #b - 1 byte userfield_num
    #Calculate signing
    keydata = base58check.b58decode(src_priv)
    #Create ed25519 object
    signing_key = ed25519.SigningKey(keydata)
    #Get sign for msg
    sign = signing_key.sign(serial_transaction)
    tr.signature = sign

    print('send money:' + serial_transaction.hex())
    #Run TransactionFlow()

    res = api.TransactionFlow(tr)
    print(res.status.message)
    return res

# get a balance of the wallet
def get_balance(api, wallet_priv, wallet_id, smart_id, method, params):
    # Get last Inner ID
    wallet = api.WalletTransactionsCountGet(base58check.b58decode(wallet_id))
    lastInnerId = bytearray((wallet.lastTransactionInnerId + 1).to_bytes(6,'little'))

    tr = Transaction()
    tr.id = int.from_bytes(lastInnerId,byteorder='little', signed=False)
    tr.source = base58check.b58decode(wallet_id)
    tr.target = base58check.b58decode(smart_id)

    # Amount
    tr.amount = Amount()
    tr.amount.integral = 0
    tr.amount.fraction = 0
    # Balance
    tr.balance = Amount()
    tr.balance.integral = 0
    tr.balance.fraction = 0
    tr.currency = 1
    # Fee
    tr.fee = AmountCommission()
    tr.fee.commission = 19128 # frac=494, exp=16 => value = 0.00482422

    # SmartContractInvocation (USERFIELDS)
    tr.smartContract = SmartContractInvocation()
    tr.smartContract.method = 'register'
    tr.smartContract.params = params
    usedContracts = []
    tr.smartContract.usedContracts = usedContracts
    tr.smartContract.forgetNewState = False

    # Serialize SmartContractInvocation() by Thrift
    transportOut = TTransport.TMemoryBuffer()
    protocolOut = TBinaryProtocol.TBinaryProtocol(transportOut)
    tr.smartContract.write(protocolOut)
    serial_smart = transportOut.getvalue()


    serial_transaction = pack('=6s32s32slqhbbi',  #'=' - without alignment
                       lastInnerId,     #6s - 6 byte InnerID (char[] C Type)
                       tr.source,       #32s - 32 byte source public key (char[] C Type)
                       tr.target,       #32s - 32 byte target pyblic key (char[] C Type)
                       tr.amount.integral, #i - 4 byte integer(int C Type)
                       tr.amount.fraction, #q - 8 byte integer(long long C Type)
                       tr.fee.commission,  #h - 2 byte integer (short C Type)
                       tr.currency,        #b - 1 byte integer (signed char C Type)
                       1,                  #b - 1 byte userfield_num
                       len(serial_smart))  #i - 4 byte userfield_len

    #Add serialized SmartContractInvocation
    full_serial_transaction = serial_transaction + serial_smart
    print (tr.smartContract)
    #Calculate signing
    keydata = base58check.b58decode(wallet_priv)
    signing_key = ed25519.SigningKey(keydata)
    #Получаем цифровую подпись msg
    sign = signing_key.sign(full_serial_transaction)
    tr.signature = sign

    print('balance:' + full_serial_transaction.hex())

    #Run TransactionFlow()
    result = api.TransactionFlow(tr)
    print(result.status.message)
    return result


# register token in the wallet
def reg_token(api, wallet_priv, wallet_id, smart_id, method, params):
    # Get last Inner ID
    wallet = api.WalletTransactionsCountGet(base58check.b58decode(wallet_id))
    lastInnerId = bytearray((wallet.lastTransactionInnerId + 1).to_bytes(6,'little'))

    tr = Transaction()
    tr.id = int.from_bytes(lastInnerId,byteorder='little', signed=False)
    tr.source = base58check.b58decode(wallet_id)
    tr.target = base58check.b58decode(smart_id)

    # Amount
    tr.amount = Amount()
    tr.amount.integral = 0
    tr.amount.fraction = 0
    # Balance
    tr.balance = Amount()
    tr.balance.integral = 0
    tr.balance.fraction = 0
    tr.currency = 1
    # Fee
    tr.fee = AmountCommission()
    tr.fee.commission = 19128 # frac=494, exp=16 => value = 0.00482422 

    # SmartContractInvocation (USERFIELDS)
    tr.smartContract = SmartContractInvocation()
    tr.smartContract.method = method
    tr.smartContract.params = params
    usedContracts = []
    tr.smartContract.usedContracts = usedContracts
    tr.smartContract.forgetNewState = False

    # Serialize SmartContractInvocation() by Thrift
    transportOut = TTransport.TMemoryBuffer()
    protocolOut = TBinaryProtocol.TBinaryProtocol(transportOut)
    tr.smartContract.write(protocolOut)
    serial_smart = transportOut.getvalue()

    serial_transaction = pack('=6s32s32slqhbbi',  #'=' - without alignment
                       lastInnerId,     #6s - 6 byte InnerID (char[] C Type)
                       tr.source,       #32s - 32 byte source public key (char[] C Type)
                       tr.target,       #32s - 32 byte target pyblic key (char[] C Type)
                       tr.amount.integral, #i - 4 byte integer(int C Type)
                       tr.amount.fraction, #q - 8 byte integer(long long C Type)
                       tr.fee.commission,  #h - 2 byte integer (short C Type)
                       tr.currency,        #b - 1 byte integer (signed char C Type)
                       1,                  #b - 1 byte userfield_num
                       len(serial_smart))  #i - 4 byte userfield_len

    #Add serialized SmartContractInvocation
    full_serial_transaction = serial_transaction + serial_smart
    print (tr.smartContract)
    #Calculate signing
    keydata = base58check.b58decode(wallet_priv)
    signing_key = ed25519.SigningKey(keydata)
    #Получаем цифровую подпись msg
    sign = signing_key.sign(full_serial_transaction)
    tr.signature = sign

    print(' ' + full_serial_transaction.hex())

    #Run TransactionFlow()
    result = api.TransactionFlow(tr)
    print(result.status.message)
    return result

# transfer token to the wallet
def send_token(api, wallet_priv, wallet_id, smart_id, method, params):
    # Get last Inner ID
    wallet = api.WalletTransactionsCountGet(base58check.b58decode(wallet_id))
    lastInnerId = bytearray((wallet.lastTransactionInnerId + 1).to_bytes(6,'little'))

    tr = Transaction()
    tr.id = int.from_bytes(lastInnerId,byteorder='little', signed=False)
    tr.source = base58check.b58decode(wallet_id)
    tr.target = base58check.b58decode(smart_id)

    # Amount
    tr.amount = Amount()
    tr.amount.integral = 0
    tr.amount.fraction = 0
    # Balance
    tr.balance = Amount()
    tr.balance.integral = 0
    tr.balance.fraction = 0
    tr.currency = 1
    # Fee
    tr.fee = AmountCommission()
    tr.fee.commission = 19128 # frac=494, exp=16 => value = 0.00482422

    # SmartContractInvocation (USERFIELDS)
    tr.smartContract = SmartContractInvocation()
    tr.smartContract.method = 'transfer'
    tr.smartContract.params = params
    usedContracts = []
    tr.smartContract.usedContracts = usedContracts
    tr.smartContract.forgetNewState = False

    # Serialize SmartContractInvocation() by Thrift
    transportOut = TTransport.TMemoryBuffer()
    protocolOut = TBinaryProtocol.TBinaryProtocol(transportOut)
    tr.smartContract.write(protocolOut)
    serial_smart = transportOut.getvalue()

    serial_transaction = pack('=6s32s32slqhbbi',  #'=' - without alignment
                       lastInnerId,     #6s - 6 byte InnerID (char[] C Type)
                       tr.source,       #32s - 32 byte source public key (char[] C Type)
                       tr.target,       #32s - 32 byte target pyblic key (char[] C Type)
                       tr.amount.integral, #i - 4 byte integer(int C Type)
                       tr.amount.fraction, #q - 8 byte integer(long long C Type)
                       tr.fee.commission,  #h - 2 byte integer (short C Type)
                       tr.currency,        #b - 1 byte integer (signed char C Type)
                       1,                  #b - 1 byte userfield_num
                       len(serial_smart))  #i - 4 byte userfield_len

    #Add serialized SmartContractInvocation
    full_serial_transaction = serial_transaction + serial_smart
    print (tr.smartContract)
    #Calculate signing
    keydata = base58check.b58decode(wallet_priv)
    signing_key = ed25519.SigningKey(keydata)
    #Получаем цифровую подпись msg
    sign = signing_key.sign(full_serial_transaction)
    tr.signature = sign

    print('                             ' + full_serial_transaction.hex())

    #Run TransactionFlow()
    result = api.TransactionFlow(tr)
    print(result.status.message)
    return result


def main():
    client = node_setup()
    

# Send 5 token src_pub -> dstN_pub. dst1 and dst2 must to call Register, for example through the wallet.
    mparams = [Variant(),Variant()]
    method = 'transfer'
   
    mparams[0].v_string = dst1_pub
    mparams[1].v_string = '1.000'
    send_token(client, src_priv, src_pub, smartpub, method, mparams) 

    timeDelay = random.randrange(1, 10)
    time.sleep(timeDelay)

    mparams[0].v_string = dst2_pub
    mparams[1].v_string = '1.000'
    send_token(client, src_priv,  src_pub, smartpub, method, mparams) 



if __name__ == '__main__':
    try:
        main()
    except Thrift.TException as tx:
        print('%s' % tx.message)
