using Chaos.NaCl;
using NodeApi;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using Thrift.Protocol;
using Thrift.Transport;
using SauceControl.Blake2Fast;
using System.Text;

namespace CreditsCSAPIDemo
{
    class Keys
    {
        public string PublicKey { get; set; }
        public string PrivateKey { get; set; }
        public byte[] PublicKeyBytes
        {
            get
            {
                return PublicKey != null ? SimpleBase.Base58.Bitcoin.Decode(PublicKey).ToArray() : null;
            }
        }
        public byte[] PrivateKeyBytes
        {
            get
            {
                return PrivateKey != null ? SimpleBase.Base58.Bitcoin.Decode(PrivateKey).ToArray() : null;
            }
        }
    }

    class ClientEx
    {
        Keys sourceKeys;
        Keys targetKeys;
        API.Client client;

        public ClientEx(API.Client client, Keys sk, Keys tk)
        {
            sourceKeys = sk;
            targetKeys = tk;
            this.client = client;
        }

        public TransactionFlowResult ExecuteTransaction()
        {
            return client.TransactionFlow(CreateTransaction());
        }

        public TransactionFlowResult ExecuteTransactionWithSmartContract(string smCode)
        {
            return client.TransactionFlow(CreateTransactionWithSmartContract(smCode));
        }

        private Transaction CreateTransaction()
        {
            var transaction = new Transaction();
            transaction.Id = client.WalletTransactionsCountGet(sourceKeys.PublicKeyBytes).LastTransactionInnerId + 1;
            transaction.Source = sourceKeys.PublicKeyBytes;
            transaction.Target = targetKeys.PublicKeyBytes;
            transaction.Amount = new Amount(1, 0);
            transaction.Fee = new AmountCommission(Fee(0.9));
            transaction.Currency = 1;

            var bytes = new byte[86];
            Array.Copy(BitConverter.GetBytes(transaction.Id), 0, bytes, 0, 6);
            Array.Copy(transaction.Source, 0, bytes, 6, 32);
            Array.Copy(transaction.Target, 0, bytes, 38, 32);
            Array.Copy(BitConverter.GetBytes(transaction.Amount.Integral), 0, bytes, 70, 4);
            Array.Copy(BitConverter.GetBytes(transaction.Amount.Fraction), 0, bytes, 74, 8);
            Array.Copy(BitConverter.GetBytes(transaction.Fee.Commission), 0, bytes, 82, 2);
            bytes[84] = 1;
            bytes[85] = 0;

            var signature = Ed25519.Sign(bytes, sourceKeys.PrivateKeyBytes);
            var verifyResult = Ed25519.Verify(signature, bytes, sourceKeys.PublicKeyBytes);
            if (!verifyResult) throw new Exception("Signature could not be verified");

            transaction.Signature = signature;
            return transaction;
        }

        private byte[] Reverse(byte[] arr)
        {
            Array.Reverse(arr, 0, arr.Length);
            return arr;
        }

