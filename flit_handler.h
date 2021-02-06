#pragma once
#ifndef flit_handler_h
#define flit_handler_h

#include "flit.h"

class flitdb
{
private:
	char buffer[1024];
	bool configured = false;
	int file_descriptor;
	char* err_message;
public:
	flitdb();
	~flitdb();
	int setup(const char *filename, int flags);
	int read_at(uint16_t column_position, uint16_t row_position);
	int insert_at(uint16_t column_position, uint16_t row_position);
	int insert_value(int64_t value);
	int insert_value(double value);
	int insert_value(float value);
	int insert_value(char* value);
};

#endif