#include "rpi_ui_utils.h"
#include <string.h>


#define QUEUE_SIZE 256


typedef rpi_ui_queue* QUEUE;
typedef void*         ITEM;


int rpi_ui_queue_init (QUEUE queue, size_t item_size)
{
	queue->item_size = item_size;
	queue->capacity  = QUEUE_SIZE;
	queue->length    = 0;

	queue->items = malloc (item_size * queue->capacity);
	if (queue->items == NULL)
		return 1;

	queue->first =
	queue->last  = queue->items;
	return 0;
}

void rpi_ui_queue_destroy (QUEUE queue)
{
	free (queue->items);
	queue->item_size =
	queue->capacity  =
	queue->length    = 0;

	queue->first     =
	queue->last      = NULL;
}

void rpi_ui_queue_push (QUEUE queue, ITEM item)
{
	if (queue->length == queue->capacity)
	{
		// allocate larger buffer
		unsigned char* tmp = queue->items;
		size_t         off = queue->first - queue->items;
		size_t         tot = queue->capacity * queue->item_size - off;

		queue->capacity += QUEUE_SIZE;
		queue->items = malloc (queue->capacity * queue->item_size);
		memcpy (queue->items, queue->first, tot);
		memcpy (queue->items + tot, tmp, off);

		queue->first = queue->items;
		queue->last  = queue->items + queue->length * queue->item_size;

		free (tmp);
	}
	// copy value to queue
	queue->length ++;
	memcpy (queue->last, &item, queue->item_size);
	queue->last += queue->item_size;

	// loop last pointer
	if (queue->last == queue->items + queue->capacity * queue->item_size)
		queue->last = queue->items;
}

void rpi_ui_queue_pop (QUEUE  queue, ITEM item)
{
	// nothing in queue
	if (queue->length == 0)
	{
		item = NULL;
		return;
	}
	// copy data
	memcpy (item, queue->first, queue->item_size);
	queue->length --;
	queue->first += queue->item_size;
	// loop first pointer
	if (queue->first == queue->items + queue->capacity * queue->item_size)
		queue->first = queue->items;
}
