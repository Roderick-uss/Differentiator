#include<stdio.h>
#include<stdlib.h>

#include "commoner.h"
#include "differentiator.h"

const char * const  input_file = "files/in.txt";
const char * const output_file = "files/out.txt";

int main() {
    fclose(fopen (output_file, "w"));

    LOG_INFO("START\n");

    root_t* d0_expr = ctor_expr();
    LOG_INFO("CTOR\n");

    scan_expr(d0_expr, input_file);
    LOG_INFO("SCAN\n");

    LOG_INFO("D0 " YELLOW "%p\n", d0_expr);
    print_expr(d0_expr, output_file);

    root_t* d1_expr = get_expr_diff(d0_expr);
    LOG_INFO("D1 " YELLOW "%p\n", d1_expr);
    print_expr(d1_expr, output_file);

    root_t* d2_expr = get_expr_diff(d1_expr);
    LOG_INFO("D2 " YELLOW "%p\n", d2_expr);
    print_expr(d2_expr, output_file);

    free(graph_expr(d0_expr));
    free(graph_expr(d1_expr));
    free(graph_expr(d2_expr));

    dtor_expr(d0_expr);
    dtor_expr(d1_expr);
    dtor_expr(d2_expr);

    LOG_INFO("DTOR\n");
}
