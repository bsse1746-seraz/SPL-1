#include <stdio.h>
int main()

{
    int a, b, c;
    printf("Enter the value of a and b\n");
    scanf("%d %d", &a, &b);
    c = a + b;
    printf("The sum of a and b is %d", c);
    return 0;
}