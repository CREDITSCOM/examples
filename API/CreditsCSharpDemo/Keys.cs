using System;
using System.Collections.Generic;
using System.Text;

namespace CreditsCSAPIDemo
{
    public class Keys
    {
        public string PublicKey { get; set; }
        public string PrivateKey { get; set; }
        public string TargetKey { get; set; }
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
        public byte[] TargetKeyBytes
        {
            get
            {
                return TargetKey != null ? SimpleBase.Base58.Bitcoin.Decode(TargetKey).ToArray() : null;
            }
        }
    }
}
