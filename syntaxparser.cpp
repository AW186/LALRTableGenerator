#include "syntaxparser.hpp"
#include <deque>
#include <map>
#include <stdlib.h>

#define OFFSET 256
#define F (OFFSET+0)
#define L (OFFSET+1)
#define I (OFFSET+2)
#define E (OFFSET+3)
#define D (OFFSET+4)

using namespace std;

void panic(const char *str) {
    printf("panic: %s\n", str);
    exit(-1);
}
/*lalrtable
    
    /   [0-9]   >   \n  $   |   F   L   E   I   D
1   s8                      |   g2  g3      g5
2                       a   |
3                   s4  r1  |
4   s8                      |   g11 g3      g5
5               s6          |
6   s8                      |           g12 g7
7   s8              r5  r5  |           g14 g7
8       s9                  |                   g10
9   r8  s9      r8  r8  r8  |                   g13     
10  r6          r6  r6  r6  |
11                      r2  |
12                  r3  r3  |
13  r7          r7  r7  r7  |
14                  r4      |

*/
action lalrtable[15][10] = {
    {NA,    NA,     NA,     NA,     NA,     NA,     NA,     NA,     NA,     NA},
    {S(8),  NA,     NA,     NA,     NA,     G(2),   G(3),   NA,     G(5),   NA},
    {NA,    NA,     NA,     NA,     ACK,    NA,     NA,     NA,     NA,     NA},
    {NA,    NA,     NA,     S(4),   R(1),   NA,     NA,     NA,     NA,     NA},
    {S(8),  NA,     NA,     NA,     NA,     G(11),  G(3),   NA,     G(5),   NA},
    {NA,    NA,     S(6),   NA,     NA,     NA,     NA,     NA,     NA,     NA},
    {S(8),  NA,     NA,     NA,     NA,     NA,     NA,     G(12),  G(7),   NA},
    {S(8),  NA,     NA,     R(5),   R(5),   NA,     NA,     G(14),  G(7),  NA},
    {NA,    S(9),   NA,     NA,     NA,     NA,     NA,     NA,     NA,     G(10)},
    {R(8),  S(9),   R(8),   R(8),   R(8),   NA,     NA,     NA,     NA,     G(13)},
    {R(6),  NA,     R(6),   R(6),   R(6),   NA,     NA,     NA,     NA,     NA},
    {NA,    NA,     NA,     NA,     R(2),   NA,     NA,     NA,     NA,     NA},
    {NA,    NA,     NA,     R(3),   R(3),   NA,     NA,     NA,     NA,     NA},
    {R(7),  NA,     R(7),   R(7),   R(7),   NA,     NA,     NA,     NA,     NA},
    {NA,    NA,     NA,     R(4),   NA,     NA,     NA,     NA,     NA,     NA}
};

typedef struct stackblk {
    int type;
    int state;
    union {
        File        *file;
        Line        *line;
        Digits      *digits;
        Id          *id;
        Exp         *exp;
        u_int64_t   c;
    } u;
} stackblk;

File *newFile(Line *line, File *next) {
    File *file = (File *)malloc(sizeof(File));
    file->line = line;
    file->next = next;
    return file;
}

Exp *newExp(Id *id, Exp *next) {
    Exp *exp = (Exp *)malloc(sizeof(Exp));
    exp->id = id;
    exp->next = next;
    return exp;
}

Line *newLine(Id *id, Exp *exp) {

    Line *line = (Line *)malloc(sizeof(Line));
    line->exp = exp;
    line->id = id;
    return line;
}

Id *newId(Digits *digits) {
    Id *id = (Id *)malloc(sizeof(Id));
    id->digits = digits;
    return id;
}

Digits *newDigits(int val, Digits *next) {
    Digits *digits = (Digits *)malloc(sizeof(Digits));
    digits->next = next;
    digits->val = val;
    return digits;
}

stackblk makeStackBlk(int type, int state, void *data) {
    stackblk blk;
    blk.state = state;
    blk.type = type;
    switch (type)
    {
    case F:
        blk.u.file = (File *)data;
        break;
    case L:
        blk.u.line = (Line *)data;
        break;
    case I:
        blk.u.id =  (Id *)data;
        break;
    case D:
        blk.u.digits = (Digits *)data;
        break;
    case E:
        blk.u.exp = (Exp *)data;
    default:
        blk.u.c = (u_int64_t)data;
        break;
    }
    return blk;
}

map<int, int> getMap() {
    map<int, int> res;
    res['/'] = 0;
    res['0'] = 1;
    res['1'] = 1;
    res['2'] = 1;
    res['3'] = 1;
    res['4'] = 1;
    res['5'] = 1;
    res['6'] = 1;
    res['7'] = 1;
    res['8'] = 1;
    res['9'] = 1;
    res['>'] = 2;
    res['\n'] = 3;
    res[EOF] = 4;
    res[F] = 5;
    res[L] = 6;
    res[E] = 7;
    res[I] = 8;
    res[D] = 9;
    return res;
}


typedef auto (*reduce_fun)(deque<stackblk> &) -> stackblk;


stackblk r1(deque<stackblk> &stack) {
    auto line = stack.back().u.line;
    if (stack.back().type != L) panic("r1");
    stack.pop_back();
    stackblk res;
    res.type = F;
    res.u.file = newFile(line, NULL);
    return res;
}

