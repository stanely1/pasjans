#ifndef STACK_INCLUDE
#define STACK_INCLUDE

typedef struct StackAux Stack;

struct StackAux {
    int size;
    void *top;
    Stack *next;
};

Stack* stack_new(); // allocate and return empty stack
void stack_insert(Stack **stack, void* val); // insert val on top of stack
void* stack_pop(Stack **stack); // pop top element of stack and return this element (NULL if stack is empty)
void* stack_top(Stack *stack); // returns top element of stack (NULL if empty)
int stack_is_empty(Stack *stack); // check if stack is empty
void stack_clear(Stack **stack); // delete all elements of stack and free their memory

#endif