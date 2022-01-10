#include <stdlib.h>
#include "stack.h"

Stack* stack_new()
{
    Stack *r = malloc(sizeof(Stack));
    r->size = 0;
    r->top = NULL;
    r->next = NULL;
    return r;
}

void stack_insert(Stack **stack, void* val)
{
    Stack *r = stack_new();
    r->size = stack_is_empty(*stack) ? 1 : (*stack)->size+1;
    r->top = val;
    r->next = *stack;
    *stack = r;
}

void* stack_pop(Stack **stack)
{
    if(stack_is_empty(*stack)) return NULL;
    void *top = (*stack)->top;

    Stack *to_free = *stack;
    *stack = (*stack)->next;
    free(to_free);

    return top;
}

void* stack_top(Stack *stack)
{
    return stack_is_empty(stack) ? NULL : stack->top;
}

int stack_is_empty(Stack *stack)
{
    return stack == NULL || stack->size == 0;
}

void stack_clear(Stack **stack)
{
    while(!stack_is_empty(*stack)) stack_pop(stack);
}