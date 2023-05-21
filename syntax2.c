
#define PROGRAM 0
#define LINE    1
#define PRINT   2
#define ASSIGN  3
#define ID      4
#define LBR     5
#define RBR     6
#define IMM     7
#define EXP     8
#define RET     9
#define OP      10
#define END     11
#define FILE    12

/FILE>/PROGRAM/END
/PROGRAM>/LINE/RET/PROGRAM
/PROGRAM>/LINE
/LINE>/PRINT/LBR/EXP/RBR
/LINE>/ID/ASSIGN/EXP
/EXP>/LBR/EXP/OP/EXP/RBR
/EXP>/ID
/EXP>/IMM


