#pragma once


typedef struct segment {
	value_t count;
	object_t **value;
} segment_t;


typedef struct islot {
    struct islot *previous;
    struct islot *next;
    segment_t *value;
} islot_t;

typedef struct slot {
    islot_t *begin;
    islot_t *end;
} slot_t;

typedef struct frame {
	class_t *class;
	function_t *fn;

	slot_t *slot;
} frame_t;


segment_t *
segment_create();

object_t *
segment_push(segment_t *fr, object_t *value);

object_t *
segment_pop(segment_t *fr);

value_t
segment_count(segment_t *fr);



frame_t *
frame_create(class_t *cls, function_t *fn);

object_t *
frame_push(frame_t *stk, object_t *value);

object_t *
frame_pop(frame_t *stk);

void
frame_destroy(frame_t *stk, value_t (*f)(object_t *));



slot_t *
slot_create();

value_t
slot_isempty(slot_t *slt);

segment_t *
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
slot_sort(slot_t *slt, value_t (*f)(segment_t *, segment_t *));

islot_t*
slot_remove(slot_t *slt, value_t (*f)(segment_t *));

islot_t*
slot_rpop(slot_t *slt);

islot_t *
slot_rpush(slot_t *slt, segment_t *value);

islot_t*
slot_lpop(slot_t *slt);

islot_t *
slot_lpush(slot_t *slt, segment_t *value);

islot_t *
slot_insert(slot_t *slt, islot_t *current, segment_t *value);

value_t
slot_null(slot_t *slt);

islot_t *
slot_at(slot_t *slt, value_t key);

islot_t *
slot_first(slot_t *slt);

islot_t *
slot_last(slot_t *slt);
