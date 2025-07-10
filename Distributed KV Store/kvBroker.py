import socket
import argparse
import sys
import random
from typing import List, Set, Tuple

class BrokerError(Exception):
    """Base exception for broker errors"""
    pass

class ServerConnectionError(BrokerError):
    """Exception raised for server connection issues"""
    pass

class KVBroker:
    def __init__(self, servers: List[Tuple[str, int]], replication_factor: int):
        """
        Initialize the broker with a list of servers and replication factor.
        
        Args:
            servers: List of (ip, port) tuples
            replication_factor: Number of servers to replicate data to
        """
        self.servers = servers
        self.k = replication_factor
        self.active_servers: Set[Tuple[str, int]] = set()
        self.initial_servers: Set[Tuple[str, int]] = set()
        self.connect_to_servers()

    def connect_to_servers(self) -> None:
        """Attempt to connect to all servers and maintain list of active ones."""
        for ip, port in self.servers:
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                    sock.settimeout(2)
                    sock.connect((ip, port))
                self.active_servers.add((ip, port))
                print(f"Successfully connected to server {ip}:{port}")
            except (socket.timeout, ConnectionRefusedError):
                print(f"Warning: Could not connect to server {ip}:{port}")

        if len(self.active_servers) < self.k:
            raise ServerConnectionError(
                f"Not enough active servers. Need at least {self.k}, but only "
                f"{len(self.active_servers)} are available."
            )
        
        # Store the initial set of active servers
        self.initial_servers = self.active_servers.copy()
        print(f"\nInitial Server Status: {len(self.active_servers)}/{len(self.servers)} active servers.")

    def check_active_servers(self) -> None:
        """Check all active servers and update the list of active ones."""
        servers_to_remove = set()
        for server in list(self.active_servers):
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                    sock.settimeout(2)
                    sock.connect(server)
            except:
                servers_to_remove.add(server)
        
        # Update active_servers
        self.active_servers -= servers_to_remove

    def k_servers_down(self) -> bool:
        """Check and report if k or more of the initial active servers are down."""
        down_servers = self.initial_servers - self.active_servers
        num_down = len(down_servers)
        
        if num_down >= self.k:
            print(f"WARNING: Cannot guarantee correct output as {self.k} or more initially "
                  f"active servers are now down.")
            print(f"Initial servers down: {down_servers}")
            return True
        
        return False


    def send_to_server(self, server: Tuple[str, int], command: str) -> str:
        """
        Send a command to a specific server and get the response.
        
        Args:
            server: (ip, port) tuple
            command: Command string to send
            
        Returns:
            Server response
        """
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(5)
                sock.connect(server)
                data = command.encode() # Send data in chunks
                total_sent = 0
                while total_sent < len(data):
                    sent = sock.send(data[total_sent:])
                    if sent == 0:
                        raise ServerConnectionError("Socket connection broken")
                    total_sent = total_sent + sent
                
                # Close sending end
                sock.shutdown(socket.SHUT_WR)
                
                # Receive response in chunks
                chunks = []
                while True:
                    chunk = sock.recv(8192)
                    if not chunk:
                        break
                    chunks.append(chunk)
                    
                return b''.join(chunks).decode()
        except Exception as e:
            self.active_servers.discard(server)
            raise ServerConnectionError(f"Error communicating with {server}: {e}")

    def put_data(self, key_value_pair: str) -> None:
        """
        Store data with replication.
        
        Args:
            key_value_pair: The key-value data to store
        """

        if len(self.active_servers) < self.k:
            raise ServerConnectionError(
                f"Not enough active servers for replication. Need {self.k}, have {len(self.active_servers)}"
            )

        # Convert to list to avoid modification during iteration
        servers_to_try = list(self.initial_servers)
        # Select k random servers for replication
        target_servers = random.sample(servers_to_try, self.k)
        command = f"PUT {key_value_pair}"
        failed_servers = []
        successful_puts = 0
        
        for server in target_servers:
            try:
                response = self.send_to_server(server, command)
                if response != "OK":
                    print(f"Warning: Server {server} returned: {response}")
                    failed_servers.append(server)
                else:
                    successful_puts += 1
            except ServerConnectionError as e:
                print(f"Warning: {e}")
                failed_servers.append(server)
        
        if failed_servers:
            print(f"PUT failed on servers: {failed_servers}.")
            print(f"Data replicated to {successful_puts}/{self.k} required servers.")

    def retrieve_data(self, key: str, command: str) -> str:
        """
        Retrieve data for a given key.
        
        Args:
            key: The key to look up
            command: Command string to excecute
            
        Returns:
            The value associated with the key
        """

        if self.k_servers_down():
            return ""

        key = key.strip().strip('"')  # Handle quoted keys
        servers_to_try = list(self.active_servers)
        
        for server in servers_to_try:
            try:
                response = self.send_to_server(server, f"{command} {key}")
                if response != "NOT FOUND":
                    return response
            except ServerConnectionError:
                continue
                
        return "NOT FOUND"

    def delete_data(self, key: str) -> bool:
        """
        Delete data from all servers.
        
        Args:
            key: The key to delete
            
        Returns:
            True if successful, False otherwise
        """
        # First identify which servers have the key
        servers_with_key = []
        servers_to_try = list(self.active_servers)
        for server in servers_to_try:
            try:
                response = self.send_to_server(server, f"GET {key}")
                if response != "NOT FOUND":
                    servers_with_key.append(server)
            except ServerConnectionError:
                continue
        if not servers_with_key:
            return False

        # Delete only from servers that have the key
        success = True
        failed_servers = []
        for server in servers_with_key:
            try:
                response = self.send_to_server(server, f"DELETE {key}")
                print(f"Server {server} delete response: {response}")
                if response != "OK":
                    success = False
                    failed_servers.append((server, response))
            except ServerConnectionError as e:
                success = False
                failed_servers.append((server, str(e)))

        if not success:
            print(f"DELETE failed on servers that had the key: {failed_servers}.")
            return False
        
        print(f"Successfully deleted {key} from {len(servers_with_key)} servers.")
        return True


