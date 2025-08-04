#include "crypto.hpp"

#include <iostream>
#include <stdexcept>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include <openssl/x509.h>
#include <openssl/md5.h>
#include <openssl/crypto.h>

#define w_start void* temp_ptr = nullptr; int temp_int = 0; unsigned int temp_uint = 0; \
    (void)temp_ptr; (void)temp_int; (void)temp_uint;

#define w_ptr(a) \
    reinterpret_cast<decltype(a)>(temp_ptr = reinterpret_cast<void*>(a)); \
    if (!temp_ptr) { check_error(); } \
    temp_ptr = nullptr

#define w_int(a)  (temp_int  = a); if (temp_int  < 0) { check_error(); } temp_int  = 0
#define w_int1(a) (temp_int  = a); if (temp_int != 1) { check_error(); } temp_int  = 0
#define w_uint(a) (temp_uint = a); if (!temp_uint)    { check_error(); } temp_uint = 0

void check_error();
inline int calc_base64_size(int data_length);

Crypto
Crypto::from_key(const std::string& key) {
    w_start;

    auto* bio = w_ptr(BIO_new_mem_buf(key.c_str(), key.size()));
    auto* keyy = w_ptr(PEM_read_bio_RSAPrivateKey(
        bio    ,
        nullptr,
        nullptr,
        nullptr
    ));

    EVP_PKEY* pkey = w_ptr(EVP_PKEY_new());

    w_int(EVP_PKEY_assign_RSA(pkey, keyy));

    return { pkey, bio };
}

Crypto
Crypto::from_cert(const std::string& cert_str) {
    w_start;

    auto*  bio = w_ptr(BIO_new_mem_buf(cert_str.c_str(), cert_str.size()));
    auto* cert = w_ptr(PEM_read_bio_X509(bio, nullptr, nullptr, nullptr));
    auto* pkey = w_ptr(X509_get_pubkey(cert));

    return { pkey, bio };
}

Crypto::Crypto(EVP_PKEY* pkey, BIO* bio) : pkey(pkey), bio(bio) {
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();
}

Crypto::~Crypto() {
    EVP_PKEY_free(pkey);
    BIO_free(bio);
}

Crypto::Data
Crypto::private_encrypt(const Data& data) const {
    w_start;

    auto* rsa = w_ptr(EVP_PKEY_get1_RSA(pkey));

    size_t rsa_size  = RSA_size(rsa);
    size_t data_size = data.size;
    size_t size = 0;

    unsigned char buff[(data_size / (rsa_size - RSA_PKCS1_PADDING_SIZE) + 1) * rsa_size];

    for (size_t i = 0; i < data_size; i += rsa_size - RSA_PKCS1_PADDING_SIZE) {
        size += w_int(RSA_private_encrypt(
            std::min<size_t>(rsa_size - RSA_PKCS1_PADDING_SIZE, data_size),
            data.bytes + i,
            buff + size,
            rsa,
            RSA_PKCS1_PADDING
        ));
    }

    return Data(buff, size, true);
}

Crypto::Data
Crypto::public_decrypt(const Data& data) const {
    w_start

    auto* rsa = (RSA*)w_ptr(EVP_PKEY_get1_RSA(pkey));

    size_t rsa_size  = RSA_size(rsa);
    size_t data_size = data.size;
    size_t size = 0;

    unsigned char buff[data_size];

    for (size_t i = 0; i < data_size; i += rsa_size) {
        size += w_int(RSA_public_decrypt(
            rsa_size,
            data.bytes + i,
            buff + size,
            rsa,
            RSA_PKCS1_PADDING
        ));
    }

    return Data(buff, size, true);
}

void
check_error() {
    unsigned long errNum = ERR_get_error();
    char err[512];

    if (errNum > 0) {
        ERR_error_string(errNum, err);
    } else {
        sprintf(err, "unknown error");
    }

    throw std::runtime_error(std::string("Crypto error! ") + err);
}

std::string
Crypto::to_base64(Data data) {
    w_start;

    const int str_length = calc_base64_size(data.size) + 1;

    char string[str_length];

    w_int(EVP_EncodeBlock((unsigned char*) string, data.bytes, data.size));

    return string;
}

int
Crypto::from_base64(const std::string& string, unsigned char* output, int max_size) {
    if ((string.size() / 4) * 3 > max_size) {
        throw std::runtime_error("Too small buffer provided");
    }

    w_start;

    auto* evp_ctx = w_ptr(EVP_ENCODE_CTX_new());

    EVP_DecodeInit(evp_ctx);

    int size = 0;

    w_int(EVP_DecodeUpdate(evp_ctx, output, &size, (unsigned char*) string.c_str(), string.size()));

    int size2 = 0;

    w_int1(EVP_DecodeFinal(evp_ctx, output + size, &size2));

    if (size + size2 > max_size) {
        throw std::runtime_error("Oops ¯\\_(ツ)_/¯");
    }

    EVP_ENCODE_CTX_free(evp_ctx);

    return size + size2;
}

inline int
calc_base64_size(int data_length) {
    // Magic happens here
    return ((4 * data_length / 3) + 3) & ~3;
}

Crypto::Data
Crypto::md5(const Data& data) {
    const size_t size = MD5_DIGEST_LENGTH;
    unsigned char buff[size];

    MD5(data.bytes, data.size, buff);

    return Data(buff, size, true);
}

std::string
Crypto::to_hex(const Data &data) {
    char buff[data.size * 2 + 1];

    int i;

    for (i = 0 ; i < data.size; i++) {
        sprintf(buff + i * 2, "%02x", data.bytes[i]);
    }

    buff[i * 2] = '\0';

    return std::string(buff);
}
