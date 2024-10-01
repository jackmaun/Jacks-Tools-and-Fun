import argparse
from argon2 import manual_hash


MEMORY_COST = 65536 #memory in KBs
TIME_COST = 3 #number of iterations
PARALLELISM = 1 #number of threads

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Verify a password against a hashed value.")
    parser.add_argument("password", help="Password to verify")
    parser.add_argument("salt", help="Salt used for hashing (hex)")
    parser.add_argument("hash", help="Stored hashed password (hex)")

    args = parser.parse_args()

    salt_bytes = bytes.fromhex(args.salt)
    stored_hash_bytes = bytes.fromhex(args.hash)

    computed_hash = manual_hash(args.password, salt_bytes, MEMORY_COST, TIME_COST, PARALLELISM)

    if computed_hash == stored_hash_bytes:
        print("password is valid")
    else:
        print("invalid password")
