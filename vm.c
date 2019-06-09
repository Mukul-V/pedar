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
	fr->value = (object_t **)pedar_calloc (FRAME_MAX, sizeof (object_t *));
	return fr;
}

object_t *
frame_push(frame_t *fr, object_t *value){
	if(fr->count >  FRAME_MAX){
		return 0;
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
	fr->count--;
	object_t *value = fr->value[fr->count];
	fr->value[fr->count] = 0;
	return value;
}

value_t
frame_count(frame_t *fr){
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

islot_t*
slot_next(islot_t *current)
{
    return current->next;
}

islot_t*
slot_previous(islot_t *current)
{
    return current->previous;
}

frame_t *
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
slot_sort(slot_t *slt, value_t (*f)(frame_t *, frame_t *))
{
    islot_t *b, *n;
    for(b = slt->begin; b != slt->end; b = n){
        n = b->next;
        if(n != slt->end){
            if((*f)(b->value, n->value)){
                slot_unlink(slt, b);
                slot_link(slt, n, b);
            }
        }
    }
    return 0;
}

islot_t*
slot_remove(slot_t *slt, value_t (*f)(frame_t *))
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
slot_rpush(slot_t *slt, frame_t *value)
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
slot_lpush(slot_t *slt, frame_t * value)
{
    islot_t *it;

    if(!(it = (islot_t *)pedar_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return slot_link(slt, slt->begin, it);
}

islot_t *
slot_insert(slot_t *slt, islot_t *current, frame_t * value){
    islot_t *it;

    if(!(it = (islot_t *)pedar_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return slot_link(slt, current, it);
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
slot_at(slot_t *slt, value_t key)
{
    islot_t *b, *n;
    for(b = slt->begin; b && (b != slt->end); b = n){
        n = b->next;
        if (key-- <= 0){
            return b;
        }
    }

    if(b == slt->end){
        return 0;
    }

    islot_t *it;
    if(!(it = (islot_t *)pedar_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = 0;

    if (slt->begin == slt->end)
    {
        it->next = slt->end;
        it->previous = slt->end;
        slt->begin = it;
        slt->end->next = it;
        slt->end->previous = it;
    }
    else
    {
        it->next = slt->end;
        it->previous = slt->end->previous;
        slt->end->previous->next = it;
        slt->end->previous = it;
    }

    return it;
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

islot_t *
slot_first_or_default(slot_t *slt, value_t (*f)(frame_t *))
{
    islot_t *b, *n;
    for(b = slt->begin; b && (b != slt->end); b = n){
        n = b->next;
        if ((*f)(b->value)){
            return b;
        }
    }
    return 0;
}

islot_t *
slot_last_or_default(slot_t *slt, value_t (*f)(frame_t *))
{
    islot_t *b, *p;
    for(b = slt->end->previous; b && (b != slt->end); b = p){
        p = b->previous;
        if ((*f)(b->value)){
            return b;
        }
    }
    return 0;
}

frame_t *
slot_aggregate(slot_t *slt, frame_t *(*f)(frame_t *, frame_t *))
{
    if (slot_isempty(slt))
        return 0;

    frame_t * result = 0;

    islot_t *b, *n;
    for(b = slt->begin; b && (b != slt->end); b = n){
        n = b->next;
        result = (*f)(b->value, result);
    }

    return result;
}








stack_t *
stack_create(class_t *cls, function_t *fn){
	stack_t *stk;
	if(!(stk = pedar_malloc (sizeof (stack_t)))){
	   return nullptr;
	}
	stk->class = cls;
	stk->fn = fn;

	stk->slot = slot_create();
	return stk;
}

object_t *
stack_push(stack_t *stk, object_t *value){
	islot_t *b = slot_last (stk->slot);
	frame_t *fr;

stack_repush:
	if(b && (b != stk->slot->end)){
		fr = slot_content(b);
		if((frame_push (fr, value))){
			return value;
		}
	}

	if((fr = frame_create ())){
		b = slot_rpush (stk->slot, fr);
		goto stack_repush;
	}
	return nullptr;
}

object_t *
stack_pop(stack_t *stk){
	islot_t *b;
stack_repop:
	if((b = slot_last (stk->slot)) && (b != stk->slot->end)){
		frame_t *fr = slot_content(b);
		object_t *value;
		if((value = frame_pop (fr))){
			return value;
		}
		b = slot_rpop (stk->slot);
		pedar_free (b);
		goto stack_repop;
	}

	return nullptr;
}

void
stack_destroy(stack_t *stk, value_t (*f)(object_t *)){
	object_t *value;
	while((value = stack_pop(stk))){
		f(value);
	}
	pedar_free (stk);
}