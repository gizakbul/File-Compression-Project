#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#define OFFSETBITS 5
#define LENGTHBITS (8-OFFSETBITS)

#define OFFSETMASK ((1 << (OFFSETBITS)) - 1)
#define LENGTHMASK ((1 << (LENGTHBITS)) - 1)

#define GETOFFSET(x) (x >> LENGTHBITS)
#define GETLENGTH(x) (x & LENGTHMASK)
#define OFFSETLENGTH(x,y) (x << LENGTHBITS | y)



/*
*
*
*
*
*
*LZ77 Algoritmasına ait kodlar https://ysar.net/algoritma/lz77.html sitesinde bulunan koddan uyarlanmıştır.
*
*
*DEFLATE ve HUFFMAN KODLAMASI'na ait kodlar  https://gist.github.com/yasar11732/30d2fc9c1c404d776218424e5e3ca795#file-huffman-c-L243  sitesinde bulunan koddan uyarlanmıştır.
*
*
*
*
*
*/

FILE *lz77_output,*deflate_output;
float lz77_p, deflate_p;

#ifdef _MSC_VER

__pragma(pack(push, 1))
struct huffmancode
{
    uint16_t code;
    uint8_t len;
};
__pragma(pack(pop))
#elif defined(__GNUC__)
struct __attribute__((packed)) huffmancode
{
    uint16_t code;
    uint8_t len;
};
#endif

// her bir code için uzunluk ve kod değerlerini
// tuttuğumuz array
struct huffmancode tree[0X100];

// her uzunlukta kaç adet kod
// olduğunu saymak için
uint8_t bl_count[0x10];

// her uzunluktaki kodlama için
// atanacak kodu tutar
uint16_t next_code[0x10];


typedef struct _huffman
{
    char c;
    int freq;
    struct _huffman *left;
    struct _huffman *right;
} HUFFMANTREE;

typedef struct _huffman_array
{
    int cap;
    int size;
    HUFFMANTREE **items;
} HUFFMANARRAY;

HUFFMANTREE *huffmantree_new(char c, int freq)
{
    HUFFMANTREE *t = malloc(sizeof(HUFFMANTREE));
    t->c = c;
    t->freq = freq;
    t->left = NULL;
    t->right = NULL;
    return t;
}

struct token
{
    uint8_t offset_len;
    char c;
};
int prefix_match_length(char *s1, char *s2, int limit)
{
    int len=0;
    while(*s1++ == *s2++ && len<limit)
    {
        len++;
    }
    return len;
}
void lz77memcpy(char *s1,char *s2,int size)
{
    while(size--)
        *s1++=*s2++;
}
struct token* lz77(char *text,int limit,int *numTokens)
{

    int cap=1<<3;
    int _numTokens=0,max_len,len,a;
    char b[limit];
    int i=0,n=0;

    lz77_output=fopen("LZ77Output.txt","w");

    struct token t;
    struct token *encoded=malloc(cap*sizeof(struct token));
    char *search_buffer, *next_buffer;

    printf("  TOKEN LIST: \n");
    fprintf(lz77_output,"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  TOKEN LIST: \n");

    for(next_buffer=text; next_buffer<text+limit ; next_buffer++)
    {
        search_buffer=next_buffer-OFFSETMASK;
        if(search_buffer<text)
        {
            search_buffer=text;
        }

        max_len=0;

        char *max_match=next_buffer;

        for(; search_buffer<next_buffer; search_buffer++)
        {
            len=prefix_match_length(search_buffer,next_buffer,LENGTHMASK);

            if(len>max_len)
            {
                max_len=len;
                max_match=search_buffer;
            }
        }

        if(next_buffer+max_len>= text+limit)
        {
            max_len=text+limit-next_buffer-1;
        }

        t.offset_len=OFFSETLENGTH(next_buffer-max_match,max_len);

        a=prefix_match_length(next_buffer,search_buffer,max_len);

        next_buffer+=max_len;
        t.c=*next_buffer;

        printf("  [ %d %d %c ]\n",t.offset_len/8,a,t.c);
        fprintf(lz77_output,"  [ %d %d %c ]\n",t.offset_len/8,a,t.c);

        b[i]=t.c;
        i++;
        n++;

        if(_numTokens+1>cap)
        {
            cap=cap<<1;
            encoded=realloc(encoded,cap*sizeof(struct token));
        }
        encoded[_numTokens++]=t;
    }
    if(numTokens)
        *numTokens=_numTokens;

