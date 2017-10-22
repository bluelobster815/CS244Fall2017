package edu.uci.cs244.team07.endpoints;

import edu.uci.cs244.team07.persistence.CSVFileUtils;

import javax.ws.rs.*;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.StreamingOutput;
import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;


/**
 * Resource with endpoints for uploading and downloading sensor data.
 *
 * @author Janus Varmarken
 */
@Path("/data")
public class SensorDataReceiver {

    /**
     * The name of the system property that points to the upload directory. The value of this system property is stored
     * in {@link #UPLOAD_DIRECTORY} during class initialization.
     */
    private static final String SYSTEM_PROPERTY_UPLOAD_DIRECTORY = "uploaddir";

    /**
     * Path to the root directory where the uploaded data is stored. The path is specified by setting the system
     * property {@link #SYSTEM_PROPERTY_UPLOAD_DIRECTORY}.
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
     * Endpoint that combines readings for the four different current settings into one output.
     * The output is streamed to the client as the files may be large, hence we do not want to load all data into memory.
     * @return A response that streams the output to the client.
     */
    @Path("/all")
    @GET
    public Response downloadCombinedSensorReadings() {
        StreamingOutput responseStream = output -> {
            // Use try-with: resources are automatically closed after use. No need to call close().
            try (
                    // Open up a reader for each of the four files that contain sensor readings.
                    BufferedReader r1 = Files.newBufferedReader(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName400uA)));
                    BufferedReader r2 = Files.newBufferedReader(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName6400uA)));
                    BufferedReader r3 = Files.newBufferedReader(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName25400uA)));
                    BufferedReader r4 = Files.newBufferedReader(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName50000uA)));
                    // And create a writer for writing the streamed response.
                    BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(output))
            ) {
                // Write a header line
                writer.write("IR1,RED1,IR2,RED2,IR3,RED3,IR4,RED4" + System.lineSeparator());
                String l1, l2, l3, l4 = null;
                // Files may be of different length.
                // We only join rows (lines) as long as there is data available in all 4 files.
                while ((l1 = r1.readLine()) != null &&
                        (l2 = r2.readLine()) != null &&
                        (l3 = r3.readLine()) != null &&
                        (l4 = r4.readLine()) != null) {
                    // Join the data spread across the four files into one row (line).
                    String combinedRow = String.format("%s,%s,%s,%s%s", l1, l2, l3, l4, System.lineSeparator());
                    // Output to the client.
                    writer.write(combinedRow);
                }
                // Flush the stream to ensure that client receives the data.
                writer.flush();
            }
        };
        return Response.ok(responseStream).build();
        /* Wanted to do a clean solution using Java 8 Streams, but can't get it to work :-(.
        // Open a Java 8 Stream for each of the four data files.
        Stream<String> f1 = Files.lines(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName400uA)));
        Stream<String> f2 = Files.lines(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName6400uA)));
        Stream<String> f3 = Files.lines(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName25400uA)));
        Stream<String> f4 = Files.lines(Paths.get(String.format("%s/%s", UPLOAD_DIRECTORY, fileName50000uA)));
        Stream<String> combined = f1.flatMap(f1l -> f2.flatMap(f2l -> f3.flatMap(f3l -> f4.map(f4l -> String.format("%s,%s,%s,%s%s", f1l, f2l, f3l, f4l, System.lineSeparator())))));
        // This doesn't work: for some reason the stream has already performed a terminal operation at this point.
        String s = combined.reduce((s1, s2) -> s1 + s2).orElse("");
        return Response.ok(s).build();
        */
    }

    /**
     * Endpoint for uploading 0.4mA readings.
     *
     * @param sensorReadings
     *         A CSV-style formatted string with 0.4mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if the
     * server is unable to store the data (e.g. in case the server process cannot access the file).
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
     *
     * @param sensorReadings
     *         A CSV-style formatted string with 6.4mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if the
     * server is unable to store the data (e.g. in case the server process cannot access the file).
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
     *
     * @param sensorReadings
     *         A CSV-style formatted string with 25.4mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if the
     * server is unable to store the data (e.g. in case the server process cannot access the file).
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
     *
     * @param sensorReadings
     *         A CSV-style formatted string with 50.0mA sensor readings.
     * @return A 200 HTTP response if the data was successfully stored on the server. A 500 Internal Server Error if the
     * server is unable to store the data (e.g. in case the server process cannot access the file).
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
     *
     * @param targetFile
     *         The absolute path to the file where the readings are to be stored.
     * @param sensorReadings
     *         The uploaded sensor data.
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
     * Verify that the upload directory has been set. If the directory has not been set, a {@link
     * WebApplicationException} is thrown, resulting in a 500 Internal Server response which includes debugging
     * information.
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
