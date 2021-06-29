#include "common.h"


static void run_dll_tests()
{
    for(test_func *test=&dll_tests[0]; test->name; test++)
    {
        test->func();
    }
}

static void run_memtools_tests()
{
    for(test_func *test=&memtools_tests[0]; test->name; test++)
    {
        test->func();
    }
}

static void run_halloc_tests()
{
    for(test_func *test=&halloc_tests[0]; test->name; test++)
    {
        test->func();
    }
}


int main()
{
    printf("\nrunning tests...\n\n");

    printf("running dll tests...\n");
    run_dll_tests();

    init_memtools_testing();

    printf("\nrunning memtools tests...\n");
    run_memtools_tests();

    printf("\nrunning halloc tests...\n");
    run_halloc_tests();

    printf("\n");
}