    fprintf(lz77_output,"\n");
    printf("\n");
    fprintf(lz77_output,"  Baslangic Boyut : %d",limit);
    fprintf(lz77_output,"\n\n");
    fprintf(lz77_output,"  LZ77 Sonrasi Boyut : %d",(_numTokens*sizeof(struct token))/2);
    printf("  LZ77 Sonrasi Boyut : %d\n\n",(_numTokens*sizeof(struct token))/2);
    fprintf(lz77_output,"\n\n");
    printf("  Encoded Boyut : %d",_numTokens*sizeof(struct token));
    printf("\n");
    fprintf(lz77_output,"  Encoded Boyut : %d",_numTokens*sizeof(struct token));
    fprintf(lz77_output,"\n\n");
    fprintf(lz77_output,"  LZ77 ile Sikistirilmis Metin : ");
    printf("\n  LZ77 ile Sikistirilmis Metin : ");
    for(int k=0; k<n; k++)
    {
        fprintf(lz77_output,"%c",b[k]);
        printf("%c",b[k]);
    }
    printf("\n\n");
    fprintf(lz77_output,"\n\n");

    lz77_p= (float)limit / (float) ((_numTokens*sizeof(struct token))/2);
    float lz77_sonuc=(float)100/lz77_p;
    printf("  LZ77 Algoritmasi ile sikistirilmis metin ,orjinal metnin %%%f kadaridir.(%.2f / %.2f = %.2f)\n",lz77_sonuc, (float)limit, (float) ((_numTokens*sizeof(struct token))/2), lz77_sonuc);
    fprintf(lz77_output,"  LZ77 Algoritmasi ile sikistirilmis metin , orjinal metnin %%%f kadaridir.(%.2f / %.2f = %.2f)\n",lz77_sonuc, (float)limit, (float) ((_numTokens*sizeof(struct token))/2), lz77_sonuc);
    fclose(lz77_output);
    return encoded;
}
/*
* Selection sort, büyükten küçüðe
*/

void huffman_array_sort(HUFFMANARRAY *arr)
{
    int i, k;
    int max_index, max_value;
    for (i = 0; i < arr->size - 1; i++)
    {
        max_index = i;
        max_value = arr->items[i]->freq;

        for (k = i + 1; k < arr->size; k++)
        {
            if (arr->items[k]->freq > max_value)
            {
                max_value = arr->items[k]->freq;
                max_index = k;
            }
        }
        if (i != max_index)
        {
            HUFFMANTREE *_tmp = arr->items[i];
            arr->items[i] = arr->items[max_index];
            arr->items[max_index] = _tmp;
        }
    }
}

HUFFMANTREE *huffman_array_pop(HUFFMANARRAY *arr)
{
    return arr->items[--arr->size];
}
void *huffman_array_add(HUFFMANARRAY *arr, HUFFMANTREE *t)
{
    if (arr->size + 1 == arr->cap)
    {
        arr->cap *= 2;
        arr->items = realloc(arr->items, arr->cap * sizeof(HUFFMANTREE *));
    }
    arr->items[arr->size++] = t;
}

HUFFMANARRAY *huffman_array_new()
{
    HUFFMANARRAY *arr = malloc(sizeof(HUFFMANARRAY));
    arr->cap = 8;
    arr->size = 0;
    arr->items = malloc(arr->cap * sizeof(HUFFMANTREE *));
    return arr;
}

void huffmantree_print(HUFFMANTREE *t, char *prefix, int size_prefix)
{
    if (t->left == NULL && t->right == NULL)
    {
        prefix[size_prefix] = 0;
        printf("%c: %s\n", t->c, prefix);
        return;
    }
    if (t->left)
    {
        prefix[size_prefix++] = '0';
        huffmantree_print(t->left, prefix, size_prefix);
        size_prefix--;
    }
    if (t->right)
    {
        prefix[size_prefix++] = '1';
        huffmantree_print(t->right, prefix, size_prefix);
        size_prefix--;
    }
}

void load_canonical_codes_from_tree(HUFFMANTREE *t, int length)
{
    if (!t)
        return;
    if (t->c != 0)
    {
        tree[t->c].len = length;
    }

    load_canonical_codes_from_tree(t->left, length + 1);
    load_canonical_codes_from_tree(t->right, length + 1);
}

