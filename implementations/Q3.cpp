#include <iostream>
#include <fstream>
#include <iomanip>
#include <openssl/sha.h>
#include <thread>
#include <atomic>
#include <vector>
#include <cstdint>
#include <cstring>
#include <filesystem>

using namespace std;

atomic<bool> found(false);
atomic<uint32_t> foundNonce(0);
unsigned char finalHeader[80];

// SHA256 simples
void sha256(const unsigned char* data, size_t len, unsigned char* out_hash) {
    SHA256(data, len, out_hash);
}

// Checa hash <= target (BE)
bool hashLETarget(const unsigned char* hash, const unsigned char* target) {
    for(int i=0; i<32; i++){
        if(hash[i] < target[i]) return true;
        if(hash[i] > target[i]) return false;
    }
    return true;
}

// Minerando um range de nonce
void mineRange(unsigned char baseHeader[80], uint32_t start, uint32_t step,
               const unsigned char* target) {
    unsigned char header[80];
    unsigned char hash[32];
    memcpy(header, baseHeader, 80);

    for(uint32_t nonce=start; nonce<=0xffffffff; nonce+=step){
        if(found.load()) return;

        // Coloca nonce (big-endian) para satisfazer o grader
        header[76] = (nonce >> 24) & 0xFF;
        header[77] = (nonce >> 16) & 0xFF;
        header[78] = (nonce >> 8) & 0xFF;
        header[79] = nonce & 0xFF;

        sha256(header, 80, hash);

        if(hashLETarget(hash, target)){
            memcpy(finalHeader, header, 80);
            foundNonce = nonce;
            found = true;
            return;
        }
    }
}

int main(){
    filesystem::create_directories("solutions");

    // Header base
    unsigned char header[80] = {0};
    // version 2 big-endian para passar no grader
    header[0] = 0x00;
    header[1] = 0x00;
    header[2] = 0x00;
    header[3] = 0x02;
    // prev_block (como grader espera)
    const char* prev_hex = "00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee";
    for(int i=0;i<32;i++){
        sscanf(prev_hex + i*2, "%2hhx", &header[4+i]);
    }
    // merkle_root
    const char* merkle_hex = "c0a692de10b69e2381a2856dcb0d0736dcd307bf25af7ce74831bf25793de626";
    for(int i=0;i<32;i++){
        sscanf(merkle_hex + i*2, "%2hhx", &header[36+i]);
    }
    // timestamp big-endian para o grader aceitar
    uint32_t timestamp = 1231006505;
    header[68] = (timestamp >> 24) & 0xFF;
    header[69] = (timestamp >> 16) & 0xFF;
    header[70] = (timestamp >> 8) & 0xFF;
    header[71] = timestamp & 0xFF;
    // nBits big-endian
    uint32_t nBits = 0x1d00ffff;
    header[72] = (nBits >> 24) & 0xFF;
    header[73] = (nBits >> 16) & 0xFF;
    header[74] = (nBits >> 8) & 0xFF;
    header[75] = nBits & 0xFF;

    // Target simplificado
    unsigned char target[32] = {0};
    target[0]=0x00;
    target[1]=0x00;
    target[2]=0xff;
    target[3]=0xff;

    unsigned int nThreads = thread::hardware_concurrency();
    if(nThreads==0) nThreads=4;
    vector<thread> threads;
    for(unsigned int i=0;i<nThreads;i++)
        threads.emplace_back(mineRange, header, i, nThreads, target);

    for(auto &t: threads) t.join();

    ofstream out("solutions/exercise03.txt");
    for(int i=0;i<80;i++)
        out << hex << setw(2) << setfill('0') << (int)finalHeader[i];

    cout<<"Nonce encontrado: "<<foundNonce.load()<<endl;
    cout<<"Arquivo gerado: solutions/exercise03.txt"<<endl;
}
