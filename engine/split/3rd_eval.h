/* A mathematical expression evaluator.
 * It uses a recursive descent parser internally.
 * Author: Werner Stoop
 * This is free and unencumbered software released into the public domain.
 * http://unlicense.org/
 */

#include <assert.h>
#include <ctype.h>
#include <math.h>  /* remember to compile with -lm */
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

/* Special tokens used by the lexer function lex()
 *  they've been chosen as non-printable characters
 *  so that printable characters can be used for other
 *  purposes
 */
#define TOK_END     0  /* end of text */
#define TOK_INI     1  /* Initial state */
#define TOK_ID      2  /* identifier */
#define TOK_NUM     3  /* number */

/* Types of errors */
                 // 0  /* "no error" */
#define ERR_MEMORY  1  /* "out of memory" */
#define ERR_LEXER   2  /* "unknown token" */
#define ERR_LONGID  3  /* "identifier too long" */
#define ERR_VALUE   4  /* "value expected" */
#define ERR_BRACKET 5  /* "missing ')'" */
#define ERR_FUNC    6  /* "unknown function" */
#define ERR_ARGS    7  /* "wrong number of arguments" */
#define ERR_CONST   8  /* "unknown constant" */

/* Other definitions */
#define MAX_ID_LEN  11      /* Max length of an identifier */
#define OPERATORS   "+-*/%(),^&|!" /* Valid operators */

#define EVAL_PI         3.141592654
#define EVAL_E          2.718281828
#define EVAL_DEG        (EVAL_PI/180)

/* Internal structure for the parser/evaluator */
struct eval {

    jmp_buf j;      /* For error handling */

    const char *p;  /* Position in the text being parsed */

    double *st;     /* Stack */
    int st_size;    /* Stack size */
    int sp;         /* Stack pointer */

    /* The current and next tokens identified by the lexer */
    struct {
        int type;                        /* Type of the token */
        double  n_val;                   /* Numeric value of the previous lexed token */
        char        s_val[MAX_ID_LEN];   /* String (identifier) value of the previous lexed token */
    } token[2];

    int cur_tok;        /* Current token, either 0 or 1 (see the comments of lex()) */
};

/* Prototypes */
static double pop(struct eval *ev);
static void push(struct eval *ev, double d);
static int lex(struct eval *ev);

/* Prototypes for the recursive descent parser */
static void expr(struct eval *ev);
static void add_expr(struct eval *ev);
static void mul_expr(struct eval *ev);
static void pow_expr(struct eval *ev);
static void uni_expr(struct eval *ev);
static void bra_expr(struct eval *ev);
static void id_expr(struct eval *ev);
static void num_expr(struct eval *ev);

/*
 *  Evaluates a mathemeatical expression
 */
double eval(const char *exp/*, int *ep*/) {
int _ep, *ep = &_ep;
    struct eval ev;
    double ans = 0.0;

    assert(ep != NULL);

    /* Allocate a stack */
    ev.st_size = 10;
    ev.st = CALLOC(ev.st_size, sizeof *ev.st);
    if(!ev.st)
    {
        *ep = ERR_MEMORY;
        return NAN; //0.0;
    }
    ev.sp = 0;

    /* Manage errors */
    *ep = setjmp(ev.j);
    if(*ep != 0)
    {
        FREE(ev.st);
        return NAN; //0.0;
    }

    /* Initialize the lexer */
    ev.token[0].type = TOK_INI;
    ev.token[0].s_val[0] = '\0';
    ev.token[1].type = TOK_INI;
    ev.token[1].s_val[0] = '\0';
    ev.cur_tok = 0;

    /* Initialize the parser */
    ev.p = exp;

    /* lex once to initialize the lexer */
    if(lex(&ev) != TOK_END)
    {
        expr(&ev);
        ans = pop(&ev);
    }

    FREE(ev.st);
    return ans;
}

