#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <colors.h>
#include <commoner.h>
#include <differentiator.h>

//*__________________________DSL___________________________
#define NUM_(val)         ctor_node(NUM_T, val,  NULL,  NULL)
#define X_                ctor_node(ARG_T,   1,  NULL,  NULL)
#define ADD_(left, right) ctor_node(OP_T, ADD_T, left, right)
#define SUB_(left, right) ctor_node(OP_T, SUB_T, left, right)
#define MUL_(left, right) ctor_node(OP_T, MUL_T, left, right)
#define DIV_(left, right) ctor_node(OP_T, DIV_T, left, right)
#define LOG_(left, right) ctor_node(OP_T, LOG_T, left, right)
#define POW_(left, right) ctor_node(OP_T, POW_T, left, right)
#define EXP_(right)       ctor_node(OP_T, EXP_T, NULL, right)
#define  LN_(right)       ctor_node(OP_T, ADD_T, NULL, right)
#define SIN_(right)       ctor_node(OP_T, SIN_T, NULL, right)
#define COS_(right)       ctor_node(OP_T, COS_T, NULL, right)
#define  TG_(right)       ctor_node(OP_T,  TG_T, NULL, right)
#define  SH_(right)       ctor_node(OP_T,  SH_T, NULL, right)
#define  CH_(right)       ctor_node(OP_T,  CH_T, NULL, right)
#define  TH_(right)       ctor_node(OP_T,  TH_T, NULL, right)

#define DIFF(node)  get_diff(node)
#define  CPY(node) copy_node(node)
#define  DEL(node) dtor_node(node)

#define L_ node->left
#define R_ node->right
//*________________________________________________________

static const int MAX_EXPRESSION_SIZE = 500;

static math_node* scan_node(const char* src);
static math_node* ctor_node(NODE_TYPES_T type, int val, math_node* left, math_node* right);
static math_node* copy_node(const math_node* node);
static math_node* get_diff (const math_node* node);
static int print_node (const math_node* node, char* src);
static int dtor_node(math_node* node);
static int calc_node  (const math_node* node, int x_val, int* total);

