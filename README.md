# MapReduce

### Project Overview

In this project we will be implementing a MapReduce pipeline for helping to process large web server log datasets. A main controller will be coordinating splitting up the work between child processes.

The main program will parse and validate arguments, and evenly distribute the log processes between the different mappers. Each of the mappers will read the log files line by line and use a hash table to insert a new IP if it is unseen, or else add to the count if it has been seen. This will return a 0 if the program succeeds, and a 1 if it fails. The reducer reads all the files and add them only if the IPs fall within the range. Again, a 0 will be returned on a success and a 1 on a fail. 

The hash table will help us add the IP counts. This includes functionality like Creating tables, inserting and updating data, and looking up entries.

### Assumptions

- Log lines follow expected format

### Work Split

 Andrew Lange - Working on main.c and map.c
 Ayden Deboer - Working on table.c and reduce.c

 We will also be reviewing all code before submitting.
 
### AI Use

We will not be using AI for anything more than high-level design clarification.
