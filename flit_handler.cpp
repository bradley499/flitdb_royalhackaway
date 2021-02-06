#pragma once
#ifndef flit_handler_cpp
#define flit_handler_cpp

#include <sys/file.h>
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
	strncpy(value.char_value, "\0", sizeof(value.char_value));
}

int flitdb::setup(const char *filename, int flags)
{
	if (configured)
	{
		err_message = "The database handler has already been attributed to handle another database\0";
		return FLITDB_ERROR;
	}
	file_descriptor = open(filename, flags);
	configured = true;
	return FLITDB_SUCCESS;
}

int flitdb::insert_value(unsigned long set_value)
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

int flitdb::insert_value(char *set_value)
{
	clear_values();
	value_type = 4;
	strncpy(value.char_value, set_value, sizeof(value.char_value));
}

int flitdb::insert_value(bool set_value)
{
	clear_values();
	value_type = 5;
	value.bool_value = set_value;
}

int flitdb::read_at(unsigned short column_position, unsigned short row_position)
{
	if (column_position == 0 || column_position > 1000)
	{
		err_message = "The requested range was outside of the database's range";
		return FLITDB_RANGE;
	}
	if (row_position == 0 || row_position > 1000)
	{
		err_message = "The requested range was outside of the database's range";
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
			err_message = "An error occurred in attempting to read data from the database\0";
			return FLITDB_ERROR;
		}
		else if (read_status < 15)
		{
			err_message = "The database contracted a malformed structure request\0";
			return FLITDB_CORRUPT;
		}
		char * skip_amount_read = new char[4];
		skip_amount_read[0] = buffer[0];
		skip_amount_read[1] = buffer[1];
		skip_amount_read[2] = buffer[2];
		skip_amount_read[3] = buffer[3];
		skip_offset += (atoi(skip_amount_read) + 1);
		delete skip_amount_read;
		if (skip_offset > column_position)
			return FLITDB_DONE;
		char * row_count_read = new char[3];
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
		char * response_length_read = new char[4];
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
				err_message = "The database yielded an invalid datatype\0";
				return FLITDB_CORRUPT;
			}
		}
		if (response_length_read == 0)
		{
			err_message = "A reference to an imposed data declaration holds no length\0";
			return FLITDB_CORRUPT;
		}
		else if (data_type == 5 && response_length != 1)
		{
			err_message = "The database holds a boolean of a possible elongated length\0";
			return FLITDB_CORRUPT;
		}
		if (store_response)
		{
			char *response_value = new char[response_length];
			read_status = pread64(file_descriptor, response_value, response_length, offset);
			if (read_status == -1)
			{
				err_message = "An error occurred in attempting to read data from the database\0";
				return FLITDB_ERROR;
			}
			switch (data_type)
			{
			case 1:
				value.int_value = strtoull(response_value, NULL, 0);
				break;
			case 2:
				value.double_value = strtod_l(response_value, NULL, 0);
				break;
			case 3:
				value.float_value = strtof32_l(response_value, NULL, 0);
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
			break;
		}
		offset += response_length;
	}
	return FLITDB_DONE;
}

#endif