#pragma once
#ifndef flit_handler_cpp
#define flit_handler_cpp

#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "flit_handler.h"
#include "misc.cpp"

flitdb::flitdb()
{
	configured = false;
	clear_values();
}

flitdb::~flitdb()
{
}

void flitdb::clear_values()
{
	value.int_value = 0;
	value.double_value = 0;
	value.float_value = 0;
	value.bool_value = false;
	strncpy(value.char_value, "\0", sizeof(value.char_value));
	value_type = 0;
}

int flitdb::setup(const char *filename, int flags)
{
	if (configured)
	{
		err_message = (char *)"The database handler has already been attributed to handle another database\0";
		return FLITDB_ERROR;
	}
	if (!(flags & FLITDB_CREATE))
	{
		struct stat buffer;
		if (stat(filename, &buffer) != 0)
			return FLITDB_NOT_FOUND;
	}
	file_descriptor = open(filename, (flags & FLITDB_READONLY) ? O_RDONLY : O_RDWR);
	configured = true;
	return FLITDB_SUCCESS;
}

int flitdb::insert_value(signed long set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 1;
	value.int_value = set_value;
	return FLITDB_DONE;
}

int flitdb::insert_value(double set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 2;
	value.double_value = set_value;
	return FLITDB_DONE;
}

int flitdb::insert_value(float set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 3;
	value.float_value = set_value;
	return FLITDB_DONE;
}

int flitdb::insert_value(char *set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 4;
	strncpy(value.char_value, set_value, sizeof(value.char_value));
	return FLITDB_DONE;
}

int flitdb::insert_value(bool set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 5;
	value.bool_value = set_value;
	return FLITDB_DONE;
}

int flitdb::retrieve_value_int()
{
	int response_value = value.int_value;
	clear_values();
	return response_value;
}

double flitdb::retrieve_value_double()
{
	double response_value = value.double_value;
	clear_values();
	return response_value;
}

float flitdb::retrieve_value_float()
{
	float response_value = value.float_value;
	clear_values();
	return response_value;
}

char *flitdb::retrieve_value_char()
{
	char *response_value = value.char_value;
	clear_values();
	return response_value;
}

bool flitdb::retrieve_value_bool()
{
	bool response_value = value.bool_value;
	clear_values();
	return response_value;
}

int flitdb::read_at(unsigned short column_position, unsigned short row_position)
{
	clear_values();
	if (column_position == 0 || column_position > 1000 || row_position == 0 || row_position > 1000)
	{
		err_message = (char *)"The requested range was outside of the database's range";
		return FLITDB_RANGE;
	}
	row_position -= 1;
	bool store_response = false;
	size_t offset = 0;
	size_t skip_offset = 0;
	while (true)
	{
		ssize_t read_status = pread64(file_descriptor, &buffer, 15, offset);
		offset += 15;
		if (read_status == -1)
		{
			err_message = (char *)"An error occurred in attempting to read data from the database\0";
			return FLITDB_ERROR;
		}
		else if (read_status < 15)
		{
			err_message = (char *)"The database contracted a malformed structure request\0";
			return FLITDB_CORRUPT;
		}
		char *skip_amount_read = new char[4];
		skip_amount_read[0] = buffer[0];
		skip_amount_read[1] = buffer[1];
		skip_amount_read[2] = buffer[2];
		skip_amount_read[3] = buffer[3];
		skip_offset += (atoi(skip_amount_read) + 1);
		delete skip_amount_read;
		if (skip_offset > column_position)
			return FLITDB_DONE;
		char *row_count_read = new char[3];
		row_count_read[0] = buffer[4];
		row_count_read[1] = buffer[5];
		row_count_read[2] = buffer[6];
		unsigned short row_count = atoi(row_count_read);
		delete row_count_read;
		if (skip_offset == column_position)
		{
			if (row_count < row_position)
				return FLITDB_DONE;
			char position_read[3];
			position_read[0] = buffer[7];
			position_read[1] = buffer[8];
			position_read[2] = buffer[9];
			unsigned short position = atoi(position_read);
			if (position == row_position)
				store_response = true;
		}
		char *response_length_read = new char[4];
		response_length_read[0] = buffer[10];
		response_length_read[1] = buffer[11];
		response_length_read[2] = buffer[12];
		response_length_read[3] = buffer[13];
		unsigned short response_length = atoi(response_length_read);
		delete response_length_read;
		unsigned char data_type;
		switch (buffer[14])
		{
		case '1':
			data_type = 1;
			break;
		case '2':
			data_type = 2;
			break;
		case '3':
			data_type = 3;
			break;
		case '4':
			data_type = 4;
			break;
		case '5':
			data_type = 5;
			break;
		default:
		{
			err_message = (char *)"The database yielded an invalid datatype\0";
			return FLITDB_CORRUPT;
		}
		}
		if (response_length_read == 0)
		{
			err_message = (char *)"A reference to an imposed data declaration holds no length\0";
			return FLITDB_CORRUPT;
		}
		else if (data_type == 5 && response_length != 1)
		{
			err_message = (char *)"The database holds a boolean of a possible elongated length\0";
			return FLITDB_CORRUPT;
		}
		if (store_response)
		{
			char *response_value = new char[response_length];
			read_status = pread64(file_descriptor, response_value, response_length, offset);
			if (read_status == -1)
			{
				err_message = (char *)"An error occurred in attempting to read data from the database\0";
				return FLITDB_ERROR;
			}
			switch (data_type)
			{
			case 1:
				value.int_value = strtol(response_value, NULL, 0);
				break;
			case 2:
				value.double_value = strtod(response_value, NULL);
				break;
			case 3:
				value.float_value = strtof(response_value, NULL);
				break;
			case 4:
				strncpy(value.char_value, response_value, sizeof(value.char_value));
				break;
			case 5:
				value.bool_value = (response_value[0] == '1');
				break;
			default:
				break;
			}
			delete response_value;
			return FLITDB_DONE;
		}
		offset += response_length;
	}
	return FLITDB_NULL;
}

#endif