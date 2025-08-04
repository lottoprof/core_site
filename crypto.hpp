#pragma once

#include <cstring>
#include <string>
#include <cstdint>

#include <openssl/evp.h>
#include <openssl/bio.h>

class Crypto {
public:
    using Data = struct _Data {
        const unsigned char* bytes;
        int size;

        _Data() = default;
        _Data(const unsigned char* bytes, int size, bool copy = false) : bytes(bytes), size(size) {
            if (copy) {
                owns_data = true;

                _Data::bytes = new unsigned char[size];

                std::memcpy((void*) _Data::bytes, (void*) bytes, size);
            }
        };

        ~_Data() {
            if (owns_data) {
                // delete [] bytes;
            }
        }

    private:
        bool owns_data = false;
    };

    static Crypto from_key (const std::string& key);
    static Crypto from_cert(const std::string& cert);

    Crypto(const Crypto& sign_ed) = delete;
    ~Crypto();

    Data private_encrypt(const Data& data) const;
    Data public_decrypt(const Data& data) const;

    static std::string to_base64(Data data);
    static int from_base64(const std::string& string, unsigned char* output, int max_size);

    static std::string to_hex(const Data &data);

    static Data md5(const Data& data);

private:
    EVP_PKEY* pkey;
    BIO* bio;

    Crypto(EVP_PKEY* pkey, BIO* bio);
};

