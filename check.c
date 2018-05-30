#include <stdio.h>

/***
** Reimplement this function to check, and print Out-of-Bounds Errors
***/
void OutofBoundsError( unsigned long array_size, unsigned long index, unsigned long line, char* file ) {
  //Line 5:Array Out of Bounds Error(index 3/size 3).
  if(index >= array_size || index < 0)
  	printf("%s:%lu:Array Out of Bounds Error(index %lu/size %lu).\n",file,line,index,array_size);
  return;
}
