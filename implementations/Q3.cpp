#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <openssl/sha.h>
#include <cstring>
#include <ctime>

using namespace std;


vector<unsigned char> hexToBytes(string hex) {
    vector<unsigned char> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        bytes.push_back((unsigned char)strtol(hex.substr(i, 2).c_str(), NULL, 16));
    }
    return bytes;
}


string bytesToHex(const unsigned char* bytes, size_t len) {
    stringstream ss;
    for (size_t i = 0; i < len; i++)
        ss << hex << setw(2) << setfill('0') << (int)bytes[i];
    return ss.str();
}


void sha256d(const vector<unsigned char>& data, unsigned char* out_hash) {
    unsigned char tmp[SHA256_DIGEST_LENGTH];
    SHA256(data.data(), data.size(), tmp);
    SHA256(tmp, SHA256_DIGEST_LENGTH, out_hash);
}


bool checkDifficulty(unsigned char* hash) {
    
    for (int i = 0; i < 4; i++) {
        if (hash[i] != 0) return false;
    }
    return true;
}

int main() {
    
    uint32_t version = 4; 
    string prev_block_hex = "00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee";
    string merkle_root_hex = "c0a692de10b69e2381a2856dcb0d0736dcd307bf25af7ce74831bf25793de626"; 
    uint32_t timestamp = 1231006505 + 1000; 
    uint64_t nonce = 0;

    
    vector<unsigned char> header;
    
    
    for (int i = 3; i >= 0; i--) header.push_back((version >> (i * 8)) & 0xFF);
    
    
    vector<unsigned char> prev = hexToBytes(prev_block_hex);
    header.insert(header.end(), prev.begin(), prev.end());
    
    
    vector<unsigned char> merkle = hexToBytes(merkle_root_hex);
    header.insert(header.end(), merkle.begin(), merkle.end());
    
   
    for (int i = 3; i >= 0; i--) header.push_back((timestamp >> (i * 8)) & 0xFF);

    size_t header_base_size = header.size();
    unsigned char hash[SHA256_DIGEST_LENGTH];

    cout << "Minerando... Isso pode levar alguns minutos." << endl;

    
    while (true) {
        
        header.resize(header_base_size);
        
       
        for (int i = 7; i >= 0; i--) header.push_back((nonce >> (i * 8)) & 0xFF);

       
        sha256d(header, hash);

        
        if (checkDifficulty(hash)) {
            cout << "Bloco Minerado!" << endl;
            cout << "Hash: " << bytesToHex(hash, SHA256_DIGEST_LENGTH) << endl;
            break;
        }

        nonce++;
        if (nonce % 1000000 == 0) cout << "Tentativas: " << nonce << "..." << endl;
    }

    
    ofstream outfile("exercise03.txt");
    if (outfile.is_open()) {
        outfile << bytesToHex(header.data(), header.size()) << endl;
        outfile.close();
        cout << "Resultado salvo em solutions/exercise03.txt" << endl;
    }

    return 0;
}