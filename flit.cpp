#pragma once
#ifndef flit_cpp
#define flit_cpp

#include "flit.h"

int setup(const char *filename, flitdb &handler, int flags)
{
	return handler.setup(filename, flags);
}

int insert(flitdb &handler, int64_t column_position, int64_t row_position, int64_t value)
{
	handler.insert_value(value);
	return handler.insert_at(column_position, row_position);
}
int insert(flitdb &handler, int64_t column_position, int64_t row_position, double value)
{
	handler.insert_value(value);
	return handler.insert_at(column_position, row_position);
}
int insert(flitdb &handler, int64_t column_position, int64_t row_position, float value)
{
	handler.insert_value(value);
	return handler.insert_at(column_position, row_position);
}
int insert(flitdb &handler, int64_t column_position, int64_t row_position, char* value)
{
	handler.insert_value(value);
	return handler.insert_at(column_position, row_position);
}
int insert(flitdb &handler, int64_t column_position, int64_t row_position, bool value)
{
	handler.insert_value(value);
	return handler.insert_at(column_position, row_position);
}
int extract(flitdb &handler, int64_t column_position, int64_t row_position)
{
	return handler.read_at(column_position, row_position);
}


#endif