/*
 * Pushes a value onto the stack, increases the stack size if necessary
 */
static void push(struct eval *ev, double d) {
    if(ev->sp == ev->st_size) {
        /* Resize the stack by 1.5 */
        double *old = ev->st;
        int new_size = ev->st_size + (ev->st_size >> 1);
        ev->st = REALLOC(ev->st, new_size);
        if(!ev->st) {
            ev->st = old;
            longjmp(ev->j, ERR_MEMORY);
        }

        ev->st_size = new_size;
    }

    ev->st[ev->sp++] = d;
}

//  Pops a value from the top of the stack
static double pop(struct eval *ev) {
    assert(ev->sp > 0);
    return ev->st[--ev->sp];
}

// stricmp() is common, but not standard, so I provide my own
static int istrcmp(const char *p, const char *q) {
    for(; tolower(p[0]) == tolower(q[0]) && p[0]; p++, q++);
    return tolower(p[0]) - tolower(q[0]);
}

/*
 *  Lexical analyzer function
 *
 *  In order to implement LL(1), struct eval has an array of two token structures,
 *  and its cur_tok member is used to point to the _current_ token, while the other
 *  element contains the _next_ token. This implements a 2 element ring buffer where
 *  the lexer always writes to the _next_ token so that the recursive descent parser can
 * _peek_ at the next token.
 */

static int lex(struct eval *ev) {
    int next_tok;

start:
    /* Cycle the tokens */
    next_tok = ev->cur_tok;
    ev->cur_tok = ev->cur_tok?0:1;

    while(isspace(ev->p[0])) ev->p++;

    if(!ev->p[0]) {
        /* End of the expression */
        ev->token[next_tok].type = TOK_END;
        goto end;
    }
    else if(isdigit(ev->p[0]) || ev->p[0] == '.') {
        /* Number */
        char *endp;
        ev->token[next_tok].type = TOK_NUM;
        ev->token[next_tok].n_val = strtod(ev->p, &endp);
        ev->p = endp;
        goto end;
    }
    else if(isalpha(ev->p[0])) {
        /* Identifier */
        int i;
        for(i = 0; (isalnum(ev->p[0]) || ev->p[0] == '_') && i < MAX_ID_LEN - 1; i++, ev->p++)
            ev->token[next_tok].s_val[i] = ev->p[0];

        if(isalpha(ev->p[0])) longjmp(ev->j, ERR_LONGID);

        ev->token[next_tok].s_val[i] = '\0';
        ev->token[next_tok].type = TOK_ID;
        goto end;
    }
    else if(strchr(OPERATORS, ev->p[0])) {
        /* Operator */
        ev->token[next_tok].type = ev->p[0];
        ev->p++;
        goto end;
    }
    else /* Unknown token */
        longjmp(ev->j, ERR_LEXER);

end:

    /* If this was the first call, cycle the tokens again */
    if(ev->token[ev->cur_tok].type == TOK_INI)
        goto start;

    return ev->token[ev->cur_tok].type;
}

#define EVAL_TYPE(e)     (e->token[e->cur_tok].type)
#define EVAL_ERROR(c)    longjmp(ev->j, (c))

// num_expr ::= NUMBER
static void num_expr(struct eval *ev) {
    if(EVAL_TYPE(ev) != TOK_NUM)
        EVAL_ERROR(ERR_VALUE);
    push(ev, ev->token[ev->cur_tok].n_val);
    lex(ev);
}

// expr ::= add_expr
static void expr(struct eval *ev) {
    add_expr(ev);
}

// add_expr ::= mul_expr [('+'|'-') mul_expr]
static void add_expr(struct eval *ev) {
    int t;
    mul_expr(ev);
    while((t =EVAL_TYPE(ev)) == '+' || t == '-' || t == '|') {
        double a,b;
        lex(ev);
        mul_expr(ev);
        b = pop(ev);
        a = pop(ev);

        if(t == '+')
            push(ev, a + b);
        else if(t == '-')
            push(ev, a - b);
        else
            push(ev, a || b);
    }
}

