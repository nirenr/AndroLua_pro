/*
AXML Ambiguity
author: wanchouchou
Blog: http://www.cnblogs.com/wanyuanchun/
*/

#include "AxmlModify.h"
#include "AxmlParser.h"

/* attribute structure within tag */
typedef struct{
	char uri[4];		/* uri of its namespace */
	char name[4];
	char string[4];	/* attribute value if type == ATTR_STRING */
	char type[4];		/* attribute type, == ATTR_*  指定属性值得类型，如string, int, float等*/
	char data[4];		/* attribute value, encoded on type 属性的值，该值根据属性的 type进行编码*/
} Attribute_t;


static void copyDataWithPosChange(FileAssit_t *to, FileAssit_t *src, size_t size){
	memcpy(to->data + to->cur, src->data + src->cur, size);
	to->cur += size;
	src->cur += size;
}

static uint32_t getUint32(FileAssit_t *hp){
	uint32_t value = 0;
	unsigned char *p = hp->data + hp->cur;
	value = p[0] | p[1]<<8 | p[2]<<16 | p[3]<<24;
	hp->cur += 4;
	return value;
}
static void skipUint32(FileAssit_t *hp){
	hp->cur += 4;
}
/*
将大端表示的uint_32转换成小端表示的4字节数据后，存储到目标地址
*/
static void copyUint32(char *to, uint32_t value){
	/**/
	char p[4];
	p[0] = value & 0xff ;
	p[1] = (value >>8) &0xff;
	p[2] = (value >> 16) &0xff;
	p[3] = (value >> 24) &0xff;
	memcpy(to, p, 4);

}

/*
在string中插入两个字符串chouchou.class，和name
*/
void modifyStringChunk(FileAssit_t *in, FileAssit_t *out, size_t *externSize){
	char appendString[32] = {0x0E,0x00,0x63,0x00,0x68,0x00,0x6F,0x00,0x75,0x00,0x63,0x00,0x68,0x00,0x6F,0x00,0x75,0x00,
		0x2E,0x00,0x63,0x00,0x6C,0x00,0x61,0x00,0x73,0x00,0x73,0x00,0x00,0x00}; //chouchou.class的UTF-16编码
	char appendString2[12] = {0x04,0x00,0x6E,0x00,0x61,0x00,0x6D,0x00,0x65,0x00,0x00,0x00}; //name的UTF-16编码
	//问题，一定要进行4字节对齐操作！考虑到如果有styleData段，那么原string本身就没进行4字节对齐，直接在后面加上string，然后决定是否对新加入的string进行4字节对齐。
	//如果没styleData段，那么原string本身就进行了4字节对齐，我们只需要根据添加的string length来觉得是否进行4字节对齐即可。
	//这就涉及到stringchunksize的改变和styleOffset的改变,所以我们只有在最后才能确定两者的大小~
	//总之对齐操作比较麻烦，需要特别注意
	uint32_t stringChunkSize = 0;
	uint32_t stringCount = 0;
	uint32_t styleCount = 0;
	uint32_t stringOffset = 0;
	uint32_t styleOffset = 0;
	uint32_t stringLen = 0;
	uint32_t addStringOffset = 0;
	uint32_t addStringOffset2 = 0;
	uint32_t addStringOffset_alied = 0;
	uint32_t addStringOffset_alied2 = 0;

	copyDataWithPosChange(out, in, 4); //StringTag
	stringChunkSize = getUint32(in);
	out->cur += 4;
	stringCount = getUint32(in);
	g_curStringCount = stringCount + 2;
	copyUint32(out->data + out->cur, stringCount + 2);
	out->cur += 4;
	styleCount = getUint32(in);
	in->cur -= 4;
	copyDataWithPosChange(out, in, 4*2); //stylesCount 和 reverse
	stringOffset = getUint32(in);
    copyUint32(out->data + out->cur, stringOffset + 4*2); //由于插入了2个字符串，所以需要添加两个4字节的偏移条目，所以stringOffset要加8！
	out->cur += 4;
	styleOffset = getUint32(in); //styleOffset可能会改变，所以这里就不直接copy了
	if(styleOffset == 0){ //说明没有style，可以直接copy
		in->cur -= 4;
		copyDataWithPosChange(out, in, 4);
		stringLen = stringChunkSize - stringOffset;
	}else{ //说明有style，那么在插入string后styleOffset会改变
		//styleOffset最后再进行赋值
		out->cur += 4;
		stringLen = styleOffset - stringOffset;
	}
	//copy stringCount 个 string 偏移值
	copyDataWithPosChange(out, in, stringCount * 4);
	/*然后在此时的out->data中插入2个string偏移值，指向我们插入的字符串的首地址*/
	addStringOffset = stringLen;
	addStringOffset_alied = (addStringOffset+0x03)&(~0x03);  //虽然在没有style的情况下stringLen本身就是4的整数倍，但在有style的情况下就不一定了~
	addStringOffset2 = addStringOffset_alied + 32;  //添加上chouchou.class的大小
	addStringOffset_alied2 = (addStringOffset2+0x03)&(~0x03); //为了方便以后扩展，这里addStringOffset2虽然是4的整数倍，但还是进行一次对齐吧
	*externSize += (addStringOffset_alied - addStringOffset + addStringOffset_alied2 - addStringOffset2);
	//设置chochou.class的偏移值
	copyUint32(out->data + out->cur, addStringOffset_alied);
	out->cur += 4;
	//设置name的偏移值
	copyUint32(out->data + out->cur, addStringOffset_alied2);
	out->cur += 4;

	//然后就是连续的n个 styles偏移值。
	copyDataWithPosChange(out, in, styleCount * 4);
	//然后就是string data
	copyDataWithPosChange(out, in, stringLen);

	//插入“chochou.class” 这里由于插入的32字节刚好为4的倍数，所以就不需要进行对齐了。留待以后添加吧~
	memcpy(out->data + out->cur, appendString, 32);
	out->cur += 32;
	//插入"name",同样不需要对齐
	memcpy(out->data + out->cur, appendString2, 12);
	out->cur += 12;

	/*添加长度刚好为4的整数倍，所以添加的字符串本身不涉及对齐操作。如果要添加其他的字符串，进行相应的修改即可*/
	//chouchou.class
	*externSize += 32; //字符串长度
	*externSize += 4;  //需要为该字符串添加一个4字节的偏移表项
	//name
	*externSize +=12;
	*externSize += 4;

	if(styleOffset != 0){
		//根据sxternSize的值,确定当前styleOffset值
		copyUint32(out->data + 0x20,styleOffset + *externSize);
	}
	//根据sxternSize的值，确定当前stringChunck的大小
	copyUint32(out->data + 0xc, stringChunkSize + *externSize);

}

