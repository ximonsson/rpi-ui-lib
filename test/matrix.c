#include "rpi_ui_utils.h"
#include <stdio.h>
#include <assert.h>


GLfloat eye[4][4] =
{
	{1.f, 0.f, 0.f, 0.f},
	{0.f, 1.f, 0.f, 0.f},
	{0.f, 0.f, 1.f, 0.f},
	{0.f, 0.f, 0.f, 1.f},
};

GLfloat m1[4][4] =
{
	{1.f, 1.f, 1.f, 1.f},
	{1.f, 1.f, 1.f, 1.f},
	{1.f, 1.f, 1.f, 1.f},
	{1.f, 1.f, 1.f, 1.f},
};

GLfloat m2[4][4] =
{
	{2.f, 0.f, 0.f, 0.f},
	{0.f, 3.f, 0.f, 1.f},
	{0.f, 0.f, 2.f, 0.f},
	{1.f, 0.f, 0.f, 4.f},
};


void print_matrix (GLfloat matrix[4][4])
{
	for (int m = 0; m < 4; m ++)
	{
		for (int n = 0; n < 4; n ++)
		{
			printf ("%6.2f", matrix[n][m]);
		}
		printf ("\n");
	}
}

int main (int argc, char** argv)
{
	printf ("Performing matrix test\n");

	printf ("  multiply test\n");
	GLfloat m3[4][4];
	rpi_ui_matrix_mul (eye, m2, m3);
	print_matrix (m3);
	printf ("\n");

	printf ("  push and pop test\n");
	rpi_ui_matrix_stack stack;

	rpi_ui_stack_init (&stack);
	rpi_ui_stack_push (&stack, m1);
	rpi_ui_stack_push (&stack, m2);

	GLfloat x[4][4];
	rpi_ui_stack_pop (&stack, &x);
	print_matrix (x); printf ("\n");
	rpi_ui_stack_pop (&stack, &x);
	print_matrix (x); printf ("\n");

	rpi_ui_stack_destroy (&stack);

	return 0;
}
