#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <time.h>

#define SECONDS_INCREASE 1 * 10
#define SECONDS_DECREASE 2 * 10
#define INITIAL_DIFFICULTY 16
#define DIFFICULTY_ADJUSTMENT_PERIODS 50

typedef struct block {
    int index;
    time_t timestamp;
    unsigned char signature[SHA256_DIGEST_LENGTH];
    unsigned char nonce[33];
    int difficulty;
    struct block* parent;
    char* data;
} block;

typedef struct blockchain {
    block* block;
} blockchain;

// print_sha_hash prints a char with sha256_digest_length
void print_sha_hash(unsigned char d[SHA256_DIGEST_LENGTH]) {
    int i;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
		printf("%02x", d[i]);
	putchar('\n');
}

// print_hex_hash prints hex of char * with length specified by len
void print_hex_hash(unsigned char * d, size_t len) {
    int i;
    for (i = 0; i < len; i++)
		printf("%02x", d[i]);
	putchar('\n');
}

// print_chain prints a chain
void print_chain(blockchain * chain) {
    block * cur = chain->block;
    while(cur != NULL) {
        printf("==============================\n");
        char tbuff[20];
        strftime(tbuff, 20, "%Y-%m-%d %H:%M:%S", localtime(&cur->timestamp));
        printf("Block ID: %d, generated on %s, difficulty %d\n", cur->index, tbuff, cur->difficulty);
        printf("Block Data: %s\n", cur->data);
        if(cur->parent != NULL) {
            printf("Block Parent Data: %s\n", cur->parent->data);
        }
        printf("Block Signature: ");
        print_sha_hash(cur->signature);
        printf("Block Nonce: ");
        print_hex_hash(cur->nonce, 32);
        cur = cur->parent;
    }
}

// free_blockchain traverses a blockchain and frees the memory all around 
void free_blockchain(blockchain * b) {
    block * cur = b->block;
    block * o;
    while(cur != NULL) {
        o = cur;
        free(o->data);
        cur = o->parent;
        free(o);
    }
    free(b);
}

// rand_bytes generates some random bytes for our nonce (use srand to seed)
void rand_bytes(char *bin, size_t size)
{
    if (size) {
        size_t n;
        --size;
        for (n = 0; n < size ; n++) {
            bin[n] = rand();
        }
        bin[size] = '\0';
    }
} 

// block_sa generates an average between the difference of the timestamps over n periods. multiplies result by accuracy to save on floating point operations
int block_sa(block * b, int n, int accuracy) {
    block * cur = b;
    int diff = 0;
    for(int i = 0; i < n; i++) {
        if(cur->parent == NULL) return -1; // If not enough periods available yet, don't worry about it
        diff += (cur->timestamp - cur->parent->timestamp);
        cur = cur->parent;
    }
    return (diff * accuracy) / n;
}

// blockchain_push places the element into the block pointer (linked list-like structure, might be beneficial to also implement a dynamic array)
int blockchain_push(blockchain* b, block* element) {
    // add our nooby difficulty to the element
    if(element->parent == NULL) 
        element->difficulty = INITIAL_DIFFICULTY; // Genesis block - set difficulty to 8
    else if(element->index % DIFFICULTY_ADJUSTMENT_PERIODS != 0) {
        element->difficulty = element->parent->difficulty; // Only set every 50 blocks
    }
    else {
        // Check if we need to adjust the difficulty
        int td = block_sa(element, DIFFICULTY_ADJUSTMENT_PERIODS, 10);
        if(td == -1) {
            // Not enough info yet, keep difficulty the same
            element->difficulty = element->parent->difficulty;
        } else if(td < SECONDS_INCREASE) {
            // if we're less than x seconds per block, make it a little harder
            element->difficulty = element->parent->difficulty + 1;
        } else if(td > SECONDS_DECREASE) {
            // If we're more than x seconds per block, make it a little easier
            element->difficulty = element->parent->difficulty - 1;
        } else {
            // We're in a comfy place, don't change it
            element->difficulty = element->parent->difficulty;
        }
        printf("difficulty adjustment: %d\n", element->difficulty);
    }
    b->block = element;
    return 0;
}

// block_gen_sha256 generates a sha256 signature with various block info into buff
void block_gen_sha256(char * data, char parent_signature[SHA256_DIGEST_LENGTH], char nonce[32], char * buff) {
    size_t len = strlen(data) + SHA256_DIGEST_LENGTH + 32 + 1; // Establish the length of our new string
    char * toSign = malloc(len); // Allocate memory to our buffer
    strcpy(toSign, data);
    strcat(toSign, parent_signature);
    strcat(toSign, nonce);
    SHA256(toSign, len, buff);
    free(toSign); // Original string no longer needed.
}

// block_sign signs a block
int block_sign(block* b) {
    block_gen_sha256(b->data, b->parent->signature, b->nonce, b->signature);
    return 0;
}

// block_sign_genesis signs the genesis block
int block_sign_genesis(block* b) {
    SHA256(b->data, strlen(b->data), b->signature);
    return 0;
}

// difficulty_test verifies the hash complies with our difficulty
int difficulty_test(char hash[SHA256_DIGEST_LENGTH], int difficulty) {
    int i = 0;
    while(difficulty > 0) {
        if(difficulty >= 8) {
            if(hash[i] != 0x00) return 1;
        }
        else {
            if (hash[i] >> (8 - difficulty) != 0) return 1;
        } 
        i++;
        difficulty -= 8;
    }
    return 0;
}

// block_verify verifies a signature is fine and checks the proof of work
int block_verify(blockchain* c, block* b) {
    if(b->nonce[0] == 0) {
        return -1; 
    }
    unsigned char hash[SHA256_DIGEST_LENGTH];
    block_gen_sha256(b->data, c->block->signature, b->nonce, hash);
    if(strcmp(hash, b->signature) == 0 && difficulty_test(hash, c->block->difficulty) == 0) {
        return 0;
    } else {
        return -1;
    }
}

// create_block generates a block
block * create_block(block * parent, char * data) {
    block * n = malloc(sizeof(block));
    n->parent = parent;
    if(parent != NULL) n->index = parent->index + 1;
    else n->index = 0;
    n->data = malloc(strlen(data) + 1);
    strcpy(n->data, data);
    n->timestamp = time(NULL);
    n->nonce[0] = 0;
    return n;
}

int main (void) {

    srand ( time(NULL) );

    blockchain * MyTest = malloc(sizeof(blockchain));

    block * MyGenesisBlock = create_block(NULL, "Genesis Block");
    block_sign_genesis(MyGenesisBlock);
    blockchain_push(MyTest, MyGenesisBlock);

    for(int i = 0; i < 200; i++) {
        block * NextBlock = create_block(MyTest->block, "Another Test Block");
        while(block_verify(MyTest, NextBlock) != 0) {
            rand_bytes(NextBlock->nonce, 33);
            block_sign(NextBlock);
        }
        if(blockchain_push(MyTest, NextBlock) != 0) {
            return 0;
        }
    }

    print_chain(MyTest);

    free_blockchain(MyTest);
 
	return 0;
}