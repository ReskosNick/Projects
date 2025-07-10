import socket
import argparse
import json
import sys
from typing import Dict, Tuple
from trie import Trie

class ServerError(Exception):
    """Base exception for server errors"""
    pass

class KVServer:
    def __init__(self, ip: str, port: int):
        self.ip = ip
        self.port = port
        self.trie = Trie()
        self._running = False
    
    def start(self) -> None:
        """Start the server and listen for connections."""
        self._running = True
        try:
            server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_socket.bind((self.ip, self.port))
            server_socket.listen(1) # Single connection from broker
            print(f"Server listening on {self.ip}:{self.port}")
                
            while self._running:
                try:
                    client_socket, _ = server_socket.accept()
                    self.handle_client(client_socket)
                    client_socket.close()
                except Exception as e:
                    if self._running:
                        print(f"Error accepting connection: {e}")
                            
        except socket.error as e:
            print(f"Failed to start the server: {e}")
        finally:
            server_socket.close()
            print("Server shut down.")
    
    def stop(self) -> None:
        """Stop the server."""
        self._running = False
    
    def handle_client(self, client_socket: socket.socket) -> None:
        """Handle a client connection."""
        try:
            with client_socket:
                # Receive data in chunks
                chunks = []
                while True:
                    chunk = client_socket.recv(8192) # Increased buffer size
                    if not chunk:
                        break
                    chunks.append(chunk)
                
                if not chunks:
                    return
                    
                # Combine all chunks
                data = b''.join(chunks).decode()
                response = self.process_command(data)
                client_socket.sendall(response.encode())
        except Exception as e:
            print(f"Error handling client: {e}")
    
    def process_command(self, command: str) -> str:
        """Process a command and return the response."""
        try:
            # Normalize command format
            command = command.strip()
            if not command:
                return "ERROR: Empty command"

            # Split command preserving quoted strings
            parts = command.split(maxsplit=1)
            if len(parts) < 2:
                return "ERROR: Invalid command format. Expected: <COMMAND> <key>"
            
            cmd, args = parts
            cmd = cmd.upper()  # Normalize command case
            
            if cmd not in ["PUT", "GET", "DELETE", "QUERY"]:
                return f"ERROR: Unknown command '{cmd}'. Valid commands: PUT, GET, DELETE, QUERY"
                
            # Strip quotes from key if present
            args = args.strip().strip('"')
            
            if cmd == "PUT":
                return self.handle_put(args)
            elif cmd == "GET":
                return self.handle_get(args)
            elif cmd == "DELETE":
                return self.handle_delete(args)
            elif cmd == "QUERY":
                return self.handle_query(args)
                
        except Exception as e:
            return f"ERROR: {str(e)}"
    
    def parse_key_value(self, data: str) -> Tuple[str, Dict]:
        """Parse a key-value string into its components."""
        try:
            # Find the first colon, splitting key and value parts
            key_end = data.find(':')
            if key_end == -1:
                raise ValueError("Missing colon separator")
                
            key_part = data[:key_end]
            value_part = data[key_end + 1:]
            
            # Clean up the key (remove quotes and whitespaces)
            key = key_part.strip().strip('"')
            if not key:
                raise ValueError("Empty key")
            
            # Parse the value part using json.loads
            value = value_part.strip()
            try:
                if value == "{}":
                    value = {}
                else:
                    # Convert to JSON format first (replace ; with ,)
                    value = value.replace(";", ",")
                    value = json.loads(value)
            except json.JSONDecodeError as e:
                raise ValueError(f"Invalid value format: {e}")
            
            return key, value
            
        except Exception as e:
            raise ValueError(f"Invalid key-value format: {e}")
    
    def handle_put(self, data: str) -> str:
        """Handle PUT command."""
        try:
            key, value = self.parse_key_value(data)
            self.trie.insert(key, value)
            return "OK"
        except Exception as e:
            return f"ERROR {str(e)}"
    
    def handle_get(self, key: str) -> str:
        """Handle GET command."""
        key = key.strip().strip('"')
        value = self.trie.search(key)
        
        if value is None:
            return "NOT FOUND"
        
        # Convert to JSON string and then replace commas with semicolons
        formatted_value = json.dumps(value).replace(",", ";")
        return f"{key} : {formatted_value}"
        
    def handle_delete(self, key: str) -> str:
        """Handle DELETE command."""
        key = key.strip().strip('"')
        # First check if key exists
        if self.trie.search(key) is None:
            return "NOT FOUND"
        # Then try to delete it
        if self.trie.delete(key):
            return "OK"
        return "ERROR: Failed to delete key"
    
    def handle_query(self, path: str) -> str:
        """Handle QUERY command."""
        path = path.strip().strip('"')
        value = self.trie.query_path(path)
        
        if value is None:
            return "NOT FOUND"
            
        # Convert to JSON string and then replace commas with semicolons
        formatted_value = json.dumps(value).replace(",", ";")
        return f"{path} : {formatted_value}"

def main() -> None:
    parser = argparse.ArgumentParser(description='KV Store Server')
    parser.add_argument('-a', required=True, help='IP address')
    parser.add_argument('-p', type=int, required=True, help='Port number')
    
    args = parser.parse_args()
    
    try:
        # Create and start server
        server = KVServer(args.a, args.p)
        
        try:
            server.start()
        except KeyboardInterrupt:
            print("\nShutting down server...")
            server.stop()
        
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()