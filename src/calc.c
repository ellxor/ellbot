#include "calc.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define OPERATOR   0
#define LITERAL    1
#define LEFTPAREN  2
#define RIGHTPAREN 3
#define NEGATION   4

typedef struct TOKEN TOKEN;

struct TOKEN
{
        char kind;
        union { double d; char c; } value;
};

/// QUEUE IMPLEMENTATION ///

const int max_tok_count = 500;

typedef struct Q Q;

struct Q
{
        TOKEN toks[max_tok_count];
        int len;
};

static void
push(Q *queue, TOKEN tok)
{
        assert(queue->len < max_tok_count);
        queue->toks[queue->len++] = tok;
}

static TOKEN
pop(Q *queue)
{
        assert(queue->len > 0);
        return queue->toks[--queue->len];
}

static TOKEN
peek(Q *queue)
{
        assert(queue->len > 0);
        return queue->toks[queue->len-1];
}

/// PARSER IMPLEMENTATION ///

static size_t
parse_token(SV *sv, TOKEN *tok)
{
        switch (*sv->mem)
        {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
                tok->kind = OPERATOR;
                tok->value.c = *sv->mem;
                break;

        case '(':
                tok->kind = LEFTPAREN;
                break;

        case ')':
                tok->kind = RIGHTPAREN;
                break;

        default:
                tok->kind = LITERAL;

                char *end;
                tok->value.d = strtod(sv->mem, &end);

                size_t len = end - sv->mem;
                sv->count -= len;
                sv->mem = end;
                return len;
        }

        sv->mem++;
        sv->count--;
        return 1;
}

static SV
UNEXPECTED[] =
{
        [  OPERATOR] = SV("unexpected operator"),
        [   LITERAL] = SV("unexpected literal"),
        [ LEFTPAREN] = SV("unexpected ("),
        [RIGHTPAREN] = SV("unexpected )"),
        [  NEGATION] = SV("unexpected unary negation"),
};

static int
check_tokens(TOKEN *toks, int len, SV *err)
{
        if (len == 0)
        {
                *err = SV("empty expression");
                return -1;
        }

        int parend = 0;

        for (int i = 0; i < len; i++)
        {
                switch (toks[i].kind)
                {
                case  LEFTPAREN: parend++; break;
                case RIGHTPAREN: parend--; break;
                }

                if (parend < 0)
                {
                        *err = SV("unmatched `)`");
                        return -1;
                }
        }

        if (parend != 0)
        {
                *err = SV("unmatched `(`");
                return -1;
        }

        TOKEN last = { .kind = OPERATOR };

        for (int i = 0; i < len; i++)
        {
                TOKEN tok = toks[i];
                char valid = 0;

                switch (last.kind)
                {
                case LITERAL:
                        valid = tok.kind == OPERATOR
                             || tok.kind == RIGHTPAREN;
                        break;

                case OPERATOR:
                        valid = tok.kind == LITERAL
                             || tok.kind == LEFTPAREN
                             || tok.value.c == '-';

                        if (tok.value.c == '-')
                                toks[i].kind = NEGATION;

                        break;

                case NEGATION:
                        valid = tok.kind == LITERAL
                             || tok.kind == LEFTPAREN;
                        break;

                case LEFTPAREN:
                        valid = tok.kind == LITERAL
                             || tok.value.c == '-';

                        if (tok.value.c == '-')
                                toks[i].kind = NEGATION;

                        break;

                case RIGHTPAREN:
                        valid = tok.kind == OPERATOR;
                        break;
                }

                if (!valid)
                {
                        *err = UNEXPECTED[(int)tok.kind];
                        return -1;
                }

                last = toks[i];
        }

        if (last.kind == OPERATOR)
        {
                *err = SV("unexpected EOF");
                return -1;
        }

        return 0;
}

/// CALCULATOR ENGINE ///

static int
prec(TOKEN tok)
{
        assert(tok.kind == OPERATOR
               && "expected operator");

        static int TBL[128] =
        {
                ['+'] = 1,
                ['-'] = 1,
                ['*'] = 2,
                ['/'] = 2,
                ['%'] = 2,
                ['^'] = 3,
                ['$'] = 0xff,
        };

        return TBL[(int)tok.value.c];
}

static Q
to_rpn(TOKEN *toks, int len)
{
        //"converts tokens to reverse polish notation";
        Q output = {0};
        Q operators = {0};

        for (int i = 0; i < len; i++)
        {
                TOKEN tok = toks[i];
                TOKEN arg;

                switch (tok.kind)
                {
                case LITERAL:
                        push(&output, tok);
                        break;

                case LEFTPAREN:
                        push(&operators, tok);
                        break;

                case RIGHTPAREN:
                        while (peek(&operators).kind != LEFTPAREN)
                        {
                                push(&output, pop(&operators));
                        }

                        pop(&operators);
                        break;

                case NEGATION:
                        arg.kind = LITERAL;
                        arg.value.d = 0.0;
                        push(&output, arg);

                        tok.kind = OPERATOR;
                        tok.value.c = '$';

                        //"fallthough";

                case OPERATOR:
                        while (operators.len
                               && peek(&operators).kind != LEFTPAREN
                               && prec(peek(&operators)) >= prec(tok))
                        {
                                push(&output, pop(&operators));
                        }

                        push(&operators, tok);
                        break;
                }
        }

        while (operators.len > 0)
        {
                push(&output, pop(&operators));
        }

        return output;
}


static double
evaluate(Q *toks)
{
        assert(toks->len > 0 && "queue must not be empty");
        TOKEN tok = pop(toks);

        if (tok.kind == LITERAL)
        {
                return tok.value.d;
        }

        double A = evaluate(toks);
        double B = evaluate(toks);

        assert(tok.kind == OPERATOR);

        switch (tok.value.c)
        {
        case '+': return B + A;
        case '-': return B - A;
        case '*': return B * A;
        case '/': return B / A;
        case '$': return B - A;
        case '%': return fmod(B, A);
        case '^': return  pow(B, A);
        }

        assert(0 && "unreachable!");
}

/// WRAPPER ///

SV
eval(SV expr, SV *err)
{
        TOKEN toks[100];
        int tokc = 0;

        trim(&expr);

        while (expr.count > 0 && tokc < 100)
        {
                TOKEN tok = {0};
                size_t len = parse_token(&expr, &tok);

                if (len == 0)
                {
                        *err = sv_from(expr.mem, 1);
                        return SV("unexpected char: ");
                }

                toks[tokc++] = tok;
                trim(&expr);
        }

        if (expr.count > 0)
        {
                *err = SV("exceeded token limit");
                return SV("error: ");
        }

        if (check_tokens(toks, tokc, err) == -1)
        {
                return SV("syntax error: ");
        }

        Q rpn = to_rpn(toks, tokc);
        double v = evaluate(&rpn);

        static char mem[30];
        int len = 0;

        if (fabs(v) < 1e15 && floor(v) == v)
        {
                len = snprintf(mem, sizeof(mem), "%ld", (uint64_t)v);
        }


        else {
                len = snprintf(mem, sizeof(mem), "%g", v);
        }

        return sv_from(mem, len);
}
