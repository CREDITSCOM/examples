import com.credits.general.util.GeneralConverter;
import com.credits.scapi.annotations.Getter;
import com.credits.scapi.v0.SmartContract;

import java.io.Serializable;
import java.util.*;
import java.util.stream.Collectors;

public class ExchangeAuction extends SmartContract {

    private List<Token> tokens = new ArrayList<>();
    private List<Order> purchaseOrders = new ArrayList<>();
    private List<Order> sellOrders = new ArrayList<>();
    private double transactionFee = 0.01;
    private String owner;

    public ExchangeAuction() {
        super();
        owner = initiator;
    }

    private Order getSellOrderWithMaxPrice(String tokenKey, int tokenCount) throws OrderNotFoundException {
        List<Order> selectedOrders = sellOrders.stream()
                .filter(x -> (x.getTokenKey().equals(tokenKey) && x.getTokenCount() == tokenCount))
                .collect(Collectors.toList());
        if (selectedOrders.size() == 0) {
            throw new OrderNotFoundException();
        }
        return selectedOrders.stream()
                .max(Comparator.comparing(Order::getProposedPricePerOneToken))
                .orElseThrow(OrderNotFoundException::new);
    }

    private Order getPurchaseOrderWithMinPrice(String tokenKey, int tokenCount) throws OrderNotFoundException {
        List<Order> selectedOrders = purchaseOrders.stream()
                .filter(x -> (x.getTokenKey().equals(tokenKey) && x.getTokenCount() == tokenCount))
                .collect(Collectors.toList());
        if (selectedOrders.size() == 0) {
            throw new OrderNotFoundException();
        }
        return selectedOrders.stream()
                .min(Comparator.comparing(Order::getProposedPricePerOneToken))
                .orElseThrow(OrderNotFoundException::new);
    }


    private void checkPermission() throws ExchangeAuctionException {
        if (!initiator.equals(owner)) {
            throw new ExchangeAuctionException("403 Permission denied");
        }
    }

    /**
     * Registration of a smart contract on the exchange
     * @param key
     * @param price
     * @throws ExchangeAuctionException
     */
    public void registerToken(String key, double price) throws ExchangeAuctionException {
        Token token = tokens.stream()
                .filter(x -> key.equals(x.getKey()))
                .findAny()
                .orElse(null);
        if (token != null) {
            throw new ExchangeAuctionException(String.format("Token with key %s already exists", key));
        }
        tokens.add(new Token(tokens.size(), key, price));
    }

    /**
     * Getting an array of registered tokens and their market price
     * @return
     */
    public List<String> getTokens() {
        return tokens.stream()
                .map(token -> {
                    return token.toString();
                }).collect(Collectors.toList());
    }

    /**
     * Get one registered token by id
     * @param id
     * @return
     * @throws ExchangeAuctionException
     */
    public String getToken(int id) throws ExchangeAuctionException {
        Token token = tokens.stream()
                .filter(x -> (x.getId() == id))
                .findAny()
                .orElse(null);
        if (token == null) {
            throw new ExchangeAuctionException(String.format("Token with id %s is not found", id));
        }
        return token.toString();
    }

