#include "flit_handler.cpp"
#include "flit.cpp"

int main()
{
	flitdb* flit = new flitdb();
	if (flit->setup("test.db",NULL) == FLITDB_SUCCESS)
	{
		extract(*flit,3,1);
		std::cout<<"Did you know that FlitDB is "<<retrieve_char(*flit)<<"?\n";
		insert(*flit,1,1,"it is very cool!");
	}
	return 0;
}