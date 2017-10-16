package edu.uci.cs244.huanght.jvarmark.endpoints;

import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import java.io.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;


/**
 * TODO add class documentation.
 *
 * @author Janus Varmarken
 */
@Path("/data/upload")
public class SensorDataReceiver {

    // System property that denotes the path to the file where the uploaded data is stored.
    private static final String SYSTEM_PROPERTY_UPLOADED_CONTENT_FILE = "uploadedcontentfile";

    @POST
    @Consumes(MediaType.TEXT_PLAIN)
    @Produces(MediaType.TEXT_PLAIN)
    public Response uploadDummyData(String messageBody) {
        // Read the location of the file for uploaded content from the system properties.
        String pathToDataFile = System.getProperty(SYSTEM_PROPERTY_UPLOADED_CONTENT_FILE);
        if (pathToDataFile == null) {
            String errDesc = String.format("System property '%s' not set. Please specify the path to the file where the server is to store the uploaded data using this system property.", SYSTEM_PROPERTY_UPLOADED_CONTENT_FILE);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR).
                    entity(errDesc).build();
        }
        // File where the server stores the uploaded data.
        File f = new File(pathToDataFile);
        // Init the PrintWriter responsible for dumping data to a file on the server.
        PrintWriter writer;
        try {
            // First create file if it doesn't already exist.
            f.createNewFile();
            writer = new PrintWriter(new FileOutputStream(f, true));
        } catch (IOException e) {
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR).
                    entity("Server could not open the file for storing data.").build();
        }
        // Put current server time in front of message.
        Date now = new Date();
        DateFormat dateFormat = new SimpleDateFormat("yyyy-dd-MM HH:mm:ss");
        String line = "[" + dateFormat.format(now) + "] " + messageBody;
        // Output line to file.
        writer.println(line);
        if (!writer.checkError()) {
            // Let client know that everything went well.
            String respString = String.format("Successfully stored message '%s' on server.", line);
            return Response.ok(respString, MediaType.TEXT_PLAIN).build();
        } else {
            // Let client know that we encountered an error when writing its message to persistent storage.
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR).
                    entity("Error writing data to file").build();
        }
    }

}
