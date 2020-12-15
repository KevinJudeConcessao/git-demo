#include <common.h>
#include <entities.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

void msg_init(struct msg *m, long type, enum animal_id id, enum state_t state,
              unsigned int position) {
  if (m) {
    m->mtype = type;
    m->message =
        (struct message_t){.id = id, .state = state, .position = position};
  }
}

void handle_message(struct comm_stub_t *stub, struct subject_t *subject,
                    struct message_t *message) {
  comm_stub_send(stub, message->id, message->state, message->position);
}

struct comm_stub_t *comm_stub_new(int msgqid, long type) {
  struct comm_stub_t *self = malloc(sizeof(struct comm_stub_t));
  self->msgqid = msgqid;
  self->type = type;
  self->observer = observer_new(
      self, (void (*)(void *, struct subject_t *, void *))(handle_message));
  self->this = subject_new(self);
  return self;
}

void comm_stub_dtor(struct comm_stub_t *comm_stub) {
  observer_dtor(comm_stub->observer);
  free(comm_stub);
}

void comm_stub_send(struct comm_stub_t *comm_stub, enum animal_id id,
                    enum state_t state, unsigned int position) {
  struct msg message;
  msg_init(&message, comm_stub->type, id, state, position);
  msgsnd(comm_stub->msgqid, &message, sizeof(message.message), IPC_NOWAIT);
}

int comm_stub_receive(struct comm_stub_t *comm_stub, struct message_t *message,
                      _Bool wait) {
  struct msg m;
  int ret;

  ret = msgrcv(comm_stub->msgqid, &m, sizeof(struct message_t),
               comm_stub->type, MSG_NOERROR | (wait ? 0 : IPC_NOWAIT));
  if (ret != -1)
    memcpy(message, &m.message, sizeof(struct message_t));
  return ret;
}

int comm_stub_receive_last(struct comm_stub_t *comm_stub, struct message_t *message) {
  struct msg m;
  int ret = 0;
  struct msqid_ds ds;
  int message_n;
  int i = 0;

  msgctl(comm_stub->msgqid, IPC_STAT, &ds);
  message_n = ds.msg_qnum;
  
  while (i < message_n && ret != -1) {
    ret = msgrcv(comm_stub->msgqid, &m, sizeof(struct message_t),
               comm_stub->type, MSG_NOERROR | IPC_NOWAIT);
    if (ret != -1)
      ++i;
  }

  if (i != 0)
    memcpy(message, &m.message, sizeof(struct message_t));
  return -(!i);  
}

int comm_stub_receive_notify(struct comm_stub_t *comm_stub,
                             struct message_t *message, _Bool wait) {
  struct msg m;
  struct message_t *new_msg = NULL;
  int ret;

  ret = msgrcv(comm_stub->msgqid, &m, sizeof(struct message_t),
               comm_stub->type, MSG_NOERROR | (wait ? 0 : IPC_NOWAIT));
  if (ret != -1) {
    memcpy(message, &m.message, sizeof(struct message_t));
    new_msg = new_message(message->id, message->state, message->position);
    subject_notify(comm_stub->this, new_msg);
    consume_message(new_msg);
  }
  return ret;
}

int comm_stub_receive_notify_last(struct comm_stub_t *comm_stub,
                                  struct message_t *message) {
  struct msg m;
  struct message_t *new_msg = NULL;
  int ret = 0;
  struct msqid_ds ds;
  int message_n;
  int i = 0;

  msgctl(comm_stub->msgqid, IPC_STAT, &ds);
  message_n = ds.msg_qnum;

  while (i < message_n && ret != -1) {
    ret = msgrcv(comm_stub->msgqid, &m, sizeof(struct message_t),
                 comm_stub->type, MSG_NOERROR | IPC_NOWAIT);
    if (ret != -1)
      ++i;
  }

  if (i != 0) {
    memcpy(message, &m.message, sizeof(struct message_t));
    new_msg = new_message(message->id, message->state, message->position);
    subject_notify(comm_stub->this, new_msg);
    consume_message(new_msg);
  }
  return -(!i);
}

void comm_stub_add_to_listener(struct comm_stub_t *comm_stub,
                               struct observer_t *observer) {
  subject_register_observer(comm_stub->this, observer);
}