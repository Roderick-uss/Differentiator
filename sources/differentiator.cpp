#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <colors.h>
#include <commoner.h>
#include <differentiator.h>

//todo separate file with nodes(all static functions)
//*__________________________DSL___________________________
#define NUM_(val)         ctor_node(NUM_T, val,  NULL,  NULL)
#define VAR_              ctor_node(VAR_T,   1,  NULL,  NULL)
#define ADD_(left, right) ctor_node(OP_T, ADD_T, left, right)
#define SUB_(left, right) ctor_node(OP_T, SUB_T, left, right)
#define MUL_(left, right) ctor_node(OP_T, MUL_T, left, right)
#define DIV_(left, right) ctor_node(OP_T, DIV_T, left, right)
#define LOG_(left, right) ctor_node(OP_T, LOG_T, left, right)
#define POW_(left, right) ctor_node(OP_T, POW_T, left, right)
#define EXP_(right)       ctor_node(OP_T, EXP_T, NULL, right)
#define  LN_(right)       ctor_node(OP_T,  LN_T, NULL, right)
#define SIN_(right)       ctor_node(OP_T, SIN_T, NULL, right)
#define COS_(right)       ctor_node(OP_T, COS_T, NULL, right)
#define  TG_(right)       ctor_node(OP_T,  TG_T, NULL, right)
#define  SH_(right)       ctor_node(OP_T,  SH_T, NULL, right)
#define  CH_(right)       ctor_node(OP_T,  CH_T, NULL, right)
#define  TH_(right)       ctor_node(OP_T,  TH_T, NULL, right)

#define DIFF(node)  get_node_diff(node)
#define  CPY(node) copy_node(node)
#define  DEL(node) dtor_node(node)

#define L_ node->left
#define R_ node->right
//*________________________________________________________

//*_________________DEFINITIONS____________________________
static math_node* ctor_node(NODE_TYPES_T type, int val, math_node* left, math_node* right);
static int        dtor_node(math_node* node);
//todo verify

static math_node* copy_node(const math_node* node);
static int simplify_node(math_node** place);

static math_node* get_node_diff (const math_node* node);
static int calc_node  (const math_node* node, int x_val, int* total);

static int print_node (const math_node* node, char* src);
static int graph_node (const math_node* node, FILE* src, int* num);

static int scan_node(const char* src, int* shift, math_node** node);
static int scan_op  (const char* src, int* shift, int* val);
static int scan_num (const char* src, int* shift, int* val);
static int scan_var (const char* src, int* shift, int* val);
//*________________________________________________________

static const int MAX_EXPRESSION_SIZE   = 500;
static const int MAX_OPERATOR_STR_SIZE =  20;
static const int MAX_NUMBER_STR_SIZE   =  20;
static const int MAX_VARIABLE_STR_SIZE =  20;

static const int MAX_DOT_FILE_NAME_SIZE    = 100;
static const int MAX_TERMINAL_REQUEST_SIZE = 500;

