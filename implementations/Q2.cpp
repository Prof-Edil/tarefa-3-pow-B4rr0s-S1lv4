#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <openssl/sha.h>
#include <algorithm>

using namespace std;

// Estrutura para armazenar o hash como bytes brutos
struct Hash {
    unsigned char data[SHA256_DIGEST_LENGTH];

    bool operator==(const string& hexStr) const {
        return toHexString() == hexStr;
    }

    string toHexString() const {
        stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            ss << hex << setw(2) << setfill('0') << (int)data[i];
        return ss.str();
    }
};

// Converte string hexadecimal para bytes brutos
Hash hexToBytes(string hex) {
    Hash h;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        string byteString = hex.substr(i, 2);
        h.data[i / 2] = (unsigned char) strtol(byteString.c_str(), NULL, 16);
    }
    return h;
}

// Calcula o SHA256 de uma concatenação de dois hashes
Hash sha256_combine(const Hash& a, const Hash& b) {
    Hash result;
    unsigned char combined[SHA256_DIGEST_LENGTH * 2];
    memcpy(combined, a.data, SHA256_DIGEST_LENGTH);
    memcpy(combined + SHA256_DIGEST_LENGTH, b.data, SHA256_DIGEST_LENGTH);
    
    SHA256(combined, SHA256_DIGEST_LENGTH * 2, result.data);
    return result;
}

int main() {
    string target_tx = "49ff8cccf1ca12179e9ae7a4760f550b5a18401b27e1e057604e27c3e10c08fb";
    vector<Hash> current_level;
    string line;

    // 1. Ler transações do arquivo
    ifstream infile("ex02_txid_list.txt");
    if (!infile.is_open()) {
        cerr << "Erro ao abrir ex02_txid_list.txt" << endl;
        return 1;
    }

    while (infile >> line) {
        current_level.push_back(hexToBytes(line));
    }
    infile.close();

    vector<string> proof;
    int target_idx = -1;

    // Localizar índice da transação alvo
    for (int i = 0; i < current_level.size(); i++) {
        if (current_level[i].toHexString() == target_tx) {
            target_idx = i;
            break;
        }
    }

    // 2. Construir a Árvore de Merkle e extrair a prova
    while (current_level.size() > 1) {
        vector<Hash> next_level;
        
        // Se o índice alvo existe neste nível, adicionar o "irmão" à prova
        if (target_idx != -1) {
            if (target_idx % 2 == 0) {
                // Alvo é o da esquerda. Irmão é o da direita (ou ele mesmo se for o último)
                if (target_idx + 1 < current_level.size())
                    proof.push_back(current_level[target_idx + 1].toHexString());
                else
                    proof.push_back(current_level[target_idx].toHexString());
            } else {
                // Alvo é o da direita. Irmão é o da esquerda.
                proof.push_back(current_level[target_idx - 1].toHexString());
            }
            // O índice no próximo nível será a metade
            target_idx /= 2;
        }

        for (size_t i = 0; i < current_level.size(); i += 2) {
            if (i + 1 < current_level.size()) {
                next_level.push_back(sha256_combine(current_level[i], current_level[i+1]));
            } else {
                // Regra: se não houver par, concatena consigo mesmo
                next_level.push_back(sha256_combine(current_level[i], current_level[i]));
            }
        }
        current_level = next_level;
    }

    // 3. Salvar resultados
    ofstream outfile("exercise02.txt");
    if (outfile.is_open()) {
        // Primeira linha: Merkle Root
        outfile << current_level[0].toHexString() << endl;
        // Linhas seguintes: Proofs
        for (const string& p : proof) {
            outfile << p << endl;
        }
        outfile.close();
        cout << "Sucesso! Resultado salvo em solutions/exercise02.txt" << endl;
    }

    return 0;
}