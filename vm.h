#pragma once


typedef struct frame {
	value_t count;
	object_t **value;
} frame_t;


typedef struct islot {
    struct islot *previous;
    struct islot *next;
    frame_t *value;
} islot_t;

typedef struct slot {
    islot_t *begin;
    islot_t *end;
} slot_t;

typedef struct stack {
	class_t *class;
	function_t *fn;

	slot_t *slot;
} stack_t;


frame_t *
frame_create();

object_t *
frame_push(frame_t *fr, object_t *value);

object_t *
frame_pop(frame_t *fr);

value_t
frame_count(frame_t *fr);



stack_t *
stack_create(class_t *cls, function_t *fn);

object_t *
stack_push(stack_t *stk, object_t *value);

object_t *
stack_pop(stack_t *stk);

void
stack_destroy(stack_t *stk, value_t (*f)(object_t *));



slot_t *
slot_create();

value_t
slot_isempty(slot_t *slt);

islot_t*
slot_next(islot_t *current);

islot_t*
slot_previous(islot_t *current);

frame_t *
slot_content(islot_t *b);

value_t
slot_count(slot_t *slt);

value_t
slot_clear(slot_t *slt, value_t (*f)(islot_t*));

void
slot_destroy(slot_t *slt, value_t (*f)(islot_t*));

islot_t*
slot_link(slot_t *slt, islot_t *current, islot_t *it);

islot_t*
slot_unlink(slot_t *slt, islot_t* it);

islot_t*
slot_sort(slot_t *slt, value_t (*f)(frame_t *, frame_t *));

islot_t*
slot_remove(slot_t *slt, value_t (*f)(frame_t *));

islot_t*
slot_rpop(slot_t *slt);

islot_t *
slot_rpush(slot_t *slt, frame_t *value);

islot_t*
slot_lpop(slot_t *slt);

islot_t *
slot_lpush(slot_t *slt, frame_t *value);

islot_t *
slot_insert(slot_t *slt, islot_t *current, frame_t *value);

value_t
slot_null(slot_t *slt);

islot_t *
slot_at(slot_t *slt, value_t key);

islot_t *
slot_first(slot_t *slt);

islot_t *
slot_last(slot_t *slt);

islot_t *
slot_first_or_default(slot_t *slt, value_t (*f)(frame_t *));

islot_t *
slot_last_or_default(slot_t *slt, value_t (*f)(frame_t *));

frame_t *
slot_aggregate(slot_t *slt, frame_t * (*f)(frame_t *, frame_t *));
