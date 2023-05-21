#include "lrgen.hpp"
#include <deque>

/***************************************************
 *                      RULE
 **************************************************/
Rule::Rule(Line *line, int index) {
    this->mIndex = index;
    this->mFrom = id2int(line->id);
    for (Exp *exp = line->exp; exp; exp = exp->next) {
        this->mTo.push_back(id2int(exp->id));
    }
}

int Rule::getIndex() {
    return this->mIndex;
}

int Rule::getFrom() {
    return mFrom;
}

int Rule::getTo(int index) {
    if (mTo.size() <= index) return -1;
    return mTo[index];
}

int Rule::getSize() {
    return mTo.size();
}

MappedRules mapRules(const Rules &rules) {
    MappedRules res;
    for (int i = 0; i < rules.size(); i++)
        res[rules[i]->getFrom()].push_back(rules[i]);
    return res;
}

set<int> first(int id, MappedRules &rules) {
    set<int> res;
    set<int> visited;
    deque<int> next;
    next.push_back(id);
    visited.insert(id);
    while(!next.empty()) {
        int node = next.front();
        next.pop_front();
        if (rules[node].empty()) {
            res.insert(node);
            continue;
        }
        for (int i = 0; i < rules[node].size(); i++) {
            if (!visited.count(rules[node][i]->getTo(0)))
                next.push_back(rules[node][i]->getTo(0));
        }
    }
    return res;
}


/********************************************************
 *                      ITEM
********************************************************/
Item::Item(Rule *rule, int index, set<int> endings) {
    this->mRule = rule;
    this->mIndex = index;
    this->mEndings = endings;
}
int Item::next() {
    return this->mRule->getTo(mIndex);
}
int Item::doubleNext() {
    return this->mRule->getTo(mIndex+1);
}
Item * Item::advance() {
    return new Item(this->mRule, this->mIndex+1, this->mEndings);
}
void Item::unionEnding(set<int> endings) {
    for (auto it = endings.begin(); it != endings.end(); it++) {
        this->mEndings.insert(*it);
    }
}
set<int> Item::getEndings() {
    return mEndings;
}
bool Item::compare(Item *item) {
    if (item->mRule->getFrom() > this->mRule->getFrom()) return true;
    if (item->mRule->getFrom() < this->mRule->getFrom()) return false;
    if (item->mRule->getSize() > this->mRule->getSize()) return true;
    if (item->mRule->getSize() < this->mRule->getSize()) return false;
    if (item->mIndex > this->mIndex) return true;
    for (int i = 0; i < this->mRule->getSize(); i++) {
        if (item->mRule->getTo(i) > this->mRule->getTo(i)) return true;
        if (item->mRule->getTo(i) < this->mRule->getTo(i)) return false;
    }
    return false;
}

bool itemcmp(Item * const &a, Item * const &b) {
    return a->compare(b);
}

Rule * Item::getRule() {
    return mRule;
}

void printItems(set<Item *, decltype(itemcmp)*> items, const set<int> &ids, const set<int> &complexIds) {
    map<int, char> dict;
    int i = 0;
    for (auto it = ids.begin(); it != ids.end(); it++) {
        dict[*it] = (complexIds.count(*it) ? 'A' : 'a') + i++;
    }
    dict[-1] = '$';
    dict[11] = 'S';
    dict[12] = 'L';
    for (auto it = items.begin(); it != items.end(); it++) {
        printf("/%c --> ", dict[(*it)->getRule()->getFrom()]);
        int to;
        for(int j = 0; (to = (*it)->getRule()->getTo(j)) >= 0; j++) {
            printf("/%c", dict[to]);
        }
        printf(" at: %c     ", dict[(*it)->next()]);
        auto endings = (*it)->getEndings();
        for (auto end = endings.begin(); end != endings.end(); end++) {
            printf("/%c", dict[*end]);
        }
        printf("\n");
    }
}
/*****************************************************
 *                      Closure
*****************************************************/

Closure::Closure(MappedRules &rules, set<Item *> items, int state) {
    mState = state;
    deque<Item *> temp;
    set<Item *, decltype(itemcmp)*> newSet(itemcmp);
    this->mItems = newSet;
    for (auto i = items.begin(); i != items.end(); i++) {
        this->mItems.insert(*i);
        temp.push_back(*i);
    }
    while (!temp.empty()) {
        Item *item = temp.front();
        temp.pop_front();
        int node;
        if ((node = item->next()) < 0) continue;
        set<int> endings = item->doubleNext() < 0 ? item->getEndings() : first(item->doubleNext(), rules);
        for (int i = 0; i < rules[node].size(); i++) {
            Item *newItem = new Item(rules[node][i], 0, endings);
            if (this->mItems.count(newItem)) {
                auto item = *(this->mItems.find(newItem));
                item->unionEnding(endings);
                continue;
            }
            temp.push_back(newItem);
            this->mItems.insert(newItem);
        }
    }
}