int        scan_expr(root_t* root, const char* const file_name) {
    assert(file_name);

    char* data = (char*)calloc(MAX_EXPRESSION_SIZE, sizeof(char));
    if (!data) {
        LOG_FATAL("NO EXPRESSION CALL\n");
        return 1;
    }

    FILE* input_std = fopen(file_name, "r");
    fread(data, sizeof(char), MAX_EXPRESSION_SIZE, input_std);
    fclose(input_std);

    int shift = 0;

    if (scan_node(data, &shift, &root->start)) {
        LOG_FATAL("%s\nERROR BROCKEN EXPESION\nNO DATA SCANNED\n", data);
        dtor_node(root->start);
    }

    free(data);

    return 0;
}
static int scan_node(const char* src, int* shift, math_node** place) {
    LOG_BLUE("shift : %d\n", *shift);
    assert(src);
    if (src[*shift] != '(') return 1;
    ++*shift;

    if (src[*shift] == 'N' && src[*shift + 1] == ')') {
        *shift += 2;
        *place = NULL;
        return 0;
    }

    math_node* node = ctor_node(OP_T, 0, NULL, NULL);
    if (node == NULL) {
        LOG_FATAL("CANNOT CREATE NODE\n");

        LOG_YELLOW("%s\n", src);

        char* arrow = (char*)calloc(*shift + 1, sizeof(char));
        memset(arrow, '_', *shift - 1);
        arrow[*shift] = '^';
        LOG_RED("%s (start position)\n", arrow);
        free(arrow);

        return 1;
    }

    *place = node;

    if (src[*shift] == '(') {
        LOG_PURPLE("shift : %d\n", *shift);
        if (scan_node(src, shift, &L_       )) return 1;
        LOG_PURPLE("shift : %d\n", *shift);
        if (scan_op  (src, shift, &node->val)) return 1;
        LOG_PURPLE("shift : %d\n", *shift);
        if (scan_node(src, shift, &R_       )) return 1;
    }
    else if(isdigit(src[*shift]) || (isdigit(src[*shift + 1]) && (src[*shift] == '+' || src[*shift] == '-'))) {
        node->type = NUM_T;
        if (scan_num (src, shift, &node->val)) return 1;
    }
    else if(isalpha(src[*shift])) {
        node->type = VAR_T;
        if (scan_var (src, shift, &node->val)) return 1;
    }
    else return 1;

    if (src[*shift] != ')') return 1;
    ++*shift;

    return 0;
}

static int scan_op  (const char* src, int* shift, int* val) {
    char operator_[MAX_OPERATOR_STR_SIZE] = {};
    int delta = 0;

    sscanf(src + *shift, "%[a-z*/^+-]%n", operator_, &delta);
    *shift += delta;

    if (src[*shift] != '(') {
        LOG_FATAL("OPERATOR NOT FULL\n");
        return 1;
    }

    for(size_t i = 0; i < sizeof(OPERATORS) / sizeof(OPERATORS[0]); ++i) if (!strcmp(OPERATORS[i].name, operator_)) {
        *val = OPERATORS[i].value;
        return  0;
    }
    LOG_FATAL("NO SUCH OPERATOR %s\n", operator_);
    return 1;
}
static int scan_num (const char* src, int* shift, int* val) {
    char number[MAX_NUMBER_STR_SIZE] = {};
    int delta = 0;

    sscanf(src + *shift, "%[0-9.]%n", number, &delta);
    *shift += delta;

    if (strrchr(number, '.') != strchr(number, '.')) {
        LOG_FATAL("TWO DOTS IN NUMBER: %s\n", number);
        return 1;
    }
    if (number[0] == '.' || number[strlen(number) - 1] == '.') {
        LOG_FATAL("DOT IN NUMBER ON WRONG POSITION: %s\n", number);
        return 1;
    }
    if (strlen(number) > 9) {
        LOG_FATAL("TOO LONG NUMBER: %s\n", number);
        return 1;
    }

    *val = atoi(number);
    return 0;
}
static int scan_var (const char* src, int* shift, int* val) {
    char variable[MAX_VARIABLE_STR_SIZE] = {};
    int delta = 0;

    sscanf(src + *shift, "%[a-z]%n", variable, &delta);
    *shift += delta;

    if (strcmp(variable, "x")) {
        LOG_FATAL("ONLY \"x\" CAN BE VARIABLE NOT \"%s\"\n", variable);
        return 1;
    }

    *val = 1;
    return 0;
}


root_t* ctor_expr () {
    root_t* root = (root_t*)calloc(1, sizeof(root_t));
    if (!root) {
        LOG_FATAL("NO ROOT CALL\n");
        return 0;
    }
    return root;
}
int     dtor_expr (root_t* root) {
    assert(root);

    dtor_node(root->start);
    free(root);

    return 0;
}