void modifyResourceChunk(FileAssit_t *in, FileAssit_t *out, size_t *externSize){
	//首先获取原始resourceChunk大小
	uint32_t resChunkSize = 0;
	uint32_t resCount = 0;
	uint32_t needAppendCount = 0;
	resChunkSize = getUint32(in);
	resCount = resChunkSize /4 -2;
	needAppendCount = g_curStringCount - resCount;

	copyUint32(out->data + out->cur, resChunkSize + needAppendCount * 4);
	out->cur += 4;

	//copy 原来的resCount个resourceID
	copyDataWithPosChange(out, in, resCount * 4);

	//用0x0填充剩下的resourcesID
	memset(out->data + out->cur, 0, needAppendCount * 4);
	out->cur += needAppendCount*4;

	*externSize += (needAppendCount * 4);

}

void modifyAppTagChunk(FileAssit_t *in, FileAssit_t *out, size_t *externSize){
	uint32_t curAttrCount = 0;
	uint32_t curChunkSize = 0;
	Attribute_t attr;

	memset(&attr, 0, sizeof(Attribute_t));
	copyUint32(attr.uri, g_appURIindex);
	copyUint32(attr.name, g_curStringCount - 1);
	copyUint32(attr.string, g_curStringCount - 2);
	copyUint32(attr.type, 0x03000008);
	copyUint32(attr.data,  g_curStringCount - 2);

	//修改chunksize!!
	in->cur -= 0x10; //指向chunksize;
	curChunkSize = getUint32(in);
	curChunkSize += sizeof(Attribute_t);
	copyUint32(out->data + out->cur - 0x10, curChunkSize);
	in->cur += 0xc; //指向tagname

	copyDataWithPosChange(out, in, 8); //tagname and flags
	curAttrCount = getUint32(in);
	curAttrCount++;
	copyUint32(out->data + out->cur, curAttrCount);
	out->cur += 4;
	copyDataWithPosChange(out, in, 4); //classAttribute

	//插入新添加的属性结构体
	memcpy(out->data + out->cur, (char*)&attr, sizeof(Attribute_t));
	out->cur += sizeof(Attribute_t);
	*externSize += sizeof(Attribute_t);


	/*后面的数据由于不用改变，所以直接copy即可*/

}

int axmlModify(char* inbuf, size_t insize, char *out_filename){
	FILE *fp;
	char *outbuf;
	char *filename = out_filename;
	size_t externSize = 0;  //扩张的字节数
	uint32_t filesize = 0;
	size_t ret = 0;
	FileAssit_t input_h, output_h;
	fp = fopen(filename, "wb");

	if(fp == NULL)
	{
		fprintf(stderr, "Error: open output file failed.\n");
		return -1;
	}
	outbuf = (char *)malloc((insize + 300) * sizeof(char));  //多分配300字节，可以根据自己添加字符串的大小进行扩充。
	if(outbuf == NULL){
		fprintf(stderr, "Error: malloc outbuf failed.\n");
		return -1;
	}
	memset(outbuf, 0, insize);
	input_h.data = inbuf;
	input_h.cur = 0;
	output_h.data = outbuf;
	output_h.cur = 0;
	//首先copy魔数
	copyDataWithPosChange(&output_h, &input_h, 4);
	//然后获取文件大小
	filesize = getUint32(&input_h);
	output_h.cur += 4;

	//插入string并更改stringChunk块
	modifyStringChunk(&input_h, &output_h, &externSize);
	//style data到g_res_ChunkSizeOffset之间的数据，由于不需要改动，所以直接copy
	copyDataWithPosChange(&output_h, &input_h, g_res_ChunkSizeOffset - input_h.cur);

	//根据curStringChunk的大小对ResourceChunk块进行填充，以便新添加的attr的ID号为0x00000000
	modifyResourceChunk(&input_h, &output_h, &externSize);
	copyDataWithPosChange(&output_h, &input_h, g_appTag_nameOff - input_h.cur);

	//插入attrubition并更改该attr所属的tagChunk块
	modifyAppTagChunk(&input_h, &output_h, &externSize);
	//修改文件大小
	copyUint32(output_h.data + 4, filesize + externSize);
	//copy剩余的部分
	copyDataWithPosChange(&output_h, &input_h, filesize - input_h.cur);

	ret = fwrite(output_h.data, 1, output_h.cur, fp);
	if(ret != output_h.cur){
		fprintf(stderr, "Error: fwrite outbuf error.\n");
		return -1;
	}

	free(output_h.data);
	fclose(fp);

	return 0;







}
