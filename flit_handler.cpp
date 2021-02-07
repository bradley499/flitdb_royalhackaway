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
	strncpy(buffer, "\0", sizeof(buffer));
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

int flitdb::drop_value(signed long set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 1;
	value.bool_value = set_value;
	return FLITDB_DONE;
}

int flitdb::drop_value(double set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 2;
	value.bool_value = set_value;
	return FLITDB_DONE;
}

int flitdb::drop_value(float set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 3;
	value.bool_value = set_value;
	return FLITDB_DONE;
}

int flitdb::drop_value(char* set_value)
{
	if (value_type != 0)
	{
		err_message = (char *)"Data insertion avoided due to unexpected tennant";
		return FLITDB_ERROR;
	}
	clear_values();
	value_type = 4;
	value.bool_value = set_value;
	return FLITDB_DONE;
}

int flitdb::drop_value(bool set_value)
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
	unsigned char read_length = 15;
	unsigned short row_count = 0;
	unsigned short row_position_count = 0;
	while (true)
	{
		strncpy(buffer, "\0", sizeof(buffer));
		ssize_t read_status = pread64(file_descriptor, &buffer, read_length, offset);
		offset += read_length;
		if (read_status == -1)
		{
			err_message = (char *)"An error occurred in attempting to read data from the database\0";
			return FLITDB_ERROR;
		}
		else if (read_status < read_length)
		{
			err_message = (char *)"The database contracted a malformed structure request\0";
			return FLITDB_CORRUPT;
		}
		if (read_length == 15)
		{
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
			row_count = atoi(row_count_read);
			row_position_count = 0;
			delete row_count_read;
		}
		unsigned char set_read_length = 15;
		if (skip_offset == column_position)
		{
			if (row_count < row_position)
				return FLITDB_DONE;
			char position_read[3];
			position_read[0] = buffer[(read_length < 15) ? 0 : 7];
			position_read[1] = buffer[(read_length < 15) ? 1 : 8];
			position_read[2] = buffer[(read_length < 15) ? 2 : 9];
			unsigned short position = atoi(position_read);
			if (position == row_position)
			{
				store_response = true;
				row_count = 0;
			}
		}
		else if (read_length != read_length)
		{
			err_message = (char *)"A database coordination offset error occurred\0";
			return FLITDB_ERROR;
		}
		else if (row_position_count == row_count)
			row_count = 0;
		row_position_count += 1;
		if (row_count > 1)
			set_read_length = 8;
		char *response_length_read = new char[4];
		response_length_read[0] = buffer[(read_length < 15) ? 3 : 10];
		response_length_read[1] = buffer[(read_length < 15) ? 4 : 11];
		response_length_read[2] = buffer[(read_length < 15) ? 5 : 12];
		response_length_read[3] = buffer[(read_length < 15) ? 6 : 13];
		unsigned short response_length = atoi(response_length_read);
		delete response_length_read;
		unsigned char data_type;
		switch (buffer[(read_length < 15) ? 7 : 14])
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
			err_message = (char *)"The database yielded an invalid datatype\0";
			return FLITDB_CORRUPT;
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
			char *response_value = new char[(response_length + 1)];
			response_value[response_length] = '\0';
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
		read_length = set_read_length;
		offset += response_length;
	}
	return FLITDB_NULL;
}

int flitdb::insert_at(unsigned short column_position, unsigned short row_position)
{
	if (column_position == 0 || column_position > 1000 || row_position == 0 || row_position > 1000)
	{
		err_message = (char *)"The requested range was outside of the database's range";
		return FLITDB_RANGE;
	}
	row_position -= 1;
	bool store_response = false;
	size_t offset = 0;
	size_t skip_offset = 0;
	unsigned char read_length = 15;
	unsigned short row_count = 0;
	unsigned short row_position_count = 0;
	size_t current_length = 0;
	size_t insert_position = 0;
	while (true)
	{
		insert_position += offset;
		strncpy(buffer, "\0", sizeof(buffer));
		ssize_t read_status = pread64(file_descriptor, &buffer, read_length, offset);
		offset += read_length;
		if (read_status == -1)
		{
			err_message = (char *)"An error occurred in attempting to read data from the database\0";
			return FLITDB_ERROR;
		}
		else if (read_status < read_length)
		{
			err_message = (char *)"The database contracted a malformed structure request\0";
			return FLITDB_CORRUPT;
		}
		if (read_length == 15)
		{
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
			row_count = atoi(row_count_read);
			row_position_count = 0;
			delete row_count_read;
		}
		unsigned char set_read_length = 15;
		if (skip_offset == column_position)
		{
			if (row_count < row_position)
				return FLITDB_DONE;
			char position_read[3];
			position_read[0] = buffer[(read_length < 15) ? 0 : 7];
			position_read[1] = buffer[(read_length < 15) ? 1 : 8];
			position_read[2] = buffer[(read_length < 15) ? 2 : 9];
			unsigned short position = atoi(position_read);
			if (position == row_position)
			{
				store_response = true;
				row_count = 0;
			}
		}
		else if (read_length != read_length)
		{
			err_message = (char *)"A database coordination offset error occurred\0";
			return FLITDB_ERROR;
		}
		else if (row_position_count == row_count)
			row_count = 0;
		row_position_count += 1;
		if (row_count > 1)
			set_read_length = 8;
		char *response_length_read = new char[4];
		response_length_read[0] = buffer[(read_length < 15) ? 3 : 10];
		response_length_read[1] = buffer[(read_length < 15) ? 4 : 11];
		response_length_read[2] = buffer[(read_length < 15) ? 5 : 12];
		response_length_read[3] = buffer[(read_length < 15) ? 6 : 13];
		unsigned short response_length = atoi(response_length_read);
		delete response_length_read;
		if (response_length_read == 0)
		{
			err_message = (char *)"A reference to an imposed data declaration holds no length\0";
			return FLITDB_CORRUPT;
		}
		if (store_response)
		{
			current_length = response_length;
			break;		
		}
		read_length = set_read_length;
		offset += response_length;
	}
	struct stat stat_buf;
    int rc = fstat(file_descriptor, &stat_buf);
    size_t bytes_to_move = (((rc == 0) ? stat_buf.st_size : 0) - insert_position);
	return FLITDB_NULL;
}

#endif