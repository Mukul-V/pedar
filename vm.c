#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "array.h"
#include "table.h"
#include "parser.h"
#include "memory.h"
#include "vm.h"

#define FRAME_MAX 4096

frame_t *
frame_create(){
	frame_t *fr = pedar_malloc (sizeof (frame_t));
	fr->count = 0;
	fr->size = FRAME_MAX;
	fr->value = (object_t **)pedar_calloc (FRAME_MAX, sizeof (object_t *));
	return fr;
}

object_t *
frame_push(frame_t *fr, object_t *value){
	if(fr->count >  fr->size){
		fr->size += FRAME_MAX;
		if(!(fr = pedar_realloc(fr, fr->size))){
			printf("FRAME, unable to allocated a block of %llu bytes", fr->size);
			exit(-1);
		}
	}
	fr->value[fr->count] = value;
	fr->count++;
	return value;
}

object_t *
frame_pop(frame_t *fr){
	if(fr->count < 1){
		return 0;
	}
	if(fr->count < (fr->size - FRAME_MAX)){
		fr->size -= FRAME_MAX;
		if(!(fr = pedar_realloc(fr, fr->size))){
			printf("FRAME, unable to allocated a block of %llu bytes", fr->size);
			exit(-1);
		}
	}
	fr->count--;
	object_t *value = fr->value[fr->count];
	fr->value[fr->count] = 0;
	return value;
}

value_t
frame_size(frame_t *fr){
	return fr->size;
}

value_t
frame_count(frame_t *fr){
	return fr->count;
}
