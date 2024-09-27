#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <ctime>

using namespace std;

// RC4 Implementation
class RC4 {
private:
    vector<unsigned char> S;
    int i, j;

public:
    RC4(const vector<unsigned char>& key) {
        S.resize(256);
        for (int i = 0; i < 256; ++i) {
            S[i] = i;
        }

        int j = 0;
        for (int i = 0; i < 256; ++i) {
            j = (j + S[i] + key[i % key.size()]) % 256;
            swap(S[i], S[j]);
        }

        i = 0;
        j = 0;
    }

    unsigned char generate() {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(S[i], S[j]);
        return S[(S[i] + S[j]) % 256];
    }
};

// Function to read image pixels from file
vector<vector<unsigned char>> readImage(const string& filename, int& width, int& height) {
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Error opening file: " + filename);
    }

    vector<vector<unsigned char>> pixels;
    file.seekg(18); // Skip bitmap header
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    file.read(reinterpret_cast<char*>(&height), sizeof(height));
    file.seekg(54); // Skip to pixel data

    pixels.resize(height, vector<unsigned char>(width * 3)); // Assuming 24-bit RGB image

    for (int i = 0; i < height; ++i) {
        file.read(reinterpret_cast<char*>(pixels[i].data()), width * 3);
    }

    return pixels;
}

// Function to write image pixels to file
void writeImage(const string& filename, const vector<vector<unsigned char>>& pixels, int width, int height) {
    ofstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Error opening file for writing: " + filename);
    }

    // Write bitmap header
    int fileSize = 54 + width * height * 3;
    file.write("BM", 2);
    file.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));
    int reserved = 0;
    file.write(reinterpret_cast<const char*>(&reserved), sizeof(reserved));
    int dataOffset = 54;
    file.write(reinterpret_cast<const char*>(&dataOffset), sizeof(dataOffset));
    int headerSize = 40;
    file.write(reinterpret_cast<const char*>(&headerSize), sizeof(headerSize));
    file.write(reinterpret_cast<const char*>(&width), sizeof(width));
    file.write(reinterpret_cast<const char*>(&height), sizeof(height));
    short int planes = 1;
    file.write(reinterpret_cast<const char*>(&planes), sizeof(planes));
    short int bpp = 24; // Bits per pixel
    file.write(reinterpret_cast<const char*>(&bpp), sizeof(bpp));
    int compression = 0;
    file.write(reinterpret_cast<const char*>(&compression), sizeof(compression));
    int imageSize = width * height * 3;
    file.write(reinterpret_cast<const char*>(&imageSize), sizeof(imageSize));
    int xResolution = 2835; // Pixels per meter (72 DPI)
    file.write(reinterpret_cast<const char*>(&xResolution), sizeof(xResolution));
    int yResolution = 2835; // Pixels per meter (72 DPI)
    file.write(reinterpret_cast<const char*>(&yResolution), sizeof(yResolution));
    int colorsUsed = 0;
    file.write(reinterpret_cast<const char*>(&colorsUsed), sizeof(colorsUsed));
    int importantColors = 0;
    file.write(reinterpret_cast<const char*>(&importantColors), sizeof(importantColors));

    // Write pixel data
    for (int i = 0; i < height; ++i) {
        file.write(reinterpret_cast<const char*>(pixels[i].data()), width * 3);
    }
}

int main() {
    // Read image
    string inputFilename = "test_image.bmp";
    int width, height;
    vector<vector<unsigned char>> pixels = readImage(inputFilename, width, height);

    // Encrypt image
    vector<unsigned char> key;
    default_random_engine eng(time(nullptr));
    uniform_int_distribution<unsigned char> dist(0, 255);
    for (int i = 0; i < width * height * 3; ++i) {
        key.push_back(dist(eng));
    }

    RC4 rc4(key);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width * 3; ++j) {
            pixels[i][j] ^= key[(i * width * 3 + j) % key.size()]; // XOR with OTP key
            pixels[i][j] ^= rc4.generate(); // XOR with RC4 generated key
            pixels[i][j] %= 256; // Mod with 256
        }
    }

    // Write encrypted image
    string encryptedFilename = "encrypted.bmp";
    writeImage(encryptedFilename, pixels, width, height);

    // Decrypt image (using same key)
    pixels = readImage(encryptedFilename, width, height);

    rc4 = RC4(key); // Reinitialize RC4 with same key

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width * 3; ++j) {
            pixels[i][j] ^= rc4.generate(); // XOR with RC4 generated key
            pixels[i][j] ^= key[(i * width * 3 + j) % key.size()]; // XOR with OTP key
            pixels[i][j] %= 256; // Mod with 256
        }
    }

    // Write decrypted image
    string decryptedFilename = "decrypted.bmp";
    writeImage(decryptedFilename, pixels, width, height);

    cout << "Encryption and decryption completed successfully." << endl;

    return 0;
}
