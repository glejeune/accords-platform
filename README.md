File : README 
Project : COMPATIBLE ONE ACCORDS
Author : Iain James Marshall, Grégoire Lejeune
Created : 19th November 2011
Updated : 19th September 2013

# INTRODUCTION 

This file provides useful information about the production, installation, behaviour and use of the [CompatibleOne Accords Brokering and Provisioning Platform](http://compatibleone.org). It is divided into three primary sections covering production, installation and operation. Additional sections then following providing background information pertaining to Security, Authentication, Monitoring, Data and Temporary Files. 

## PRODUCTION

If you are reading this file then you will have cloned the accords-platform from the CompatibleOne GitHub repository and will perhaps want to build the binary package.

Currently two build systems are provided. The first one for debian based systems is based on the standard Debian packaging system. Make sure you have `dpkg-dev` installed and type `dpkg-buildpackage -rsudo -b` in this directory.

The other one, the build.linux script, is to be used for non Debian based Linux platforms.

In both cases, to build the Accords Platform Binary package, simply execute the required build script and wait till the compilation and link has all completed.

The Accords platforms depend on the following other tools and libraries which need to be installed on your platform before launching your build script.

    autoconf
    automake
    autotools
    python-dev
    python-boto
    make
    gcc
    openssl
    UUID

If you get this package from git repository, you must run:

    ./autogen.sh

To build this package, you can run:

    ./configure [options] (see ./configure --help for options)
    make

## PLATFORM SPECIFICS

On some compilers (`gcc 4.4`), some additional flags are needed for the compiler.

Try a compbination of:

    CFLAGS="-Wl,--no-as-needed"
    LDFLAGS="-ldl"

You can pass these flags to configure script, or to make invocation.

## INSTALLATION

To install all files, run (as root):

    make install

## PYACCORDSSDK CONFIGURATION

To configure the PyAccordsSDK path, run the following (as root):

    ./pyaccords_config.sh

## OPERATION

If you have not installed files, you must run the following commands from 'scripts' dir.

ACCORDS package provides a configuration script called `accords-config`.

The configuration script may take several parameters which will influence the way in which the installation will be performed.

1. configuration file name
2. host name of the platform
3. base port number of the installation
4. publisher url of the form http://host:port
5. this parameter when present will activate security
6. this parameter when present will specify the monitoring options

When no parameters are passed the installation will be performed as if it had been launched as :

    accords-config accords.xml http://127.0.0.1 8086 http://127.0.0.1:8086

The configuration procedure will create the master configuration file for all of the components of the Accords platform and when security is active will generate the security configurations, certificates and private keys for all the OCCI server modules comprising the system. In addition, a collection of command scripts will be generated that are configured facilitating the use of the different components of the platform.

The following collection of command scripts are to be used for the operation of the platform. 

`co-start`
: This script must be launched to start the Accords Platform and bring the servers online and takes no command line parameters. The script launchs the collection of run scripts, one for each of the accords platform component servers. Each server will be launched in background operation mode as a daemon process.

`co-status`
: This script may be launched to inspect the status of the Accords Platform. The status inspection operation may be performed by hand using either the ps command and grepping for the processes which use the the accords.xml configuration file. It may also be performed using netstat to detect the individual server's listening socket.

`co-parser`
: This script allows a manifest to be submitted for parsing and results in the creation of a fully qualified plan. The script takes a single parameter, the stub name of the xml manifest file without the xml file extension. The script launchs the executable `bin/testcp` and defines the required publisher and security information as described during installation.

`co-broker`
: This script allows the resources described by a plan to be provisioned as a service and started. The script takes a single parameter, the stub name of the xml plan file without the xml file extension. The script launchs the executable `bin/testcb` and defines the required publisher and security information as described during installation.

`co-resolver`
: This script allows the publisher to be queried for category service provider information. The script takes any quantity of command line parameters, expected to be category names that will be requested of the publisher for resolution of their service provider identity. The recent version of the executable `bin/testresolver` allows a command line option `--price` allowing activation of resolution of price information for each category processed.

        co-resolver compute storage network system package
        co-resolver --price manifest service contract openstack opennebula

`co-command`
: This script allows a provisioned service to be started, stopped or saved. The script expects any number of command line parameters, the first of which must be one of the two keywords `start` or  `stop`. The subsequent parameters are to be valid service instance description filenames in the service directory. The corresponding command will be performed on each of the service instance descriptions bringing all contracts of the service up or down as required.

        co-command start service/uuid-of-service-instance
        co-command stop service/uuid-of-service-instance

`co-stop`
: This script must be launched to stop the Accords Platform and bring the servers offline. This script takes no command line parameter and will perform a `killall -15` operation for each of the service components of the accords platform. Service components intercept this signal and perform a garceful exit after releasing all ressources and saving any data.

Several additional binary executable test programs are offered by the package allowing access to the different cloud provider platforms currently supported for use by the Accords Platform. These tools allow inspection of the different resources of the platforms and have a platform specific command line syntax.

`testos`
: This program provides a test client interface for the OpenStack platform and although it was initially aligned for operation with the version 1.0 since the arrival of Diablo and version 1.1 interoperability has not been maintained by the platform itself. The OpenStack platform offers several resource types: flavors, images, servers and metadata. The specific command line syntax for the management of resources using this tool may be consulted in the integrated help screen that may be obtained by simply launching the tool with no command line parameters.

`teston`

`testaz`

## SECURITY

During the installation operation if security has been requested then a security directoy will be created into which the security configuration files of each of the platform components will be generated. In addition the private key and certificates will also be generated into this directory.

The files generated into the security directory are not to be world readable for obvious reasons.

Each individual security configuration file defines the private key, certificate, user name, password and the security options of each of the server components. 

The Private Keys and Certificates are currently only self signed and will therefore cause Incorrect Certificate alerts and errors to be thrown when used with the Certificate Exchange and Validation Mechanisms activated. This highlights the need for a mechanism within the CompatibleOne platform for the production and delivery of Valid Certification Authority Signed certificates for use not only by the Accords platform components but also by the provisioned resources and products delivered by the platform. 

The security option values are defined as follows :

`1`, `2`, `4`
: These three option values are undefined in this SSL / TLS implementation for ACCORDS.

`8`
: **REQUEST PEER CERTIFICATE** -- When this option value is set then the server will issue a Send Certificate request to the client during the Server Hello phase of handshake and cypher negociation. This option instructs the Client to validate the Certificate chain received from the Server prior to sending the Client side certificate if one is available. Self Signed Certificates with throw "Incorrect Certificate" alerts when this option is active. If a Certificate is not exchanged then no failure will occur since it not mandatory.

`16`
: **REQUIRE PEER CERTIFICATE** -- This options requires an exchange of valid Certificates by both Client and Servers. An error will occur if no Certificate is made available or if the Certificate is not Correct.

`32`
: **DER KEY FILE FORMAT** -- When this option is set it indicates that the Private Key file is not a base64 PEM Ascii encoded file but rather a binary DSR,ASN or otherwise encoded file.

`64`
: **DER CERTIFICATE FILE FORMAT** -- When this option is set it indicates that the Certificate File is not a base64 PEM Ascii encoded file but rather a binary DSR,ASN or otherwise encoded file.

`128`
: **SSL COMPATIBLE** -- When this option is set then the SSL compatible mode will be activated and as such the client connection handshake will commence with an SSL V2 or V3 Hello Message. This may be accepted by SSL V1, V2, V3 and TLS v1 servers. However an RFC dating from April 2011 is now recommending that this is bad practice since it allows MTM attacks to influence both the client and server to be misled into believing that the other party requires an inferior protocole. The RFC is strongly advocating that this v23 Client Hello should be phased out as quickly as possible. In compliance with this recommendation the TLS / SSL functions of the Accords Platform will issue a TLS V1 Client Hello Message and as a server will only communicate with Clients behaving accordingly. This requirement may be relaxed by use of this option value.

`256`
: **INTERNAL SSL CERTIFICATE VERIFICATION** -- This option may be set when Exchange of Self-Signed Certificates by Clients and Servers is required. This is necessary for instance when working with Windows Azure platforms which expressly request provision of a self signed certificate by the client endpoint.

These options are necessary during the development and test phases of the Accords platform and should be used carefully and only by persons with a solid understanding of the RFC for TLS and their implications towards the resulting platform security. 

## HINT

The value of the security options is provided by two different variables `options` and `optionst` inside the `install.linux` and `install.debian` scripts to allow the value to be set for servers and for tools independantly. It is however recommened that they be set to the same value to ensure homogenous operation.

## AUTHENTICATION

When security has been requested and activated during the installation phase, an individual user name and associated password will have been generated to the security configuration file of each component. The user category management service database will be generated to include the collection of valid user name and password authentication credentials. The corresponding authorisation option of the security configuration file, when set, will cause the service component to present the credentials to COSS for authentication prior to requesting the creation of an authorization token allowing collaboration by the service in the Accords Platform. This authorization credential will be presented in the OCCI/REST/HTTP header of ALL Request and Response messages sent by the Client of Server endpoints of the Service component. For obvious reasons neither the PUBLISHER nor the COSS modules are required to perform authentication and authorization though this will be introduced as a retro fit requirement at a later date.
 
## MONITORING

During the platform operation, log and trace information will be generated as described by the monitoring parameter of the installation script.

`0`
: No monitoring

`1`
: All monitoring is sent to the file co-log

`2`
: All monitoring is sent to the Module COMONS, except for the PUBLISHER, COMONS and COSS

`3`
: Monitoring information is sent to both the file co-log and to COMONS

`6`
: Optimised Monitoring is sent to the Module COMONS, except for the PUBLISHER, COMONS and COSSand to the file co-log.

When COMONS monitoring is active then only the service modules started after COMONS are able to make use of the centralised monitoring service. Therefore the activation of CO-LOG monitoring is necessary for the fundamental modules of the platform : PUBLISHER, COSS and COMONS. 

## DATA

The collection of XML files `cords_*.xml` and `links_*.xml` represent the internal state of the different server components of the platform. They are updated as their internal state changes and will be reloaded when the corrresponding component restarts.

## TEMPORARY FILES

The directory `rest/` contains the contents of the messages exchanged by the server components during operation and may be consulted. These may have several different extensions depending on the source of the information and may be XML, JSON, OCCI, HTML or TMP files. 

# HINT

This directory will need to be cleaned at regular intervals in cases of prolonged operation otherwise performance degradation will occur due to the number of files that accumulate here. 
