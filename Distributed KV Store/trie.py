from typing import Optional, Dict, Any

class TrieNode:
    def __init__(self):
        self.children: Dict[str, TrieNode] = {}
        self.value: Any = None
        
class Trie:
    def __init__(self):
        self.root = TrieNode()
    
    def insert(self, key: str, value: Any) -> None:
        """Insert a key-value pair into the trie."""
        node = self.root
        
        for char in key:
            if char not in node.children: 
                # Create new nodes for paths that don't already exist
                node.children[char] = TrieNode()
            node = node.children[char]
        
        node.value = value
    
    def search(self, key: str) -> Optional[Any]:
        """Search for a key and return its value if found."""
        node = self.root
        
        # Follow the path in the trie
        for char in key:
            if char not in node.children:
                return None
            node = node.children[char]
        
        # Return value if this is a valid end of key
        return node.value
    
    def delete(self, key: str) -> bool:
        """Delete a key from the trie. Returns True if deleted, False if not found."""
        def _delete_recursive(node: TrieNode, key: str, depth: int) -> bool:
            if depth == len(key):
                if node.value is None:
                    return False
                node.value = None
                return True 
            
            char = key[depth]
            if char not in node.children:
                return False
            
            should_delete_child = _delete_recursive(node.children[char], key, depth + 1)
            
            # If child can be deleted and has no other children
            if should_delete_child and not node.children[char].children:
                del node.children[char]
                
            return should_delete_child
        
        return _delete_recursive(self.root, key, 0)

    def query_path(self, path: str) -> Optional[Any]:
        """Query a nested path using dot notation."""
        # First find the top-level key with trie search
        parts = path.split('.')
        top_level_value = self.search(parts[0])
        
        if top_level_value is None:
            return None
            
        # Navigate through the nested structure
        current = top_level_value
        
        for part in parts[1:]:
            if not isinstance(current, dict) or part not in current:
                return None
            current = current[part]
  
        return current