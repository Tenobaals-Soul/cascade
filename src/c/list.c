#include <list.h>
#include <memory.h>
#include <assert.h>

#define LIST_CAPACITY_STEP 4096

void list_init(list_t list, size_t type_size) {
    list->bdata = NULL;
    list->wsize = 0;
    list->wcapacity = 0;
    assert(type_size == 1 || type_size == 2 || type_size == 4 || type_size == 8);
    list->bexp = type_size == 1 ? 0 : type_size == 2 ? 1 : type_size == 4 ? 2 : 3;
}

static inline void realloc_on_full(list_t list) {
    if (list->wsize >= list->wcapacity) {
        list->wcapacity = (list->wcapacity << list->bexp) + LIST_CAPACITY_STEP;
        list->bdata = realloc(list->bdata, list->wcapacity);
        list->wcapacity >>= list->bexp;
    }
}

void ensure_capacity(list_t list, size_t items) {
    if (list->wsize < items) {
        list->bdata = realloc(list->bdata, items << list->bexp);
    }
}

void ensure_byte_capacity(list_t list, size_t bytes) {
    if (list->wsize << list->bexp < bytes) {
        list->bdata = realloc(list->bdata, bytes);
    }
}

#define implement_append { \
    realloc_on_full(list); \
    char* off = ((char*) list->bdata) + (list->wsize << list->bexp);\
    *((typeof(val)*) off) = val;\
    list->wsize++;\
}

void append_chr(list_t list, char val) implement_append
void append_sht(list_t list, short val) implement_append
void append_int(list_t list, int val) implement_append
void append_lng(list_t list, long val) implement_append
void append_llg(list_t list, long long val) implement_append
void append_ptr(list_t list, void* val) implement_append

#define implement_set { \
    realloc_on_full(list); \
    char* off = ((char*) list->bdata) + (i << list->bexp);\
    *((typeof(val)*) off) = val;\
    list->wsize++;\
}

void set_chr(list_t list, size_t i, char val) implement_set
void set_sht(list_t list, size_t i, short val) implement_set
void set_int(list_t list, size_t i, int val) implement_set
void set_lng(list_t list, size_t i, long val) implement_set
void set_llg(list_t list, size_t i, long long val) implement_set
void set_ptr(list_t list, size_t i, void* val) implement_set

#define implement_get(type) { \
    char* off = ((char*) list->bdata) + (i << list->bexp);\
    return *((type*) off);\
}

char get_chr(list_t list, size_t i) implement_get(char)
short get_sht(list_t list, size_t i) implement_get(short)
int get_int(list_t list, size_t i) implement_get(int)
long get_lng(list_t list, size_t i) implement_get(long)
long long get_llg(list_t list, size_t i) implement_get(long long)
void* get_ptr(list_t list, size_t i) implement_get(void*)

#define implement_remove_at(type) { \
    char* off = ((char*) list->bdata) + (i << list->bexp);\
    type out = *((type*) off); \
    memmove( \
        list->bdata + (i << list->bexp), \
        list->bdata + ((i + 1) << list->bexp), \
        (1 << list->bexp) * (list->wsize - 1) * 1 << list->bexp \
    ); \
    list->wsize--; \
    return out; \
}

char remove_chr_at(list_t list, size_t i) implement_remove_at(char)
short remove_sht_at(list_t list, size_t i) implement_remove_at(short)
int remova_int_at(list_t list, size_t i) implement_remove_at(int)
long remova_lng_at(list_t list, size_t i) implement_remove_at(long)
long long remove_llg_at(list_t list, size_t i) implement_remove_at(long long)
void* remove_ptr_at(list_t list, size_t i) implement_remove_at(void*)


void list_to_array(list_t list, char out[list->wsize * (1 << list->bexp)]) {
    for (size_t i = 0; i < list->wsize; i++) {
        char* roff = ((char*) list->bdata) + (i << list->bexp);
        char* woff = out + (i << list->bexp);
        memcpy(woff, roff, 1 << list->bexp);
    }
}

void* list_disown(list_t list) {
    return realloc(list->bdata, list->wsize * (1 << list->bexp));
}

void list_destroy(list_t list) {
    if (list->bdata) free(list->bdata);
}