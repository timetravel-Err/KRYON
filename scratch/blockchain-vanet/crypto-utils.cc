#include "crypto-utils.h"
#include <openssl/hmac.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>

CryptoUtils::KeyPair
CryptoUtils::GenerateKeyPair ()
{
  KeyPair kp;
  EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id (EVP_PKEY_EC, nullptr);
  if (!pctx || EVP_PKEY_keygen_init (pctx) <= 0
      || EVP_PKEY_CTX_set_ec_paramgen_curve_nid (pctx, NID_X9_62_prime256v1) <= 0
      || EVP_PKEY_keygen (pctx, &kp.pkey) <= 0)
    {
      if (pctx) EVP_PKEY_CTX_free (pctx);
      throw std::runtime_error ("ECDSA key generation failed");
    }
  EVP_PKEY_CTX_free (pctx);
  return kp;
}

void
CryptoUtils::FreeKeyPair (KeyPair &kp)
{
  if (kp.pkey)
    {
      EVP_PKEY_free (kp.pkey);
      kp.pkey = nullptr;
    }
}

std::vector<uint8_t>
CryptoUtils::ExportPublicKey (const KeyPair &kp)
{
  EC_KEY *ec = EVP_PKEY_get1_EC_KEY (kp.pkey);
  if (!ec) throw std::runtime_error ("ExportPublicKey: no EC key");
  const EC_POINT *pt = EC_KEY_get0_public_key (ec);
  const EC_GROUP *grp = EC_KEY_get0_group (ec);

  size_t len = EC_POINT_point2oct (grp, pt, POINT_CONVERSION_UNCOMPRESSED,
                                    nullptr, 0, nullptr);
  std::vector<uint8_t> out (len);
  EC_POINT_point2oct (grp, pt, POINT_CONVERSION_UNCOMPRESSED, out.data (), len, nullptr);
  EC_KEY_free (ec);
  return out;
}

EVP_PKEY *
CryptoUtils::ImportPublicKey (const std::vector<uint8_t> &raw)
{
  EC_KEY *ec = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);
  const EC_GROUP *grp = EC_KEY_get0_group (ec);
  EC_POINT *pt = EC_POINT_new (grp);
  if (!EC_POINT_oct2point (grp, pt, raw.data (), raw.size (), nullptr))
    {
      EC_POINT_free (pt);
      EC_KEY_free (ec);
      throw std::runtime_error ("ImportPublicKey: bad point encoding");
    }
  EC_KEY_set_public_key (ec, pt);
  EC_POINT_free (pt);

  EVP_PKEY *pkey = EVP_PKEY_new ();
  EVP_PKEY_set1_EC_KEY (pkey, ec);
  EC_KEY_free (ec);
  return pkey;
}

std::vector<uint8_t>
CryptoUtils::Sha256 (const std::vector<uint8_t> &data)
{
  std::vector<uint8_t> digest (EVP_MAX_MD_SIZE);
  unsigned int outLen = 0;
  EVP_MD_CTX *ctx = EVP_MD_CTX_new ();
  EVP_DigestInit_ex (ctx, EVP_sha256 (), nullptr);
  EVP_DigestUpdate (ctx, data.data (), data.size ());
  EVP_DigestFinal_ex (ctx, digest.data (), &outLen);
  EVP_MD_CTX_free (ctx);
  digest.resize (outLen);
  return digest;
}

std::vector<uint8_t>
CryptoUtils::Sha256 (const std::string &data)
{
  return Sha256 (std::vector<uint8_t> (data.begin (), data.end ()));
}

std::vector<uint8_t>
CryptoUtils::Sign (const KeyPair &kp, const std::vector<uint8_t> &data)
{
  EVP_MD_CTX *ctx = EVP_MD_CTX_new ();
  size_t sigLen = 0;
  std::vector<uint8_t> sig;

  if (EVP_DigestSignInit (ctx, nullptr, EVP_sha256 (), nullptr, kp.pkey) <= 0)
    { EVP_MD_CTX_free (ctx); throw std::runtime_error ("DigestSignInit failed"); }
  if (EVP_DigestSignUpdate (ctx, data.data (), data.size ()) <= 0)
    { EVP_MD_CTX_free (ctx); throw std::runtime_error ("DigestSignUpdate failed"); }
  if (EVP_DigestSignFinal (ctx, nullptr, &sigLen) <= 0)
    { EVP_MD_CTX_free (ctx); throw std::runtime_error ("DigestSignFinal (size) failed"); }
  sig.resize (sigLen);
  if (EVP_DigestSignFinal (ctx, sig.data (), &sigLen) <= 0)
    { EVP_MD_CTX_free (ctx); throw std::runtime_error ("DigestSignFinal failed"); }
  sig.resize (sigLen);
  EVP_MD_CTX_free (ctx);
  return sig;
}

bool
CryptoUtils::Verify (EVP_PKEY *pubKey, const std::vector<uint8_t> &data,
                      const std::vector<uint8_t> &signature)
{
  EVP_MD_CTX *ctx = EVP_MD_CTX_new ();
  bool ok = false;
  if (EVP_DigestVerifyInit (ctx, nullptr, EVP_sha256 (), nullptr, pubKey) > 0)
    {
      int rc = EVP_DigestVerify (ctx, signature.data (), signature.size (),
                                  data.data (), data.size ());
      ok = (rc == 1);
    }
  EVP_MD_CTX_free (ctx);
  return ok;
}

std::vector<uint8_t>
CryptoUtils::HmacSha256 (const std::vector<uint8_t> &key, const std::vector<uint8_t> &data)
{
  unsigned int len = 0;
  std::vector<uint8_t> out (EVP_MAX_MD_SIZE);
  HMAC (EVP_sha256 (), key.data (), static_cast<int> (key.size ()),
        data.data (), data.size (), out.data (), &len);
  out.resize (len);
  return out;
}

std::string
CryptoUtils::ToHex (const std::vector<uint8_t> &data)
{
  std::ostringstream oss;
  oss << std::hex << std::setfill ('0');
  for (uint8_t b : data) oss << std::setw (2) << static_cast<int> (b);
  return oss.str ();
}

std::vector<uint8_t>
CryptoUtils::FromHex (const std::string &hex)
{
  std::vector<uint8_t> out;
  out.reserve (hex.size () / 2);
  for (size_t i = 0; i + 1 < hex.size (); i += 2)
    {
      out.push_back (static_cast<uint8_t> (std::stoi (hex.substr (i, 2), nullptr, 16)));
    }
  return out;
}
