#if !defined(stack_frame_h)
    #define stack_frame_h

    #include <stdbool.h>
    #include <stdint.h>

    /*
    // v0.1
    struct StackFrame {
        const void *type;
        uint32_t nesting_level;
        const void *linked_frame;
        // hashtable of names to runtime values
        void *memory_map;
    };
    */

    // v0.2
    struct StackFrame {
        const void *type;

        // The unique id of the frame in the runtime stack
        int frame_id;

        // The nesting level of the frame.
        int nesting_level;

        // The frame to return back to when done with this call frame.
        const void *linked_frame;

        // The hashtable of names to values.
        void *memory_map;
    };

    extern const void *StackFrame;

    #define IS_STACK_FRAME(ptr) ((ptr) && (((const struct StackFrame *) ptr)->type == StackFrame))

    #define STACK_FRAME_NESTING_LEVEL(stack_frame) (((const struct StackFrame *) stack_frame)->nesting_level)
    #define STACK_FRAME_LINKED_FRAME(stack_frame)  (((const struct StackFrame *) stack_frame)->linked_frame)

    void sframe_set_linked_frame(void *stack_frame, const void *linked_frame);

    void sframe_put_value(void *stack_frame, const char *key, const void *value);
    const void *sframe_get_value(const void *stack_frame, const char *key);
    void sframe_set_value(void *stack_frame, const char *key, const void *value);

    void sframe_copy(void *stack_frame, const void *other_frame);
    void sframe_clear(void *stack_frame);

#endif // stack_frame_h
