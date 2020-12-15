#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct list_node list_node;

typedef enum list_element_type {
	PRIMITIVE,
	POINTER
} list_element_type;	

typedef union list_element {
	int integer;
	char character;
	float floating_value;
	double double_value;
	long int long_integer;
	void * pointer; 
} list_element;

typedef void  (*element_destructor)(void *pointer);
typedef void* (*element_copy)(void *data);

typedef struct list {
	list_node* __head;
	list_node* __tail;
	list_element_type __element_type;
	size_t __number_of_elements;
	element_destructor __element_destructor;
	element_copy       __element_copy;
} list;

typedef struct list_iterator {
	list_node* __node;
	list *__list;
} list_iterator;

list* list_constructor(list_element_type type, element_destructor destruct_function, element_copy copy_function);
void  list_destructor(void* list); 

size_t list_size(list *list); 
list_iterator list_begin(list *list); 
list_iterator list_end(list *list);  

/* List access and modifier functions */
void list_push_front(list *list, list_element data);
void list_push_back(list *list, list_element data);
void list_pop_front(list *list);
void list_pop_back(list *list);

list_iterator list_insert(list_iterator position, list_element data);
list_iterator list_insert_elements(list_iterator position, list_iterator first, list_iterator last);
list_iterator list_erase(list_iterator position);
list_iterator list_erase_elements(list_iterator first, list_iterator last);

/* List iterator functions */
list_element list_iterator_get_data(list_iterator iterator); 
list_element list_iterator_clone_data(list_iterator iterator);

void list_iterator_previous(list_iterator *iterator);  
void list_iterator_next(list_iterator *iterator);  
void list_iterator_advance(list_iterator *iterator, int n); 
int  list_iterator_distance(list_iterator first, list_iterator last); 
bool list_iterator_is_same(const list_iterator first, const list_iterator second); 

#endif
