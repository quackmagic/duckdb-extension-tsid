#ifndef UUTID_HPP
#define UUTID_HPP

#include <array>
#include <string>
#include <random>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <memory>
#include <cstring>

class UUTID {
public:
    static constexpr size_t SIZE = 16;
    using ByteArray = std::array<uint8_t, SIZE>;

private:
    ByteArray data_;
    static int version_;
    static std::unique_ptr<std::mt19937_64> rng_;
    
    // Base64 encoding table
    static constexpr char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    static std::string bytes_to_hex(const uint8_t* data, size_t len) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            ss << std::setw(2) << static_cast<int>(data[i]);
        }
        return ss.str();
    }

    static void hex_to_bytes(const std::string& hex, uint8_t* output) {
        try {
            for (size_t i = 0; i < hex.length(); i += 2) {
                std::string byteString = hex.substr(i, 2);
                if (!std::all_of(byteString.begin(), byteString.end(), 
                    [](char c) { return std::isxdigit(c); })) {
                    throw std::runtime_error("Invalid hexadecimal character");
                }
                output[i/2] = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
            }
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid hexadecimal string");
        }
    }

public:
    UUTID() : data_() {}

    static void set_rand(std::unique_ptr<std::mt19937_64> new_rng) {
        if (new_rng) {
            rng_ = std::move(new_rng);
        } else {
            rng_.reset(new std::mt19937_64(
                std::chrono::high_resolution_clock::now().time_since_epoch().count()
            ));
        }
    }

    static void set_version(int v) {
        if (v < 0 || v > 9) {
            throw std::runtime_error("version must be a positive integer smaller than 10");
        }
        version_ = v;
    }

    static UUTID new_id() {
        return new_with_time(std::chrono::system_clock::now());
    }

    static UUTID new_with_time(std::chrono::system_clock::time_point t) {
        UUTID id;
        
        auto duration = t.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
        
        uint32_t secs = static_cast<uint32_t>(seconds.count());
        uint32_t nsec = static_cast<uint32_t>(nanos.count());

        nsec = nsec << 2;
        uint16_t ns1 = (nsec >> 16) & 0xffff;
        uint16_t ns2 = nsec & 0xffff;
        ns2 = (ns2 >> 4) & 0x0fff;
        ns2 |= version_ << 12;

        id.data_[0] = (secs >> 24) & 0xFF;
        id.data_[1] = (secs >> 16) & 0xFF;
        id.data_[2] = (secs >> 8) & 0xFF;
        id.data_[3] = secs & 0xFF;
        
        id.data_[4] = (ns1 >> 8) & 0xFF;
        id.data_[5] = ns1 & 0xFF;
        
        id.data_[6] = (ns2 >> 8) & 0xFF;
        id.data_[7] = ns2 & 0xFF;

        std::uniform_int_distribution<uint64_t> dist(0, 255);
        for (size_t i = 8; i < SIZE; ++i) {
            id.data_[i] = static_cast<uint8_t>(dist(*rng_));
        }

        id.data_[8] = (id.data_[8] & 0x3f) | 0x80;

        return id;
    }

    std::string base64() const {
        std::string result;
        result.reserve(22);
        
        for (size_t i = 0; i < SIZE; i += 3) {
            uint32_t n = static_cast<uint32_t>(data_[i]) << 16;
            if (i + 1 < SIZE) n |= static_cast<uint32_t>(data_[i + 1]) << 8;
            if (i + 2 < SIZE) n |= static_cast<uint32_t>(data_[i + 2]);

            result += base64_chars[(n >> 18) & 0x3F];
            result += base64_chars[(n >> 12) & 0x3F];
            if (i + 1 < SIZE) result += base64_chars[(n >> 6) & 0x3F];
            if (i + 2 < SIZE) result += base64_chars[n & 0x3F];
        }
        
        return result;
    }

    static UUTID from_base64(const std::string& b64) {
        if (b64.length() != 22) {
            throw std::runtime_error("Invalid base64 string length");
        }

        auto find_char = [](char c) -> int {
            const char* pos = ::strchr(base64_chars, c);
            if (!pos) throw std::runtime_error("Invalid base64 character");
            return pos - base64_chars;
        };

        UUTID id;
        size_t out_idx = 0;
        
        for (size_t i = 0; i < b64.length(); i += 4) {
            uint32_t n = static_cast<uint32_t>(find_char(b64[i])) << 18;
            n |= static_cast<uint32_t>(find_char(b64[i + 1])) << 12;
            if (i + 2 < b64.length()) n |= static_cast<uint32_t>(find_char(b64[i + 2])) << 6;
            if (i + 3 < b64.length()) n |= static_cast<uint32_t>(find_char(b64[i + 3]));

            if (out_idx < SIZE) id.data_[out_idx++] = (n >> 16) & 0xFF;
            if (out_idx < SIZE) id.data_[out_idx++] = (n >> 8) & 0xFF;
            if (out_idx < SIZE) id.data_[out_idx++] = n & 0xFF;
        }

        return id;
    }

    std::string to_string() const {
        return bytes_to_hex(data_.data(), SIZE);
    }

    std::string to_uuid_string() const {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < 4; ++i)
            ss << std::setw(2) << static_cast<int>(data_[i]);
        ss << '-';
        for (size_t i = 4; i < 6; ++i)
            ss << std::setw(2) << static_cast<int>(data_[i]);
        ss << '-';
        for (size_t i = 6; i < 8; ++i)
            ss << std::setw(2) << static_cast<int>(data_[i]);
        ss << '-';
        for (size_t i = 8; i < 10; ++i)
            ss << std::setw(2) << static_cast<int>(data_[i]);
        ss << '-';
        for (size_t i = 10; i < 16; ++i)
            ss << std::setw(2) << static_cast<int>(data_[i]);
        
        return ss.str();
    }

    static UUTID from_string(const std::string& str) {
        UUTID id;
        if (str.length() == 32) {
            hex_to_bytes(str, id.data_.data());
        } else if (str.length() == 36) {
            std::string clean;
            clean.reserve(32);
            for (char c : str) {
                if (c != '-') clean += c;
            }
            hex_to_bytes(clean, id.data_.data());
        } else {
            throw std::runtime_error("Invalid string length for UUTID");
        }
        return id;
    }

    std::chrono::system_clock::time_point time() const {
        uint32_t sec = (static_cast<uint32_t>(data_[0]) << 24) |
                      (static_cast<uint32_t>(data_[1]) << 16) |
                      (static_cast<uint32_t>(data_[2]) << 8) |
                       static_cast<uint32_t>(data_[3]);

        uint16_t ns1 = (static_cast<uint16_t>(data_[4]) << 8) |
                       static_cast<uint16_t>(data_[5]);
        
        uint16_t ns2 = (static_cast<uint16_t>(data_[6]) << 8) |
                       static_cast<uint16_t>(data_[7]);

        ns2 = ns2 & 0x0fff;
        ns2 = ns2 << 4;
        uint32_t nsec = (static_cast<uint32_t>(ns1) << 16) | ns2;
        nsec = nsec >> 2;

        return std::chrono::system_clock::time_point(
            std::chrono::seconds(sec) + std::chrono::nanoseconds(nsec)
        );
    }

    const ByteArray& bytes() const { return data_; }
    ByteArray& bytes() { return data_; }
};

// Initialize static members
int UUTID::version_ = 4;
std::unique_ptr<std::mt19937_64> UUTID::rng_(new std::mt19937_64(
    std::chrono::high_resolution_clock::now().time_since_epoch().count()
));
constexpr char UUTID::base64_chars[];

#endif // UUTID_HPP