static math_node* ctor_node(NODE_TYPES_T type, int val, math_node* left, math_node* right) {
    math_node* node = (math_node*)calloc(1, sizeof(math_node));
    if (!node) {
        LOG_FATAL("NO NODE CALL\n");
        return 0;
    }

    node->left  = left;
    node->right = right;
    node->type  = type;
    node->val   = val;
    return node;
}
static int        dtor_node(math_node* node) {
    if (!node) return 0;
    dtor_node(L_);
    dtor_node(R_);
    free(node);
    return 0;
}


int        print_expr (const root_t* root, const char* file_name) {
    assert(root);
    assert(file_name);

    char* data = (char*)calloc(MAX_EXPRESSION_SIZE, sizeof(char));
    if (!data) {
        LOG_FATAL("NO EXPRESSION CALL\n");
        return 1;
    }

    int size = print_node(root->start, data);
    data[size] = '\n';
    if (data[MAX_EXPRESSION_SIZE - 1]) LOG_FATAL("exceeded expression size\n");
    else {
        FILE* output_std = fopen(file_name, "a");
        fwrite(data, sizeof(char), strlen(data), output_std);
        fclose(output_std);
    }

    free(data);

    return 0;
}
static int print_node (const math_node* node, char* dst) {
    assert(dst);
    int delta = 0, global_delta = 0;
    if (!node) {
        sprintf(dst, "(N)%n", &delta);
        return delta;
    }
    if (node->type == NUM_T) {
        sprintf(dst, "(%d)%n", node->val, &delta);
        return delta;
    }
    if (node->type == VAR_T) {
        sprintf(dst, "(x)%n", &delta);
        return delta;
    }

    sprintf(dst, "(%n", &delta);
    dst += delta;
    global_delta += delta;

    delta = print_node(L_, dst);
    dst += delta;
    global_delta += delta;

    for (size_t i = 0; i < sizeof(OPERATORS) / sizeof(OPERATORS[0]); ++i) {
        if ((OPERATORS_VALUES_T)node->val == OPERATORS[i].value) {
            sprintf(dst, "%s%n", OPERATORS[i].name, &delta);
            break;
        }
        else if (i == sizeof(OPERATORS) / sizeof(OPERATORS[0]) - 1) {
            LOG_FATAL("UNKNOWN OPERATION: %d\n", node->val);
            return -1;
        }
    }
    dst += delta;
    global_delta += delta;

    delta = print_node(R_, dst);
    dst += delta;
    global_delta += delta;

    sprintf(dst, ")%n", &delta);
    global_delta += delta;

    return global_delta;
}


