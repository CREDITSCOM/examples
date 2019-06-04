import com.credits.scapi.v0.SmartContract;
import java.io.Serializable;
import java.util.ArrayList;

public class Contract extends SmartContract {
    private final ArrayList<Geo> geoList;

    public Contract() {
        geoList = new ArrayList<>();
    }

    public void saveGeo(int productId, String latitude, String longitude){
        geoList.add(new Geo(productId, latitude, longitude));
    }

    public ArrayList<Geo> loadGeo(){
        return geoList;
    }

    
    public static class Geo implements Serializable {
        private final int productId;
        private final String latitude;
        private final String longitude;

        public Geo(int productId, String latitude, String longitude) {
            this.productId = productId;
            this.latitude = latitude;
            this.longitude = longitude;
        }

    }
}
