package com.credits.model;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

import java.math.BigDecimal;

@Getter
@Setter
@ToString
public class RequestApiModel {

    private String networkAlias;

    private String MethodApi;

    private String publicKey;

    private BigDecimal amount;

    private BigDecimal fee;

    private String receiverPublicKey;

    private String tokenPublicKey;

    private String tokenMethod;

    private String smartContractSource;

    private String userData;

    private String transactionSignature;

    private Boolean needPack;

}