def load_server_file(file_path: str) -> List[Tuple[str, int]]:
    """Load server IP addresses and ports from file."""
    servers = []
    try:
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    ip, port = line.split()
                    servers.append((ip, int(port)))
                except ValueError:
                    raise BrokerError(f"Invalid server file format. Expected 'IP PORT', got: {line}")
    except FileNotFoundError:
        raise BrokerError(f"Server file not found: {file_path}")
        
    if not servers:
        raise BrokerError("No servers specified in server file")
    return servers

def validate_replication_factor(k: int, num_servers: int) -> None:
    """Validate replication factor against number of servers."""
    if k <= 0:
        raise ValueError("Replication factor must be positive")
    if k > num_servers:
        raise ValueError(f"Replication factor ({k}) cannot be greater than number of servers ({num_servers})")

def main() -> None:
    parser = argparse.ArgumentParser(description='KV Store Broker')
    parser.add_argument('-s', required=True, help='Server file path')
    parser.add_argument('-i', required=True, help='Data file to index')
    parser.add_argument('-k', type=int, required=True, help='Replication factor')
    
    args = parser.parse_args()
    
    try:
        servers = load_server_file(args.s)
        validate_replication_factor(args.k, len(servers))
        
        # Initialize broker and start distributing data
        broker = KVBroker(servers, args.k)
        print("\nLoading and distributing data...")
        with open(args.i, 'r') as f:
            for line in f:
                line = line.strip()
                if line:
                    broker.put_data(line)
        
        print("\nData distribution was successfully completed.")
        
        # Start accepting commands
        print("\nReady for commands (GET, DELETE, QUERY):")
        while True:
            try:
                command = input().strip()
                if not command:
                    continue
                
                # Split command preserving quoted strings
                parts = command.split(maxsplit=1)
                if len(parts) < 2:
                    print("ERROR: Invalid command format. Expected: <COMMAND> <key>")
                    continue
                
                cmd, arg = parts
                cmd = cmd.upper()  # Normalize command case
                
                if cmd not in ["GET", "DELETE", "QUERY"]:
                    print(f"ERROR: Unknown command '{cmd}'. Valid commands: GET, DELETE, QUERY")
                    continue
                
                broker.check_active_servers()
                if cmd == "GET":
                    result = broker.retrieve_data(arg, cmd)
                    if result:
                        print(result)
                elif cmd == "DELETE":
                    if len(broker.active_servers) < len(broker.initial_servers):
                        print("WARNING: Cannot guarantee consistent deletion when some servers are down.")
                        continue  # Skip to next command
                    if broker.delete_data(arg):
                        pass
                    else:
                        print("ERROR: key not found")
                elif cmd == "QUERY":
                    result = broker.retrieve_data(arg, cmd)
                    if result:
                        print(result)
                    
            except (KeyboardInterrupt, EOFError):
                print("\nShutting down broker...")
                break
            except Exception as e:
                print(f"Error: {e}")
                
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()