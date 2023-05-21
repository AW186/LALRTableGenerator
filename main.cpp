
#include "syntaxparser.hpp"
#include "reader.hpp"
#include "lrgen.hpp"


void printFile(File *file);
void printLine(Line *line);
void printExp(Exp *exp);
void printId(Id *id);
void printDigit(Digits *digits);

void printFile(File *file) {
    if (!file) return;
    printLine(file->line);
    printf("\n");
    printFile(file->next);
}

void printLine(Line *line) {
    printf("line(");
    printId(line->id);
    printf(">");
    printExp(line->exp);
    printf(")");
}

void printExp(Exp *exp) {
    if (!exp) return;
    printf("exp(");
    printId(exp->id);
    printExp(exp->next);
    printf(")");
}

void printId(Id *id) {
    printf("id(");
    printDigit(id->digits);
    printf(")");
}

void printDigit(Digits *digits) {
    if (!digits) return;
    printf("%c", digits->val);
    printDigit(digits->next);
}

void printTable(const vector<vector<action>> &actions) {
    printf("print table\n");
    for (int i = 0; i < actions.size(); i++) {
        printf("%d: ", i);
        if (i < 10) printf(" ");
        for (int j = 0; j < actions[i].size(); j++) {
            switch(actions[i][j].type) {
            case SHIFT:
                printf("s%d  ", actions[i][j].num);
                break;
            case GOTO:
                printf("g%d  ", actions[i][j].num);
                break;
            case REDUCE:
                printf("r%d  ", actions[i][j].num);
                break;
            case FAIL:
                printf("NA   ", actions[i][j].num);
                break;
            }
            if (actions[i][j].type != FAIL && actions[i][j].num < 10) {
                printf(" ");
            }
        }
        printf("\n");
    }
}

int main() {
    File *file = parse(new FileReader("syntax.lr"));
    LRTable *table = new LRTable(file);
    printTable(table->getTable());
    File *test = parse(new FileReader("syntax.lr"), table->getTable(), table->getMapping());
    auto testRules = file2Rules(test);
    printRules(testRules);
}