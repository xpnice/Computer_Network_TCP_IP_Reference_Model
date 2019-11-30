
#include "common.h"
#include <stdio.h>
int file_id = 0;

int main()
{
    current_protocol = PROTOCOL1;
    to_final_file(&file_id);
    return 0;
}