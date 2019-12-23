package com.credits.model;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;
import org.springframework.beans.factory.annotation.Autowired;

import java.math.BigDecimal;

@Getter
@Setter
@ToString
public class ResponseApiModel {

    private Long transactionInnerId;

    private Boolean success;

    private String message;

    private String messageError;

    private BigDecimal amount;

    @Autowired
    private DataResponseApiModel dataResponse;

}
