#include <iostream>
#include <set>
#include <map>
#include <string>

using namespace std;

int main()
{
    set<string> stringSet;
    map<string, void *> stringMap;

    char hi[] = "hi";

    string s = hi;
    stringMap.insert(make_pair(hi, &hi));

    for (auto _s : stringMap) {
        cout << _s.first << " : " << _s.second << "\n";
    }

    stringSet.insert(hi);
    stringSet.insert(s);
 
    for (string _s : stringSet) {
        cout << _s << "\n";
    }

    return 0;
}
