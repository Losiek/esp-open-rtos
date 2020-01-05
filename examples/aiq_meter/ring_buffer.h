#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdio.h>

typedef struct {
    void *buffer;
    int length;
    int start;
    int end;
} ring_buffer_t;

ring_buffer_t *ring_buffer_create(int length, size_t size);
void ring_buffer_destroy(ring_buffer_t *buffer);
int ring_buffer_read(ring_buffer_t *buffer, void *target, int amount);
int ring_buffer_write(ring_buffer_t *buffer, void *data, int length);
int ring_buffer_empty(ring_buffer_t *buffer);
int ring_buffer_full(ring_buffer_t *buffer);
int ring_buffer_available_data(ring_buffer_t *buffer);
int ring_buffer_available_space(ring_buffer_t *buffer);
//void *ring_buffer_gets(ring_buffer_t *buffer, int amount);

#define ring_buffer_available_data(B) (\
        (B)->end % (B)->length - (B)->start)

#define ring_buffer_available_space(B) (\
        (B)->length - (B)->end - 1)

#define ring_buffer_full(B) (ring_buffer_available_space(B) == 0)

#define ring_buffer_empty(B) (ring_buffer_available_data(B) == 0)

#define ring_buffer_puts(B, D) ring_buffer_write(\
        (B), bdata((D)), blength((D)))

#define ring_buffer_get_all(B) ring_buffer_gets(\
        (B), ring_buffer_available_data((B)))

#define ring_buffer_starts_at(B) (\
        (B)->buffer + (B)->start)

#define ring_buffer_ends_at(B) (\
        (B)->buffer + (B)->end)

#define ring_buffer_commit_read(B, A) (\
        (B)->start = ((B)->start + (A)) % (B)->length)

#define ring_buffer_commit_write(B, A) (\
        (B)->end = ((B)->end + (A)) % (B)->length)

#define ring_buffer_clear(B) ring_buffer_commit_read((B),\
        ring_buffer_available_data((B)));

#endif