int scan_expr(root_t* root, const char* const file_name) {
    assert(file_name);

    char* data = (char*)calloc(MAX_EXPRESSION_SIZE, sizeof(char));
    if (!data) {
        LOG_FATAL("NO EXPRESSION CALL\n");
        return 1;
    }

    FILE* input_std = fopen(file_name, "r");
    fread(data, sizeof(char), MAX_EXPRESSION_SIZE, input_std);
    fclose(input_std);

    root->start = scan_node(data);

    free(data);

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

int dtor_expr (root_t* root) {
    assert(root);

    dtor_node(root->start);
    free(root);

    return 0;
}
static int dtor_node(math_node* node) {
    if (!node) return 0;
    dtor_node(L_);
    dtor_node(R_);
    free(node);
    return 0;
}

root_t* get_expr_diff(const root_t* root) {
    assert(root);
    root_t* d_root = (root_t*)calloc(1, sizeof(root_t));
    if (!d_root) {
        LOG_FATAL("NO ROOT CALL\n");
        return 0;
    }
    d_root->start = DIFF(root->start);
    return d_root;
}
static math_node* get_diff (const math_node* node) {
    if (!node) return 0;

    if (node->type == NUM_T) return NUM_(0);
    if (node->type == PAR_T) return NUM_(1);
    if (node->type != OP_T) {
        LOG_FATAL("STRANGE NODE\n");
        return 0;
    }
    switch (node->type)
    {
    case ADD_T: return ADD_(DIFF(L_), DIFF(R_));
    case SUB_T: return SUB_(DIFF(L_), DIFF(R_));
    case MUL_T: return ADD_(MUL_(DIFF(L_), CPY(R_)), MUL_(CPY(L_), DIFF(R_)));
    case DIV_T: return DIV_(SUB_(MUL_(DIFF(L_), CPY(R_)), MUL_(CPY(L_), DIFF(R_))), POW_(CPY(R_), NUM_(2)));
    case  LN_T: return DIV_(DIFF(R_), CPY(R_));
    case EXP_T: return MUL_(DIFF(R_), CPY(node));
    case LOG_T: return DIV_(SUB_(DIV_(DIFF(L_), CPY(L_)), MUL_(DIV_(DIFF(R_), CPY(R_)), CPY(node))), LN_(CPY(R_)));
    case POW_T: return ADD_(MUL_(CPY(node), MUL_(LN_(CPY(L_)), DIFF(R_))), MUL_(POW_(CPY(L_), SUB_(CPY(R_), NUM_(1))), MUL_(CPY(R_), DIFF(L_))));
    case SIN_T: return MUL_(COS_(CPY(R_)), DIFF(R_));
    case COS_T: return MUL_(SUB_(NUM_(0), SIN_(CPY(R_))), DIFF(R_));
    case  SH_T: return MUL_( CH_(CPY(R_)), DIFF(R_));
    case  CH_T: return MUL_( SH_(CPY(R_)), DIFF(R_));
    case  TG_T: return MUL_(DIV_(NUM_(1), POW_(COS_(CPY(R_)), NUM_(2))), DIFF(R_));
    case  TH_T: return MUL_(DIV_(NUM_(1), POW_( CH_(CPY(R_)), NUM_(2))), DIFF(R_));
    default:
        LOG_FATAL("UNKNOWN OPERATION: %d\n", node->val);
        break;
    }
    return 0;
}
int calc_expr(const root_t* root, int x_val, int* total) {
    assert(root);
    assert(total);

    calc_node(root->start, x_val, total);
    return 0;
}

static math_node* scan_node(const char* src);
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
static math_node* copy_node(const math_node* node) {
    if (!node) return 0;
    return ctor_node(node->type, node->val, CPY(L_), CPY(R_));
}
static int calc_node  (const math_node* node, int x_val, int* total);
int print_expr (const root_t* root, const char* file_name) {
    assert(root);
    assert(file_name);

    char* data = (char*)calloc(MAX_EXPRESSION_SIZE, sizeof(char));
    if (!data) {
        LOG_FATAL("NO EXPRESSION CALL\n");
        return 1;
    }

    print_node(root->start, data);

    FILE* output_std = fopen(file_name, "w");
    fwrite(data, sizeof(char), MAX_EXPRESSION_SIZE, output_std);
    fclose(output_std);

    free(data);

    return 0;
}
static int print_node (const math_node* node, char* dst) {
    assert(dst);
    int delta = 0, global_delta;
    if (!node) {
        sprintf(dst, "(N)%n", delta);
        return delta;
    }
    if (node->type == NUM_T) {
        sprintf(dst, "(%d)%n", node->val, delta);
        return delta;
    }
    if (node->type == PAR_T) {
        sprintf(dst, "(x)%n", delta);
        return delta;
    }
    sprintf(dst, "(");
    dst++;
    global_delta++;
    delta = print_node(L_, dst);
    dst += delta;
    global_delta += delta;
    switch (node->type)
    {
    case  ADD_T: {sprintf(dst,      "+%n", delta); break;}
    case  SUB_T: {sprintf(dst,      "-%n", delta); break;}
    case  MUL_T: {sprintf(dst,      "*%n", delta); break;}
    case  DIV_T: {sprintf(dst,      "/%n", delta); break;}
    case   LN_T: {sprintf(dst,     "ln%n", delta); break;}
    case  EXP_T: {sprintf(dst,    "exp%n", delta); break;}
    case  LOG_T: {sprintf(dst,    "log%n", delta); break;}
    case  POW_T: {sprintf(dst,      "^%n", delta); break;}
    case  SIN_T: {sprintf(dst,    "sin%n", delta); break;}
    case  COS_T: {sprintf(dst,    "cos%n", delta); break;}
    case   SH_T: {sprintf(dst,     "sh%n", delta); break;}
    case   CH_T: {sprintf(dst,     "ch%n", delta); break;}
    case   TG_T: {sprintf(dst,     "tg%n", delta); break;}
    case   TH_T: {sprintf(dst,     "th%n", delta); break;}
    case ASIN_T: {sprintf(dst, "arcsin%n", delta); break;}
    case ACOS_T: {sprintf(dst, "arccos%n", delta); break;}
    case  ATG_T: {sprintf(dst,  "arctg%n", delta); break;}
    case  ASH_T: {sprintf(dst,  "arcsh%n", delta); break;}
    case  ACH_T: {sprintf(dst,  "arcch%n", delta); break;}
    case  ATH_T: {sprintf(dst,  "arcth%n", delta); break;}
    default:
        LOG_FATAL("UNKNOWN OPERATION: %d\n", node->val);
        return -1;
    }
    dst += delta;
    global_delta += delta;

    delta = print_node(R_, dst);
    dst += delta;
    global_delta += delta;
    

    return global_delta;
}
