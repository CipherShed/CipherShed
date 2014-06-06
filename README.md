# CipherShed
CipherShed is on-the-fly encryption software based off of the discontinued [TrueCrypt][TC] project.

URLs: [http://ciphershed.org][MAIN] and our [GitHub][GH]

To compile the sources under Ubuntu / Debian, copy these three missing files over from RSA
* wget 'ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/v2-20/pkcs11.h' 
* wget 'ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/v2-20/pkcs11f.h' 
* wget 'ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/v2-20/pkcs11t.h'

and then compile with

* LIBS="-ldl" make

## Project Goals
* Audit and defend the security of the existing TrueCrypt 7.1a code base.
* Improve security using modern techniques such as memory-hard password hashing.
* Secure the code through audits, simplification, and a secure architecture.
* Migrate towards a truly FOSS code base.
* Work closely with existing efforts such as [OpenCryptoAuditProject][OCAP], [LUKS][LUKS], [GELI][GELI], and [tc-play][TCP].

## Getting Involved
Please use the following means for contributions, questions, and support.

* The [mailing list][ML] on FreeLists
* The [#CipherShed IRC channel][WC] on OFTC: [ircs://irc.oftc.net:6697/CipherShed][IRC]

[TC]: https://en.wikipedia.org/wiki/TrueCrypt
[MAIN]: http://ciphershed.org/
[GH]: https://github.com/CipherShed/CipherShed
[OCAP]: https://opencryptoaudit.org/
[LUKS]: https://code.google.com/p/cryptsetup/
[GELI]: https://en.wikipedia.org/wiki/Geli_(software)
[TCP]: https://github.com/bwalex/tc-play
[ML]: http://www.freelists.org/list/geekcrypt
[WC]: https://webchat.oftc.net/?channels=%23CipherShed
[IRC]: ircs://irc.oftc.net:6697/CipherShed
