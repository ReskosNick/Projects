from typing import Dict, List, Optional
import argparse
import random
import string
import sys

class ValidationError(Exception):
    """Exception raised for invalid input parameters"""
    pass

def validate_positive(value: int, param_name: str) -> None:
    """Validate if a parameter is positive (> 0)"""
    if value <= 0:
        raise ValidationError(f"Parameter {param_name} must be positive. Got: {value}")

def validate_non_negative(value: int, param_name: str) -> None:
    """Validate if a parameter is non-negative (>= 0)"""
    if value < 0:
        raise ValidationError(f"Parameter {param_name} must be non-negative. Got: {value}")

def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Create test data for KV store')
    parser.add_argument('-k', required=True, help='Key file path')
    parser.add_argument('-n', type=int, required=True, help='Number of lines to generate (must be positive)')
    parser.add_argument('-d', type=int, required=True, help='Maximum nesting depth (must be non-negative)')
    parser.add_argument('-l', type=int, required=True, help='Maximum string length (must be positive)')
    parser.add_argument('-m', type=int, required=True, help='Maximum keys per value (must be non-negative)')
    
    args = parser.parse_args()
    
    try:
        validate_positive(args.n, 'n')
        validate_positive(args.l, 'l')
        validate_non_negative(args.d, 'd')
        validate_non_negative(args.m, 'm')
    except ValidationError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
        
    return args

def load_key_file(file_path: str) -> Dict[str, str]:
    keys = {}
    with open(file_path, 'r') as f:
        for line in f:
            name, data_type = line.strip().split()
            keys[name] = data_type
    return keys

def generate_random_string(max_length: int) -> str:
    length = random.randint(1, max_length)
    chars = string.ascii_letters + string.digits
    return ''.join(random.choice(chars) for _ in range(length))

def generate_value_by_type(data_type: str, max_length: int) -> Optional[str]:
    if data_type == 'string':
        return f'"{generate_random_string(max_length)}"'
    elif data_type == 'int':
        return str(random.randint(0, 1000))
    elif data_type == 'float':
        return f"{random.uniform(0, 1000):.2f}"
    return None

def generate_nested_value(
    keys_dict: Dict[str, str],
    current_depth: int,
    target_depth: int,
    max_keys: int,
    max_length: int
) -> str:
    """
    Generate a nested value structure with a specific target depth.
    
    Args:
        keys_dict: Dictionary mapping key names to their data types
        current_depth: Current nesting level
        target_depth: Target nesting level to reach
        max_keys: Maximum number of keys per value
        max_length: Maximum length for generated strings
    
    Returns:
        A string representing the generated nested value structure
    """
    if current_depth == target_depth or max_keys == 0:
        num_keys = random.randint(0, min(max_keys, len(keys_dict))) # At terminal depth, we can have 0 to max_keys
        if num_keys == 0: # Return empty object if num_keys is 0
            return "{}"
        
        # Otherwise generate terminal values
        selected_keys = random.sample(list(keys_dict.items()), num_keys)
        key_values = []
        for key_name, data_type in selected_keys:
            value = generate_value_by_type(data_type, max_length)
            if value is not None:
                key_values.append(f'"{key_name}" : {value}')
        return "{ " + " ; ".join(key_values) + " }"
    
    # If we haven't reached the target depth, we need at least one key to continue nesting
    num_keys = random.randint(1, min(max_keys, len(keys_dict)))
    selected_keys = random.sample(list(keys_dict.items()), num_keys)

    # Randomly choose how many keys will have nested values
    num_nesting_keys = random.randint(1, len(selected_keys))
    nesting_keys = random.sample(selected_keys, num_nesting_keys)
    key_values = []
    
    for key_name, data_type in selected_keys:
        if (key_name, data_type) in nesting_keys:
            nested_value = generate_nested_value(keys_dict, current_depth + 1, target_depth, max_keys, max_length)
            key_values.append(f'"{key_name}" : {nested_value}')
        else:  # Other keys can be terminal values
            value = generate_value_by_type(data_type, max_length)
            if value is not None:
                key_values.append(f'"{key_name}" : {value}')
    
    return "{ " + " ; ".join(key_values) + " }"

def generate_data(
    num_lines: int,
    keys_dict: Dict[str, str],
    max_depth: int,
    max_length: int,
    max_keys: int
) -> List[str]:

    data = []
    for i in range(num_lines):
        
        target_depth = random.randint(0, max_depth) # Randomly select a target depth for this line
        top_key = f"key{i + 1}"
        value = generate_nested_value(keys_dict, 0, target_depth, max_keys, max_length)
        
        # Create the full line
        line = f'"{top_key}" : {value}'
        data.append(line)
    
    return data

def main() -> None:
    
    try:
        args = parse_arguments()
        keys = load_key_file(args.k)
        data = generate_data(args.n, keys, args.d, args.l, args.m)
        
        output_file = "dataToIndex.txt"
        with open(output_file, 'w') as f:
            for line in data:
                f.write(line + '\n')
            
        print(f"Data successfully generated and saved to {output_file}")
            
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()