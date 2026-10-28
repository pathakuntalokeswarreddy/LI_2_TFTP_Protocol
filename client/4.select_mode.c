#include "../header.h"
#include "1.client.h"

void select_mode(int *MODE, int *MODE_BETWEEN)
{
    printf("1. Normal\n"
            "2. Octet\n"
            "3. NetASCII\n");

    
    printf("Enter Mode number: ");
    while( scanf("%d",MODE) != 1 || !(*MODE>0 && *MODE<4) )
    {
        __fpurge(stdin);
        printf("Invalid entry, Enter again : ");
    }
    printf("Selected Mode: %s\n", ( *MODE==1 ? "Normal" : (*MODE==2 ? "Octet" : "NetASCII") ));

    if(*MODE == 3)
    {
        printf("1. Windows to Linux\n"
               "2. Windows to MacOS\n"
               "3. Linux to Windows\n"
               "4. Linux to MacOS\n"
               "5. MacOS to Windows\n"
               "6. MacOS to Linux\n");
        
               printf("Enter Mode number: ");
        
        while( scanf("%d",MODE_BETWEEN) != 1 || !(*MODE_BETWEEN>0 && *MODE_BETWEEN<7) )
        {
            __fpurge(stdin);
            printf("Invalid entry, Enter again : ");
        }
        printf("Selected option: %d\n",*MODE_BETWEEN);
    }
}