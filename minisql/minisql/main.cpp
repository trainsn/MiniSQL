#include <iostream>
#include "Interpreter.h"
// #include "CatalogManager.h"
// #include "RecordManager.h"
// #include "IndexManager.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*#include "API.h"*/

void init()
{
	FILE *fp;
	fp = fopen("Indexs", "r");
	if (fp == NULL)
	{
		fp = fopen("Indexs", "w+");
		return;
	}
	fclose(fp);
}

clock_t start;

int main(int argc, char * argv[])
{
	init();

	API api;
	CatalogManager cm;
	RecordManager rm;

	api.rm = &rm;
	api.cm = &cm;
	IndexManager im(&api);

	api.im = &im;
	rm.api = &api;

	start = 0;
	clock_t finish;

	cout << "******************WelCome to use our MiniSQL***********************" << endl;
	int fileRead = 0;
	ifstream file;
	Interpreter in;
	in.ap = &api;
	string s;
	int status = 0;
	while (1)
	{
		if (fileRead)
		{
			file.open(in.fileName.c_str());

			if (!file.is_open())
			{
				cout << "Fail to open" << in.fileName << endl;
				fileRead = 0;
				continue;
			}
			while (getline(file, s, ';'))
			{
				in.interpreter(s);
			}
			file.close();
			fileRead = 0;
			finish = clock();
			double duration = (double)(finish - start);
			printf("%2.1f milliseconds\n", duration);
		}
		else
		{
			cout << "minisql>>";
			getline(cin, s, ';');
			start = clock();
			status = in.interpreter(s);
			if (status == 2)
			{
				fileRead = 1;
			}
			else if (status==587)
			{
				break;
			}
			else
			{
				finish = clock();
				double duration = (double)(finish - start);
				printf("The duration is %2.1f millseconds\n", duration);
			}
		}
		
	}

	return 0;
}