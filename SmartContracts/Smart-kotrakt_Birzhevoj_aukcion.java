import com.credits.general.util.GeneralConverter;
import com.credits.scapi.v0.SmartContract;

import java.io.Serializable;
import java.util.*;
import java.util.stream.Collectors;

public class ExchangeAuction extends SmartContract {

    private static double TRANSACTION_FEE = 0;

    private List<Token> tokens = new ArrayList<>();
    private List<Order> purchaseOrders = new ArrayList<>();
    private List<Order> sellOrders = new ArrayList<>();

    public ExchangeAuction() {
        super();
    }

    private Order getSellOrderWithMaxPrice(String tokenKey, int tokenCount) throws OrderNotFoundException {
        List<Order> selectedOrders = sellOrders.stream()
                .filter(x -> (x.getTokenKey().equals(tokenKey) && x.getTokenCount() == tokenCount))
                .collect(Collectors.toList());
        if (selectedOrders.size() == 0) {
            throw new OrderNotFoundException();
        }
        return selectedOrders.stream()
                .max(Comparator.comparing(Order::getProposedPrice))
                .orElseThrow(OrderNotFoundException::new);
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
     * @param price
     */
    public void addOrderToSellTokens(String tokenKey, int tokenCount, double price) throws ExchangeAuctionException {
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
            if (order.getProposedPrice() < price) {
                throw new OrderNotFoundException();
            }
            // Если найдено достаточно токенов в ордерах на покупку,
            // то смарт переводит токены на кошельки ордеров
            // на покупку в кол-ве указанном в ордерах на покупку,
            sendTransaction(contractAddress, order.creatorKey, order.tokenCount, TRANSACTION_FEE);
            // затем переводит CS на кошелёк ордера на продажу
            // в кол-ве токенов умноженных на цена указанную в ордере на покупку,
            sendTransaction(contractAddress, tokenKey, GeneralConverter.toDouble(tokenCount) * price, TRANSACTION_FEE);
            // затем удаляет ордера на покупку из книги заявок
            sellOrders.remove(order);
            // и ставит стоимость одного токена как рыночную стоимость
            tokens.stream().filter(x -> (x.getKey().equals(tokenKey))).findAny().ifPresent(token -> token.setPrice(price));
        } catch (OrderNotFoundException e) {
            // Если найдено недостаточно токенов в ордерах на покупку,
            // то ордер добавляется в книгу заявок как ордер на продажу
            sellOrders.add(new Order(
                    sellOrders.size(),
                    initiator,
                    tokenCount,
                    price,
                    tokenKey
            ));
        }
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
        private double proposedPrice;
        private String tokenKey;

        public Order(int id, String creatorKey, int tokenCount, double proposedPrice, String tokenKey) {
            this.id = id;
            this.creatorKey = creatorKey;
            this.tokenCount = tokenCount;
            this.proposedPrice = proposedPrice;
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

        public double getProposedPrice() {
            return proposedPrice;
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
                    ", proposedPrice=" + proposedPrice +
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