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
	flock(file_descriptor, LOCK_UN);
	close(file_descriptor);
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
	file_descriptor = open(filename, ((flags & FLITDB_READONLY) ? O_RDONLY : O_RDWR));
	if (flock(file_descriptor, LOCK_EX | LOCK_NB) != 0)
	{
		err_message = (char *)"Exclusive rights to access the database could not be obtained\0";
		return FLITDB_BUSY;
	}
	struct stat stat_buf;
	int rc = fstat(file_descriptor, &stat_buf);
	size = ((rc == 0) ? stat_buf.st_size : 0);

	configured = true;
	return FLITDB_SUCCESS;
}

int flitdb::insert_value(int set_value)
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
	if (strlen(set_value) >= 10000)
	{
		err_message = (char *)"Data insertion avoided due to the length of a string being too large";
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
	return value.int_value;
}

double flitdb::retrieve_value_double()
{
	return value.double_value;
}

float flitdb::retrieve_value_float()
{
	return value.float_value;
}

char *flitdb::retrieve_value_char()
{
	return value.char_value;
}

bool flitdb::retrieve_value_bool()
{
	return value.bool_value;
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
	char *input_buffer;
	size_t input_size = 0;
	switch (value_type)
	{
	case 1:
	{
		input_buffer = new char[21];
		int ret = snprintf(input_buffer, sizeof(input_buffer), "%d", value.int_value);
		if (ret <= 0)
		{
			err_message = (char *)"Value encoding of type: int; has failed\0";
			return FLITDB_RANGE;
		}
		input_size = strlen(input_buffer);
		break;
	}
	case 2:
	{
		input_buffer = new char[87];
		int ret = snprintf(input_buffer, sizeof(input_buffer), "%lf", value.double_value);
		if (ret <= 0)
		{
			err_message = (char *)"Value encoding of type: double; has failed\0";
			return FLITDB_RANGE;
		}
		input_size = strlen(input_buffer);
		break;
	}
	case 3:
	{
		input_buffer = new char[87];
		int ret = snprintf(input_buffer, sizeof(input_buffer), "%f", value.float_value);
		if (ret <= 0)
		{
			err_message = (char *)"Value encoding of type: float; has failed\0";
			return FLITDB_RANGE;
		}
		input_size = strlen(input_buffer);
		break;
	}
	case 4:
	{
		input_size = strlen(value.char_value);
		input_buffer = new char[input_size + 1];
		input_buffer[input_size] = '\0';
		strncpy(input_buffer, value.char_value, input_size);
		break;
	}
	case 5:
	{
		input_buffer = new char[1];
		input_buffer[0] = value.bool_value;
		input_size = 1;
		break;
	}
	}
	char size_buffer[4];
	strncpy(size_buffer, "\0", 4);
	int ret = snprintf(size_buffer, 4, "%d", (int)input_size);
	if (input_size < 10)
	{
		size_buffer[3] = size_buffer[0];
		size_buffer[2] = '0';
		size_buffer[1] = '0';
		size_buffer[0] = '0';
	}
	else if (input_size < 100)
	{
		size_buffer[3] = size_buffer[1];
		size_buffer[2] = size_buffer[0];
		size_buffer[1] = '0';
		size_buffer[0] = '0';
	}
	else if (input_size < 100)
	{
		size_buffer[3] = size_buffer[2];
		size_buffer[2] = size_buffer[1];
		size_buffer[1] = size_buffer[0];
		size_buffer[0] = '0';
	}
	if (ret <= 0)
	{
		err_message = (char *)"Length determination failed\0";
		return FLITDB_ERROR;
	}
	bool use_response = false;
	size_t offset = 0;
	size_t skip_offset = 0;
	unsigned char read_length = 15;
	unsigned short row_count = 0;
	unsigned short row_position_count = 0;
	size_t current_length = 0;
	size_t insert_position = 0;
	unsigned char insertion_state = 0;
	ssize_t insert_after = -1;
	unsigned short int column_length[2] = {0, 0};
	while (size > 0)
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
			break;
		insertion_state = 2;
		if (read_length == 15)
		{
			column_length[0] = skip_offset;
			insertion_state = 1;
			char *skip_amount_read = new char[4];
			skip_amount_read[0] = buffer[0];
			skip_amount_read[1] = buffer[1];
			skip_amount_read[2] = buffer[2];
			skip_amount_read[3] = buffer[3];
			skip_offset += (atoi(skip_amount_read) + 1);
			delete skip_amount_read;
			if (skip_offset > column_position)
			{
				offset -= read_length;
				break;
			}
			char *row_count_read = new char[3];
			row_count_read[0] = buffer[4];
			row_count_read[1] = buffer[5];
			row_count_read[2] = buffer[6];
			row_count = atoi(row_count_read);
			column_length[1] = row_count;
			row_position_count = 0;
			delete row_count_read;
		}
		unsigned char set_read_length = 15;
		if (skip_offset == column_position)
		{
			if (skip_offset != insert_after && insert_after >= 0)
			{
				offset -= read_length;
				break;
			}
			if (row_count < row_position)
				insert_after = skip_offset;
			char position_read[3];
			position_read[0] = buffer[(read_length < 15) ? 0 : 7];
			position_read[1] = buffer[(read_length < 15) ? 1 : 8];
			position_read[2] = buffer[(read_length < 15) ? 2 : 9];
			unsigned short position = atoi(position_read);
			if (position == row_position)
			{
				use_response = true;
				row_count = 0;
			}
			else if (position > row_position)
			{
				offset -= read_length;
				break;
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
			return FLITDB_CORRUPT;
		}
		if (use_response)
		{
			current_length = response_length;
			break;
		}
		read_length = set_read_length;
		offset += response_length;
	}
	if (current_length == 0 && input_size == 0)
		return FLITDB_DONE;
	else if (input_size == 0)
	{
		unsigned short transaction_size_max = 24;
		unsigned short transaction_size = transaction_size_max;
		input_size += (insertion_state == 1) ? 15 : 8;
		offset += current_length;
		unsigned long remove_offset = ((insertion_state == 1) ? 15 : 8) + current_length;
		while (transaction_size > 0)
		{
			transaction_size = transaction_size_max;
			if (size < (offset + transaction_size))
				transaction_size = size - offset;
			if ((offset + transaction_size) > size)
				transaction_size = transaction_size_max - ((offset + transaction_size) - size);
			if (transaction_size == 0)
				break;
			strncpy(buffer, "\0", sizeof(buffer));
			pread64(file_descriptor, buffer, transaction_size, offset);
			pwrite64(file_descriptor, buffer, transaction_size, (offset - remove_offset));
			offset += transaction_size;
			if (transaction_size < transaction_size_max)
				break;
		}
		size -= remove_offset;
		if (ftruncate(file_descriptor, (size - remove_offset)) != 0)
		{
			err_message = (char *)"A failed to truncate database\0";
			return FLITDB_CORRUPT;
		}

	}
	else if (current_length > input_size)
	{
		perror("Undefined behaviour will occur - not fully implemented!");
		if (input_size > 0)
		{
			input_size += ((current_length != 0) ? 0 : ((insertion_state == 1) ? 15 : 8));
			offset -= ((current_length != 0) ? 0 : ((insertion_state == 1) ? 15 : 8));
		}
		unsigned short transaction_size_max = 24;
		unsigned short transaction_size = transaction_size_max;
		const size_t origin_offset = offset;
		const size_t resize = (current_length - input_size) - 1;
		if (current_length > 0)
		{
			while (offset > size)
			{
				transaction_size = transaction_size_max;
				if (size < (offset + transaction_size))
				{
					transaction_size += offset;
					transaction_size -= size;
				}
				offset += transaction_size;
				if ((offset + transaction_size) > size)
					transaction_size = transaction_size_max - ((offset + transaction_size) - size);
				if (transaction_size == 0)
					break;
				strncpy(buffer, "\0", sizeof(buffer));
				pread64(file_descriptor, buffer, transaction_size, offset);
				pwrite64(file_descriptor, buffer, transaction_size, (offset - resize));
				if (transaction_size < transaction_size_max)
					break;
			}
			pwrite64(file_descriptor, input_buffer, input_size, origin_offset);
			pwrite64(file_descriptor, (char *)value_type, transaction_size, (origin_offset - 1));
			pwrite64(file_descriptor, size_buffer, 4, (origin_offset - 5));
			size -= (current_length - input_size);
			if (ftruncate(file_descriptor, size) != 0)
			{
				clear_values();
				err_message = (char *)"A failed to truncate database\0";
				return FLITDB_CORRUPT;
			}
		}
	}
	else if (current_length < input_size)
	{
		bool empty = (size == 0);
		if (!empty)
		{
			const size_t growth_padding_increase_difference = (input_size + ((current_length != 0) ? 0 : ((insertion_state == 1) ? 15 : 8)) - current_length);
			const size_t growth_padding_increase = size + growth_padding_increase_difference;
			size_t growth_padding = growth_padding_increase;
			size_t growth_padding_increase_offset = size;
			size_t growth_padding_offset = growth_padding_increase - growth_padding_increase_difference;
			size_t growth_insert_inverse = 0;
			bool ending = false;
			unsigned short transaction_size_max = 1024;
			while (growth_padding_offset > offset && !ending)
			{
				size_t transaction_size = transaction_size_max;
				if (growth_padding_increase_offset > transaction_size_max)
					growth_padding_increase_offset -= transaction_size_max;
				else
				{
					if (transaction_size_max > (size - offset))
					{
						transaction_size = (size - offset);
						growth_padding_increase_offset = offset;
					}
					else
					{
						transaction_size = (transaction_size - growth_padding_increase_offset);
						growth_padding_increase_offset -= transaction_size;
					}
					ending = true;
				}
				growth_padding_offset -= transaction_size;
				strncpy(buffer, "\0", sizeof(buffer));
				pread64(file_descriptor, buffer, transaction_size, growth_padding_increase_offset);
				pwrite64(file_descriptor, buffer, transaction_size, (growth_padding_increase_offset + growth_padding_increase_difference));
			}
			if (column_position == skip_offset)
				skip_offset = 0;
			else
				skip_offset = (column_position - skip_offset - 1);
		}
		else
		{
			for (size_t i = 0; i < (16 + input_size); i++)
				pwrite64(file_descriptor, "0", sizeof(char), i);
			current_length = input_size;
			input_size += 15;
			size += input_size;
			offset = 15;
			insertion_state = 0;
		}
		char skip_offset_buffer[5];
		strncpy(skip_offset_buffer, "\0", 5);
		int ret = snprintf(skip_offset_buffer, 4, "%d", skip_offset);
		if (skip_offset < 10)
		{
			skip_offset_buffer[3] = skip_offset_buffer[0];
			skip_offset_buffer[2] = '0';
			skip_offset_buffer[1] = '0';
			skip_offset_buffer[0] = '0';
		}
		else if (skip_offset < 100)
		{
			skip_offset_buffer[3] = skip_offset_buffer[1];
			skip_offset_buffer[2] = skip_offset_buffer[0];
			skip_offset_buffer[1] = '0';
			skip_offset_buffer[0] = '0';
		}
		else if (skip_offset < 100)
		{
			skip_offset_buffer[3] = skip_offset_buffer[2];
			skip_offset_buffer[2] = skip_offset_buffer[1];
			skip_offset_buffer[1] = skip_offset_buffer[0];
			skip_offset_buffer[0] = '0';
		}
		char column_length_buffer[4];
		strncpy(column_length_buffer, "\0", 4);
		if (column_length[1] < row_position)
			column_length[1] += 1;
		ret = snprintf(column_length_buffer, 3, "%d", column_length[1]);
		if (column_length[1] < 10)
		{
			column_length_buffer[2] = column_length_buffer[0];
			column_length_buffer[1] = '0';
			column_length_buffer[0] = '0';
		}
		else if (column_length[1] < 100)
		{
			column_length_buffer[2] = column_length_buffer[1];
			column_length_buffer[1] = column_length_buffer[0];
			column_length_buffer[0] = '0';
		}
		char insert_position[3];
		strncpy(insert_position, "\0", 3);
		ret = snprintf(insert_position, 3, "%d", row_position);
		if (column_length[1] < 10)
		{
			insert_position[2] = insert_position[0];
			insert_position[1] = '0';
			insert_position[0] = '0';
		}
		else if (column_length[1] < 100)
		{
			insert_position[2] = insert_position[1];
			insert_position[1] = insert_position[0];
			insert_position[0] = '0';
		}
		char insert_length[5];
		strncpy(insert_length, "\0", 5);
		ret = snprintf(insert_length, 4, "%d", input_size);
		if (input_size < 10)
		{
			insert_length[3] = insert_length[0];
			insert_length[2] = '0';
			insert_length[1] = '0';
			insert_length[0] = '0';
		}
		else if (input_size < 100)
		{
			insert_length[3] = insert_length[1];
			insert_length[2] = insert_length[0];
			insert_length[1] = '0';
			insert_length[0] = '0';
		}
		else if (input_size < 100)
		{
			insert_length[3] = insert_length[2];
			insert_length[2] = insert_length[1];
			insert_length[1] = insert_length[0];
			insert_length[0] = '0';
		}
		offset += 1;
		if (insertion_state != 2)
		{
			pwrite64(file_descriptor, skip_offset_buffer, 4, (offset - 16));
			pwrite64(file_descriptor, column_length_buffer, 3, (offset - 12));
		}
		else
			pwrite64(file_descriptor, column_length_buffer, 3, (column_length[0] - 12));
		pwrite64(file_descriptor, insert_position, 3, (offset - 9));
		pwrite64(file_descriptor, insert_length, 4, (offset - 6));
		switch (value_type)
		{
			case 1:
				pwrite64(file_descriptor, "1", 1, (offset - 2));
				break;
			case 2:
				pwrite64(file_descriptor, "2", 1, (offset - 2));
				break;
			case 3:
				pwrite64(file_descriptor, "3", 1, (offset - 2));
				break;
			case 4:
				pwrite64(file_descriptor, "4", 1, (offset - 2));
				break;
			case 5:
				pwrite64(file_descriptor, "5", 1, (offset - 2));
				break;
		}
		pwrite64(file_descriptor, input_buffer, input_size, (offset - 1));
		if (!empty)
			size += insert_length + (insertion_state == 1) ? 15 : 8;
		if (ftruncate(file_descriptor, size) != 0)
		{
			clear_values();
			err_message = (char *)"A failed to truncate database\0";
			return FLITDB_CORRUPT;
		}
	}
	else
		pwrite64(file_descriptor, input_buffer, input_size, offset);
	clear_values();
	return FLITDB_DONE;
}

#endif