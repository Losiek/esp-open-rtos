#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbg.h"
#include "ring_buffer.h"

ring_buffer_t *ring_buffer_create(int length, size_t size)
{
    ring_buffer_t *buffer = calloc(1, sizeof(ring_buffer_t));
    buffer->length = length + 1;
    buffer->start = 0;
    buffer->end = 0;
    buffer->buffer = calloc(buffer->length, size);

    return buffer;
}

void ring_buffer_destroy(ring_buffer_t *buffer)
{
    if(buffer) {
        free(buffer->buffer);
        free(buffer);
    }
}

int ring_buffer_write(ring_buffer_t *buffer, void *data, int length)
{
    if(ring_buffer_available_data(buffer) == 0) {
        buffer->start = buffer->end = 0;
    }

    check(length <= ring_buffer_available_space(buffer), \
            "Not enough space: %d request, %d available", \
            ring_buffer_available_data(buffer), length);

    void *result = memcpy(ring_buffer_ends_at(buffer), data, length);
    check(result != NULL, "Failed to write data into buffer.");

    ring_buffer_commit_write(buffer, length);

    return length;
error:
    return -1;
}

int ring_buffer_read(ring_buffer_t *buffer, void *target, int amount)
{
    check_debug(amount <= ring_buffer_available_data(buffer), \
            "Not enough in the buffer: has %d, needs %d", \
            ring_buffer_available_data(buffer), amount);

    void *result = memcpy(target, ring_buffer_starts_at(buffer), amount);
    check(result != NULL, "Failed to write buffer into data.");

    ring_buffer_commit_read(buffer, amount);

    if(buffer->end == buffer->start) {
        buffer->start = buffer->end = 0;
    }

    return amount;
error:
    return -1;
}
