using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using CreditsCSAPIDemo;
using Microsoft.AspNetCore.Mvc;
using NodeApi;

namespace CreditsRestApiCSharpDemo.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class CreditsController : ControllerBase
    {
        [HttpGet("{publicKey}")]
        public ActionResult<WalletBalanceGetResult> GetBalance(string publicKey)
        {
            using (var client = new Client("127.0.0.1", 9091, publicKey, "", ""))
            {
                return client.WalletGetBalance();
            }
        }

        [HttpGet("{integral}/{fraction}/{fee}/{publicKey}/{privateKey}/{targetKey}")]
        public ActionResult<TransactionFlowResult> TransferCoins(int integral, long fraction, double fee, string publicKey, string privateKey, string targetKey)
        {
            using (var client = new Client("127.0.0.1", 9091, publicKey, privateKey, targetKey))
            {
                return client.TransferCoins(integral, fraction, fee);
            }
        }
    }
}
