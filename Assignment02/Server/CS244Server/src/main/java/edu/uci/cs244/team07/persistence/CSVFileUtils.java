package edu.uci.cs244.team07.persistence;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;

/**
 * Static utility methods for interacting the with the CSV files.
 *
 * @author Janus Varmarken
 */
public class CSVFileUtils {


    /**
     * Append a string to a given file. The file will be created if it does not already exist. <em>However</em>, if any
     * directory in the file path does not exist, the file will <em>not</em> be created and hence this method will fail.
     * <p>
     * It is up to the caller to specify new lines by embedding the platform's newline in the string.
     * </p>
     * @param absoluteFilePath The absolute path to the file.
     * @throws IOException If the file cannot be accessed/created (e.g. if the provided path is invalid).
     * @return {@code true} if the string was successfully appended, {@code false} if an error occurred while writing
     * the string to the file. Note that errors due to improper file paths and insufficient access rights are
     * communicated to the caller using the thrown IOException.
     */
    public static boolean appendStringToFile(String absoluteFilePath, String stringToAppend) throws IOException {
        // Replace any OS dependant line separator with the line separator of this system.
        stringToAppend = stringToAppend.replaceAll("\\R", System.lineSeparator());
        File f = new File(absoluteFilePath);
        // Init the PrintWriter responsible for dumping data to a file on the server.
        PrintWriter writer;
        // First create file if it doesn't already exist.
        f.createNewFile();
        writer = new PrintWriter(new FileOutputStream(f, true));
        writer.print(stringToAppend);
        // Flush stream and check for error.
        boolean errorOccurred = writer.checkError();
        // Cleanup: close stream.
        writer.close();
        return !errorOccurred;
    }

}
