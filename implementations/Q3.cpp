#include <iostream>
#include <fstream>
#include <iomanip>
#include <openssl/sha.h>
#include <thread>
#include <atomic>
#include <vector>
#include <cstdint>
#include <cstring>

using namespace std;

// Convert hex string to bytes
void hexToBytes(const char* hex, unsigned char* bytes, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
}

// SHA256 duplo
inline void sha256d(const unsigned char* data, size_t len, unsigned char* out_hash) {
    unsigned char tmp[SHA256_DIGEST_LENGTH];
    SHA256(data, len, tmp);
    SHA256(tmp, SHA256_DIGEST_LENGTH, out_hash);
}

// uint32 little-endian
inline void uint32ToLE(uint32_t val, unsigned char* buf) {
    buf[0] = val & 0xFF;
    buf[1] = (val >> 8) & 0xFF;
    buf[2] = (val >> 16) & 0xFF;
    buf[3] = (val >> 24) & 0xFF;
}

// Compare hash with target (BE)
bool hashLETarget(const unsigned char* hash, const unsigned char* target) {
    for (int i = 31; i >= 0; --i) {
        if (hash[i] < target[i]) return true;
        if (hash[i] > target[i]) return false;
    }
    return true; // equal
}

// Shared flag for threads
atomic<bool> found(false);
atomic<uint32_t> foundNonce(0);
unsigned char finalHeader[80];

void mineRange(unsigned char baseHeader[80], uint32_t start, uint32_t step,
               const unsigned char* target) {
    unsigned char header[80];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    memcpy(header, baseHeader, 80);

    for (uint32_t nonce = start; nonce <= 0xffffffff; nonce += step) {
        if (found.load()) return;

        uint32ToLE(nonce, header + 76);
        sha256d(header, 80, hash);

        if (hashLETarget(hash, target)) {
            memcpy(finalHeader, header, 80);
            foundNonce = nonce;
            found = true;
            return;
        }
    }
}

int main() {
    const uint32_t version = 2;
    const char* prev_block_hex = "00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee";
    const char* merkle_root_hex = "c0a692de10b69e2381a2856dcb0d0736dcd307bf25af7ce74831bf25793de626";
    const uint32_t timestamp = 1231006505;
    const uint32_t nbits_val = 0x1d00ffff;

    unsigned char header[80];
    uint32ToLE(version, header);
    hexToBytes(prev_block_hex, header + 4, 32);
    hexToBytes(merkle_root_hex, header + 36, 32);
    uint32ToLE(timestamp, header + 68);
    uint32ToLE(nbits_val, header + 72);

    // Target 0x1d00ffff -> initial Bitcoin difficulty
    unsigned char target[32] = {0};
    target[31] = 0xff;
    target[30] = 0xff;
    target[29] = 0x00;
    target[28] = 0x00;
    // remaining bytes are 0 by default

    unsigned int nThreads = thread::hardware_concurrency();
    if (nThreads == 0) nThreads = 4; // fallback
    cout << "Minerando com " << nThreads << " threads..." << endl;

    vector<thread> threads;
    for (unsigned int i = 0; i < nThreads; ++i) {
        threads.emplace_back(mineRange, header, i, nThreads, target);
    }

    for (auto& t : threads) t.join();

    if (found.load()) {
        cout << "Bloco minerado! Nonce: " << foundNonce.load() << endl;
        ofstream outfile("solutions/exercise03.txt");
        for (int i = 0; i < 80; ++i)
            outfile << hex << setw(2) << setfill('0') << (int)finalHeader[i];
        cout << "Arquivo gerado: solutions/exercise03.txt" << endl;
    } else {
        cout << "Nonce válido não encontrado." << endl;
    }

    return 0;
}
