#include "sslstuff.h"

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <QString>
#include <memory>

#define SSL_CODE(x) do{ x }while(0)
#define SSL_CHECKRET_PTR(x) SSL_CODE(if(!(x)) return false;)
#define SSL_CHECKRET_LEN(x) SSL_CODE(if((x)<=0) return false;)
#define SSL_CHECKRET_INT1(x) SSL_CODE(if((x)!=1) return false;)
#define SSL_SMARTFILE(varname,qsfn,mode) std::unique_ptr<BIO, void (*)(BIO *)> varname { BIO_new_file((qsfn).toLocal8Bit(), (mode)), BIO_free_all  }; SSL_CHECKRET_PTR(varname)
#define SSL_SMARTPTR_VAL4(type,varname,val,pfx) std::unique_ptr<type, void (*)(type *)> varname { val, pfx ## _free }; SSL_CHECKRET_PTR(varname)
#define SSL_SMARTPTR_VAL(type,varname,val) SSL_SMARTPTR_VAL4(type,varname,val,type);
#define SSL_SMARTPTR_NEW3(type,varname,pfx) SSL_SMARTPTR_VAL4(type,varname,pfx ## _new(),pfx)
#define SSL_SMARTPTR_NEW(type,varname) SSL_SMARTPTR_NEW3(type,varname,type)
#define SSL_SMARTPTR_READ(type,varname,file) SSL_SMARTPTR_VAL(type,varname,PEM_read_bio_ ## type(file.get(),nullptr,nullptr,nullptr))

BIGNUM *BIGNUM_new() { return BN_new(); }
void BIGNUM_free(BIGNUM *ptr) { return BN_free(ptr); }

//the following routine is based on EXTERNAL CODE from https://stackoverflow.com/questions/256405/programmatically-create-x509-certificate-using-openssl
bool generateX509(const QString &certFileName, const QString &keyFileName, long daysValid, unsigned int length_in_bits)
{
    SSL_SMARTFILE(certFile,certFileName,"wb");
    SSL_SMARTFILE(keyFile,keyFileName,"wb");

    SSL_SMARTPTR_NEW(RSA,rsa);
    SSL_SMARTPTR_NEW(BIGNUM,bn);

    SSL_CHECKRET_INT1(BN_set_word(bn.get(), RSA_F4));
    SSL_CHECKRET_INT1(RSA_generate_key_ex(rsa.get(), length_in_bits, bn.get(), nullptr));

    // --- cert generation ---
    SSL_SMARTPTR_NEW(X509,cert);
    SSL_SMARTPTR_NEW(EVP_PKEY,pkey);

    // The RSA structure will be automatically freed when the EVP_PKEY structure is freed.
    SSL_CHECKRET_INT1(EVP_PKEY_assign(pkey.get(), EVP_PKEY_RSA, reinterpret_cast<char*>(rsa.release())));
    SSL_CHECKRET_INT1(ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1)); // serial number

    SSL_CHECKRET_PTR(X509_gmtime_adj(X509_get_notBefore(cert.get()), 0                       )); // now
    SSL_CHECKRET_PTR(X509_gmtime_adj(X509_get_notAfter (cert.get()), daysValid * 24 * 60 * 60)); // accepts secs

    SSL_CHECKRET_INT1(X509_set_pubkey(cert.get(), pkey.get()));

    // 1 -- X509_NAME may disambig with wincrypt.h
    // 2 -- DO NO FREE the name internal pointer
    X509_name_st *name = X509_get_subject_name(cert.get());
    SSL_CHECKRET_PTR(name);

    static const uchar country[] = "HU";
    static const uchar company[] = "MyCompany, PLC";
    static const uchar rootca_cn[] = "root ca";

    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(name, "C" , MBSTRING_ASC, country  , -1, -1, 0));
    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(name, "O" , MBSTRING_ASC, company  , -1, -1, 0));
    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, rootca_cn, -1, -1, 0));

    SSL_CHECKRET_INT1(X509_set_issuer_name(cert.get(), name));
    SSL_CHECKRET_LEN(X509_sign(cert.get(), pkey.get(), EVP_sha256())); // some hash type here

    SSL_CHECKRET_INT1(PEM_write_bio_PrivateKey(keyFile.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr));
    SSL_CHECKRET_INT1(PEM_write_bio_X509(certFile.get(), cert.get()));

    return true;
}

