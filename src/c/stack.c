#include <stack.h>
#include <stdlib.h>

#define ceil_div(x, y) ((x + y - 1) / y)
#define ceil(x, ceil) (ceil_div(x, ceil) * ceil)

void init_stack(stack_t stack) {
    stack->bcapacity = STACK_CAPACITY_STEP;
    stack->bsize = 0;
    stack->bdata = malloc(stack->bcapacity);
}

static inline void realloc_on_full(stack_t stack, int push_s) {
    if (ceil_div(stack->bsize, push_s) * push_s < stack->bcapacity) {
        stack->bcapacity += STACK_CAPACITY_STEP;
        stack->bdata = realloc(stack->bdata, stack->bcapacity);
    }
}

#define implement_push(type) {\
    realloc_on_full(stack, sizeof(type));\
    ((type*) stack->bdata)[ceil_div(stack->bsize, sizeof(type))] = val;\
    stack->bsize += sizeof(type);\
}

void push_chr(stack_t stack, char val) implement_push(char)
void push_sht(stack_t stack, short val) implement_push(short)
void push_int(stack_t stack, int val) implement_push(int)
void push_lng(stack_t stack, long val) implement_push(long)
void push_llg(stack_t stack, long long val) implement_push(long long)
void push_ptr(stack_t stack, void* val) implement_push(void*)

void push_str(stack_t stack, char* str) {
    for (size_t i = 0; str[i]; i++) {
        push_chr(stack, str[i]);
    }
    push_chr(stack, 0);
}

#define implement_peek(type) {\
    if (stack->bsize < sizeof(type)) return (type) 0;\
    return ((type*) stack->bdata)[ceil_div(stack->bsize, sizeof(type)) - 1];\
}

char peek_chr(stack_t stack) implement_peek(char)
short peek_sht(stack_t stack) implement_peek(short)
int peek_int(stack_t stack) implement_peek(int)
long peek_lng(stack_t stack) implement_peek(long)
long long peek_llg(stack_t stack) implement_peek(long long)
void* peek_ptr(stack_t stack) implement_peek(void*)

#define implement_pop(type) {\
    if (stack->bsize < sizeof(type)) return (type) 0;\
    stack->bsize = ceil(stack->bsize, sizeof(type));\
    stack->bsize -= sizeof(type);\
    return ((type*) stack->bdata)[ceil_div(stack->bsize, sizeof(type))];\
}

char pop_chr(stack_t stack) implement_pop(char)
short pop_sht(stack_t stack) implement_pop(short)
int pop_int(stack_t stack) implement_pop(int)
long pop_lng(stack_t stack) implement_pop(long)
long long pop_llg(stack_t stack) implement_pop(long long)
void* pop_ptr(stack_t stack) implement_pop(void*)

void destroy_stack(stack_t stack) {
    stack->bsize = 0;
    stack->bcapacity = 0;
    if (stack->bdata) free(stack->bdata);
    stack->bdata = NULL;
}

void* stack_disown(stack_t stack) {
    push_chr(stack, 0);
    void* to_ret = stack->bdata;
    to_ret = realloc(to_ret, stack->bsize);
    stack->bsize = 0;
    stack->bcapacity = 0;
    stack->bdata = NULL;
    return to_ret;
}