        private Transaction CreateTransactionWithSmartContract(string smCode)
        {
            if (smCode == "")
                smCode =
                "import com.credits.scapi.annotations.*; import com.credits.scapi.v0.*; public class MySmartContract extends SmartContract { public MySmartContract() {} public String hello() { return \"Hello\"; } }";

            var transaction = new Transaction();
            transaction.Id = client.WalletTransactionsCountGet(sourceKeys.PublicKeyBytes).LastTransactionInnerId + 1;
            transaction.Source = sourceKeys.PublicKeyBytes;
            transaction.Target = targetKeys.PublicKeyBytes;
            transaction.Amount = new Amount(0, 0);
            transaction.Fee = new AmountCommission(Fee(1.0));
            transaction.Currency = 1;

            List<byte> target = new List<byte>(transaction.Source);
            target.AddRange(BitConverter.GetBytes(transaction.Id));
            var byteCode = client.SmartContractCompile(smCode);

            if (byteCode.Status.Code == 0)
            {
                for (int i = 0; i < byteCode.ByteCodeObjects.Count; i++)
                {
                    target.AddRange(byteCode.ByteCodeObjects[i].ByteCode);
                }
            }
            else
            {
                Console.WriteLine(byteCode.Status.Message);
                return null;
            }

            transaction.SmartContract = new SmartContractInvocation();
            transaction.SmartContract.SmartContractDeploy = new SmartContractDeploy()
            {
                SourceCode = smCode,
            };
            transaction.SmartContract.ForgetNewState = false;
            transaction.Target = SauceControl.Blake2Fast.Blake2s.ComputeHash(target.ToArray());

            var tarr = new byte[6];
            var bytes = new List<byte>();

            Array.Copy(BitConverter.GetBytes(transaction.Id), 0, tarr, 0, 6);
            bytes.AddRange(tarr);
            bytes.AddRange(transaction.Source);
            bytes.AddRange(transaction.Target);
            bytes.AddRange(BitConverter.GetBytes(transaction.Amount.Integral));
            bytes.AddRange(BitConverter.GetBytes(transaction.Amount.Fraction));
            bytes.AddRange(BitConverter.GetBytes(transaction.Fee.Commission));
            bytes.Add(1);
            bytes.Add(1);

            var uf = new List<byte>();
            uf.AddRange(new byte[] { 11, 0, 1, 0, 0, 0, 0, 15, 0, 2, 12, 0, 0, 0, 0, 15, 0, 3, 11, 0, 0, 0, 0, 2, 0, 4, 0, 12, 0, 5, 11, 0, 1 });

            uf.AddRange(Reverse(BitConverter.GetBytes(smCode.Length))); //reverse ???

            uf.AddRange(Encoding.Default.GetBytes(smCode));
            uf.AddRange(new byte[] { 15, 0, 2, 12 });
            uf.AddRange(Reverse(BitConverter.GetBytes(byteCode.ByteCodeObjects.Count))); //reverse ???

            foreach (var bco in byteCode.ByteCodeObjects)
            {

                uf.AddRange(new byte[] { 11, 0, 1 });
                uf.AddRange(Reverse(BitConverter.GetBytes(bco.Name.Length))); //reverse ???
                uf.AddRange(Encoding.Default.GetBytes(bco.Name));
                uf.AddRange(new byte[] { 11, 0, 2 });
                uf.AddRange(Reverse(BitConverter.GetBytes(bco.ByteCode.Length))); //reverse ???
                uf.AddRange(bco.ByteCode);
                transaction.SmartContract.SmartContractDeploy.ByteCodeObjects = new List<ByteCodeObject>()
                {
                    new ByteCodeObject()
                    {
                        Name = bco.Name,
                        ByteCode = bco.ByteCode
                    }
                };

                uf.Add(0);
            }

            uf.AddRange(new byte[] { 11, 0, 3, 0, 0, 0, 0, 8, 0, 4, 0, 0, 0, 0, 0 });
            uf.Add(0);

            bytes.AddRange(BitConverter.GetBytes(uf.Count)); //reverse ???
            bytes.AddRange(uf.ToArray());

            var signature = Ed25519.Sign(bytes.ToArray(), sourceKeys.PrivateKeyBytes);
            var verifyResult = Ed25519.Verify(signature, bytes.ToArray(), sourceKeys.PublicKeyBytes);
            if (!verifyResult) throw new Exception("Signature could not be verified");

            transaction.Signature = signature;
            return transaction;
        }

        private short Fee(Double value)
        {
            byte sign = (byte)(value < 0.0 ? 1 : 0); // sign
            int exp;   // exponent
            long frac; // mantissa
            value = Math.Abs(value);
            double expf = value == 0.0 ? 0.0 : Math.Log10(value);
            int expi = Convert.ToInt32(expf >= 0 ? expf + 0.5 : expf - 0.5);
            value /= Math.Pow(10, expi);
            if (value >= 1.0)
            {
                value *= 0.1;
                ++expi;
            }
            exp = expi + 18;
            if (exp < 0 || exp > 28)
            {
                throw new Exception($"exponent value {exp} out of range [0, 28]");
            }
            frac = (long)Math.Round(value * 1024);
            return (short)(sign * 32768 + exp * 1024 + frac);
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Credits API Simple Demo");

            if(args.Length != 4)
            {
                Console.WriteLine("Usage: CreditsCSAPIDemo NodeIpAddress YourPublicKey YourPrivateKey TargetPublicKey");
                return;
            }

            var SourceKeys = new Keys
            {
                PublicKey = args[1],
                PrivateKey = args[2]
            };

            var TargetKeys = new Keys
            {
                PublicKey = args[3],
            };

            using (var transport = new TSocket(args[0], 9090))
            {
                using (var protocol = new TBinaryProtocol(transport))
                {
                    using (var client = new API.Client(protocol))
                    {
                        transport.Open();

                        var balance = client.WalletBalanceGet(SourceKeys.PublicKeyBytes);
                        Console.WriteLine($"[{SourceKeys.PublicKey}] Balance: {balance.Balance.ToString()}");

                        var clientEx = new ClientEx(client, SourceKeys, TargetKeys);
                        Console.WriteLine("Result of the transaction execution:");
                        //Console.WriteLine(clientEx.ExecuteTransaction());
                        Console.WriteLine(clientEx.ExecuteTransactionWithSmartContract(""));
                    }
                }
            }

            Console.WriteLine("Press [Enter] to exit...");
            Console.ReadLine();
        }

    }
}
