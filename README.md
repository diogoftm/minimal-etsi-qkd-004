# QKD Application Interface

*This repository is a fork of [ETSI ISG QKD](https://www.etsi.org/committee/qkd) [gs004-app-int](https://forge.etsi.org/rep/qkd/gs004-app-int/-/tree/edition3?ref_type=heads). Small changes were made to be possible to request oblivious keys and to have minimal operation capacity for upper-layer application testing using the server as a Key Management System.*

From this sentence down, all the text is unchanged from the original version.

Module that enables ETSI GS QKD 004 in C projects. The implementation consists of TLS socket connections using a packet
with an ETSI GS QKD 004 compliant structure.

# Contents

* [Dependencies](#dependencies)
    * [Cmake](#cmake)
    * [Libexplain](#libexplain)
* [Build](#build)
* [Usage](#usage)
    * [Library location](#library-location)
    * [Server example](#server-example)
    * [Client example](#client-example)

## Dependencies <a name="dependencies"></a>

### Cmake <a name="cmake"></a>

Cmake is used to build the library and the examples.

```shell
sudo apt install cmake
```

### Libexplain <a name="libexplain"></a>

Libexplain is used to provide detailed info about the socket connection problems.

```shell
sudo apt install libexplain-dev
```

## Build <a name="build"></a>

The user can build both the library and the examples using the following commands.

```shell
cd etsi-gs-qkd-004-c
mkdir cmake-build-dir
cd build-dir
cmake ..
make
cd ..
```

## Usage <a name="usage"></a>

### Library location <a name="library-location"></a>

You can find the ETSI GS QKD 004 library in:

```
etsi-gs-qkd-004-c/build-dir/libetsi-gs-qkd-004.a
```

### Server example <a name="server-example"></a>

There is a server example using the ETSI GS QKD 004 library with a simple data processing, please adapt this server
example to your needs.

The server example can be found at:

```
etsi-gs-qkd-004-c/src/server_example.c
```

It can be executed using the following command:

```shell
etsi-gs-qkd-004-c/cmake-build-dir/server_example
```

IMPORTANT NOTE: As in any TLS server the server needs a certificate files (private key and certificate).
You can generate self-signed certificates using:

```shell
# Generate CA
CA_NAME_=your_name_CA # CA name
openssl req -x509 -nodes -newkey rsa:4096 -sha256 -days 3650 -keyout $CA_NAME_.key -out $CA_NAME_.pem -subj "/CN=$CA_NAME_"

# Generate certificate
CN_=127.0.0.1  #localhost
openssl req -new -newkey rsa:4096 -nodes -keyout $CN_.key -out $CN_.csr -subj "/CN=$CN_" -addext "subjectAltName=IP:$CN_" || openssl req -new -newkey rsa:4096 -nodes -keyout $CN_.key -out $CN_.csr -subj "/CN=$CN_" -addext "subjectAltName=DNS:$CN_"

# Sign certificate with the CA
openssl x509 -req -in $CN_.csr -CA $CA_NAME_.pem -CAkey $CA_NAME_.key -CAcreateserial -out $CN_.pem -days 3650 -sha256

# Check certificate
openssl x509 -in $CN_.pem -text -noout  # openssl x509 -in file.pem -enddate -noout 
```

The CA public certificate can be used in the client to validate the server.

### Client example <a name="client-example"></a>

There is a client example using the library at:

```
etsi-gs-qkd-004-c/src/client_example.c
```

It can be executed using the following command:

```shell
etsi-gs-qkd-004-c/cmake-build-dir/client_example
```

In the case of the client the certificate is not mandatory but can be also added and checked in the server.

## Contact

For enquiries, contact `isgsupport` at `etsi` dot `org`.

## License

See LICENSE file and https://forge.etsi.org/legal-matters