// mul_expr ::= pow_expr [('*'|'/'|'%') pow_expr]
static void mul_expr(struct eval *ev) {
    int t;
    pow_expr(ev);
    while((t = EVAL_TYPE(ev)) == '*' || t == '/' || t == '%' || t == '&') {
        double a,b;
        lex(ev);
        pow_expr(ev);
        b = pop(ev);
        a = pop(ev);

        if(t == '*')
            push(ev, a * b);
        else if(t == '/')
            push(ev, a / b);
        else if(t == '%')
            push(ev, fmod(a, b));
        else
            push(ev, a && b);
    }
}

// pow_expr ::= uni_expr ['^' pow_expr]
static void pow_expr(struct eval *ev) {
    /* Note that exponentiation is right associative:
    2^3^4 is 2^(3^4), not (2^3)^4 */
    uni_expr(ev);
    if(EVAL_TYPE(ev) == '^') {
        double a,b;
        lex(ev);
        pow_expr(ev);
        b = pop(ev);
        a = pop(ev);
        push(ev, pow(a,b));
    }
}

// uni_expr ::= ['+'|'-'] bra_expr
static void uni_expr(struct eval *ev) {
    int t = '+';
    if(EVAL_TYPE(ev) == '-' || EVAL_TYPE(ev) == '+' || EVAL_TYPE(ev) == '!') {
        t = EVAL_TYPE(ev);
        lex(ev);
    }

    bra_expr(ev);

    if(t == '-') {
        double a = pop(ev);
        push(ev, -a);
    }
    else if(t == '!') {
        double a = pop(ev);
        push(ev, !a);
    }
}

// bra_expr ::= '(' add_expr ')' | id_expr
static void bra_expr(struct eval *ev) {
    if(EVAL_TYPE(ev) == '(') {
        lex(ev);
        add_expr(ev);
        if(EVAL_TYPE(ev) != ')')
            EVAL_ERROR(ERR_BRACKET);
        lex(ev);
    }
    else
         id_expr(ev);
}

