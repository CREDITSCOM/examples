## CreditsCSharpDemo
https://developers.credits.com/en/Articles/a_Using_of_Credits_API_in_C_(demo)

A simple console C# (.NET Core) application.<br>
Using the public key of the wallet displays the balance.<br>
Allows to create a transaction (transfer coins from the current wallet to the specified one) and execute it.

## Requirements

Git is a free and open source distributed version of control system designed to handle everything from small to very large projects with speed and efficiency.<br>
https://git-scm.com/

.NET is a free, cross-platform, open source developer platform designed to build many different types of applications. With .NET, you can use multiple languages, editors, and libraries to build for web, mobile, desktop, gaming, and IoT.<br>
https://dotnet.microsoft.com/download

The Apache Thrift software framework is designed for development of scalable cross-language services. It combines a software stack with a code generation engine in order to build services that interact efficiently and seamlessly with C++, Java, Python, PHP, Ruby, Erlang, Perl, Haskell, C#, Cocoa, JavaScript, Node.js,Smalltalk, OCaml and Delphi and other languages.<br>
https://thrift.apache.org/

## Creation and execution of transactions with Credits API support (in C#)
https://developers.credits.com/en/Articles/a_Creating_and_execution_of_transaction_C_

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
//Create a transaction (transfer from a wallet to another wallet)
var transaction = new Transaction();

//Internal transaction number (id)
transaction.Id = client.WalletTransactionsCountGet(sourceKeys.PublicKeyBytes).LastTransactionInnerId + 1;

//Original/input/initial wallet byte array
transaction.Source = sourceKeys.PublicKeyBytes;

//Target wallet byte array
transaction.Target = targetKeys.PublicKeyBytes;

//Quantity of transferable coins
transaction.Amount = new Amount(1, 0);

//Commission
transaction.Fee = new AmountCommission(Fee(0.9));

transaction.Currency = 1;
```

Field completion for transaction signature<br>
It is necessary create a byte array with a size of 85 bytes<br>
Last element of the array is filled out with zero (0).<br>

## Concern
It is necessary to convert commission value from double to short. For example:

```shell
// commission
transaction.Fee = new AmountCommission(Fee(0.9));

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
```

## Using:
### Build for Windows
```shell
build-windows.cmd
```
### Run
```shell
run.cmd
```

### Build for Linux:
```shell
./build-linux.sh
```
### Run
```shell
./runme.sh
```

