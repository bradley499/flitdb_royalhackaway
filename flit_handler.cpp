#pragma once
#ifndef flit_handler_cpp
#define flit_handler_cpp

#include "flit_handler.h"

flitdb::flitdb()
{
	configured = false;
	err_message = "\0";
}

flitdb::~flitdb()
{
}

void flitdb::clear_values()
{
	value.int_value = 0;
	value.double_value = 0;
	value.float_value = 0;
	value.char_value = 0;
}

int flitdb::setup(const char *filename, int flags)
{
	if (configured)
	{
		err_message = "The database handler has already been attributed to handle another database\0";
		return FLITDB_ERROR;
	}
	return FLITDB_SUCCESS;
}

int flitdb::insert_value(int64_t set_value)
{
	clear_values();
	value_type = 1;
	value.int_value = set_value;
}

int flitdb::insert_value(double set_value)
{
	clear_values();
	value_type = 2;
	value.double_value = set_value;
}

int flitdb::insert_value(float set_value)
{
	clear_values();
	value_type = 3;
	value.float_value = set_value;
}

int flitdb::insert_value(char* set_value)
{
	clear_values();
	value_type = 4;
	value.char_value = set_value;
}

int flitdb::insert_value(bool set_value)
{
	clear_values();
	value_type = 5;
	value.char_value = set_value;
}

#endif