/* AXML Parser
 * https://github.com/claudxiao/AndTools
 * Claud Xiao <iClaudXiao@gmail.com>
 */
#ifndef AXMLPARSER_H
#define AXMLPARSER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern uint32_t g_styleDataOff;   //style数据块的起始地址
extern uint32_t g_appTag_nameOff;  //保存application tag中的tagname成员较文件头的偏移值，tagname成员的值为一个字符串索引。
                            //我们可以根据该偏移值进行attribution插入和相关修改操作。

extern uint32_t g_curStringCount; //保存当前stringChunk总共含有的string个数
extern uint32_t g_appURIindex; //保存application所属命名空间的uri索引值
extern uint32_t g_res_ChunkSizeOffset; //保存resourcesChunk的chunksize偏移值


typedef enum{
	AE_STARTDOC = 0,
	AE_ENDDOC,
	AE_STARTTAG,
	AE_ENDTAG,
	AE_TEXT,
	AE_ERROR,
} AxmlEvent_t;


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

void *AxmlOpen(char *buffer, size_t size);

AxmlEvent_t AxmlNext(void *axml);

char *AxmlGetTagPrefix(void *axml);
char *AxmlGetTagName(void *axml);

int AxmlNewNamespace(void *axml);
char *AxmlGetNsPrefix(void *axml);
char *AxmlGetNsUri(void *axml);

uint32_t AxmlGetAttrCount(void *axml);
char *AxmlGetAttrPrefix(void *axml, uint32_t i);
char *AxmlGetAttrName(void *axml, uint32_t i);
char *AxmlGetAttrValue(void *axml, uint32_t i);

char *AxmlGetText(void *axml);

int AxmlClose(void *axml);

int AxmlToXml(char **outbuf, size_t *outsize, char *inbuf, size_t insize);

#ifdef __cplusplus
#if __cplusplus
};
#endif
#endif

#endif
