#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define N 64
#define Q 12289
#define ROOT 3

uint32_t mod_exp(uint32_t base, uint32_t exp, uint32_t mod){
    uint32_t result = 1;
    while(exp>0){
        if(exp%2==1){
            result=(result*base)%mod;
        }
        base=(base*base)%mod;
        exp/=2;
    }
    return result;
}

void bit_reverse(uint32_t *arr, int n){
    int j=0;
    for(int i=1;i<n;i++){
        int bit = n>>1;
        while(j&bit){
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if(i<j){
            uint32_t temp=arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
}

void ntt(uint32_t *a, uint32_t n, uint32_t q){
    bit_reverse(a,n);
    for(int len=2;len<=n;len<<=1){
        uint32_t wlen = mod_exp(ROOT,(q-1)/len,q);
        for(int i=0;i<n;i+=len){
            uint32_t w = 1;
            for(int j=0;j<len/2;j++){
                uint32_t u = a[i+j];
                uint32_t v = (a[i+j+len/2]*w)%q;
                a[i+j] = (u+v) % q;
                a[i+j+len/2] = (u+q-v) % q;
                w = (w*wlen) % q;
            }
        }
    }
}

void intt(uint32_t *a, uint32_t n, uint32_t q){
    ntt(a,n,q);
    uint32_t n_inv = mod_exp(n,q-2,q);
    for(int i=0;i<n;i++){
        a[i] = (a[i]*n_inv) % q;
    }
}

void poly_mult(uint32_t *a, uint32_t *b, uint32_t *result, uint32_t n, uint32_t q){
    ntt(a,n,q);
    ntt(b,n,q);
    for(int i=0;i<n;i++){
        result[i] = (a[i]*b[i]) % q;
    }
    intt(result,n,q);
}

void swifft(uint8_t *input, uint32_t *hash){
    uint32_t poly1[N] ={ 0}, poly2[N] = {0}, result[N] = {0};
    for(int i=0;i<N;i++){
        poly1[i] = input[i] % Q;
        poly2[i] = (input[i]*3+7) % Q;
    }
    poly_mult(poly1,poly2,result,N,Q);
    for(int i=0;i<N;i++){
        hash[i] = result[i] % 256;
    }
}

// can add input in this struct
struct Swifft{
    static void run(){
        uint8_t input[N] = "JACKISSOCOOL";
        uint32_t hash[N];
        swifft(input,hash);
        printf("SWIFFT Hash:\n");
        for(int i=0;i<N;i++){
            printf("%02x ",hash[i]);
        }
        printf("\n");
    }
};

int main(){
    Swifft::run();
    return 0;
}

