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
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Credits API Simple Demo");

            if(args.Length != 5)
            {
                Console.WriteLine("Usage: CreditsCSAPIDemo NodeIpAddress NodePort YourPublicKey YourPrivateKey TargetPublicKey");
                return;
            }

            using (var client = new Client(args[0], Convert.ToInt32(args[1]), args[2], args[3], args[4]))
            {
                var balance = client.WalletGetBalance();
                Console.WriteLine($"[{client.keys.PublicKey}] Balance: {balance.Balance.ToString()}");

                Console.WriteLine("Result of the transfer coins:");
                Console.WriteLine(client.TransferCoins(1, 0, 0.9));
                
                Console.WriteLine(client.DeploySmartContract(""));
            }

            Console.WriteLine("Press [Enter] to exit...");
            Console.ReadLine();
        }

    }
}
