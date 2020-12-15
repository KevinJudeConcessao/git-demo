#include <list.h>

struct list_node {
  list_element __data;
  struct list_node *__previous_node;
  struct list_node *__next_node;
};

static list_node *create_list_node(list_element data, list_node *previous,
                                   list_node *next) {
  list_node *node = (list_node *)(malloc(sizeof(list_node)));
  node->__data = data;
  node->__previous_node = previous;
  node->__next_node = next;
  return node;
}

list *list_constructor(list_element_type type,
                       element_destructor destruct_function,
                       element_copy copy_function) {
  list *list = malloc(sizeof(struct list));

  list->__head = list->__tail =
      create_list_node((list_element){.pointer = NULL}, NULL, NULL);
  list->__element_type = type;
  list->__element_destructor = destruct_function;
  list->__element_copy = copy_function;
  list->__number_of_elements = 0;

  return list;
}

void list_destructor(void *list_ptr) {
  list *list = list_ptr;

  list_erase_elements(list_begin(list), list_end(list));
  free(list->__tail);
  free(list);
}

inline size_t list_size(list *list) { return list->__number_of_elements; }

void list_push_front(list *list, list_element data) {
  list_node *node = create_list_node(data, NULL, list->__head);

  list->__head->__previous_node = node;
  list->__head = node;
  ++list->__number_of_elements;
}

void list_push_back(list *list, list_element data) {
  list_node *tail = list->__tail;
  list_node *prev = list->__tail->__previous_node;
  list_node *node = create_list_node(data, prev, tail);

  tail->__previous_node = node;
  if (prev != NULL)
    prev->__next_node = node;
  else
    list->__head = node;

  ++list->__number_of_elements;
}

void list_pop_front(list *list) {
  list_node *node = list->__head;
  list_node *next_node = node->__next_node;

  if (node != list->__tail) {
    next_node->__previous_node = NULL;
    list->__head = next_node;

    if (list->__element_type != PRIMITIVE && list->__element_destructor != NULL)
      list->__element_destructor((node->__data).pointer);

    free(node);
    --list->__number_of_elements;
  }
}

void list_pop_back(list *list) {
  list_node *node = list->__tail->__previous_node;

  if (node != NULL) {
    list_node *previous = node->__previous_node;
    list->__tail->__previous_node = previous;

    if (previous != NULL)
      previous->__next_node = list->__tail;
    else
      list->__head = list->__tail;

    if (list->__element_type != PRIMITIVE && list->__element_destructor != NULL)
      list->__element_destructor((node->__data).pointer);

    free(node);
    --list->__number_of_elements;
  }
}

inline list_iterator list_begin(list *list) {
  return (list_iterator){
      .__node = list->__head,
      .__list = list,
  };
}

inline list_iterator list_end(list *list) {
  return (list_iterator){
      .__node = list->__tail,
      .__list = list,
  };
}

list_iterator list_insert(list_iterator position, list_element data) {
  list_node *current = position.__node;
  list_node *previous = current->__previous_node;
  list_node *new_node = create_list_node(data, previous, current);

  current->__previous_node = new_node;
  if (previous != NULL)
    previous->__next_node = new_node;
  else
    (position.__list)->__head = new_node;

  ++(position.__list)->__number_of_elements;

  return (list_iterator){
      .__node = new_node,
      .__list = position.__list,
  };
}

list_iterator list_insert_elements(list_iterator position, list_iterator first,
                                   list_iterator last) {
  if (list_iterator_is_same(first, last))
    return position;

  list *new_list =
      list_constructor((position.__list)->__element_type, NULL, NULL);
  while (!list_iterator_is_same(first, last)) {
    list_push_back(new_list, list_iterator_clone_data(first));
    list_iterator_next(&first);
  }

  list_node *node = position.__node;
  list_node *node_prev = node->__previous_node;
  list_node *new_list_start = new_list->__head;
  list_node *new_list_end = new_list->__tail->__previous_node;

  if (node_prev != NULL)
    node_prev->__next_node = new_list_start;
  else
    (position.__list)->__head = new_list_start;

  new_list_start->__previous_node = node_prev;
  node->__previous_node = new_list_end;
  new_list_end->__next_node = node;
  (position.__list)->__number_of_elements += list_size(new_list);

  free(new_list->__tail);
  free(new_list);

  return (list_iterator){
      .__node = new_list_start,
      .__list = position.__list,
  };
}

list_iterator list_erase(list_iterator position) {
  if (position.__node == (position.__list)->__tail)
    return position;

  list_iterator next = position;
  list_iterator_next(&next);
  return list_erase_elements(position, next);
}

list_iterator list_erase_elements(list_iterator first, list_iterator last) {
  if (list_iterator_is_same(first, last))
    return last;

  list_node *previous = (first.__node)->__previous_node;
  list_node *next = last.__node;

  next->__previous_node = previous;
  if (previous == NULL)
    (first.__list)->__head = next;
  else
    previous->__next_node = next;

  while (!list_iterator_is_same(first, last)) {
    if ((first.__list)->__element_type != PRIMITIVE &&
        (first.__list)->__element_destructor != NULL)
      (first.__list)
          ->__element_destructor(list_iterator_get_data(first).pointer);

    list_node *node = first.__node;
    list_iterator_next(&first);

    free(node);
    --(first.__list)->__number_of_elements;
  }

  return last;
}

list_element list_iterator_get_data(list_iterator iterator) {
  return (iterator.__node)->__data;
}

list_element list_iterator_clone_data(list_iterator iterator) {
  if ((iterator.__list)->__element_type == PRIMITIVE ||
      (iterator.__list)->__element_copy == NULL)
    return list_iterator_get_data(iterator);

  return (list_element){
      .pointer = (iterator.__list)
                     ->__element_copy(list_iterator_get_data(iterator).pointer),
  };
}

void list_iterator_next(list_iterator *iterator) {
  iterator->__node = iterator->__node->__next_node;
}

void list_iterator_previous(list_iterator *iterator) {
  iterator->__node = iterator->__node->__previous_node;
}

void list_iterator_advance(list_iterator *iterator, int n) {
  void (*f_ptr)(list_iterator *) =
      (n >= 0) ? list_iterator_next : list_iterator_previous;
  n = (n >= 0) ? n : -n;

  while (n > 0) {
    f_ptr(iterator);
    --n;
  }
}

int list_iterator_distance(list_iterator begin, list_iterator end) {
  int count = 0;

  for (; !list_iterator_is_same(begin, end); ++count)
    list_iterator_next(&begin);

  return count;
}

bool list_iterator_is_same(const list_iterator first,
                           const list_iterator second) {
  return first.__node == second.__node;
}
