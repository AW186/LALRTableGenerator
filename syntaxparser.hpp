#ifndef SYNTAX_PARSER_HPP
#define SYNTAX_PARSER_HPP

#include "reader.hpp"
#include <vector>
#include <map>
/*Grammar of how to define gramar

F' -> .F&       ?                   r0

.F -> .L        $                   r1
.F -> .L\nF     $                   r2

.L -> .I>E      \n, $               r3

.E -> .IE       \n, $               r4

.E -> .I        \n, $               r5

.I -> ./D       >, /, \n, $         r6

.D -> .[0-9]D   >, /, \n, $         r7

.D -> .[0-9]    >, /, \n, $         r8

lalrtable:
    
    /   [0-9]   >   \n  $   |   F   L   E   I   D
1   s8                      |   g2  g3      g5
2                       a   |
3                   s4  r1  |
4   s8                      |   g11 g3      g5
5               s6          |
6   s8                      |               g7
7   s8              r5  r5  |
8       s9                  |                   g10
9   r8  s9      r8  r8  r8  |                   g13     
10  r6          r6  r6  r6  |
11                      r2  |
12                  r3  r3  |
13  r7          r7  r7  r7  |
14                  r4  r4  |

*/
#define NUM_RULES 9
#define FAIL  0
#define SHIFT 1
#define GOTO  2
#define REDUCE 3
#define ACCEPT 4
#define NA (createAction(FAIL, 0))
#define S(a) (createAction(SHIFT, a))
#define G(a) (createAction(GOTO, a))
#define R(a) (createAction(REDUCE, a))
#define ACK (createAction(ACCEPT, 0))

using namespace std;

typedef struct action {
    int type;
    int num;
} action;

action createAction(int a, int b);

typedef struct File     File;
typedef struct Line     Line;
typedef struct Id       Id;
typedef struct Exp      Exp;
typedef struct Digits   Digits;

struct File
{
    Line *line;
    File *next = NULL;
};

struct Line
{
    Exp *exp;
    Id *id;
};

struct Id
{
    Digits *digits;
};

struct Exp
{
    Id *id;
    Exp *next = NULL;
};

struct Digits
{
    int val;
    Digits *next = NULL;
};

File *parse(Reader *reader);
//test purpose
File *parse(Reader *reader, vector<vector<action>> lrtable, map<int, int> mapping);
int id2int(Id *id);


#endif