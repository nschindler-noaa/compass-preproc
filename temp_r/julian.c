/* julian.c converts year/mo/da to a julian date.
The year must be entered as a 4 digit number
*/

#include<stdio.h>
 
static struct year_data
{
    char * month;
    unsigned char num_days;
} year_data[] =
{
    {"January", 31},
    {"February",28},
    {"March", 31},
    {"April", 30},
    {"May", 31},
    {"June", 30},
    {"July", 31},
    {"August", 31},
    {"September", 30},
    {"October", 31},
    {"November", 30},
    {"December", 31},
    {NULL, 0}
};

int
is_leapyr(year)
	int	year;
{
    return ((year % 4) == 0 && (year % 100) != 0) || ((year % 400) == 0);
}

int get_julian(month, day, year)
    int month, day, year;
{
    int count, julian=0;
 
    if ( month > 2 && ( (((year % 4) == 0 ) && ((year % 100) !=0 )) || ((year % 400) == 0 ) ) )
        julian += 1;
 
    for (count=1; (count < month) && year_data[count-1].month; count++)
        julian += year_data[count-1].num_days;
 
    return (julian + day);
}
 

/* Test julian */
/* 
main(){

printf(" Mar 1 1996 = %d \n", get_julian(3,1,1996) );
printf(" Mar 1 1997 = %d \n", get_julian(3,1,1997) );
printf(" Mar 1 2000 = %d \n", get_julian(3,1,2000) );
printf(" Mar 1 1900 = %d \n", get_julian(3,1,1900) );

}
*/
