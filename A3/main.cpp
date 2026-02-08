#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <set>
#include <cmath>
#include <iomanip>

class RandomStreamGen {
private:
    std::mt19937 gen;
    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";

public:
    RandomStreamGen(int seed) : gen(seed) {}

    std::string genString() {
        int len = std::uniform_int_distribution<>(1, 30)(gen);
        std::string s = "";
        for (int i = 0; i < len; ++i) {
            s += charset[std::uniform_int_distribution<>(0, charset.size() - 1)(gen)];
        }
        return s;
    }

    std::vector<std::string> getSubStream(double percent, int total) {
        int cnt = static_cast<int>(total * percent);
        std::vector<std::string> sub;
        for (int i = 0; i < cnt; ++i) {
            sub.push_back(genString());
        }
        return sub;
    }
};

class HashFuncGen {
private:
    uint64_t C; 

public:
    HashFuncGen(int seed) {
        std::mt19937_64 rng(seed);
        C = rng() | 1;
    }

    uint32_t hash(const std::string& s) {
        uint64_t h = 0;
        for (unsigned char ch : s) {
            h = h * C + ch;
        }
        h ^= (h >> 16);
        h *= 0x85ebca6b;
        h ^= (h >> 13);
        h *= 0xc2b2ae35;
        h ^= (h >> 16);
        return (uint32_t)h;
    }
};

class HyperLogLog {
private:
    int b, m;
    std::vector<uint32_t> M;
    double alpha_m;

public:
    HyperLogLog(int precision) : b(precision) {
        m = 1 << b;
        M.assign(m, 0);
        switch (m) {
            case 16: alpha_m = 0.673; break;
            case 32: alpha_m = 0.697; break;
            case 64: alpha_m = 0.709; break;
            default: alpha_m = 0.7213 / (1.0 + 1.079 / m); break;
        }
    }

    void add(uint32_t x) {
        uint32_t j = x >> (32 - b);
        uint32_t rank = __builtin_clz(x << b) + 1;
        M[j] = std::max(M[j], rank);
    }

    double estimate() {
        double Z = 0.0;
        for (uint32_t v : M) {
            Z += std::pow(2.0, -static_cast<double>(v));
        }
        double E = alpha_m * m * m / Z;
        if (E <= 2.5 * m) {
            int V = 0;
            for (uint32_t v : M) {
                if (v == 0) V++;
            }
            if (V > 0) {
                return m * std::log((double)m / V);
            }
        }
        return E;
    }
};

int main() {
    int total_elements = 100000;
    double step_percent = 0.05; 
    
    RandomStreamGen streamGen(52);
    HashFuncGen hashFunc(812);
    HyperLogLog hll(14); 
    std::set<std::string> unique; 

    std::cout << "Percent,Real_F0,HLL_Estimate,Error_%" << std::endl;
    for (int i = 1; i <= total_elements / (int)(total_elements * step_percent); ++i) {
        std::vector<std::string> sub = streamGen.getSubStream(step_percent, total_elements);
        for (auto& s : sub) {
            hll.add(hashFunc.hash(s));
            unique.insert(s);
        }
        double real_f0= static_cast<double>(unique.size());
        double hll_estimate = hll.estimate();
        double error = std::abs(hll_estimate - real_f0) / real_f0 * 100.0;
        std::cout << (i * 100 * step_percent) << "%," << real_f0 << "," << hll_estimate << ","  << error << "%" << std::endl;
    }
    return 0;
}