root_t*           get_expr_diff(const root_t* root) {
    assert(root);
    root_t* d_root = ctor_expr();

    if (!d_root) return 0;

    d_root->start = DIFF(root->start);

    simplify_node(&d_root->start);

    return d_root;
}
static math_node* get_node_diff(const math_node* node) {
    if (!node) return 0;

    if (node->type == NUM_T) return NUM_(0);
    if (node->type == VAR_T) return NUM_(1);
    if (node->type != OP_T) {
        LOG_FATAL("STRANGE NODE\n");
        return 0;
    }

    switch ((OPERATORS_VALUES_T)node->val)
    {
    case  ADD_T: return ADD_(DIFF(L_), DIFF(R_));
    case  SUB_T: return SUB_(DIFF(L_), DIFF(R_));
    case  MUL_T: return ADD_(MUL_(DIFF(L_), CPY(R_)), MUL_(CPY(L_), DIFF(R_)));
    case  DIV_T: return DIV_(SUB_(MUL_(DIFF(L_), CPY(R_)), MUL_(CPY(L_), DIFF(R_))), POW_(CPY(R_), NUM_(2)));
    case   LN_T: return DIV_(DIFF(R_), CPY(R_));
    case  EXP_T: return MUL_(DIFF(R_), CPY(node));
    case  LOG_T: return DIV_(SUB_(DIV_(DIFF(L_), CPY(L_)), MUL_(DIV_(DIFF(R_), CPY(R_)), CPY(node))), LN_(CPY(R_)));
    case  POW_T: return ADD_(MUL_(CPY(node), MUL_(LN_(CPY(L_)), DIFF(R_))), MUL_(POW_(CPY(L_), SUB_(CPY(R_), NUM_(1))), MUL_(CPY(R_), DIFF(L_))));
    case  SIN_T: return MUL_(COS_(CPY(R_)), DIFF(R_));
    case  COS_T: return MUL_(SUB_(NUM_(0), SIN_(CPY(R_))), DIFF(R_));
    case   SH_T: return MUL_( CH_(CPY(R_)), DIFF(R_));
    case   CH_T: return MUL_( SH_(CPY(R_)), DIFF(R_));
    case   TG_T: return MUL_(DIV_(NUM_(1), POW_(COS_(CPY(R_)), NUM_(2))), DIFF(R_));
    case   TH_T: return MUL_(DIV_(NUM_(1), POW_( CH_(CPY(R_)), NUM_(2))), DIFF(R_));
    // case ASIN_T: return MUL_(DIV_(NUM_( 1), POW_(SUB_(NUM_(1), POW_(CPY(R_), NUM_(2))), NUM_(0.5))), DIFF(R_));
    // case ACOS_T: return MUL_(DIV_(NUM_(-1), POW_(SUB_(NUM_(1), POW_(CPY(R_), NUM_(2))), NUM_(0.5))), DIFF(R_));
    // case ATAN_T: return MUL_(DIV_(NUM_( 1), POW_(ADD_(NUM_(1), POW_(CPY(R_), NUM_(2))), NUM_(0.5))), DIFF(R_));
    // case  ACH_T: return MUL_(DIV_(NUM_( 1), POW_(SUB_(POW_(CPY(R_), NUM_(2)), NUM_(1)), NUM_(0.5))), DIFF(R_));
    // case  ASH_T: return MUL_(DIV_(NUM_( 1), POW_(ADD_(POW_(CPY(R_), NUM_(2)), NUM_(1)), NUM_(0.5))), DIFF(R_));
    case  ATH_T: return MUL_(DIV_(NUM_( 1), SUB_(NUM_(1), POW_(CPY(R_), NUM_(2)))), DIFF(R_));
    default:
        LOG_FATAL("UNKNOWN OPERATION: %d\n", node->val);
        break;
    }
    return 0;
}


int calc_expr(const root_t* root, int x_val, int* total) {
    assert(root);
    assert(total);

    // calc_node(root->start, x_val, total);
    return 0;
}
// static int calc_node  (const math_node* node, int x_val, int* total);

static math_node* copy_node(const math_node* node) {
    if (!node) return 0;
    return ctor_node(node->type, node->val, CPY(L_), CPY(R_));
}

