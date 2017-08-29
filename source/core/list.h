

#ifndef __CORE_LIST_H
#define __CORE_LIST_H


typedef struct _list_t {

	struct _list_t *prev;
	struct _list_t *next;

} list;

typedef struct _list_node_t {

	struct _list_t  node;
	void           *ptr;

} list_node;


void   _list_append( list *self, list *item);
void   _list_prepend(list *self, list *item);
void   _list_remove( list *self);
size_t _list_count(  list *self);

static inline void       list_init(    list *self)                  { self->prev = self->next = self; }
static inline void       list_append(  list *self, list_node *item) { _list_prepend(self, &item->node); } // Yes this is correct
static inline void       list_prepend( list *self, list_node *item) { _list_append( self, &item->node); }
static inline void       list_insert(  list *self, list_node *item) { _list_prepend(self, item); }
static inline void       list_push(    list *self, list_node *item) { _list_append( self, item); }
static inline void       list_unshift( list *self, list_node *item) { _list_prepend(self, item); }
static inline list_node *list_shift(   list *self)                  { list_node *item=(list_node*)(self->next); _list_remove(self->next); return item; }
static inline list_node *list_pop(     list *self)                  { list_node *item=(list_node*)(self->prev); _list_remove(self->prev); return item; }
static inline bool       list_is_first(list *self, list_node *item) { return self->next == &item->node; }
static inline bool       list_is_last( list *self, list_node *item) { return self->prev == &item->node; }
static inline list_node *list_begin(   list *self)                  { return (list_node*)self->next; }
static inline list_node *list_end(     list *self)                  { return (list_node*)self; }
static inline list_node *list_rbegin(  list *self)                  { return (list_node*)self->prev; }
static inline list_node *list_rend(    list *self)                  { return (list_node*)self; }
static inline void      *list_front(   list *self)                  { return list_begin( self)->ptr; }
static inline void      *list_back(    list *self)                  { return list_rbegin(self)->ptr; }
static inline bool       list_empty(   list *self)                  { return self->prev == self->next; }
static inline size_t     list_size(    list *self)                  { return _list_count(self); }

static inline void list_node_init(        list_node *self, void *ptr)       { list_init(&self->node); self->ptr = ptr; }
static inline void list_node_append(      list_node *self, list_node *item) { _list_append( &self->node, &item->node); }
static inline void list_node_prepend(     list_node *self, list_node *item) { _list_prepend(&self->node, &item->node); }
static inline void list_node_insert(      list_node *self, list_node *item) { _list_append( &self->node, &item->node); }
static inline void list_node_remove(      list_node *self)                  { _list_remove(&self->node); }
static inline bool list_node_is_connected(list_node *self)                  { return self->node.prev != self->node.next; }


#endif

