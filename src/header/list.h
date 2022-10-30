#ifndef INCLUDE_LIST_H
#define INCLUDE_LIST_H
#include <stdlib.h>

typedef struct list {
    void* bdata;
    size_t wsize;
    size_t wcapacity;
    size_t bexp;
} list_t[1];

void list_init(list_t list, size_t typesize);

void ensure_capacity(list_t list, size_t items);
void ensure_byte_capacity(list_t list, size_t bytes);

void append_chr(list_t list, char val);
void append_int(list_t list, int val);
void append_sht(list_t list, short val);
void append_lng(list_t list, long val);
void append_llg(list_t list, long long val);
void append_ptr(list_t list, void* val);

void set_chr(list_t list, size_t i, char val);
void set_int(list_t list, size_t i, int val);
void set_sht(list_t list, size_t i, short val);
void set_lng(list_t list, size_t i, long val);
void set_llg(list_t list, size_t i, long long val);
void set_ptr(list_t list, size_t i, void* val);

char get_chr(list_t list, size_t i);
int get_int(list_t list, size_t i);
short get_sht(list_t list, size_t i);
long get_lng(list_t list, size_t i);
long long get_llg(list_t list, size_t i);
void* get_ptr(list_t list, size_t i);

char remove_chr_at(list_t list, size_t i);
short remove_sht_at(list_t list, size_t i);
int remova_int_at(list_t list, size_t i);
long remova_lng_at(list_t list, size_t i);
long long remove_llg_at(list_t list, size_t i);
void* remove_ptr_at(list_t list, size_t i);

void list_to_array(list_t list, char out[list->wsize * (1 << list->bexp)]);
void* list_disown(list_t list);

void list_destroy(list_t list);

#endif