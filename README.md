# flitdb

***Verb*** ~ *Move swiftly and lightly.*

FlitDB is a database designed for embedded devices. Instructions are good, but are even better, when they can be used to construct a database; FlitDB uses instructions embedded into the actual database file, to restructure the database. The database does not include any `null` (or empty) values; this done to lower the total size of the database file. With a smaller file size the data can be read in a fast and timely manor.

Use cases:
 - IOT
 - Embedded devices
 - Places where logic cooridinates are in better use for storage; than compared to: SQL, or NoSQL database.

Requirements:
 - GCC

This is only tested on a Linux system.

Compile with this command:

	g++ main.cpp -o build/flit_demo

Then go into the build directory, and run the program in your terminal called `flit_demo`; this will allow you to run the demo of FlitDB.

## Awards 🏆
 - Grand Prize - Royal Hackaway v4
