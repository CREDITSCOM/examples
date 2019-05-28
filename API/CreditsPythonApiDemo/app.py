from thrift.protocol.TBinaryProtocol import TBinaryProtocol
from thrift.transport.TSocket import TSocket
import base58

from api.API import Client

publicKey = '5B3YXqDTcWQFGAqEJQJP3Bg1ZK8FFtHtgCiFLT5VAxpe'

publicKeyBytes = base58.b58decode(publicKey)

try:

   tr = TSocket('169.38.89.217', 9090)
   protocol = TBinaryProtocol(tr)
   client = Client(protocol)
   tr.open()

   balance = client.WalletBalanceGet(publicKeyBytes)
   print(balance)

   transactionGetResult = client.WalletTransactionsCountGet(publicKeyBytes)
   print(transactionGetResult)

except:

   print("Oops. Unexpected error.")