aSocial Crypto module
=====================

Crypto module implements functions, thet required to create and verify signatures. Also it provides encryption etc...

Implementation
--------------
Is based on the bitcoin implementation of keys & addresses for profile.

## Device & Profile Address (Bitcoin compliant):
* Public/Private Key: ECDH secp256k1
* Signature: ECDSA
* Digest: SHA-2-256, RIPEMD-160
* Text encode: Base58 + Check

## Communication:
* Same keys & signatures
* Encryption: ECIES
* Digest: SHA-3-256
* Private key block cipher: Serpent

Requirements
------------
* OpenSSL: libssl, libcrypt
Android procedure: http://qt-project.org/doc/qt-5/opensslsupport.html

Support:
--------
If you like cypherpunks, you can support our open-source development by a small Bitcoin donation.

Bitcoin wallet: `15phQNwkVs3fXxvxzBkhuhXA2xoKikPfUy`
