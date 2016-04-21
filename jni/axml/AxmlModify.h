/*************************************************************************
	> File Name: AxmlModify.h
	> Author: wanchouchou
	> Blog: Blog: http://www.cnblogs.com/wanyuanchun/
	> Created Time: 11/08/2014
 ************************************************************************/

#ifndef AXMLMODIFY_H
#define AXMLMODIFY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	char *data;
	uint32_t cur;
}FileAssit_t;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif // __cplusplus
#endif // __cplusplus

//axml modify functions
void modifyStringChunk(FileAssit_t *in, FileAssit_t *out, size_t *externSize);
void modifyAppTagChunk(FileAssit_t *in, FileAssit_t *out, size_t *externSize);
int axmlModify(char* inbuf, size_t insize, char *out_filename);

#ifdef __cplusplus
#if __cplusplus
}
#endif // __cplusplus
#endif // __cplusplus

#endif // AXMLMODIFY_H
