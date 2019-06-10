# Tracking the movement of ordered goods

## Our user class
### The user class must inherit from Serializable
```shell
public static class Geo implements Serializable {                
    private final int productId;                                 private final String latitude;                               
    private final String longitude;                               
    public Geo(int productId, String latitude, String longitude) {
        this.productId = productId;                              
        this.latitude = latitude;                                this.longitude = longitude;                              
    }                                                            
}                                                               
```

### The custom list declaration
```shell
private final ArrayList<Geo> geoList;
```

### Creating the list in the constructor
```shell
public Contract() {
    geoList = new ArrayList<>();
}
```

### Adding the geo position of our product
```shell
public void saveGeo(int productId, String latitude, String longitude){
    geoList.add(new Geo(productId, latitude, longitude));
}
```

### Getting our user list
```shell
public ArrayList<Geo> loadGeo(){
    return geoList;
}
```

### Getting the count of our user list
```shell
public int geoListSize(){
    return geoList.size();
}
```