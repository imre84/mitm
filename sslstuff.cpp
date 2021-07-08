//the entirety of this file is EXTERNAL CODE from https://stackoverflow.com/questions/256405/programmatically-create-x509-certificate-using-openssl

#include "sslstuff.h"

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <QString>
#include <memory>

bool generateX509(const QString &certFileName, const QString &keyFileName, long daysValid, unsigned int length_in_bits)
{
    std::unique_ptr<BIO, void (*)(BIO *)> certFile  { BIO_new_file(certFileName.toLocal8Bit(), "wb"), BIO_free_all  };
    if(!certFile) return false;
    std::unique_ptr<BIO, void (*)(BIO *)> keyFile { BIO_new_file(keyFileName.toLocal8Bit(), "wb"), BIO_free_all };
    if(!keyFile) return false;

    std::unique_ptr<RSA, void (*)(RSA *)> rsa { RSA_new(), RSA_free };
    std::unique_ptr<BIGNUM, void (*)(BIGNUM *)> bn { BN_new(), BN_free };

    BN_set_word(bn.get(), RSA_F4);
    int rsa_ok = RSA_generate_key_ex(rsa.get(), length_in_bits, bn.get(), nullptr);

    if (rsa_ok != 1) return false;

    // --- cert generation ---
    std::unique_ptr<X509, void (*)(X509 *)> cert { X509_new(), X509_free };
    std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> pkey { EVP_PKEY_new(), EVP_PKEY_free};

    // The RSA structure will be automatically freed when the EVP_PKEY structure is freed.
    EVP_PKEY_assign(pkey.get(), EVP_PKEY_RSA, reinterpret_cast<char*>(rsa.release()));
    ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1); // serial number

    X509_gmtime_adj(X509_get_notBefore(cert.get()), 0); // now
    X509_gmtime_adj(X509_get_notAfter(cert.get()), daysValid * 24 * 60 * 60); // accepts secs

    X509_set_pubkey(cert.get(), pkey.get());

    // 1 -- X509_NAME may disambig with wincrypt.h
    // 2 -- DO NO FREE the name internal pointer
    X509_name_st *name = X509_get_subject_name(cert.get());

    const uchar country[] = "HU";
    const uchar company[] = "MyCompany, PLC";
    const uchar common_name[] = "localhost";

    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, country    , -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, company    , -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, common_name, -1, -1, 0);

    X509_set_issuer_name(cert.get(), name);
    X509_sign(cert.get(), pkey.get(), EVP_sha256()); // some hash type here

    int ret  = PEM_write_bio_PrivateKey(keyFile.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr);
    int ret2 = PEM_write_bio_X509(certFile.get(), cert.get());

    return (ret == 1) && (ret2 == 1); // OpenSSL return codes
}

bool genkey(const QString &cn,const QString &ca,const QString &certFileName, const QString &keyFileName, long daysValid, unsigned int length_in_bits)
{
    std::unique_ptr<BIO, void (*)(BIO *)> cacertFile  { BIO_new_file((ca+"/cert").toLocal8Bit(), "r"), BIO_free_all  };
    if(!cacertFile) return false;
    std::unique_ptr<BIO, void (*)(BIO *)> cakeyFile { BIO_new_file((ca+"/key").toLocal8Bit(), "r"), BIO_free_all };
    if(!cakeyFile) return false;
    std::unique_ptr<BIO, void (*)(BIO *)> certFile  { BIO_new_file(certFileName.toLocal8Bit(), "wb"), BIO_free_all  };
    if(!certFile) return false;
    std::unique_ptr<BIO, void (*)(BIO *)> keyFile { BIO_new_file(keyFileName.toLocal8Bit(), "wb"), BIO_free_all };
    if(!keyFile) return false;

    std::unique_ptr<RSA, void (*)(RSA *)> rsa { RSA_new(), RSA_free };
    std::unique_ptr<BIGNUM, void (*)(BIGNUM *)> bn { BN_new(), BN_free };

    BN_set_word(bn.get(), RSA_F4);
    int rsa_ok = RSA_generate_key_ex(rsa.get(), length_in_bits, bn.get(), nullptr);

    if (rsa_ok != 1) return false;

    // --- reading cacert from disk ---
    std::unique_ptr<X509, void (*)(X509 *)> cacert { PEM_read_bio_X509(cacertFile.get(),nullptr,nullptr,nullptr), X509_free };

    // --- reading cakey from disk ---
    std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> capkey { PEM_read_bio_PrivateKey(cakeyFile.get(),nullptr,nullptr,nullptr), EVP_PKEY_free};

    // --- cert generation ---
    std::unique_ptr<X509, void (*)(X509 *)> cert { X509_new(), X509_free };
    std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> pkey { EVP_PKEY_new(), EVP_PKEY_free};

    // The RSA structure will be automatically freed when the EVP_PKEY structure is freed.
    EVP_PKEY_assign(pkey.get(), EVP_PKEY_RSA, reinterpret_cast<char*>(rsa.release()));
    ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1); // serial number

    X509_gmtime_adj(X509_get_notBefore(cert.get()), 0); // now
    X509_gmtime_adj(X509_get_notAfter(cert.get()), daysValid * 24 * 60 * 60); // accepts secs

    X509_set_pubkey(cert.get(), pkey.get());

    // 1 -- X509_NAME may disambig with wincrypt.h
    // 2 -- DO NO FREE the name internal pointer
    X509_name_st *name = X509_get_subject_name(cert.get());

    const uchar country[] = "HU";
    const uchar company[] = "MyCompany, PLC";
    const uchar *common_name = reinterpret_cast<const uchar *>(cn.toUtf8().constData());

    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, country    , -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, company    , -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, common_name, -1, -1, 0);

    X509_set_issuer_name(cacert.get(), name);
    X509_sign(cert.get(), capkey.get(), EVP_sha256()); // some hash type here

    int ret  = PEM_write_bio_PrivateKey(keyFile.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr);
    int ret2 = PEM_write_bio_X509(certFile.get(), cert.get());

    return (ret == 1) && (ret2 == 1); // OpenSSL return codes
}
