#pragma once
#ifndef flit_h
#define flit_h

typedef unsigned short uint16_t;
typedef signed long int64_t;

// typedef class flitdb {
// public:
// 	int setup(const char *filename, int flags);
// 	int read_at(uint16_t column_position, uint16_t row_position);
// 	int insert_at(uint16_t column_position, uint16_t row_position);
// 	int insert_value(int64_t set_value);
// 	int insert_value(double set_value);
// 	int insert_value(float set_value);
// 	int insert_value(char* set_value);
// } flitdb;

#define FLITDB_SUCCESS 		0 // Successful operation
#define FLITDB_ERROR 		1 // Unsuccessful operation
#define FLITDB_PERM 		2 // Permision denied
#define FLITDB_BUSY 		3 // The database file is locked
#define FLITDB_NOT_FOUND 	4 // The database file is not found
#define FLITDB_CORRUPT 		5 // The database file is malformed
#define FLITDB_RANGE 		6 // The requested range is outside the range of the database
#define FLITDB_DONE 		7 // The operation was completed successfully
#define FLITDB_NULL			8 // Create a database if not existent
#define FLITDB_CREATE		9 // Create a database if not existent
#define FLITDB_READONLY		10 // Only allow the reading of the database

// Functions
// =========
//
// Parameters: Database handler, plot reference x, plot reference y, etc.

int setup(const char *filename, flitdb &handler, int flags);
int insert(flitdb &handler, int64_t column_position, int64_t row_position, int value);
int insert(flitdb &handler, int64_t column_position, int64_t row_position, double value);
int insert(flitdb &handler, int64_t column_position, int64_t row_position, float value);
int insert(flitdb &handler, int64_t column_position, int64_t row_position, char* value);
int insert(flitdb &handler, int64_t column_position, int64_t row_position, bool value);
int retrieve_value_int(flitdb &handler);
double retrieve_value_double(flitdb &handler);
float retrieve_value_float(flitdb &handler);
char* retrieve_value_char(flitdb &handler);
bool retrieve_value_bool(flitdb &handler);
int extract(flitdb &handler, int64_t column_position, int64_t row_position);

#endif