#include "rpi_ui_utils.h"
#include <string.h>

#define STACK_SIZE 32


typedef rpi_ui_matrix_stack* STACK;


void rpi_ui_matrix_mul (GLfloat m1[4][4], GLfloat m2[4][4], GLfloat m3[4][4])
{
	GLfloat v;
	for (int n = 0; n < 4; n ++)
	{
		for (int m = 0; m < 4; m ++)
		{
			v = 0.f;
			for (int o = 0; o < 4; o ++)
			{
				v += m1[o][n] * m2[m][o];
			}
			m3[n][m] = v;
		}
	}
}

int rpi_ui_stack_init (STACK stack)
{
	stack->size   = STACK_SIZE;
	stack->length = 0;
	stack->stack  = malloc (STACK_SIZE * sizeof (GLfloat*));
	if (!stack->stack)
	{
		// error allocating stack
		return 1;
	}
	return 0;
}

void rpi_ui_stack_push (STACK stack, GLfloat matrix[4][4])
{
	// make sure we have a allocated enough memory
	if (stack->length == stack->size)
	{
		// resize
		GLfloat** tmp = stack->stack;
		stack->size += STACK_SIZE;
		stack->stack = malloc ((stack->size) * sizeof (GLfloat*));
		memset (stack->stack,   0, (stack->size) * sizeof (GLfloat*));
		memcpy (stack->stack, tmp, stack->length * sizeof (GLfloat*));
		free (tmp);
	}
	// push the matrix
	stack->stack[stack->length] = (GLfloat*) matrix;
	stack->length ++;
}

void rpi_ui_stack_pop (STACK stack, GLfloat** matrix)
{
	// NULL if there are no elements in the stack
	if (stack->length == 0)
	{
		matrix = NULL;
		return;
	}
	// return last element
	*matrix = stack->stack[stack->length - 1];
	stack->length --;
}

void rpi_ui_stack_destroy (STACK stack)
{
	free (stack->stack);
	stack->size = 0;
	stack->length = 0;
}
