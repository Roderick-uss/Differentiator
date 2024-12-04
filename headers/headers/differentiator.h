#ifndef   DIFFERENTIATOR_H
#define   DIFFERENTIATOR_H

enum NODE_TYPES_T {
    NUM_T = 1,
    PAR_T = 2,
    OP_T  = 3,
};

enum OPERATORS_VALUES_T {
    ADD_T  =  1,
    SUB_T  =  2,
    MUL_T  =  3,
    DIV_T  =  4,
    EXP_T  =  5,
    POW_T  =  6,
    LN_T   =  7,
    LOG_T  =  8,
    SIN_T  =  9,
    COS_T  = 10,
    TG_T   = 11,
    SH_T   = 12,
    CH_T   = 13,
    TH_T   = 14,
    ASIN_T = 15,
    ACOS_T = 16,
    ATG_T  = 17,
    ASH_T  = 18,
    ACH_T  = 19,
    ATH_T  = 20,
};

struct math_node {
    NODE_TYPES_T type;
    int val;
    math_node* left, * right;
};

struct root_t {
    math_node* start;
};


root_t* ctor_expr (const char* const file_name);
int     dtor_expr (root_t* root);

root_t* get_expr_diff (const root_t* root);
int calc_expr (const root_t* root, int x_val, int* result);

int print_expr (const root_t* root, const char* file_name);

#endif // DIFFERENTIATOR_H
