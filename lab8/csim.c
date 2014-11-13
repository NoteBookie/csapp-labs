/*
 * csim.c: a cache simulator and it uses LRU to solve eviction.
 * 
 * Name: Jin Jianian
 * Number: 5130379059
 * Date: 2014.10.22
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "cachelab.h"

typedef struct{
	int valid;
	int lru;
	unsigned long tag;
}Line;

typedef struct{
	int access;
	Line *lines;
}Set;

const int max = sizeof(long) * 8;

int cHit = 0, cMiss = 0, cEviction = 0;
unsigned long setId = 0, tagId = 0;
char traceInfo[30];
Set *sets = NULL;

void printSummary(int hits, int misses, int evictions);

void help(){
	printf("Usage: ./csim [-h] [-v] -s <s> -E <E> -b <b> -t <tracefile>\n \
	• -h: Optional help flag that prints usage info\n \
	• -v: Optional verbose flag that displays trace info \n \
	• -s <s>: Number of set index bits (S = 2s is the number of sets) \n \
	• -E <E>: Associativity (number of lines per set) \n \
	• -b <b>: Number of block bits (B = 2b is the block size) \n \
	• -t <tracefile>: Name of the valgrind trace to replay\n");
	return;
}

void memAlloc(int setnum, int lines){
	sets = (Set *) malloc(setnum * sizeof(Set));
	assert(sets != NULL);

	for(int i = 0; i < setnum; i++){
		Set *set = sets + i;
		set->lines = (Line *) malloc(lines * sizeof(Line));
		assert(set->lines != NULL);
		set->access = 0;
		for(int j = 0; j < lines; j++){
			Line *line = (set->lines) + j;
			line->valid = 0;
			line->lru = 0;
			line->tag = 0;
		}
	}
	return;
}

void cache(int setId, int tagId, int lines){
	Set *set = sets + setId;
	Line *line;
	int curr_lru = set->lines->lru;
	int replaced = 0;
	int miss_state = 1;
	for(int i = 0; i < lines; i++){
		line = set->lines + i;
		if(line->tag == tagId && line->valid == 1){
			set->access ++;
			line->lru = set->access;
			cHit++;
			miss_state = 0;
			strcat(traceInfo, "hit  ");
			break;
		}
	}
	if (miss_state == 1){
		strcat(traceInfo, "miss  ");
		int i = 0;
		for(; i < lines; i++){
			line = set->lines + i;
			if(line->valid == 0){
				line->tag = tagId;
				line->valid = 1;
				set->access ++;
				line->lru = set->access;
				cMiss++;
				break;
			}
		}

		if(i == lines){
			for(int i = 0; i < lines; i++){
				line = set->lines + i;
				if(line->lru < curr_lru){
					curr_lru = line->lru;
					replaced = i;
				}
			}
			line = set->lines + replaced;
			cEviction++;
			cMiss++;
			line->tag = tagId;
			set->access ++;
			line->lru = set->access;
			strcat(traceInfo, "eviction  ");
		}
	}
	return;
}

unsigned long getTagBits(int blockbits,int setbits,unsigned long address){
	unsigned long Mask = (~(1L << 63)) << (blockbits + setbits);
	unsigned long tagId = (address & Mask) >> (setbits + blockbits);
	return tagId;
}

unsigned long getSetBits(int blockbits,int setbits,unsigned long address){
	unsigned long Mask = (1L<<(blockbits + setbits)) - (1L<<blockbits);
	unsigned long setId = (address & Mask) >> blockbits;
	return setId;
}

int getInfo(int argc, char **argv, int* setbits, int* lines, int* blockbits, int* vflag, char* trace){
	int opt = 0;
	int count_arg = 0;
	while((opt = getopt(argc, argv, "hv::s:E:b:t:")) != -1){
		switch(opt){
			case 'h':
				help();
				exit(-1);
			case 'v':
				*vflag = 1;
				break;
			case 's':
				count_arg++;
				*setbits = atoi(optarg);
				if (*setbits < 0 || *setbits > max) {
					printf("Set bits should between 0 and word length.\n");
					exit(-1);
				}
				//printf("%d \n", *setbits);
				break;
			case 'E':
				count_arg++;
				*lines = atoi(optarg);
				if(*lines < 1){
					printf("Lines Bits should bigger than 1.\n");
					exit(-1);
				}
				//printf("%d \n", *lines);
				break;
			case 'b':
				count_arg++;
				*blockbits = atoi(optarg);
				if (*blockbits < 0 || *blockbits > max) {
					printf("Block bits should between 0 and word length.\n");
					exit(-1);
				}
				//printf("%d \n", *blockbits);
				break;
			case 't':
				count_arg++;
				strcpy(trace, optarg);
				//printf("%s \n", trace);
				break;
			case '?':
				if (optopt == 'c') {
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				} else if (isprint(optopt)) {
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				} else {
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				}
			default:
				abort();
		}
	}

	if(count_arg < 4){
		printf("./csim: Missing required command line argument\n");   
		help();
		exit(-1);
	}
	return 0;
}

int main(int argc, char **argv){
	int setbits = 0, lines = 0, blockbits = 0, setnum;
	int vflag = 0;
	int size;
	unsigned long address;
	char operation;
	char trace[100];
	FILE *tracefile = NULL;

	getInfo(argc, argv, &setbits, &lines, &blockbits, &vflag, trace);
	//printf("%d %d %d %s\n", setbits, lines, blockbits, trace);
	
	tracefile = fopen(trace, "r");
	if(!tracefile)  {
		printf("Error: Cann't open file %s!\n", trace);
		return -1;
        	}

	setnum = pow(2, setbits);
	memAlloc(setnum, lines);

	//printf("%d \n", setbits);
	while(1){
		fscanf(tracefile, " %c %lx, %d", &operation, &address, &size);
		if(operation == 'I')	continue;
		if(feof(tracefile))	break;

		setId = getSetBits(blockbits, setbits, address);
		tagId = getTagBits(blockbits, setbits, address);

		traceInfo[0] = '\0';
		cache(setId, tagId, lines);

		if(operation == 'M') {
			cHit ++;
			strcat(traceInfo, "hit  ");			
		}

		if (vflag == 1)
			printf("%c %lx,%d %s\n", operation, address, size, traceInfo);
	}
	fclose(tracefile);
	printSummary(cHit, cMiss, cEviction);
	free(sets);
	return 0;
}
