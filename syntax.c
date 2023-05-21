#define SLASH   0
#define IMD     1
#define TO      2
#define RET     3
#define END     4
#define F       5
#define L       6
#define E       7
#define I       8
#define D       9
#define FP      10
//   /   [0-9]   >   \n  $   |   F   L   E   I   D
/FP>/F/END
/F>/L
/F>/L/RET/F
/L>/I/TO/E
/E>/I/E
/E>/I
/I>/SLASH/D
/D>/IMD/D
/D>/IMD


