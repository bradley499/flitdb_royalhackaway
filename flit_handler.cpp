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

int flitdb::setup(const char *filename, int flags)
{
	if (configured)
	{
		err_message = "The database handler has already been attributed to handle another database\0";
		return FLITDB_ERROR;
	}
	return FLITDB_SUCCESS;
}

#endif