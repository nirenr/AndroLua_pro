#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#define MA_VERSION 1
struct options_t{
	int parserXml;
	int modifyXml;
	int help;
	int version;
	char target_file[128];
	char output_file[128];
};


struct options_t* handle_arguments(int argc, char* argv[]);
void usage();
void show_help();
void show_version();

#endif
