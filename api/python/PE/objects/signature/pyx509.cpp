/* Copyright 2017 R. Thomas
 * Copyright 2017 Quarkslab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string>
#include <sstream>

#include "enums_wrapper.hpp"

#include "LIEF/PE/hash.hpp"
#include "LIEF/PE/signature/x509.hpp"
#include "LIEF/PE/signature/RsaInfo.hpp"

#include "pyPE.hpp"

namespace LIEF {
namespace PE {

template<class T>
using getter_t = T (x509::*)(void) const;

template<class T>
using setter_t = void (x509::*)(T);


template<>
void create<x509>(py::module& m) {

  py::class_<x509, LIEF::Object> cls_x509(m, "x509", "Interface over a x509 certificate");

  LIEF::enum_<x509::VERIFICATION_FLAGS>(cls_x509, "VERIFICATION_FLAGS", py::arithmetic(),
      "Verification flags associated with " RST_METH_REF(lief.PE.x509.verify) "")
    .value("OK",                    x509::VERIFICATION_FLAGS::OK,                    "The verification succeed")
    .value("BADCERT_EXPIRED",       x509::VERIFICATION_FLAGS::BADCERT_EXPIRED,       "The certificate validity has expired")
    .value("BADCERT_REVOKED",       x509::VERIFICATION_FLAGS::BADCERT_REVOKED,       "The certificate has been revoked (is on a CRL)")
    .value("BADCERT_CN_MISMATCH",   x509::VERIFICATION_FLAGS::BADCERT_CN_MISMATCH,   "The certificate Common Name (CN) does not match with the expected CN.")
    .value("BADCERT_NOT_TRUSTED",   x509::VERIFICATION_FLAGS::BADCERT_NOT_TRUSTED,   "The certificate is not correctly signed by the trusted CA.")
    .value("BADCRL_NOT_TRUSTED",    x509::VERIFICATION_FLAGS::BADCRL_NOT_TRUSTED,    "The CRL is not correctly signed by the trusted CA.")
    .value("BADCRL_EXPIRED",        x509::VERIFICATION_FLAGS::BADCRL_EXPIRED,        "The CRL is expired.")
    .value("BADCERT_MISSING",       x509::VERIFICATION_FLAGS::BADCERT_MISSING,       "Certificate was missing.")
    .value("BADCERT_SKIP_VERIFY",   x509::VERIFICATION_FLAGS::BADCERT_SKIP_VERIFY,   "Certificate verification was skipped.")
    .value("BADCERT_OTHERNATURE",   x509::VERIFICATION_FLAGS::BADCERT_OTHER,         "Other reason")
    .value("BADCERT_FUTURE",        x509::VERIFICATION_FLAGS::BADCERT_FUTURE,        "The certificate validity starts in the future.")
    .value("BADCRL_FUTURE",         x509::VERIFICATION_FLAGS::BADCRL_FUTURE,         "The CRL is from the future")
    .value("BADCERT_KEY_USAGE",     x509::VERIFICATION_FLAGS::BADCERT_KEY_USAGE,     "Usage does not match the keyUsage extension.")
    .value("BADCERT_EXT_KEY_USAGE", x509::VERIFICATION_FLAGS::BADCERT_EXT_KEY_USAGE, "Usage does not match the extendedKeyUsage extension.")
    .value("BADCERT_NS_CERT_TYPE",  x509::VERIFICATION_FLAGS::BADCERT_NS_CERT_TYPE,  "Usage does not match the nsCertType extension.")
    .value("BADCERT_BAD_MD",        x509::VERIFICATION_FLAGS::BADCERT_BAD_MD,        "The certificate is signed with an unacceptable hash.")
    .value("BADCERT_BAD_PK",        x509::VERIFICATION_FLAGS::BADCERT_BAD_PK,        "The certificate is signed with an unacceptable PK alg (eg RSA vs ECDSA).")
    .value("BADCERT_BAD_KEY",       x509::VERIFICATION_FLAGS::BADCERT_BAD_KEY,       "The certificate is signed with an unacceptable key (eg bad curve, RSA too short).")
    .value("BADCRL_BAD_MD",         x509::VERIFICATION_FLAGS::BADCRL_BAD_MD,         "The CRL is signed with an unacceptable hash.")
    .value("BADCRL_BAD_PK",         x509::VERIFICATION_FLAGS::BADCRL_BAD_PK,         "The CRL is signed with an unacceptable PK alg (eg RSA vs ECDSA).")
    .value("BADCRL_BAD_KEY",        x509::VERIFICATION_FLAGS::BADCRL_BAD_KEY,        "The CRL is signed with an unacceptable key (eg bad curve, RSA too short).");

  LIEF::enum_<x509::KEY_TYPES>(cls_x509, "KEY_TYPES", "Public key scheme used by the x509 certificate")
    .value("NONE",       x509::KEY_TYPES::NONE,       "Unknown scheme")
    .value("RSA",        x509::KEY_TYPES::RSA,        "RSA scheme")
    .value("ECKEY",      x509::KEY_TYPES::ECKEY,      "Elliptic-curve scheme")
    .value("ECKEY_DH",   x509::KEY_TYPES::ECKEY_DH,   "Elliptic-curve Diffie-Hellman")
    .value("ECDSA",      x509::KEY_TYPES::ECDSA,      "Elliptic-curve Digital Signature Algorithm")
    .value("RSA_ALT",    x509::KEY_TYPES::RSA_ALT,    "RSA scheme with an alternative implementation for signing and decrypting")
    .value("RSASSA_PSS", x509::KEY_TYPES::RSASSA_PSS, "RSA Probabilistic signature scheme");

  cls_x509
    .def_static("parse",
        static_cast<x509::certificates_t(*)(const std::string&)>(&x509::parse),
        "Parse " RST_CLASS_REF(lief.PE.x509) " certificate(s) from a file path given in the first parameter.\n"
        "It returns a **list** of " RST_CLASS_REF(lief.PE.x509) " objects",
        "path"_a)

    .def_static("parse",
        static_cast<x509::certificates_t(*)(const std::vector<uint8_t>&)>(&x509::parse),
        "Parse " RST_CLASS_REF(lief.PE.x509) " certificate(s) from a raw blob given in the first parameter.\n"
        "It returns a **list** of " RST_CLASS_REF(lief.PE.x509) " objects",
        "raw"_a)

    .def_property_readonly("version",
        &x509::version,
        "X.509 version. (1=v1, 2=v2, 3=v3)")

    .def_property_readonly("serial_number",
        [] (const x509& crt) -> py::bytes {
          const std::vector<uint8_t>& sn = crt.serial_number();
          return py::bytes(reinterpret_cast<const char*>(sn.data()), sn.size());
        },
        "Unique id for certificate issued by a specific CA.")

    .def_property_readonly("signature_algorithm",
        &x509::signature_algorithm,
        "Signature algorithm (OID)")

    .def_property_readonly("valid_from",
        &x509::valid_from,
        "Start time of certificate validity")

    .def_property_readonly("valid_to",
        &x509::valid_to,
        "End time of certificate validity")

    .def_property_readonly("issuer",
        [] (const x509& object) {
          return safe_string_converter(object.issuer());
        },
        "Issuer of the certificate")

    .def_property_readonly("subject",
        [] (const x509& object) {
          return safe_string_converter(object.subject());
        },
        "Subject of the certificate")

    .def_property_readonly("raw",
        [] (const x509& crt) -> py::bytes {
          const std::vector<uint8_t>& raw = crt.raw();
          return py::bytes(reinterpret_cast<const char*>(raw.data()), raw.size());
        },
        "The raw bytes associated with this x509 cert (DER encoded)")

    .def_property_readonly("key_type",
        &x509::key_type,
        "Return the underlying public-key scheme (" RST_CLASS_REF(lief.PE.x509.KEY_TYPES) ")")

    .def_property_readonly("rsa_info",
        &x509::rsa_info,
        "If the underlying public-key scheme is RSA, return the " RST_CLASS_REF(lief.PE.RsaInfo) " associated with this certificate. "
        "Otherwise, return None",
        py::return_value_policy::take_ownership)

    .def("verify",
        static_cast<x509::VERIFICATION_FLAGS(x509::*)(const x509&) const>(&x509::verify),
        R"delim(
        Verify that this certificate has been used **to trust** the given :class:`~lief.PE.x509` certificate

        It returns a set of flags defined by :class:`~lief.PE.x509.VERIFICATION_FLAGS`

        :Example:

          .. code-block:: python

            ca     = lief.PE.x509.parse("ca.crt")[0]
            signer = lief.PE.x509.parse("signer.crt")[0]
            print(ca.verify(signer))  # lief.PE.x509.VERIFICATION_FLAGS.OK

        )delim",
        "ca"_a)

    .def("is_trusted_by",
        &x509::is_trusted_by,
        R"delim(
        Verify this certificate against a list of root CA (list of :class:`~lief.PE.x509` objects)
        It returns a set of flags defined by :class:`~lief.PE.x509.VERIFICATION_FLAGS`

        :Example:

          .. code-block:: python

            signer = binary.signatures[0].signers[0]
            microsoft_ca_bundle  lief.PE.x509.parse("bundle.pem")
            print(signer.cert.is_trusted_by(microsoft_ca_bundle))
        )delim",
        "ca_list"_a)

    .def("__hash__",
        [] (const x509& obj) {
          return Hash::hash(obj);
        })

    .def("__str__",
        [] (const x509& x509_crt)
        {
          std::ostringstream stream;
          stream << x509_crt;
          return safe_string_converter(stream.str());
        });
}

}
}

