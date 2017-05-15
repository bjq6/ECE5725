#ifndef parser_h
#define parser_h

typedef int bool;
#define true 1
#define false 0

/*	read_line takes in a line to read and returns:
 * 			> true	= 	line is valid command and the proper values are set
 * 			> false = 	line is NOT valid. Values should not be read or acted on.  
 */
bool read_line(char * line, int * g_code, float * f_val, float * r_val, float * x_val, float * y_val, float * z_val);

#endif