char *code_to_binary(struct huffmancode code)
{
    char *b = malloc(code.len + 1); // +1 null
    int i;
    for (i = 0; i < code.len; i++)
    {
        b[i] = code.code & (1 << (code.len - i - 1)) ? '1' : '0';
    }
    b[code.len] = 0;
    return b;
}

void deflate(char *text,int limit, int *numTokens)
{


    char *search_buffer, *next_buffer;
    int cap=1<<3;
    int _numTokens=0,max_len,len,a;
    char b[500];
    int i=0;

    struct token t;
    struct token *encoded=malloc(cap*sizeof(struct token));

    deflate_output=fopen("deflateOutput.txt","w");

    printf("  Huffman Agacina Alinacak Cumle: ");
    fprintf(deflate_output,"  Huffman Agacina Alinan Cumle: ");
    for(next_buffer=text; next_buffer<text+limit ; next_buffer++)
    {
        search_buffer=next_buffer-OFFSETMASK;
        if(search_buffer<text)
        {
            search_buffer=text;
        }
        max_len=0;

        char *max_match=next_buffer;

        for(; search_buffer<next_buffer; search_buffer++)
        {
            len=prefix_match_length(search_buffer,next_buffer,LENGTHMASK);
            if(len>max_len)
            {
                max_len=len;
                max_match=search_buffer;
            }
        }
        if(next_buffer+max_len>= text+limit)
        {
            max_len=text+limit-next_buffer-1;
        }

        t.offset_len=OFFSETLENGTH(next_buffer-max_match,max_len);
        a=prefix_match_length(next_buffer,search_buffer,max_len);

        next_buffer+=max_len;
        t.c=*next_buffer;
        printf("%c",t.c);
        fprintf(deflate_output,"%c",t.c);
        b[i]=t.c;
        i++;
        if(_numTokens+1>cap)
        {
            cap=cap<<1;
            encoded=realloc(encoded,cap*sizeof(struct token));
        }


        encoded[_numTokens++]=t;

        if(numTokens)
            *numTokens=_numTokens;
    }
    printf("\n");
    fprintf(deflate_output,"\n");
    fprintf(deflate_output,"\n");
    fprintf(deflate_output,"  METNIN DEFLATE SIKISTIRILMIS HALI : ");
    fprintf(deflate_output,"\n");
    huffman(b);

    fclose(deflate_output);
}