bool Closure::compare(Closure *closure) {
    // printf("cmp\n");
    if (closure->mItems.size() > this->mItems.size()) return true;
    if (closure->mItems.size() < this->mItems.size()) return false;
    auto thisit = this->mItems.begin();
    auto closit = closure->mItems.begin();
    while(thisit != this->mItems.end()) {
        // printf("cmp2\n");   
        if ((*thisit)->compare(*closit)) return true;
        // printf("cmp3\n");   
        if ((*closit)->compare(*thisit)) return false;
        // printf("cmp4\n");   
        thisit++;
        closit++;
        // printf("cmp5\n");   
    }
    // printf("cmp6\n");   
    return false;
}

void printItem(Item *item) {
    printf("/%d --> ", item->getRule()->getFrom());
    int to;
    for(int j = 0; (to = item->getRule()->getTo(j)) >= 0; j++) {
        printf("/%d", to);
    }
    printf("   at: %d", item->next());
    printf("\n");
}

map<int, set<Item *>> Closure::advanceItems() {
    map<int, set<Item *>> res;
    for(auto it = this->mItems.begin(); it != this->mItems.end(); it++) {
        if ((*it)->next() >= 0) res[(*it)->next()].insert((*it)->advance());
        else {
            auto endings = (*it)->getEndings();
            for (auto end = endings.begin(); 
                end != endings.end(); end++) {
                res[-(*end)-1].insert(*it);
            }
        }
    }
    return res;
}

void Closure::combineEndings(Closure *closure) {
    if (this->compare(closure) || closure->compare(this)) {
        printf("ERROR");
        return;
    }
    auto titems = this->mItems;
    auto citems = closure->mItems;
    auto thisit = titems.begin();
    auto closit = citems.begin();
    while (thisit != titems.end()) {
        (*thisit)->unionEnding((*closit)->getEndings());
        thisit++;
        closit++;
    }
}

int Closure::getState() {
    return mState;
}

set<Item *, decltype(itemcmp)*> Closure::getItems() {
    return mItems;
}

bool closurecmp(Closure * const &a, Closure * const &b) {
    bool res = a->compare(b);
    return res;
}
/*************************************************************
 *                          LRTable
*************************************************************/
Rules file2Rules(File *file) {
    Rules res;
    for (File *i = file; i; i = i->next) {
        res.push_back(new Rule(i->line, res.size()));
    }
    return res;
}

set<int> getEOFEnding() {
    set<int> res;
    // res.insert(EOF);
    return res;
}

set<int> allIdsFromExp(Exp *exp) {
    set<int> ids;
    if (!exp) return ids;
    ids.insert(id2int(exp->id));
    auto s = allIdsFromExp(exp->next);
    for (auto it = s.begin(); it != s.end(); it++) {
        ids.insert(*it);
    }
    return ids;
}

set<int> allIdsFromLine(Line *line) {
    set<int> ids;
    ids.insert(id2int(line->id));
    auto s = allIdsFromExp(line->exp);
    for (auto it = s.begin(); it != s.end(); it++) {
        ids.insert(*it);
    }
    return ids;
}

set<int> allIdsFromFile(File *file) {
    if (!file) return set<int>();
    set<int> ids = allIdsFromLine(file->line);
    set<int> s = allIdsFromFile(file->next);
    for (auto it = s.begin(); it != s.end(); it++) {
        ids.insert(*it);
    }
    return ids;
}

set<int> complexIdsFromFile(File *file) {
    if (!file) return set<int>();
    set<int> ids;
    ids.insert(id2int(file->line->id));
    set<int> s = complexIdsFromFile(file->next);
    for (auto it = s.begin(); it != s.end(); it++) {
        ids.insert(*it);
    }
    return ids;
}

Link makeLink(int from, int to, int action, int id) {
    Link res;
    res.fromState = from;
    res.num = to;
    res.action = action;
    res.id = id;
    return res;
}

void printRules(Rules rules) {
    for (int i = 0; i < rules.size(); i++) {
        printf("/%d --> ", rules[i]->getFrom());
        int to;
        for(int j = 0; (to = rules[i]->getTo(j)) >= 0; j++) {
            printf("/%d", to);
        }
        printf("\n");
    }
}

