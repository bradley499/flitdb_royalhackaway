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
	struct value
	{
		int64_t int_value;
		double double_value;
		float float_value;
		char* char_value;
	} value;
	unsigned char value_type = 0;
	void clear_values();
public:
	flitdb();
	~flitdb();
	int setup(const char *filename, int flags);
	int read_at(uint16_t column_position, uint16_t row_position);
	int insert_at(uint16_t column_position, uint16_t row_position);
	int insert_value(int64_t set_value);
	int insert_value(double set_value);
	int insert_value(float set_value);
	int insert_value(char* set_value);
	int insert_value(bool set_value);
};

#endif