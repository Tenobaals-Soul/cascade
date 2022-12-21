#ifndef INCLUDE_STACK_H
#define INCLUDE_STACK_H

#define STACK_CAPACITY_STEP 4096

typedef struct stack_t {
    unsigned int bsize;
    unsigned int bcapacity;
    void* bdata;
} stack_t[1];

void init_stack(stack_t stack);

void push_chr(stack_t stack, char val);
void push_sht(stack_t stack, short val);
void push_int(stack_t stack, int val);
void push_lng(stack_t stack, long val);
void push_llg(stack_t stack, long long val);
void push_ptr(stack_t stack, void* ptr);
void push_str(stack_t stack, char* str);

char peek_chr(stack_t stack);
short peek_sht(stack_t stack);
int peek_int(stack_t stack);
long peek_lng(stack_t stack);
long long peek_llg(stack_t stack);
void* peek_ptr(stack_t stack);

char pop_chr(stack_t stack);
short pop_sht(stack_t stack);
int pop_int(stack_t stack);
long pop_lng(stack_t stack);
long long pop_llg(stack_t stack);
void* pop_ptr(stack_t stack);

void destroy_stack(stack_t stack);
void* stack_disown(stack_t stack);

#endif