vector<vector<action>> createTable(const vector<Link> &links, const Rules &rules, const set<int> &ids, const set<int> &complexIds, int states, map<int, int> &dict) {
    vector<int> cols(ids.size());
    auto it = ids.begin();
    int offset = ids.size()-complexIds.size();
    int k = 0;
    for (int i = 0; i < cols.size(); i++) {
        if (complexIds.count(*it)) {
            dict[*it] = offset+k;
            cols[offset+k++] = *it;
        } else {
            dict[*it] = i-k;
            cols[i-k] = *it;
        }
        it++;
    }
    vector<vector<action>> res(states, vector<action>(ids.size(), NA));
    for (int i = 0; i < links.size(); i++) {
        res[links[i].fromState][dict[links[i].id]] = createAction(links[i].action, links[i].num);
    }
    return res;
}

void printStates(vector<Closure *> states, const set<int> &ids, const set<int> &complexIds) {
    for (int i = 0; i < states.size(); i++) {
        printf("STATE %d\n", states[i]->getState());
        if (states[i]->getState() == 2) {
            auto items = states[i]->advanceItems();
            if (!items[1].empty()) {
                printf("ERROR!\n");
            }
        }
        printItems(states[i]->getItems(), ids, complexIds);
    }
}

bool isEndingEqual(Closure *c1, Closure *c2) {
    auto items1 = c1->getItems();
    auto items2 = c2->getItems();
    auto it1 = items1.begin();
    auto it2 = items2.begin();
    while (it1 != items1.end()) {
        auto endings1 = (*it1)->getEndings();
        auto endings2 = (*it2)->getEndings();
        if (endings1.size() != endings2.size()) {
            return false;
        }
        auto end1 = endings1.begin();
        auto end2 = endings2.begin();
        while(end1 != endings1.end()) {
            if (*end1 != *end2) return false;
            end1++;
            end2++;
        }
        it1++;
        it2++;
    }
    return true;
}

LRTable::LRTable(File *file) {
    //init settings
    this->rules = file2Rules(file);
    MappedRules mapped = mapRules(rules);
    set<int> ids = allIdsFromFile(file);
    set<int> complexIds = complexIdsFromFile(file);
    //init for construction
    Item *first = new Item(rules[0], 0, getEOFEnding());
    deque<Closure *> next;
    vector<Link> links;
    set<Closure *, decltype(closurecmp)*> visited(closurecmp);
    set<Item *> initialItems;
    initialItems.insert(first);
    Closure *start = new Closure(mapped, initialItems, 0);
    //start bfs for constuction
    next.push_back(start);
    visited.insert(start);
    this->states.push_back(start);
    while(!next.empty()) {
        auto node = next.front();
        next.pop_front();
        auto edges = node->advanceItems();
        bool isEnd = true;
        for (auto it = ids.begin(); it != ids.end(); it++) {
            if (edges[*it].empty()) continue;
            isEnd = false;
            Closure *newClosure = new Closure(mapped, edges[*it], visited.size());
            if (visited.count(newClosure)) {
                if (!isEndingEqual((*visited.find(newClosure)), newClosure)) {
                    (*visited.find(newClosure))->combineEndings(newClosure);
                    next.push_back(*visited.find(newClosure));
                }
                links.push_back(makeLink(node->getState(), (*visited.find(newClosure))->getState(), 
                    complexIds.count(*it) ? GOTO : SHIFT, *it));
                continue;
            }
            links.push_back(makeLink(node->getState(), newClosure->getState(), 
                complexIds.count(*it) ? GOTO : SHIFT, *it));
            visited.insert(newClosure);
            next.push_back(newClosure);
            this->states.push_back(newClosure);
        }
        for (auto it = ids.begin(); it != ids.end(); it++) {
            int index = -*it-1;
            if (edges[index].empty()) continue;
            isEnd = false;
            links.push_back(makeLink(node->getState(), (*edges[index].begin())->getRule()->getIndex(), 
                REDUCE, *it));
        }
        if (isEnd) {
            for (auto it = ids.begin(); it != ids.end(); it++) {
                links.push_back(makeLink(node->getState(), 0, ACCEPT, *it));
            }
        }
    }
    this->table = createTable(links, rules, ids, complexIds, states.size(), this->id2index);
}

int LRTable::getIndex(int id) {
    return id2index[id];
}

vector<vector<action>> LRTable::getTable() {
    return table;
}

map<int, int> LRTable::getMapping() {
    return id2index;
}