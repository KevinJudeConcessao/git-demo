#include <entities.h>
#include <string.h>
#include <stdlib.h>

struct message_t *new_message(enum animal_id id, enum state_t state,
                              unsigned int position) {
	struct message_t *self = malloc(sizeof(struct message_t));
	self->id				= id;
	self->state			= state;
	self->position	= position;
	return self;
}

void consume_message(struct message_t *self) {
	memset(self, 1, sizeof(struct message_t));
	free(self);
}