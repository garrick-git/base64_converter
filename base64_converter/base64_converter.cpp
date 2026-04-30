#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <iomanip>

const std::string BASE64_CHARS =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

void logMessage(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    struct tm time_info;
    localtime_s(&time_info, &time_t);
    std::cout << "[LOG] " << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S")
        << " - " << message << std::endl;
}

// Фильтр и базовая проверка: оставляем только символы Base64
std::string filterBase64(const std::string& input) {
    std::string result;
    for (char c : input) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '+' || c == '/' || c == '=') {
            result += c;
        }
    }
    return result;
}

// Надёжное декодирование Base64 (битовый алгоритм)
std::vector<char> decodeBase64(const std::string& encoded) {
    std::vector<char> decoded;
    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (c == '=') {
            if (valb == -8) break;
            if (valb > 0) {
                decoded.push_back(static_cast<char>((val >> (valb - 8)) & 0xFF));
            }
            break;
        }
        int index;
        if (c >= 'A' && c <= 'Z') index = c - 'A';
        else if (c >= 'a' && c <= 'z') index = c - 'a' + 26;
        else if (c >= '0' && c <= '9') index = c - '0' + 52;
        else if (c == '+') index = 62;
        else if (c == '/') index = 63;
        else continue; // на всякий случай, но не должно быть

        val = (val << 6) | index;
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(static_cast<char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return decoded;
}

std::string encodeBase64(const std::vector<char>& data) {
    std::string encoded;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (char c : data) {
        char_array_3[i++] = static_cast<unsigned char>(c);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                encoded += BASE64_CHARS[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; j < i + 1; j++)
            encoded += BASE64_CHARS[char_array_4[j]];

        while (i++ < 3)
            encoded += '=';
    }
    return encoded;
}

void encodeFile(const std::string& inputFile, const std::string& outputFile) {
    logMessage("Encoding: " + inputFile);
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input file: " + inputFile);
    in.seekg(0, std::ios::end);
    size_t fileSize = in.tellg();
    in.seekg(0, std::ios::beg);
    logMessage("Input size: " + std::to_string(fileSize) + " bytes");

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot create output file: " + outputFile);

    const size_t bufferSize = 3 * 1024;
    std::vector<char> buffer(bufferSize);
    size_t processed = 0;
    while (in.read(buffer.data(), bufferSize) || in.gcount() > 0) {
        size_t bytes = in.gcount();
        buffer.resize(bytes);
        std::string chunk = encodeBase64(buffer);
        out << chunk;
        processed += bytes;
        buffer.resize(bufferSize);
        if (fileSize > 0 && (processed * 100 / fileSize) % 10 == 0 && processed != fileSize) {
            logMessage("Progress: " + std::to_string(processed * 100 / fileSize) + "%");
        }
    }
    in.close(); out.close();
    logMessage("Encoding done -> " + outputFile);
}

void decodeFile(const std::string& inputFile, const std::string& outputFile) {
    logMessage("Decoding: " + inputFile);
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input file: " + inputFile);

    std::string raw((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    in.close();
    logMessage("Read " + std::to_string(raw.size()) + " bytes");

    std::string clean = filterBase64(raw);
    logMessage("After Base64 filter: " + std::to_string(clean.size()) + " chars");

    if (clean.empty()) throw std::runtime_error("No valid Base64 characters found");

    std::vector<char> decoded = decodeBase64(clean);
    logMessage("Decoded: " + std::to_string(decoded.size()) + " bytes");

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot create output file: " + outputFile);
    out.write(decoded.data(), decoded.size());
    out.close();
    logMessage("Decoding done -> " + outputFile);
}

void printUsage(const char* prog) {
    std::cerr << "Usage:\n  " << prog << " encode <in> <out>\n  " << prog << " decode <in> <out>\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }
    std::string mode = argv[1], input = argv[2], output = argv[3];
    try {
        if (mode == "encode") {
            encodeFile(input, output);
            std::cout << "Encoding successful.\n";
        }
        else if (mode == "decode") {
            decodeFile(input, output);
            std::cout << "Decoding successful.\n";
        }
        else {
            std::cerr << "Unknown mode: " << mode << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}