static int simplify_node(math_node** place) {
    assert(place);

    math_node* node = *place;

    if (!node || node->type != OP_T) return 0;

    if (simplify_node(&L_)) return 1;
    if (simplify_node(&R_)) return 1;

    // {
    // char* data = (char*)calloc(MAX_EXPRESSION_SIZE, 1);
    // print_node(node, data);
    // printf("%s\n", data);
    // free (data);
    // }

    int lval = L_ ? L_->val : 0;
    int rval = R_ ? R_->val : 0;

    switch ((OPERATORS_VALUES_T)node->val)
    {
    case  ADD_T:
    {
        if      (L_->type == NUM_T && R_->type == NUM_T) {*place = NUM_(lval + rval); dtor_node(node);}
        else if (L_->type == NUM_T && lval == 0)         {*place = CPY(R_)          ; dtor_node(node);}
        else if (R_->type == NUM_T && rval == 0)         {*place = CPY(L_)          ; dtor_node(node);}
        break;
    }
    case  SUB_T:
    {
        if      (L_->type == NUM_T && R_->type == NUM_T) {*place = NUM_(lval - rval); dtor_node(node);}
        else if (R_->type == NUM_T && rval == 0)         {*place = CPY(L_)          ; dtor_node(node);}
        break;
    }
    case  MUL_T:
    {
        if      (L_->type == NUM_T && R_->type == NUM_T) {*place = NUM_(lval * rval); dtor_node(node);}
        else if (R_->type == NUM_T) {
            if      (rval ==  1) {*place = CPY(L_)               ; dtor_node(node);}
            else if (rval == -1) {*place = SUB_(NUM_(0), CPY(L_)); dtor_node(node);}
            else if (rval ==  0) {*place = NUM_(0)               ; dtor_node(node);}
        }
        else if (L_->type == NUM_T) {
            if      (lval ==  1) {*place = CPY(R_)               ; dtor_node(node);}
            else if (lval == -1) {*place = SUB_(NUM_(0), CPY(R_)); dtor_node(node);}
            else if (lval ==  0) {*place = NUM_(0)               ; dtor_node(node);}
        }
        break;
    }
    case  DIV_T:
    {
        if      (R_->type == NUM_T && rval == 0) {LOG_FATAL("ZERO DIVISION\n"); return 1     ;}

        if (L_->type == NUM_T && lval == 0)                                 {*place = NUM_(0)          ; dtor_node(node);}
        else if (R_->type == NUM_T && rval == 1)                            {*place = CPY(L_)          ; dtor_node(node);}
        else if (L_->type == NUM_T && R_->type == NUM_T && lval % rval == 0) {*place = NUM_(lval / rval); dtor_node(node);}
        break;
    }
    case   LN_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {LOG_FATAL("ZERO LN\n"); return 1       ;}
        if (rval == 1) {*place = NUM_(0)      ; dtor_node(node);}
        //else           {*place = NUM_(log(rval))      ; dtor_node(node);}
        break;
    }
    case  EXP_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(1); dtor_node(node);}
        //else           {*place = NUM_(pow(e, rval)); dtor_node(node);}
        break;
    }
    case  LOG_T:
    {
        if (R_->type == NUM_T && (rval == 0 || rval == 1)) {LOG_FATAL("LOG BASE ERROR\n"); return 1;}
        if (L_->type != NUM_T) break;
        if      (lval == 0)        {LOG_FATAL("ZERO LOG\n")             ; return 1       ;}
        else if (lval == 1)        {*place = NUM_(0)                    ; dtor_node(node);}
        //else if (R_->type == NUM_T) {*place = NUM_(log(lval) / log(rval)); dtor_node(node);}
        break;
    }
    case  POW_T:
    {
        if (L_->type == NUM_T && R_->type == NUM_T && lval == 0 && rval == 0) {LOG_FATAL("uncertainty 0 0\n"); return 1;}
        if (L_->type == NUM_T && (lval == 0 || lval == 1)) {*place = NUM_(lval); dtor_node(node);}
        else if (R_->type == NUM_T && rval == 0)           {*place = NUM_(1)   ; dtor_node(node);}
        else if (R_->type == NUM_T && rval == 1)           {*place = CPY(L_)   ; dtor_node(node);}
        //else if (L_->type == NUM_T && R_->type == NUM_T)    {*place = NUM_(pow(lval, rval)); dtor_node(node);}
        break;
    }
    case  SIN_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = sin(rval); dtor_node(node);}
        break;
    }
    case  COS_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(1); dtor_node(node);}
        //else           {*place = NUM_(cos(rval)); dtor_node(node);}
        break;
    }
    case   SH_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(sinh(rval)); dtor_node(node);}
        break;
    }
    case   CH_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(1); dtor_node(node);}
        //else           {*place = NUM_(cosh(rval)); dtor_node(node);}
        break;
    }
    case   TG_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(tan(rval)); dtor_node(node);}
        break;
    }
    case   TH_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(tanh(rval)); dtor_node(node);}
        break;
    }
    case ASIN_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(asin(rval)); dtor_node(node);}
        break;
    }
    case ACOS_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 1) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(acos(rval)); dtor_node(node);}
        break;
    }
    case ATAN_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(atan(rval)); dtor_node(node);}
        break;
    }
    case  ASH_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(asinh(rval)); dtor_node(node);}
        break;
    }
    case  ACH_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(1); dtor_node(node);}
        //else           {*place = NUM_(acosh(rval)); dtor_node(node);}
        break;
    }
    case  ATH_T:
    {
        if (R_->type != NUM_T) break;
        if (rval == 0) {*place = NUM_(0); dtor_node(node);}
        //else           {*place = NUM_(atanh(rval)); dtor_node(node);}
        break;
    }
    default:
        LOG_FATAL("UNKNOWN OPERATION: %d\n", node->val);
        return 0;
    }
    return 0;
}

