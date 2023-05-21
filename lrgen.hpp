#ifndef LRGEN_HPP
#define LRGEN_HPP

#include "syntaxparser.hpp"
#include <vector>
#include <set>
#include <map>

using namespace std;


class Rule {
private:
    int mIndex;
    int mFrom;
    vector<int> mTo;
public:
    Rule(Line *line, int index);
    int getFrom();
    int getTo(int index);
    int getSize();
    int getIndex();
};
typedef vector<Rule *> Rules;
typedef map<int, vector<Rule *>> MappedRules;
MappedRules mapRules(const Rules &rules);
set<int> first(int id, MappedRules &rules);
void printRules(Rules rules);
Rules file2Rules(File *file);

class Item
{
private:
    Rule *mRule;
    int mIndex;
    set<int> mEndings;
public:
    Item(Rule *rule, int index, set<int> endings);
    int next();
    int doubleNext();
    void unionEnding(set<int> endings);
    Item *advance();
    bool compare(Item *item);
    set<int> getEndings();
    Rule *getRule();
};

bool itemcmp(Item * const &a, Item * const &b);

class Closure {
private:
    set<Item *, decltype(itemcmp)*> mItems;
    int mState;
public:
    int getState();
    bool compare(Closure *closure);
    Closure(MappedRules &rules, set<Item *> items, int state);
    map<int, set<Item *>> advanceItems();
    void combineEndings(Closure *closure);
    set<Item *, decltype(itemcmp)*> getItems();
};

bool closurecmp(Closure * const &a, Closure * const &b);

struct Link {
    int fromState;
    int num;
    int action;
    int id;
};

class LRTable {
private:
    Rules rules;
    vector<Closure *> states;
    vector<vector<action>> table;
    map<int, int> id2index;
public:
    LRTable(File *file);
    vector<vector<action>> getTable();
    int getIndex(int id);
    map<int, int> getMapping();
};

#endif