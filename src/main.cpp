#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;

bool exitbool = false;

void parseinput(vector<string> &commandinput, vector<unsigned> &linemarker)
{
	string userinput;
	getline(cin, userinput);
	int u = userinput.size();
	char *cstr = new char[u];
	strcpy(cstr, userinput.c_str());
	char *token, *save_1, *token2, *save_2;
	unsigned linecount = 0;
	token = strtok_r(cstr, ";", &save_1);
	while(token != NULL)
	{
		linecount = 0;
		token2 = strtok_r(token, " ", &save_2);
		while(token2 != NULL)
		{
			commandinput.push_back(token2);
			linecount++;
			token2 = strtok_r(NULL, " ", &save_2);
		}
		linemarker.push_back(linecount);
		token = strtok_r(NULL, ";", &save_1);
	}
	delete []cstr;
}


bool process_command(char *readcommands[])
{
	int pid = fork();
	if(pid <= -1)
	{
		perror("Fork error.");
		return false;
	}
	else if(pid == 0)
	{
		if(-1 == execvp(readcommands[0], readcommands))
		{
			perror("There was an error running execvp.");
			return false;
		}
		return true;
	}
	else if(pid > 0)
	{
		if(-1 == wait(0))
		{
			perror("There was an error with wait. ");
			return false;
		}
	}
	return true;

}


void andor_case(vector<string> tempvect, int andpos, char* readcommands[], bool andcase, bool orcase)
{
	bool commentflag = false;
	char *andorreadcommands[10000];
	int temp = 0;
	for(unsigned i = andpos; i < tempvect.size(); i++)
	{
		for(unsigned j = 0; j < tempvect[i].size(); j++)
		{
			if(tempvect[i][j] == '#')
			{
				commentflag = true;
				break;
			}
		}
		if(commentflag)
		{
			break;
		}
		if(tempvect[i] == "&&")
		{
			andor_case(tempvect, i+1, andorreadcommands, true, false);
			break;
		}
		if(tempvect[i] == "||")
		{
			andor_case(tempvect, i+1, andorreadcommands, false, true);
			break;
		}
		else
		{
			andorreadcommands[temp] = const_cast<char*>(tempvect[i].c_str());
			temp++;
		}
	}
	if(andcase)
	{
		if(process_command(readcommands))
		{
			process_command(andorreadcommands);
		}
	}
	if(orcase)
	{
		if(!process_command(readcommands))
		{
			process_command(andorreadcommands);
		}
	}
}

void ioredircase(vector<string> tempvect, unsigned pos, char* readcommands[], bool inputredirflag, bool outputredirflag, bool appendoutputredirflag, bool pipeflag)
{
	cerr << "Found redirection case." << endl;
	char ioredirreadcommands[10000];
	for(unsigned i = pos; i < tempvect.size(); i++)
	{
		if(tempvect[i] == "<")
		{
			ioredircase(tempvect, i+1, readcommands, true, false, false, false);	
		}
		if(tempvect[i] == ">")
		{
			ioredircase(tempvect, i+1, readcommands, false, true, false, false);
		}
		if(tempvect[i] == ">>")
		{
			ioredircase(tempvect, i+1, readcommands, false, false, true, false);
		}
		if(tempvect[i] == "|")
		{
			ioredircase(tempvect, i+1, readcommands, false, false, false, true);
		}
		else
		{
			strcat(ioredirreadcommands, tempvect[i].c_str());
		}
	}
	if(inputredirflag)
	{
		cerr << "Detected input redirection. Executing first." << endl;
		if(process_command(readcommands))
		{
			cerr << "Execution successful. Passing in the input." << endl;
			int fdsave;
			if(-1 == (fdsave = dup(0)))
			{
				perror("There was an error with dup.");
				exit(EXIT_SUCCESS);
			}
			if(-1 == write(0, ioredirreadcommands, 10000))
			{
				perror("There was an error with writing.");
				exit(EXIT_SUCCESS);
			}
			if(-1 == (dup2(fdsave, 0)))
			{
				perror("There was an error with dup2.");
				exit(EXIT_SUCCESS);
			}
		}
	}
	if(outputredirflag)
	{

	}
	if(appendoutputredirflag)
	{

	}
	if(pipeflag)
	{

	}

}