void huffman(char metin[500])
{
    int n=strlen(metin);
    FILE *f=fopen("metin.txt","r");
    char s[200];
    int total=0;
    while(fgets(s,500,f)!=NULL)
    {
        total+=strlen(s);
    }
    unsigned long frequencies[0xFF];
    char *pcTemp;
    int i;
    unsigned long frequencies2[0xFF];
    HUFFMANARRAY *arr = huffman_array_new();

    HUFFMANARRAY *arr2 = huffman_array_new();

    memset(&frequencies[0], 0, sizeof(frequencies));

    memset(&frequencies2[0], 0, sizeof(frequencies2));

    for (pcTemp = &metin[0]; *pcTemp != 0; pcTemp++)
    {
        frequencies[(int)*pcTemp]++;
    }
    for (i = 0; i < 255; i++)
    {
        if (frequencies[i] > 0)
        {
            huffman_array_add(arr, huffmantree_new(i, frequencies[i]));

        }
    }
    while (arr->size > 1)
    {
        huffman_array_sort(arr);
        HUFFMANTREE *t1 = huffman_array_pop(arr);
        HUFFMANTREE *t2 = huffman_array_pop(arr);
        HUFFMANTREE *t3 = calloc(1, sizeof(HUFFMANTREE));
        t3->left = t1;
        t3->right = t2;
        t3->freq = t1->freq + t2->freq;
        huffman_array_add(arr, t3);
    }
    memset(tree, 0, sizeof(tree));
    memset(bl_count, 0, sizeof(bl_count));
    memset(next_code, 0, sizeof(next_code));

    load_canonical_codes_from_tree(huffman_array_pop(arr), 0);

    for (i = 0; i < 256; i++)
    {
        bl_count[tree[i].len]++;
    }
    int code = 0;
    bl_count[0] = 0;

    for (i = 1; i < 0x10; i++)
    {
        code = (code + bl_count[i - 1]) << 1;
        next_code[i] = code;
    }

    for (i = 0; i < 0x100; i++)
    {
        int len = tree[i].len;
        if (len)
        {
            tree[i].code = next_code[len];
            next_code[len]++;
        }
    }

    int len;

    for (i = 0; i < 0x100; i++)
    {
        len = tree[i].len;
        if (len)
        {
            printf("  %c: %s\n", i, code_to_binary(tree[i]));
            fprintf(deflate_output,"  %c: %s\n", i, code_to_binary(tree[i]));
        }
    }

    printf("  %s : ",metin);
    fprintf(deflate_output,"  %s : ",metin);
    for(int j=0 ; j<n ; j++)
    {
        for(int k=0 ; k<0x100 ; k++)
        {
            len=tree[i].len;
            if(metin[j]==k)
            {
                printf("  %s",code_to_binary(tree[k]));
                fprintf(deflate_output,"  %s",code_to_binary(tree[k]));
            }
        }
    }
    printf("\n\n");
    fprintf(deflate_output,"\n\n");
    int toplam2=total*8;
    printf("  BASLANGIC TOPLAM BIT SAYISI=%d\n\n",toplam2);
    fprintf(deflate_output,"\n BASLANGIC TOPLAM BIT SAYISI=%d\n\n",toplam2);
    int toplam=0;
    for (i = 0; i < 0x100; i++)
    {
        int len = tree[i].len;
        if (len)
        {
            printf("  %c : %d*%d=>%d\n",i,frequencies[i],len,len*frequencies[i]);
            toplam+=len*frequencies[i];
        }
    }
    printf("\n Deflate ile Sikistirilmis Toplam Bit Sayisi = %d\n\n",toplam);
    fprintf(deflate_output,"  Deflate ile Sikistirilmis Toplam Bit Sayisi = %d\n\n",toplam);

    deflate_p = (float)toplam2 / (float)toplam;
    float defalte_sonuc=(float)100/deflate_p;
    printf("  DEFLATE Algoritmasi ile sikistirilmis metin ,orjinal metnin %%%f kadaridir.(%.2f / %.2f = %.2f)\n",defalte_sonuc,(float)toplam2, (float)toplam, defalte_sonuc);
    fprintf(deflate_output,"  DEFLATE Algoritmasi ile sikistirilmis metin , orjinal metnin %%%f kadaridir.(%.2f / %.2f = %.2f)\n",defalte_sonuc,(float)toplam2, (float)toplam, defalte_sonuc);

    fclose(f);

}

char *file_read(FILE *f, int *size)
{
    char *content;
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    content = malloc(*size);
    fseek(f, 0, SEEK_SET);
    fread(content, 1, *size, f);
    return content;
}
int main()
{
    FILE *dosya=fopen("metin.txt","rb");
    FILE *dosya2 = fopen("metin.txt","rb");
    lz77_output=fopen("LZ77Output.txt","w");
    char *cumle;
    char cumle2[500];
    int *boyut;
    int *tokenSayisi,i=0;
    struct token *encodedMetin;
    if(dosya=fopen("metin.txt","rb"))
    {
        cumle=file_read(dosya,&boyut);
        fclose(dosya);
    }
    printf("\n *************************\n\n");
    printf("        CUMLE: \n");
    fprintf(lz77_output,"       CUMLE:");
    fprintf(deflate_output,"       CUMLE:");

    while(fgets(cumle2,500,dosya2)!=NULL)
    {
        printf("    %s",cumle2);
        fprintf(lz77_output,"    %s",cumle2);
        fprintf(deflate_output,"    %s",cumle2);
    }
    printf("\n\n");


    printf("  BASLANGIC BOYUTU = %d\n\n",boyut);
    printf("\n   ----> LZ77 <----\n\n");
    encodedMetin=lz77(cumle,boyut,&tokenSayisi);
    printf("\n ******************************************\n\n");
    printf("\n   ----> DEFLATE <----\n\n");
    deflate(cumle, boyut,&tokenSayisi);

    printf("\n");
    printf("\n            *************************\n\n");

    if(deflate_p>lz77_p)
    {
        printf("  SONUC : DEFLATE ALGORITMASI DAHA YUKSEK PERFORMANS VERMISTIR.\n\n");
    }
    else if(lz77_p>deflate_p)
    {
        printf("  SONUC : LZ77 ALGORITMASI DAHA YUKSEK PERFORMANS VERMISTIR.\n\n");
    }
    printf("            *************************\n\n PROGRAM SONLANDI, OUTPUT DOSYALARINI KONTROL EDEBILIRSINIZ.\n\n");

    fclose(dosya);

    fclose(dosya2);


    return 0;
}