stackblk r2(deque<stackblk> &stack) {
    auto next = stack.back().u.file;
    if (stack.back().type != F) panic("r2.1");
    stack.pop_back(); 
    stack.pop_back();
    auto line = stack.back().u.line;
    if (stack.back().type != L) panic("r2.2");
    stack.pop_back();
    stackblk res;
    res.type = F;
    res.u.file = newFile(line, next);
    return res;
}

stackblk r3(deque<stackblk> &stack) {
    auto exp = stack.back().u.exp;
    if (stack.back().type != E) panic("r3.1");
    stack.pop_back(); 
    stack.pop_back();
    auto id = stack.back().u.id;
    if (stack.back().type != I) panic("r3.2");
    stack.pop_back();
    stackblk res;
    res.type = L;
    res.u.line = newLine(id, exp);
    return res;
}

stackblk r4(deque<stackblk> &stack) {
    auto exp = stack.back().u.exp;
    if (stack.back().type != E) panic("r4.1");
    stack.pop_back();
    auto id = stack.back().u.id;
    if (stack.back().type != I) panic("r4.2");
    stack.pop_back();
    stackblk res;
    res.type = E;
    res.u.exp = newExp(id, exp);
    return res;
}


stackblk r5(deque<stackblk> &stack) {
    auto id = stack.back().u.id;
    if (stack.back().type != I) panic("r5");
    stack.pop_back(); 
    stackblk res;
    res.type = E;
    res.u.exp = newExp(id, NULL);
    return res;
}


stackblk r6(deque<stackblk> &stack) {
    auto digits = stack.back().u.digits;
    if (stack.back().type != D) panic("r6");
    stack.pop_back(); 
    stack.pop_back();
    stackblk res;
    res.type = I;
    res.u.id = newId(digits);
    return res;
}

stackblk r7(deque<stackblk> &stack) {
    auto digits = stack.back().u.digits;
    if (stack.back().type != D) panic("r7");
    stack.pop_back(); 
    auto c = stack.back().u.c;
    stack.pop_back();
    stackblk res;
    res.type = D;
    res.u.digits = newDigits(c, digits);
    return res;
}


stackblk r8(deque<stackblk> &stack) {
    auto c = *(char *)((u_int64_t *)(&stack.back().u.c));
    // printf("reduce digit: %c\n", c);
    stack.pop_back(); 
    stackblk res;
    res.type = D;
    res.u.digits = newDigits(c, NULL);
    return res;
}

stackblk r0(deque<stackblk> &stack) {
    printf("accepted\n");
    auto entry = stack.front();
    stack.pop_back();
    return entry;
}
reduce_fun reduce[NUM_RULES] = {
    r0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7,
    r8
};

File *parse(Reader *reader) {
    int state = 1;
    deque<stackblk> stack;
    stackblk buffer;
    int c = reader->getc();
    int next = c;
    map<int, int> dict = getMap();
    // printf("State g%d\n", state);
    // printf("%c\n", *(char *)((int *)(&next)));
    while (1) {
        action act = lalrtable[state][dict[next]];
        switch (act.type)
        {
        case FAIL:
            printf("Syntax error on state %d, with entry %d\n", state, next);
            exit(-1);
        case SHIFT:
            state = act.num;
            stack.push_back(makeStackBlk(next, state, (void *)next));
            c = reader->getc();
            next = c;
            // printf("State s%d\n", state);
            // printf("%c\n", *(char *)((int *)(&next)));
            break;
        case GOTO:
            state = act.num;
            buffer.state = state;
            stack.push_back(buffer);
            next = c;
            // printf("State g%d\n", state);
            break;
        case REDUCE:
            buffer = reduce[act.num](stack);
            next = buffer.type;
            state = !stack.empty() ? stack.back().state : 1;
            // printf("State r%d, to state %d\n", act.num, state);
            break;
        case ACCEPT:
            printf("finish stack size: %d\n", stack.size());
            return stack.back().u.file;
        default:
            break;
        }
    }
    return NULL;
}

File *parse(Reader *reader, vector<vector<action>> lrtable, map<int, int> mapping) {
    int state = 0;
    deque<stackblk> stack;
    stackblk buffer;
    int c = reader->getc();
    int next = c;
    map<int, int> dict = getMap();
    while (1) {
        action act = lrtable[state][mapping[dict[next]]];
        switch (act.type)
        {
        case FAIL:
            printf("Syntax error on state %d, with entry %d\n", state, next);
            exit(-1);
        case SHIFT:
            state = act.num;
            stack.push_back(makeStackBlk(next, state, (void *)next));
            c = reader->getc();
            next = c;
            break;
        case GOTO:
            state = act.num;
            buffer.state = state;
            stack.push_back(buffer);
            next = c;
            break;
        case REDUCE:
            buffer = reduce[act.num](stack);
            next = buffer.type;
            state = !stack.empty() ? stack.back().state : 0;
            break;
        case ACCEPT:
            return stack.front().u.file;
        default:
            break;
        }
    }
    return NULL;
}


action createAction(int a, int b) {
    action res;
    res.num = b;
    res.type = a;
    return res;
};

int id2int(Id *id) {
    int res = 0;
    for (Digits *d = id->digits; d; d=d->next) {
        res *= 10;
        res += d->val-'0';
    }
    return res;
}