import os
import hashlib
import struct
import argparse

MEMORY_COST = 65536 #memory in KBs
TIME_COST = 3 #number of iterations
PARALLELISM = 1 #number of threads
SALT_SIZE = 16 #16 byte salt
HASH_LENGTH = 32 #hash size in bytes

# generate a string (or salt) of *length random bytes
def gen_salt(length=SALT_SIZE):
    return os.urandom(length)

def manual_hash(password: str, salt: bytes, memory_cost: int, time_cost: int, paralleslism: int) -> bytes:
    # convert original string to utf-8 encoding
    password_bytes = password.encode('utf-8')

    # concatenate password bytes and salt and apply sha512 (.digest() returns as bytes)
    initial_hash = hashlib.sha512(password_bytes + salt).digest()

    # hold all intermediate hashes
    memory_blocks = [initial_hash]

    # create more memory blocks by hashing the last block until memory cost is met
    for _ in range(memory_cost - 1):
        new_block = hashlib.sha512(memory_blocks[-1]).digest()
        memory_blocks.append(new_block)

    # for each iteration in time cost, it iterates over each time block
    # does the XOR of block_a and block_b (makes sure small changes don't happen, we want big changes hence the XOR)
    for _ in range(time_cost):
        for i in range(memory_cost):
            block_a = memory_blocks[i]
            block_b = memory_blocks[(i + 1) % memory_cost]
            mixed_block = bytes(a ^ b for a, b in zip(block_a, block_b))
            memory_blocks[i] = mixed_block

    # combine the number of blocks (PARALLELISM) then slice the remaining byte string to match hash length (HASH_LENGTH)
    final_digest = b''.join(memory_blocks[:paralleslism])[:HASH_LENGTH]
    return final_digest

def main():
    # arg parse in python sucks
    parser = argparse.ArgumentParser(description="manual argon2 hasher")
    parser.add_argument("password",help="password to hash")
    args = parser.parse_args()

    # generate salt and show it (not being used in real life, i know its not secure. chill)
    salt = gen_salt()
    print(f"generated salt (hex): {salt.hex()}")

    # do some hashing
    hashed_password = manual_hash(args.password, salt, MEMORY_COST, TIME_COST, PARALLELISM)

    # gimme the hash
    print(f"hashed password (hex): {hashed_password.hex()}")

if __name__ == "__main__":
    main()