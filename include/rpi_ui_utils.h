/** --------------------------------------------------------------------------------------------------------
 *  file: rpi_ui_utils.h
 *  description: some helpers
 *  -------------------------------------------------------------------------------------------------------- */
#ifndef RPI_UI_UTILS_H_
#define RPI_UI_UTILS_H_

#include <GLES2/gl2.h>
#include <stdlib.h>

/**
 *  rpi_ui_matrix_mul Dot multiply matrices m1 and m2, stores the result in m3.
 */
void rpi_ui_matrix_mul (GLfloat m1[4][4],
	                    GLfloat m2[4][4],
                        GLfloat m3[4][4]) ;


/**
 *  rpi_ui_matrix_stack represents a stack of 4x4 matrices.
 *  It's very specialized...
 */
typedef struct rpi_ui_matrix_stack
{
	GLfloat** stack;
	size_t    size;
	size_t    length;
}
rpi_ui_matrix_stack ;

/**
 *  rpi_ui_stack_init initializes the stack referenced.
 *  Use rpi_ui_stack_destroy to deallocate.
 */
int rpi_ui_stack_init (rpi_ui_matrix_stack* /* stack */) ;

/**
 *  rpi_ui_stack_push pushes the matrix to stack.
 */
void rpi_ui_stack_push (rpi_ui_matrix_stack* /* stack */, GLfloat matrix[4][4]) ;

/**
 *  rpi_ui_stack_pop pops the stack for a matrix. Sets the reference of matrix to the result.
 *  Sets to NULL if the stack is empty.
 */
void rpi_ui_stack_pop (rpi_ui_matrix_stack* /* stack */, GLfloat** /* matrix */) ;

/**
 *  rpi_ui_stack_destroy frees the stack referenced.
 */
void rpi_ui_stack_destroy (rpi_ui_matrix_stack* /* stack */) ;


/**
 *  rpi_ui_queue
 */
typedef struct rpi_ui_queue
{
	unsigned char* items;
	unsigned char* first;
	unsigned char* last;
	size_t capacity;
	size_t item_size;
	size_t length;
}
rpi_ui_queue;

/**
 *  rpi_ui_queue_init
 */
int rpi_ui_queue_init (rpi_ui_queue* /* queue */, size_t /* item_size */) ;

/**
 *  rpi_ui_queue_destroy
 */
void rpi_ui_queue_destroy (rpi_ui_queue* /* queue */) ;

/**
 *  rpi_ui_queue_push
 */
void rpi_ui_queue_push (rpi_ui_queue* /* queue */, void* /* item */) ;

/**
 *  rpi_ui_queue_pop
 */
void rpi_ui_queue_pop (rpi_ui_queue* /* queue */, void* /* item */) ;


#endif // RPI_UI_UTILS_H_
