#pragma once
#ifndef flit_h
#define flit_h

typedef class flitdb;

#define FLITDB_SUCCESS 		0 // Successful operation
#define FLITDB_ERROR 		1 // Unsuccessful operation
#define FLITDB_PERM 		2 // Permision denied
#define FLITDB_BUSY 		3 // The database file is locked
#define FLITDB_NOT_FOUND 	4 // The database file is not found
#define FLITDB_CORRUPT 		5 // The database file is malformed
#define FLITDB_RANGE 		6 // The requested range is outside the range of the daatabase
#define FLITDB_DONE 		7 // The operation was completed successfully

// Functions
// =========
//
// Parameters: Database handler, plot reference x, plot reference y, etc.

int setup(const char *filename, flitdb **handler, int flags);

#endif