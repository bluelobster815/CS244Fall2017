package edu.uci.cs244.team07.endpoints;

import edu.uci.cs244.team07.persistence.CSVFileUtils;

import javax.ws.rs.*;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import java.io.*;


/**
 * Resource with endpoints for uploading and downloading sensor data.
 *
 * @author Janus Varmarken
 */
@Path("/data")
public class SensorDataReceiver {

    /**
     * The name of the system property that points to the upload directory.
     * The value of this system property is stored in {@link #UPLOAD_DIRECTORY} during class initialization.
     */
    private static final String SYSTEM_PROPERTY_UPLOAD_DIRECTORY = "uploaddir";

    /**
     * Path to the root directory where the uploaded data is stored.
     * The path is specified by setting the system property {@link #SYSTEM_PROPERTY_UPLOAD_DIRECTORY}.
     */
    private static final String UPLOAD_DIRECTORY = System.getProperty(SYSTEM_PROPERTY_UPLOAD_DIRECTORY);

    /**
     * Name of file where we store 0.4mA (400uA)readings.
     */
    private static String fileName400uA = "400uA.csv";

    /**
     * Name of file where we store 6.4mA (6400uA)readings.
     */
    private static String fileName6400uA = "6400uA.csv";

    /**
     * Name of file where we store 25.4mA (25400uA)readings.
     */
    private static String fileName25400uA = "25400uA.csv";

    /**
     * Name of file where we store 50.0mA (50000uA)readings.
     */
    private static String fileName50000uA = "50000uA.csv";

    /**
     * Endpoint for uploading 0.4mA readings.
     * @param sensorReadings A CSV-style formatted string with 0.4mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if
     * the server is unable to store the data (e.g. in case the server process cannot access the file).
     */
    @Path("/400uA")
    @POST
    @Consumes(MediaType.TEXT_PLAIN)
    @Produces(MediaType.TEXT_PLAIN)
    public Response uploadSensorData400uA(String sensorReadings) {
        String filePath = String.format("%s/%s", UPLOAD_DIRECTORY, fileName400uA);
        return processUploadRequest(filePath, sensorReadings);
    }

    /**
     * Endpoint for uploading 6.4mA readings.
     * @param sensorReadings A CSV-style formatted string with 6.4mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if
     * the server is unable to store the data (e.g. in case the server process cannot access the file).
     */
    @Path("/6400uA")
    @POST
    @Consumes(MediaType.TEXT_PLAIN)
    @Produces(MediaType.TEXT_PLAIN)
    public Response uploadSensorData6400uA(String sensorReadings) {
        String filePath = String.format("%s/%s", UPLOAD_DIRECTORY, fileName6400uA);
        return processUploadRequest(filePath, sensorReadings);
    }

    /**
     * Endpoint for uploading 25.4mA readings.
     * @param sensorReadings A CSV-style formatted string with 25.4mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if
     * the server is unable to store the data (e.g. in case the server process cannot access the file).
     */
    @Path("/25400uA")
    @POST
    @Consumes(MediaType.TEXT_PLAIN)
    @Produces(MediaType.TEXT_PLAIN)
    public Response uploadSensorData25400uA(String sensorReadings) {
        String filePath = String.format("%s/%s", UPLOAD_DIRECTORY, fileName25400uA);
        return processUploadRequest(filePath, sensorReadings);
    }

    /**
     * Endpoint for uploading 50.0mA readings.
     * @param sensorReadings A CSV-style formatted string with 50.0mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if
     * the server is unable to store the data (e.g. in case the server process cannot access the file).
     */
    @Path("/50000uA")
    @POST
    @Consumes(MediaType.TEXT_PLAIN)
    @Produces(MediaType.TEXT_PLAIN)
    public Response uploadSensorData50000uA(String sensorReadings) {
        String filePath = String.format("%s/%s", UPLOAD_DIRECTORY, fileName50000uA);
        return processUploadRequest(filePath, sensorReadings);
    }

    /**
     * Process a request to upload sensor data.
     * @param targetFile The absolute path to the file where the readings are to be stored.
     * @param sensorReadings The uploaded sensor data.
     * @return A client-ready {@link Response}, i.e. a response that is preconfigured with data that informs the client
     * of the outcome of the request.
     */
    private Response processUploadRequest(String targetFile, String sensorReadings) {
        verifyUploadDir();
        try {
            boolean success = CSVFileUtils.
                    appendStringToFile(targetFile, sensorReadings);
            if (success) {
                // Let client know that the request was successfully fulfilled.
                return Response.ok().build();
            } else {
                // Let client know that we encountered an error when writing its message to persistent storage.
                return Response.status(Response.Status.INTERNAL_SERVER_ERROR).
                        entity("Error writing data to file").build();
            }
        } catch (IOException e) {
            // Could not access file :-(.
            // Return 500 Internal Server Error response with a debug message.
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR).
                    entity("Server could not open the file for storing data.").build();
        }
    }

    /**
     * Verify that the upload directory has been set.
     * If the directory has not been set, a {@link WebApplicationException} is thrown, resulting in a 500 Internal Server
     * response which includes debugging information.
     */
    private void verifyUploadDir() {
        if (UPLOAD_DIRECTORY == null || UPLOAD_DIRECTORY.equals("")) {
            String errDesc = String.format("System property '%s' not set. Please specify the directory for uploaded data using this system property.", SYSTEM_PROPERTY_UPLOAD_DIRECTORY);
            Response r = Response.status(Response.Status.INTERNAL_SERVER_ERROR).
                    type(MediaType.TEXT_PLAIN).
                    entity(errDesc).build();
            throw new WebApplicationException(r);
        }
    }

}
