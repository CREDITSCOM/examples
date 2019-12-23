package com.credits.rest;

import com.credits.model.DataResponseApiModel;
import com.credits.model.RequestApiModel;
import com.credits.model.ResponseApiModel;
import com.credits.utils.Ed25519;
import com.google.gson.Gson;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RestController;

import java.math.BigDecimal;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

@RestController
@RequestMapping("/api")
public class CreditsRestController {

    private ResponseEntity<ResponseApiModel> SendRequest(RequestApiModel model) {
        var client = HttpClient.newHttpClient();

        var httpRequest = HttpRequest.newBuilder()
                .uri(URI.create("http://apinode.credits.com/api/monitor/getdata"))
                .header("Content-Type", "application/json")
                .header("AuthKey", "87cbdd85-b2e0-4cb9-aebf-1fe87bf3afdd")
                .POST(HttpRequest.BodyPublishers.ofString(new Gson().toJson(model)))
                .build();

        try{
            var response = client.send(httpRequest, HttpResponse.BodyHandlers.ofString());
            return new ResponseEntity<>(new Gson().fromJson(response.body(), ResponseApiModel.class), HttpStatus.BAD_REQUEST);

        }catch(Exception e){
            return new ResponseEntity<>(HttpStatus.BAD_REQUEST);
        }

    }

    @RequestMapping(path = "getBalance", method = RequestMethod.GET, produces = MediaType.APPLICATION_JSON_VALUE)
    public ResponseEntity<ResponseApiModel> GetBalance() {
        var model = new RequestApiModel();
        model.setPublicKey("Fn2tJrj7TU73cqkwTzaSkbRypYK5xhWEKqrad3nEeBkX");
        model.setMethodApi("GetBalance");
        model.setNetworkAlias("MainNet");

        return SendRequest(model);
    }

    @RequestMapping(path = "transferToken", method = RequestMethod.GET, produces = MediaType.APPLICATION_JSON_VALUE)
    public void TransferToken() {

        var privateKey = "ohPH5zghdzmRDxd978r7y6r8YFoTcKm1MgW2gzik3omCuZLysjwNjTd9hnGREFyQHqhShoU4ri7q748UgdwZpzA";

        var model = new RequestApiModel();
        model.setPublicKey("FeFjpcsfHErXPk5HkfVcwH6zYaRT2xNytDTeSjfuVywt");
        model.setReceiverPublicKey("DM79n9Lbvm3XhBf1LwyBHESaRmJEJez2YiL549ArcHDC");
        model.setTokenPublicKey("FY8J5uSb2D3qX3iwUSvcUSGvrBGAvsrXxKxMQdFfpdmm");
        model.setAmount(new BigDecimal(0.51));
        model.setMethodApi("TransferToken");
        model.setFee(new BigDecimal(0.1));
        model.setNeedPack(true);

        var res1 = SendRequest(model);



        var crypt = Ed25519.sign(res1.getBody().getDataResponse().getTransactionPackagedStr().getBytes(), privateKey.getBytes());

    }

    public void TransferCs() {

    }

    public void SmartDeploy() {

    }

    public void SmartMethodExecute() {

    }
}