    /**
     * Adding order to sell tokens
     * @param tokenKey
     * @param tokenCount
     * @param pricePerOneToken
     */
    public void addOrderToSellTokens(String tokenKey, int tokenCount, double pricePerOneToken) throws ExchangeAuctionException {
        // Метод по адресу инициатора проверяет allowance на указанном токене
        Object[] params = new Object[] { initiator, contractAddress };
        String allowedTokenCountStr = (String) invokeExternalContract(tokenKey, "allowance", params);
        int allowedTokenCount = GeneralConverter.toInteger(GeneralConverter.toDouble(allowedTokenCountStr));
        if (tokenCount > allowedTokenCount) {
            throw new ExchangeAuctionException(String.format("Allowed tokens count %s", allowedTokenCount));
        }
        // Если есть доступные токена то смарт биржи переводит эти токена на свой адрес
        params = new Object[] { contractAddress , GeneralConverter.toString(tokenCount)};
        invokeExternalContract(tokenKey, "transfer", params);
        // Метод ищет в книге заявок необходимое кол-во токенов из ордеров на покупку по максимальной стоимости, но не меньше указанной
        try {
            Order order = getSellOrderWithMaxPrice(tokenKey, tokenCount);
            if (order.getProposedPricePerOneToken() < pricePerOneToken) {
                throw new OrderNotFoundException();
            }
            // Если найдено достаточно токенов в ордерах на покупку,
            // то смарт переводит токены на кошельки ордеров
            // на покупку в кол-ве указанном в ордерах на покупку,
            sendTransaction(contractAddress, order.getCreatorKey(), order.getTokenCount(), transactionFee);
            // затем переводит CS на кошелёк ордера на продажу
            // в кол-ве токенов умноженных на цена указанную в ордере на покупку,
            sendTransaction(contractAddress, initiator, order.getTokenCount() * order.getProposedPricePerOneToken(), transactionFee);
            // затем удаляет ордера на покупку из книги заявок
            sellOrders.remove(order);
            // и ставит стоимость одного токена как рыночную стоимость
            tokens.stream().filter(x -> (x.getKey().equals(tokenKey))).findAny().ifPresent(token -> token.setPrice(pricePerOneToken));
        } catch (OrderNotFoundException e) {
            // Если найдено недостаточно токенов в ордерах на покупку,
            // то ордер добавляется в книгу заявок как ордер на продажу
            sellOrders.add(new Order(
                    sellOrders.size(),
                    initiator,
                    tokenCount,
                    pricePerOneToken,
                    tokenKey
            ));
        }
    }

    /**
     * Adding order to purchase tokens
     * @param amount - transaction amount
     * @param currency - addOrderToBuyTokens params, format:  tokenKey|tokenCount|pricePerOneToken,
     *                 example: 8wxQib3MoivBxVbTnAKEzUs962JFKJnpocB1C33tsyvF|2|25
     * @throws ExchangeAuctionException
     */
    public void payable(String amount, String currency) throws ExchangeAuctionException {
        if (currency == null || currency.isEmpty()) {
            return;
        }
        // Пользователь переводит на смарт контракт CS с указанием какие токены он хочет купить и за какую цена,
        // цена указывается за единицу токена.
        String[] params = currency.split("\\|");
        String tokenKey = params[0];
        int tokenCount = GeneralConverter.toInteger(params[1]);
        double pricePerOneToken = GeneralConverter.toDouble(params[2]);
        double amountToBuy = (tokenCount * pricePerOneToken) + transactionFee;
        if (GeneralConverter.toDouble(amount) < amountToBuy) {
            throw new ExchangeAuctionException(
                    String.format(
                        "Amount %s is not enough to buy %s tokens with price per one token %s CS and pay a transaction fee of %s CS.\n " +
                        "Amount should be %s",
                        amount, tokenCount, pricePerOneToken, transactionFee, amountToBuy
                    ));
        }
        // Метод ищет из книги заявок необходимое кол-во токенов из нескольких ордеров на продажу по переданому ключу токена
        // и самой низкой стоимости, но не выше указанной.
        try {
            Order order = getPurchaseOrderWithMinPrice(tokenKey, tokenCount);
            if (order.getProposedPricePerOneToken() > pricePerOneToken) {
                throw new OrderNotFoundException();
            }
            // Если найдено достаточно токенов в ордерах на продажу, то смарт переводит CS на кошельки ордеров на продажу
            // в кол-ве указанном в ордера на продажу умноженную на цену указанную в ордере на продажу
            sendTransaction(contractAddress, order.getCreatorKey(), order.getTokenCount() * order.getProposedPricePerOneToken(), transactionFee);
            // затем переводит токена на кошелёк ордеров на покупку в кол-ве указанных в ордерах на продажу,
            // в кол-ве токенов умноженных на цена указанную в ордере на покупку,
            sendTransaction(contractAddress, initiator, order.getTokenCount() * order.getProposedPricePerOneToken(), transactionFee);
            // затем удаляет ордера на продажу
            purchaseOrders.remove(order);
            // и ставит стоимость одного токена как рыночную стоимость
            tokens.stream().filter(x -> (x.getKey().equals(tokenKey))).findAny().ifPresent(token -> token.setPrice(pricePerOneToken));
        } catch (OrderNotFoundException e) {
            // Если найдено недостаточно токенов в ордерах на продажу,
            // то ордер добавляется в книгу заявок как ордер на покупку
            purchaseOrders.add(new Order(
                    sellOrders.size(),
                    initiator,
                    tokenCount,
                    pricePerOneToken,
                    tokenKey
            ));
        }

    }

