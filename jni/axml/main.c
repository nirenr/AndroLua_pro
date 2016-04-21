/*************************************************************************
	> Project Name: ManifestAmbiguity
	> Author: wanchouchou
	> Blog: Blog: http://www.cnblogs.com/wanyuanchun/
	> Created Time: 11/08/2014
 ************************************************************************/
#include "AxmlParser.h"
#include "AxmlModify.h"
#include "options.h"

#ifdef _WIN32		/* windows */
#pragma warning(disable:4996)
#endif

/*Global vars. Used for AxmlModify*/
uint32_t g_styleDataOff = 0x0;
uint32_t g_appTag_nameOff = 0x0;
uint32_t g_curStringCount = 0x0;
uint32_t g_appURIindex = 0x0;
uint32_t g_res_ChunkSizeOffset = 0x0;

int main(int argc, char *argv[])
{
    struct options_t *g_opts;
	FILE *fp;
	char *inbuf;
	size_t insize;
	char *outbuf;
	size_t outsize;
	int ret;
	const char *target_filename;
	/* 处理命令行 */
	g_opts = handle_arguments(argc, argv);
	if (!g_opts) {
		/* 失败 */
		usage();
		return -1;
	}

	/* 处理命令行 */
	if (g_opts->help) {
		show_help();
		return 0;
	}
	if (g_opts->version) {
		show_version();
		return 0;
	}

	target_filename = g_opts->target_file;
	fp = fopen(target_filename, "rb");
	if(fp == NULL)
	{
		fprintf(stderr, "Error: open input file failed.\n");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	insize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	inbuf = (char *)malloc(insize * sizeof(char));
	if(inbuf == NULL)
	{
		fprintf(stderr, "Error: init file buffer.\n");
		fclose(fp);
		return -1;
	}

	ret = fread(inbuf, 1, insize, fp);
	if(ret != insize)
	{
		fprintf(stderr, "Error: read file.\n");
		free(inbuf);
		fclose(fp);
		return -1;
	}
	//无论是读取还是修改xml都需要先分析目标xml
	ret = AxmlToXml(&outbuf, &outsize, inbuf, insize);
	if(ret < 0){
		fprintf(stderr, "Error: parse file.\n");
		return -1;
	}
	if(g_opts->parserXml){
        printf("%s", outbuf);
        return 0;
	}
	if(g_opts->modifyXml){
        ret = axmlModify(inbuf, insize, g_opts->output_file);
        if(ret < 0){
		    fprintf(stderr, "Error: modify file.\n");
		    return -1;
	    }
	}
	free(outbuf);
	free(inbuf);
	fclose(fp);

	return ret;
}
