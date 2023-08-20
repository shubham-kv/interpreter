#if !defined(runtime_stack_h)
    #define runtime_stack_h

    #include <stdint.h>

    struct RuntimeStack {
        const void *type;

        int stack_size;

        void **stack;

        void *frame_pointer;
    };

    extern const void *RuntimeStack;

    #define IS_RUNTIME_STACK(ptr) ((ptr) && (((const struct RuntimeStack *) ptr)->type == RuntimeStack))

    const void *rstack_get_local_frame(const void *runtime_stack);

    const void *rstack_push(void *runtime_stack, int nesting_level);
    const void *rstack_push_frame(void *runtime_stack, void *stack_frame);
    void rstack_pop(void *runtime_stack);

    void rstack_put_value(const void *runtime_stack, const void *symbol, const void *value);
    void rstack_set_value(const void *runtime_stack, const void *symbol, const void *value);
    const void *rstack_get_value(const void *runtime_stack, const void *symbol);

#endif // runtime_stack_h
