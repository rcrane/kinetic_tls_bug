# kinetic_tls_bug
Demonstartion of a bug regarding TLS connections to Seagate Kinetic disks


This repository is refers to:

https://github.com/Kinetic/kinetic-c

https://developers.seagate.com/display/KV/Development+Chassis

http://www.seagate.com/files/www-content/product-content/hdd-fam/kinetic-hdd/en-us/docs/kinetic-ds1835-1-1110us.pdf


Requirements: make, gcc, autoconf, a Seagate Kinetic disk (model ST4000NK0001)


Utilizing the Kinetic C library and its TLS capability, the code in this repository shows that there seems to be a flaw in the disk's handling of TLS-encrypted messages.
More precisely, if the message size in SSL_write() library call exceeds 14427 bytes, the disk (model ST4000NK0001) responds with an error message and terminates the session.

The kinetic_client log will show this message:

code: INTERNAL_ERROR
statusMessage: Unable to process proto message



As a workaround, this repository includes the file send_helper.fixed that limits message size for the SSL_write() library call and re-calls SSL_write() until the message has been sent.



To run the code: 

Edit kintest.c at line 9 to set the IP address of the disk.

./buildall.sh

make

./kintest (with bug)
./kintest_patched (without bug)