bool ioredir(vector<string> tempvect, unsigned inputpos, char* readcommands[], bool inputredir, bool outputredir)
{
	cerr << "Entering ioredir function. " << endl;
	int pid = fork();
	if(pid <= -1)
	{
		perror("Fork error.");
		return false;
	}
	else if(pid == 0)
	{
		cerr << "Entering child process." << endl;
		if(-1 == execvp(readcommands[0], readcommands))
		{
			perror("There was an error running execvp.");
			return false;
		}
		else
		{
			return true;
		}
	}
	else if(pid > 0)
	{
		if(-1 == wait(0))
		{
			perror("There was an error with wait. ");
			return false;
		}
	}
	if(inputredir)
	{
		cerr << "Entering input redirection." << endl;
		int fdsave;
		if(-1 == (fdsave = dup(0)))
		{
			perror("There was an error with dup.");
			return false;
		}
		if(-1 == write(0, tempvect[inputpos].c_str(), tempvect[inputpos].size()))
		{
			perror("There was an error with writing.");
			return false;
		}
		if(-1 == (dup2(fdsave, 0)))
		{
			perror("There was an error with dup2.");
			return false;
		}
	}
	return true;
}

void pre_process(vector<string> tempvect)
{
	bool commentflag = false;
	bool noconnectorflag = false;

	char *readcommands[10000];
	for(unsigned i = 0; i < tempvect.size(); i++)
	{
		for(unsigned j = 0; j < tempvect[i].size(); j++)
		{
			if(tempvect[i][j] == '#')
			{
				commentflag = true;
			}
		}
		if(commentflag)
		{
			break;
		}
		if(tempvect[i] == "&&")
		{
			noconnectorflag = true;
			andor_case(tempvect, i+1, readcommands, true, false);
			break;
		}
		if(tempvect[i] == "||")
		{
			noconnectorflag = true;
			andor_case(tempvect, i+1, readcommands, false, true);
			break;
		}
		if(tempvect[i] == "<")
		{
			ioredircase(tempvect, i+1, readcommands, true, false, false, false);
		}
		if(tempvect[i] == ">")
		{
			ioredircase(tempvect, i+1, readcommands, false, true, false, false);
		}
		if(tempvect[i] == ">>")
		{
			ioredircase(tempvect, i+1, readcommands, false, false, true, false);
		}
		if(tempvect[i] == "|")
		{
			ioredircase(tempvect, i+1, readcommands, false, false, false, true);
		}
		else
		{
			readcommands[i] = const_cast<char*>(tempvect[i].c_str());
		}
	}
	if(!noconnectorflag)
	{
		process_command(readcommands);
	}
}

void processinput(vector<string> &commandinput, vector<unsigned> linemarker)
{
	int argcount = 0;
	int temp = 0;
	vector<string> tempvect;
	for(unsigned i = 0; i < linemarker.size(); i++)
	{
		tempvect.clear();
		temp = 0;
		for(unsigned j = argcount; j < (linemarker[i] + argcount); j++)
		{
			if(commandinput[j] == "exit" || commandinput[j] == "Exit" || commandinput[j] == "EXIT")
			{
				exitbool = true;
				cout << "Exited Shell. Good Day." << endl;
				exit(EXIT_SUCCESS);
			}
			tempvect.push_back(commandinput[j]);
			temp++;
		}
		argcount += temp;
		pre_process(tempvect);
	}
}

int main(int argc, char** argv)
{
	while(!exitbool)
	{
		vector<unsigned> linemarker;
		vector<string> commandinput;
		cout << "$ ";
		parseinput(commandinput, linemarker);
		processinput(commandinput, linemarker);
	}
}
