# DNS Proxy Server

A DNS proxy server with a domains blacklist feature to filter unwanted host names resolving. This project utilizes the `ldns` library.

## Features

- Reads configuration from a file during startup.
- Filters domain names based on a blacklist.
- Forwards requests to an upstream DNS server if the domain is not blacklisted.
- Generates RCODE name errors for blacklisted domains.
- Uses UDP protocol for DNS request processing and communication with the upstream DNS server.

## Functional Requirements

1. **Configuration File:** 
    - Contains the IP address of the upstream DNS server.
    - Contains a list of domain names to filter ("blacklist"). If a domain name is found in the blacklist, an RCODE name error is generated.

2. **DNS Request Handling:** 
    - If a domain name from a client's request is not found in the blacklist, the proxy server forwards the request to the upstream DNS server, waits for a response, and sends it back to the client.
    - If a domain name from a client's request is found in the blacklist, the proxy server responds with a predefined response from the configuration file.

## Installation

1. Install the `ldns` library:
    ```sh
    sudo apt-get install libldns-dev
    ```

## Usage

1. Set up DNS on localhost:
    ```sh
    sudo resolvectl dns INTERFACE 127.0.0.1
    ```

2. Create a `config.txt` file with the following parameters:
    - Upstream DNS server IP address.
    - List of blacklisted domains.

3. Compile the project:
    ```sh
    make
    ```

4. Run the DNS proxy server with sudo:
    ```sh
   sudo ./dns_proxy_server
    ```

5. Test the DNS proxy server using `dig` on port 5000:
    ```sh
    dig example.com @127.0.0.1 -p 5000
    dig google.com @127.0.0.1 -p 5000
    ```

## Contact

For any questions or inquiries, please contact [vovanlucenkojob@gmail.com.com].