//EXTERNAL CODE: the following routine is partially based on the above function
bool genkey(const QString &fqdn,const QString &ca,const QString &certFileName, const QString &keyFileName, long serial, long daysValid, unsigned int length_in_bits)
{
    SSL_SMARTFILE(cacertFile,ca+"/cert","r");
    SSL_SMARTFILE(cakeyFile,ca+"/key","r");
    SSL_SMARTFILE(certFile,certFileName,"wb");
    SSL_SMARTFILE(keyFile,keyFileName,"wb");

    SSL_SMARTPTR_NEW(RSA,rsa);
    SSL_SMARTPTR_NEW3(BIGNUM,bn,BN);

    SSL_CHECKRET_INT1(BN_set_word(bn.get(), RSA_F4));
    SSL_CHECKRET_INT1(RSA_generate_key_ex(rsa.get(), length_in_bits, bn.get(), nullptr));

    // --- reading cacert from disk ---
    SSL_SMARTPTR_READ(X509,cacert,cacertFile);

    // --- reading cakey from disk ---
    SSL_SMARTPTR_VAL(EVP_PKEY,capkey,PEM_read_bio_PrivateKey(cakeyFile.get(),nullptr,nullptr,nullptr));

    // --- csr generation ---
    //EXTERNAL CODE from https://www.dynamsoft.com/codepool/how-to-use-openssl-to-generate-x-509-certificate-request.html

    SSL_SMARTPTR_NEW(X509_REQ,x509_req);
    SSL_CHECKRET_INT1(X509_REQ_set_version(x509_req.get(), 1));
    X509_name_st *x509_name = X509_REQ_get_subject_name(x509_req.get());
    SSL_CHECKRET_PTR(x509_name);

    static const uchar country[] = "HU";
    static const uchar province[] = "Budapest";
    static const uchar city[] = "Budapest";
    static const uchar company[] = "MyCompany, PLC";
    static const uchar rootca_cn[] = "root ca";
           const uchar *cn = reinterpret_cast<const uchar *>(fqdn.toLocal8Bit().constData());

    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(x509_name, "C" , MBSTRING_ASC, country , -1, -1, 0));
    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(x509_name, "ST", MBSTRING_ASC, province, -1, -1, 0));
    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(x509_name, "L" , MBSTRING_ASC, city    , -1, -1, 0));
    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(x509_name, "O" , MBSTRING_ASC, company , -1, -1, 0));
    SSL_CHECKRET_INT1(X509_NAME_add_entry_by_txt(x509_name, "CN", MBSTRING_ASC, cn      , -1, -1, 0));

    // 4. set public key of x509 req
    SSL_SMARTPTR_NEW(EVP_PKEY,pKey);
    SSL_CHECKRET_INT1(EVP_PKEY_assign_RSA(pKey.get(), rsa.release()));
    // 5. set sign key of x509 req
    SSL_CHECKRET_LEN(X509_REQ_sign(x509_req.get(),pKey.get(),EVP_sha256()));
    //todo the x509_req isn't used, it could just be omitted

    //EXTERNAL CODE https://stackoverflow.com/questions/26658846/c-code-for-ceating-x509-certificate-and-verify-it

    // --- cert generation ---
    SSL_SMARTPTR_NEW(X509,m_req_reply);
    SSL_CHECKRET_INT1(ASN1_INTEGER_set(X509_get_serialNumber(m_req_reply.get()), serial));
    SSL_CHECKRET_PTR(X509_gmtime_adj(X509_get_notBefore(m_req_reply.get()), 0                       )); // now
    SSL_CHECKRET_PTR(X509_gmtime_adj(X509_get_notAfter (m_req_reply.get()), daysValid * 24 * 60 * 60)); // accepts secs
    SSL_CHECKRET_INT1(X509_set_pubkey(m_req_reply.get(), pKey.get()));
    X509_NAME *issuerSubject = X509_get_subject_name(cacert.get());
    SSL_CHECKRET_PTR(issuerSubject);
    SSL_CHECKRET_INT1(X509_set_issuer_name(m_req_reply.get(), issuerSubject));
    X509_set_subject_name(m_req_reply.get(), x509_name);
    SSL_CHECKRET_LEN(X509_sign(m_req_reply.get(),capkey.get(),EVP_sha256()));

    SSL_CHECKRET_INT1(PEM_write_bio_PrivateKey(keyFile.get(), pKey.get(), nullptr, nullptr, 0, nullptr, nullptr));
    SSL_CHECKRET_INT1(PEM_write_bio_X509(certFile.get(),m_req_reply.get()));

    return true;
}
