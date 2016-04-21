/*
AXML Ambiguity
author: wanchouchou
Blog: http://www.cnblogs.com/wanyuanchun/
*/

#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

void usage() {
    show_help();
    printf("Parser an AXML:\n");
	printf("      ./manifestAmbiguity -p filename;\n");
	printf("      Or ./manifestAmbiguity --parser=filename\n");
	printf("Modify an AXML:\n");
	printf("      ./manifestAmbiguity -m target_filename -o output_filename;\n");
	printf("      Or ./manifestAmbiguity --modify=target_filename --out=output_filename\n");
}

void show_version() {
    printf("+++++++++++++++++++++++++\n");
	printf("+ Learning And Sharing  +\n");
	printf("+ Version: %d            +\n", MA_VERSION);
	printf("+ From Wanchouchou ^-^! +\n");
	printf("+++++++++++++++++++++++++\n");
}

void show_help() {
	printf("\t----------------------------------------\n");
	printf("\t|==== Android Manifest Ambiguity ====  |\n");
	printf("\t----------------------------------------\n");
	printf("manifestAmbiguity [options] file\n");
	printf("-p, --parser=target_file            parser axml and print it\n");
	printf("-m, --modify=target_file            modify axml. Up to now, we just provide a single function that inserting an useless attrbution in axml\n");
	printf("-o, --out=output_file               the file to store modified axml. If not defined , the default outfile is out.xml\n");
	printf("-h, --help                          show help\n");
	printf("-v, --version                       show version\n");
	show_version();
}

struct options_t* handle_arguments(int argc, char* argv[]) {
    static struct options_t opts;  //必须static 不然就会在函数返回的时候被销毁～
	memset(&opts, 0, sizeof(opts));

	int opt;
	int longidx;
	if (argc == 1) {
		return NULL;
	}

	const char* short_opts = "hp:m:o:v";
    struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"parser", 1, NULL, 'p'},
        {"modify", 1, NULL, 'm'},
        {"out", 1, NULL, 'o'},
        {"version", 0, NULL, 'v'},
        {0, 0, 0, 0}
    };

	while ((opt = getopt_long(argc, argv, short_opts, long_opts, &longidx)) != -1) {
		switch (opt) {
        case 'h':
            opts.help = 1;
			break;
        case 'p':
            opts.parserXml = 1;
            strncpy(opts.target_file, optarg, 128);
            break;
        case 'm':
            opts.modifyXml = 1;
            strncpy(opts.target_file, optarg, 128);
            break;
        case 'o':
            strncpy(opts.output_file, optarg, 128);
            break;
        case 'v':
            opts.version = 1;
            break;
		case '?':
			//unknow options;
			return NULL;
			break;
		default :
			return NULL;
			break;
		}/* end switch */
	}/* end while */

	return &opts;
}



