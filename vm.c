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

segment_t *
segment_create(){
	segment_t *fr = pedar_malloc (sizeof (segment_t));
	fr->count = 0;
	fr->value = (object_t **)pedar_malloc (FRAME_MAX * sizeof (object_t *));
	return fr;
}

object_t *
segment_push(segment_t *fr, object_t *value){
	if(fr->count >  FRAME_MAX){
		return 0;
	}
	fr->value[fr->count] = value;
	fr->count++;
	return value;
}

object_t *
segment_pop(segment_t *fr){
	if(fr->count < 1){
		return 0;
	}
	fr->count--;
	object_t *value = fr->value[fr->count];
	fr->value[fr->count] = 0;
	return value;
}

value_t
segment_count(segment_t *fr){
	return fr->count;
}




slot_t *
slot_apply(slot_t *slt)
{
    islot_t *it;

    if(!(it = (islot_t *)pedar_malloc(sizeof(islot_t)))) {
        return 0;
    }

    it->next = it->previous = it;
    slt->end = slt->begin = it;

    return slt;
}

slot_t *
slot_create()
{
    slot_t *slt;

    if(!(slt = (slot_t *)pedar_malloc(sizeof(*slt)))) {
        return 0;
    }

    return slot_apply(slt);
}

value_t
slot_isempty(slot_t *slt)
{
    return (slt->begin == slt->end);
}

segment_t *
slot_content(islot_t *b){
	return b ? b->value : 0;
}

value_t
slot_count(slot_t *slt)
{
    value_t cnt = 0;
    islot_t *b;
    for(b = slt->begin; b && (b != slt->end); b = b->next){
        cnt++;
    }
    return cnt;
}

value_t
slot_clear(slot_t *slt, value_t (*f)(islot_t*))
{
    if (slot_isempty(slt))
        return 0;

    islot_t *b, *n;
    for(b = slt->begin; b && (b != slt->end); b = n){
        n = b->next;
        if(!(*f)(b)){
          return 0;
        }
    }

    return 1;
}

void
slot_destroy(slot_t *slt, value_t (*f)(islot_t*))
{
    slot_clear(slt, *f);
    pedar_free (slt);
}

islot_t*
slot_link(slot_t *slt, islot_t *current, islot_t *it)
{
    it->next = current;
    it->previous = current->previous;
    current->previous->next = it;
    current->previous = it;

    if(slt->begin == current){
        slt->begin = it;
    }

    return it;
}

islot_t*
slot_unlink(slot_t *slt, islot_t* it)
{
    if (it == slt->end){
        return 0;
    }

    if (it == slt->begin){
        slt->begin = it->next;
    }

    it->next->previous = it->previous;
    it->previous->next = it->next;

    return it;
}

islot_t*
slot_remove(slot_t *slt, value_t (*f)(segment_t *))
{
    islot_t *b, *n;
    for(b = slt->begin; b != slt->end; b = n){
        n = b->next;
        if((*f)(b->value)){
            return slot_unlink(slt, b);
        }
    }
    return 0;
}

islot_t*
slot_rpop(slot_t *slt)
{
    return slot_unlink(slt, slt->end->previous);
}

islot_t *
slot_rpush(slot_t *slt, segment_t *value)
{
    islot_t *it;
    if(!(it = (islot_t *)pedar_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return slot_link(slt, slt->end, it);
}

islot_t*
slot_lpop(slot_t *slt)
{
    return slot_unlink(slt, slt->begin);
}

islot_t *
slot_lpush(slot_t *slt, segment_t * value)
{
    islot_t *it;

    if(!(it = (islot_t *)pedar_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return slot_link(slt, slt->begin, it);
}

value_t
slot_null(slot_t *slt)
{
    if(slt == 0){
        return 0;
    }
    return (slt->begin == slt->end);
}

islot_t *
slot_first(slot_t *slt)
{
    if(slt->begin != 0)
        return slt->begin;
    return 0;
}

islot_t *
slot_last(slot_t *slt)
{
    if(slt->end->previous != 0)
        return slt->end->previous;
    return 0;
}





frame_t *
frame_create(){
	frame_t *stk;
	if(!(stk = pedar_malloc (sizeof (frame_t)))){
	   return nullptr;
	}

	stk->slot = slot_create();
	return stk;
}

object_t *
frame_push(frame_t *stk, object_t *value){
	islot_t *b = slot_last (stk->slot);
	segment_t *fr;
frame_repush:
	if(b && (b != stk->slot->end)){
		fr = slot_content(b);
		if((segment_push (fr, value))){
			return value;
		}
	}
	if((fr = segment_create ())){
		b = slot_rpush (stk->slot, fr);
		goto frame_repush;
	}
	return nullptr;
}

object_t *
frame_pop(frame_t *stk){
	islot_t *b;
frame_repop:
	if((b = slot_last (stk->slot)) && (b != stk->slot->end)){
		segment_t *fr = slot_content(b);
		object_t *value;
		if((value = segment_pop (fr))){
			return value;
		}
		b = slot_rpop (stk->slot);
		pedar_free (b);
		goto frame_repop;
	}
	return nullptr;
}

void
frame_destroy(frame_t *stk, value_t (*f)(object_t *)){
	object_t *value;
	while((value = frame_pop(stk))){
		f(value);
	}
	pedar_free (stk);
}
