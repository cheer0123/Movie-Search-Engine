# Movie-Search-Engine

FileParser: Responsible for reading in a file, checking for errors, and parse out movies.

Movie: A struct holding all the Movie info (id, title, type, genre, etc.)

MovieIndex: A hashtable that indexes movies according to specified field. Basically wraps a hashtable with 
            functionality specific to adding/removing/destroying with MovieSets. The key is the description 
            for the document set, and the value is the document set.

MovieReport: Responsible for printing out a report given a MovieIndex, to the command line or file.

MovieSet: A set of Movies that are connected somehow: they have the same value for a given field. If the field 
          is Year, all the movies in the set are made in the same year. It consists of a description and a LinkedList 
          of Movies.
          
To run the binaries, you need to be sure to point them to the correct data directories.

If you've copied the data directories into your project folder, the commands below should work.

Running QueryClient
./queryclient localhost 1500
where localhost can be replaced with another IP address, and 1500 can be replaced with another port number.

The port number must be the port that the server is listening on.

Running QueryServer
./queryserver ../data/ 1500
where ../data/ can be replaced with any path to a data directory, and 1500 can be replaced with any port you want the server to listen on.

NOTE: The server starts listening on the specified port, and the client must connect to that port.

Running MultiServer
./multiserver ../data/ 1500
This is run just the same as queryserver is run;

../data/ can be replaced with any data directory.

1500 can be replaced with any port you want the server to listen on.
