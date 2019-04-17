
```FileParser```:
Responsible for reading in a file, checking for errors, and parse out movies.


```Movie```:
A struct holding all the Movie info (id, title, type, genre, etc.)


```MovieIndex```:
A hashtable that indexes movies according to specified field. Basically wraps a hashtable with functionality specific to adding/removing/destroying with MovieSets. The key is the description for the document set, and the value is the document set.


```MovieReport```:
Responsible for printing out a report given a MovieIndex, to the command line or file.


```MovieSet```:
A set of Movies that are connected somehow: they have the same value for a given field. If the field is Year, all the movies in the set are made in the same year. It consists of a description and a LinkedList of Movies.


