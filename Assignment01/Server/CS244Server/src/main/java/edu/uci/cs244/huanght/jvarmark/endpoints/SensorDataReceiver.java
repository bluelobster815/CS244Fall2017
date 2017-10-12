package edu.uci.cs244.huanght.jvarmark.endpoints;

import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.MediaType;

/**
 * TODO add class documentation.
 *
 * @author Janus Varmarken
 */
@Path("/data/upload")
public class SensorDataReceiver {

    @POST
    @Consumes(MediaType.TEXT_PLAIN)
    @Produces(MediaType.TEXT_PLAIN)
    public String uploadDummyData(String messageBody) {
        // TODO store message in some file or whatever.
        return messageBody;
    }

}
