# MapReduce

### Project Overview

In this project we will be implementing a MapReduce pipeline for helping to process large web server log datasets. A main controller will coordinate by splitting up the work across mapper and reducer child processes.

The main program will parse and validate arguments, and evenly distribute the log processes between the different mappers. Each of the mappers will read the log files line by line and use a hash table to insert a new IP if it is unseen, or else add to the count if it has been seen. This will return a 0 if the program succeeds, and a 1 if it fails. 

After all mappers complete, and the main process waits, main launches reducers. Each reducer reads intermediate tables and collects only the IPs that are in an assigned key range. Final counts are written to output files. Reducers will return a 0 on success and a 1 on fail. The main process then reads the output files and prints the result.

The hash table will help us add the IP counts. This includes functionality like creating tables, inserting and updating data, and looking up entries.

### Assumptions

- Log lines follow expected format
- Input contains only files
- Number of reducers will not be more than number of mappers
- Intermediate and output directories exist before program execution

### Work Split

 Andrew Lange - Working on main.c and map.c
 Ayden Deboer - Working on table.c and reduce.c

 We will also be reviewing all code before submitting.
 
### AI Use

We will not be using AI for anything more than high-level design clarification.
