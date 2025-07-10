Distributed Key-Value Store Implementation

A fault-tolerant, distributed key-value store with support for nested queries and k-way replication.

# Dependencies
- Python 3.6 or higher
- Standard Python libraries (no external packages required)

# Files contained
- keyFile.txt: A file containing a space-separated list of key names and their data types.
- serverFile.txt: A list of server addresses and ports.
- createData.py: Data generation for the Key-Value store, with the definitions from the `keyFile.txt`.
- kvBroker.py: The broker application that manages communication between the client and the servers.
- trie.py: The Trie data structure used for storing high-level keys in the Key-Value store.
- kvServer.py: The server-side application that listens for connections and processes commands from the broker.
- Report.pdf: A concise report of the KV Store Implementation.

# Usage

-> Data Generation
Generate test data using the createData.py script:
python createData.py -k keyFile.txt -n <num_lines> -d <max_depth> -l <max_length> -m <max_keys>

-> Server Setup
Start each KV server in a separate terminal:
python kvServer.py -a <ip_address> -p <port>

-> Broker Setup
Start the broker:
python kvBroker.py -s serverFile.txt -i dataToIndex.txt -k <replication_factor>

# Implementation Notes
1. High-level keys are generated of the form key1, key2 etc. for easier debugging.
2. The system handles both quoted and unquoted keys (e.g., "key1" and key1 are treated the same).
3. The system has been tested locally using loopback IPs (e.g., 127.0.0.1).