    public String getSellOrders() {
        return sellOrders.toString();
    }

    public String getPurchaseOrders() {
        return purchaseOrders.toString();
    }

    public void cancelOrder(String tokenKey) {
        List<Order> orders = sellOrders.stream().filter(order -> order.getCreatorKey().equals(initiator)).collect(Collectors.toList());
        orders.forEach(order -> {
                    sendTransaction(contractAddress, initiator, order.getTokenCount() * order.getProposedPricePerOneToken(), transactionFee);
                });
        sellOrders.removeAll(orders);

        orders = purchaseOrders.stream().filter(order -> order.getCreatorKey().equals(initiator)).collect(Collectors.toList());
        orders.forEach(order -> {
                    Object[] params = new Object[] { initiator , GeneralConverter.toString(order.getTokenCount())};
                    invokeExternalContract(tokenKey, "transfer", params);
                });
        purchaseOrders.removeAll(orders);
    }

    public void setTransactionFee(double transactionFee) throws ExchangeAuctionException {
        checkPermission();
        this.transactionFee = transactionFee;
    }

    @Getter
    public double getTransactionFee() {
        return this.transactionFee;
    }


    static class Token implements Serializable {
        private int id;
        private String key;
        private double price;

        public Token(int id, String key, double price) {
            this.id = id;
            this.key = key;
            this.price = price;
        }

        public int getId() {
            return id;
        }

        public String getKey() {
            return key;
        }

        public double getPrice() {
            return price;
        }

        public void setPrice(double price) {
            this.price = price;
        }

        @Override
        public String toString() {
            return "Token{" +
                    "id=" + id +
                    ", key='" + key + '\'' +
                    ", price=" + price +
                    '}';
        }
    }

    static class Order  implements Serializable{
        private int id;
        private String creatorKey;
        private int tokenCount;
        private double proposedPricePerOneToken;
        private String tokenKey;

        public Order(int id, String creatorKey, int tokenCount, double proposedPricePerOneToken, String tokenKey) {
            this.id = id;
            this.creatorKey = creatorKey;
            this.tokenCount = tokenCount;
            this.proposedPricePerOneToken = proposedPricePerOneToken;
            this.tokenKey = tokenKey;
        }

        public int getId() {
            return id;
        }

        public String getCreatorKey() {
            return creatorKey;
        }

        public int getTokenCount() {
            return tokenCount;
        }

        public double getProposedPricePerOneToken() {
            return proposedPricePerOneToken;
        }

        public String getTokenKey() {
            return tokenKey;
        }

        @Override
        public String toString() {
            return "Order{" +
                    "id=" + id +
                    ", creatorKey='" + creatorKey + '\'' +
                    ", tokenCount=" + tokenCount +
                    ", proposedPricePerOneToken=" + proposedPricePerOneToken +
                    ", tokenKey='" + tokenKey + '\'' +
                    '}';
        }
    }

    static class ExchangeAuctionException extends Exception {
        ExchangeAuctionException(String message) {
            super(message);
        }
    }

    static class OrderNotFoundException extends Exception {
        OrderNotFoundException() {
            super();
        }
    }
}
