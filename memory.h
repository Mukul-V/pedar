#pragma once

void *
pedar_calloc(size_t num, size_t size);

void *
pedar_realloc(void * ptr, size_t size);

void *
pedar_malloc(size_t size);

void
pedar_free(void * ptr);
