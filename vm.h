#pragma once


typedef struct frame {
	value_t count;
	value_t size;
	object_t **value;
} frame_t;


frame_t *
frame_create();

object_t *
frame_push(frame_t *fr, object_t *value);

object_t *
frame_pop(frame_t *fr);

value_t
frame_count(frame_t *fr);

value_t
frame_size(frame_t *fr);
