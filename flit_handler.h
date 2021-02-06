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
	char* err_message = "\0";
public:
	flitdb();
	~flitdb();
	int setup(const char *filename, int flags);
};

flitdb::flitdb()
{
}

flitdb::~flitdb()
{
}

int flitdb::setup(const char *filename, int flags)
{
	if (configured)
	{
		err_message = "The database handler has already been attributed to handle another database\0";
		return FLITDB_ERROR;
	}
}

#endif