char* graph_expr(const root_t* root) {
    assert(root);

    timeval current_time;
    gettimeofday(&current_time, NULL);

    char* dot_file = (char*)calloc(MAX_DOT_FILE_NAME_SIZE, 1);
    char* svg_file = (char*)calloc(MAX_DOT_FILE_NAME_SIZE, 1);
    sprintf(dot_file, "graphs/%ld_%ld.dot", current_time.tv_sec, current_time.tv_usec);
    sprintf(svg_file, "graphs/%ld_%ld.svg", current_time.tv_sec, current_time.tv_usec);

    FILE* std_dot = fopen(dot_file, "w");

    const char begin_dot[] = "\
graph expr\n\
{\n\
    bgcolor=\"0\"\n\
    node[fontname=\"impact\"; penwidth=5; fontsize=\"25\"]\n\
    edge[fontname=\"Helvetica,Arial,sans-serif\"; color=\"#FF5E00\"; penwidth=4]\n\
    node[fontcolor=\"#A2FF05\"; color=\"#F52789\"; shape=Mrecord]\n\
    ";

    fprintf(std_dot, "%s", begin_dot);
    int num = 0;
    if (graph_node(root->start, std_dot, &num)) {
        LOG_FATAL("error during graph construction attempt");
        free(dot_file);
        free(svg_file);
        return 0;
    }
    fprintf(std_dot, "}\n");

    fclose(std_dot);

    char term_request[MAX_TERMINAL_REQUEST_SIZE] = {};
    sprintf(term_request, "dot %s -Tsvg -o %s", dot_file, svg_file);
    system(term_request);

    free(dot_file);

    return svg_file;
}
static int graph_node (const math_node* node, FILE* dot_std, int* num) {
    if (!node) return 0;
    (*num)++;
    int current_node_num = *num;
    if      (node->type == NUM_T) fprintf(dot_std, "\tn%d [label=\"%d\"; fontcolor=\"#00fefc\"; color=\"#DD0EFF\"; shape=box]\n",    current_node_num, node->val);
    else if (node->type == VAR_T) fprintf(dot_std, "\tn%d [label=\"X\" ; fontcolor=\"#FF3131\"; color=\"#7D12FF\"; shape=circle]\n", current_node_num);
    else {
        for(size_t i = 0; i < sizeof(OPERATORS) / sizeof(OPERATORS[0]); ++i) {
            if (OPERATORS[i].value == node->val) {
                fprintf(dot_std, "\tn%d [label=\"%s\"]\n", current_node_num, OPERATORS[i]);
                break;
            }
            else if (i == sizeof(OPERATORS) / sizeof(OPERATORS[0]) - 1) {
                LOG_FATAL("UNKNOWN OPERATION: %d\n", node->val);
                return 1;
            }
        }

        if (L_) fprintf(dot_std, "\tn%d -- n%d\n", current_node_num, *num + 1);
        graph_node(L_, dot_std, num);
        if (R_) fprintf(dot_std, "\tn%d -- n%d\n", current_node_num, *num + 1);
        graph_node(R_, dot_std, num);
    }
    return 0;
}
