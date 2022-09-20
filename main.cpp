#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <map>

using namespace std;

using u32 = unsigned int;
using u64 = unsigned long long;

using input_t = int;

using Uset = set<u32>;

template<typename Tkey>
bool contain(const Tkey& key, const unordered_set<Tkey> s) {
    return s.find(key) != s.end();
}
template<typename Tkey, typename Tval>
bool contain(const Tkey& key, const unordered_map<Tkey, Tval> m) {
    return m.find(key) != m.end();
}

class Set {
private:
    Uset uset_;
public:
    Set() = default;
    Set(std::initializer_list<u32> val_list) {
        for (u32 x : val_list)
            uset_.insert(x);
    }
    Set(const Set& set_) {
        uset_ = Uset(set_.uset_);
    }

    /*
     * const member function
     */
    Uset::const_iterator begin() const { return uset_.begin(); }
    Uset::const_iterator end() const { return uset_.end(); }
    size_t size() const { return uset_.size(); }

    /*
     * non-const member function
     */
    bool contain(u32 x) const { return uset_.find(x) != uset_.end(); }
    void insert(u32 x) { uset_.insert(x); }

    [[maybe_unused]] size_t erase(u32 x) { return uset_.erase(x); }

    // 根据现有状态集合，输入y，产生另一个状态集合
    Set transform(input_t input, const unordered_map<u32, unordered_map<input_t, Set>>& table) const {
        Set result;
        for (u32 x : uset_) {
            if (table.find(x) != table.end()) {
                if (::contain(x, table) && ::contain(input, table.at(x))) {
                    result += table.at(x).at(input);
                }
            }
        }
        return result; // 也许move就不用创建出另一个对象了？
    }

    // 根据现有状态集合，输入一个输入集合，产生另一个状态集合
    [[maybe_unused]] Set transforms(const unordered_set<input_t>& inputs, const unordered_map<u32, unordered_map<input_t, Set>>& table) const {
        Set result;
        for (input_t input : inputs) {
            result += transform(input, table);
        }
        return result;
    }

    // 求epsilon闭包
    Set epsilon_closure(const unordered_map<u32, unordered_map<input_t, Set>>& table) const {
        Set result = Set(*this);
        for (u32 x : *this) {
            if (::contain(x, table) && ::contain(-1, table.at(x))) {
                result += table.at(x).at(-1);
            }
        }
        // epsilon_closure每次调用只会进行一步转化，但存在连续epsilon输入的情况
        if (*this == result)
            return result;
        return result.epsilon_closure(table);
    }

    /*
     * operators
     */
    bool operator==(const Set& set_) const {
        if (uset_.size() != set_.size())
            return false;
        for (u32 x : uset_) {
            if (!set_.contain(x))
                return false;
        }
        return true;
    }

    // 并集
    void operator+=(const Set& set_) {
        for (u32 x : set_) {
            insert(x);
        }
    }

    // 交集
    Set operator&(const Set& set_) const {
        Set result;
        for (u32 x : uset_) {
            if (set_.contain(x))
                result.insert(x);
        }
        return result;
    }

    friend ostream& operator<< (ostream& out, const Set& set_) {
        out << '{';
        for (u32 x : set_)
            out << x << ", ";
        out << '}';
        return out;
    }

    /*
     * helpers
     */
    void print() const {
        printf("{");
        for (u32 x : uset_)
            printf("%u,", x);
        printf("}");
    }
    void println() const { print(); puts(""); }
};

struct Hasher {
    u64 operator()(const Set& set_) const {
        u64 result = 0;
        for (const u32 x : set_) {
            result = result * 97967 + x;  // 随便取的哈希
        }
        return result;
    }
};


void readall(unordered_map<u32, unordered_map<input_t, Set>>& transformer,
             unordered_set<input_t>& input_set,
             Set& start, Set& end) {
    freopen("../in.txt", "r", stdin);
    u32 s,e;
    scanf("%u%u", &s,&e);
    start.insert(s);
    end.insert(e);
    while (true) {
        u32 status, next_status;
        int input;
        if (scanf("%u%d%u", &status, &input, &next_status)!=3)
            break;
        transformer[status][input].insert(next_status);
        input_set.insert(input);
    }
    input_set.erase(-1);  // 默认-1为epsilon
    cout << input_set.size() << endl;
}


int main() {
    unordered_map<u32, unordered_map<input_t, Set>> transformer;
    unordered_set<input_t> input_set;
    Set start, end;
    readall(transformer, input_set, start, end);
    unordered_map<Set, u32, Hasher> number_of_set;
    unordered_set<Set, Hasher> new_;
    Set start_closure = start.epsilon_closure(transformer);
//    zero_closure.println();
    u32 cnt = 0;
    number_of_set[start_closure] = cnt++;
    new_.insert(start_closure);
    cout << "-----------------------" << endl;
    while (!new_.empty()) {
        const Set& set_ = *new_.begin(); // 随便选一个未处理过的状态
        for (input_t input : input_set) { // 对于所有可能的输入
            Set new_set = set_.transform(input, transformer).epsilon_closure(transformer); // 转换并求epsilon闭包
            if (new_set.size() == 0) continue;
            if (number_of_set.find(new_set) == number_of_set.end()) {
                number_of_set[new_set] = cnt++;
                new_.insert(new_set);
            }
            cout << number_of_set[set_] << " + " << input << " -> " << number_of_set[new_set] << endl;
//            cout << set_ << " + " << input << " -> " << new_set << endl;
        }
//        cout << endl;
        new_.erase(set_);
    }
    cout << "-----------------------" << endl;
    map<u32, Set> tmp_map;
    for (const auto& p : number_of_set) {
        tmp_map[p.second] = p.first;
    }
    for (const auto& p : tmp_map) {
        cout << p.first << ": ";
        p.second.print();
        if ((p.second & start).size() > 0) {
            cout << " start";
        }
        if ((p.second & end).size() > 0) {
            cout << " end";
        }
        cout << endl;
    }
}
