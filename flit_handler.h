#pragma once
#ifndef flit_handler_h
#define flit_handler_h


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
		char char_value[10000];
		bool bool_value;
	} value;
	unsigned char value_type = 0;
	void clear_values();
public:
	flitdb();
	~flitdb();
	int setup(const char *filename, int flags);
	int read_at(unsigned short column_position, unsigned short row_position);
	int insert_at(unsigned short column_position, unsigned short row_position);
	int insert_value(signed long set_value);
	int insert_value(double set_value);
	int insert_value(float set_value);
	int insert_value(char* set_value);
	int insert_value(bool set_value);
	int retrieve_value_int();
	double retrieve_value_double();
	float retrieve_value_float();
	char* retrieve_value_char();
	bool retrieve_value_bool();
};

#include "flit.h"


#endif