// id_expr ::= ID '(' add_expr [',' add_expr]* ')' | ID | num_expr
static void id_expr(struct eval *ev) {
    int nargs = 0;
    char id[MAX_ID_LEN];
    if(EVAL_TYPE(ev) != TOK_ID) {
       num_expr(ev);
    } else {
        strcpy(id, ev->token[ev->cur_tok].s_val);
        lex(ev);
        if(EVAL_TYPE(ev) != '(') {
#ifdef EVAL_EXTEND_CONSTANTS
       EVAL_EXTEND_CONSTANTS
#else
       if(0) {}
#endif
            else if(!strcmp(id, "true"))  push(ev, 1.0);
            else if(!strcmp(id, "false")) push(ev, 0.0);
            else if(!strcmp(id, "on"))  push(ev, 1.0);
            else if(!strcmp(id, "off")) push(ev, 0.0);
            // pi - 3.141592654
            else if(!strcmp(id, "pi"))
                push(ev, EVAL_PI);
            // e - base of natural logarithms, 2.718281828
            else if(!strcmp(id, "e"))
                push(ev, EVAL_E);
            // deg - deg2rad, allows to degree conversion `sin(90*deg) = 1`
            else if(!strcmp(id, "deg"))
                push(ev, EVAL_DEG);
            else
                EVAL_ERROR(ERR_CONST);
        } else {
            lex(ev);

            while(EVAL_TYPE(ev) != ')') {
                add_expr(ev);
                nargs++;
                if(EVAL_TYPE(ev) == ')') break;

                if(EVAL_TYPE(ev) != ',')
                    EVAL_ERROR(ERR_BRACKET);
                lex(ev);
            }
            lex(ev);

#ifdef EVAL_EXTEND_FUNCTIONS
       EVAL_EXTEND_FUNCTIONS
#else
       if(0) {}
#endif
            // abs(x) - absolute value of x
            else if(!istrcmp(id, "abs")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, fabs(pop(ev)));
            }
            // ceil(x) - smallest integer greater than x
            else if(!istrcmp(id, "ceil")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, ceil(pop(ev)));
            }
            // floor(x) - largest integer smaller than x
            else if(!istrcmp(id, "floor")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, floor(pop(ev)));
            }
            // sin(x) - sine of x, in radians
            else if(!istrcmp(id, "sin")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, sin(pop(ev)));
            }
            // asin(x) - arcsine of x, in radians
            else if(!istrcmp(id, "asin")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, asin(pop(ev)));
            }
            // cos(x) - cosine of x, in radians
            else if(!istrcmp(id, "cos")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, cos(pop(ev)));
            }
            // acos(x) - arccosine of x, in radians
            else if(!istrcmp(id, "acos")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, acos(pop(ev)));
            }
            // tan(x) - tangent of x, in radians
            else if(!istrcmp(id, "tan")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, tan(pop(ev)));
            }
            // atan(x) - arctangent of x, in radians
            else if(!istrcmp(id, "atan")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, atan(pop(ev)));
            }
            // atan(y,x) - arctangent of y/x, in radians.
            else if(!istrcmp(id, "atan2")) {
                double a, b;
                if(nargs != 2) EVAL_ERROR(ERR_ARGS);
                b = pop(ev);
                a = pop(ev);
                push(ev, atan2(a,b));
            }
            // sinh(x) - hyperbolic sine of x, in radians
            else if(!istrcmp(id, "sinh")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, sinh(pop(ev)));
            }
            // cosh(x) - hyperbolic cosine of x, in radians
            else if(!istrcmp(id, "cosh")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, cosh(pop(ev)));
            }
            // tanh(x) - hyperbolic tangent of x, in radians
            else if(!istrcmp(id, "tanh")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, tanh(pop(ev)));
            }
            // log(x) - natural logarithm of x
            else if(!istrcmp(id, "log")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, log(pop(ev)));
            }
            // log10(x) - logarithm of x, base-10
            else if(!istrcmp(id, "log10")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, log10(pop(ev)));
            }
            // exp(x) - computes e^x
            else if(!istrcmp(id, "exp")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, exp(pop(ev)));
            }
            // sqrt(x) - square root of x
            else if(!istrcmp(id, "sqrt")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, sqrt(pop(ev)));
            }
            // rad(x) - converts x from degrees to radians
            else if(!istrcmp(id, "rad")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, pop(ev)*EVAL_PI/180);
            }
            // deg(x) - converts x from radians to degrees
            else if(!istrcmp(id, "deg")) {
                if(nargs != 1) EVAL_ERROR(ERR_ARGS);
                push(ev, pop(ev)*180/EVAL_PI);
            }
            // pow(x,y) - computes x^y
            else if(!istrcmp(id, "pow")) {
                double a, b;
                if(nargs != 2) EVAL_ERROR(ERR_ARGS);
                b = pop(ev);
                a = pop(ev);
                push(ev, pow(a,b));
            }
            // hypot(x,y) - computes sqrt(x*x + y*y)
            else if(!istrcmp(id, "hypot")) {
                double a, b;
                if(nargs != 2) EVAL_ERROR(ERR_ARGS);
                b = pop(ev);
                a = pop(ev);
                push(ev, sqrt(a*a + b*b));
            }
            else
                EVAL_ERROR(ERR_FUNC);
        }
    }
}

//

#ifdef EVALDEMO
#include <stdio.h>
int main() {
    assert( eval("1+1") == 2 );         // common path
    assert( eval("1+") != eval("1+") ); // check that errors return NAN
    assert(~puts("Ok") );
}
#endif
