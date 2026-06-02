#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

using namespace std;

struct Transaction {
    string txid;
    long long fee;
    long long weight;
    vector<string> parents;
    double density;
};


vector<string> splitParents(string s) {
    vector<string> result;
    stringstream ss(s);
    string item;
    while (getline(ss, item, ';')) {
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

int main() {
    unordered_map<string, Transaction> mempool;
    vector<string> txids_order;
    const string target_txid = "4c50e3dad7f98bceb6441f96b23748dea84fbdb7cedd603441e6ea4a574d04a6";

    ifstream file("mempool.csv");
    string line;

  
    while (getline(file, line)) {
        stringstream ss(line);
        string txid, fee_str, weight_str, parents_str;

        getline(ss, txid, ',');
        getline(ss, fee_str, ',');
        getline(ss, weight_str, ',');
        getline(ss, parents_str, ',');

        Transaction tx;
        tx.txid = txid;
        tx.fee = stoll(fee_str);
        tx.weight = stoll(weight_str);
        tx.parents = splitParents(parents_str);
        tx.density = (double)tx.fee / tx.weight;

        mempool[txid] = tx;
        txids_order.push_back(txid);
    }


    sort(txids_order.begin(), txids_order.end(), [&](const string& a, const string& b) {
        return mempool[a].density > mempool[b].density;
    });

    unordered_set<string> included_txs;
    vector<string> block;
    long long current_weight = 0;
    long long current_fees = 0;
    const long long MAX_WEIGHT = 4000000;


    auto add_with_ancestors = [&](auto self, string id) -> void {
        if (included_txs.count(id) || mempool.find(id) == mempool.end()) return;

        Transaction& tx = mempool[id];
        
  
        for (const string& parent_id : tx.parents) {
            self(self, parent_id);
        }


        if (!included_txs.count(id) && (current_weight + tx.weight <= MAX_WEIGHT)) {
            current_weight += tx.weight;
            current_fees += tx.fee;
            included_txs.insert(id);
            block.push_back(id);
        }
    };


    add_with_ancestors(add_with_ancestors, target_txid);

   
    for (const string& id : txids_order) {
        if (current_weight >= MAX_WEIGHT) break;
        add_with_ancestors(add_with_ancestors, id);
    }


    for (const string& id : block) {
        cout << id << endl;
    }


    return 0;
}