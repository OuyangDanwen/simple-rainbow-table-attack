/*--------
(1) Read in a Rainbow table (built using B.cpp)
(2) Read 1000 digests from standard input and  output the preimage. 
------------*/

#include <iostream>
#include <iomanip>
#include "sha1.h"
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
using namespace std;
unsigned long TOTAL_SHA=0;       // Count the number of hashes performed.

//-------   Data Structure for searching    -----------//
unordered_map <int, vector<int>> HashTable;
unordered_map <int, vector<int>>::const_iterator G;

unsigned int L_CHAIN = 220;
const unsigned int row = (2.36 * (1 << 24)) / 220;
unsigned char M[row][3];    // array to store the word read from the table (head of chain)
unsigned char HEAD[row][3];
unsigned D[row][5];
unsigned char offset[row];

int createMask(int a, int b)
{
    int r = 0;
    for (int i = a; i<=b; i++)
        r |= 1 << i;

    return r;
}

//-----------    Hash     ----------------------------//
int Hash (unsigned char m[3], unsigned int d[5] )
{
   SHA1 sha;
   sha.Reset(); sha.Input(m[0]); sha.Input(m[1]); sha.Input(m[2]);  
   sha.Result(d);

   TOTAL_SHA = TOTAL_SHA +1;
   return(0);
}

//-----------    Reduce  -----------------------------//
//   d:   input digest
//   m:   output word
//   i:   the index of the reduce function 
//---------------------------------------------------//
int Reduce1 (unsigned int d[5], unsigned char m[3],  int i )
{

   m[0]=(unsigned char)( (d[0]+i ) %256);   //8 bits
   m[1]=(unsigned char)( (d[1]   ) %256);   //8 bits
   m[2]=(unsigned char)( (d[2]   ) %256);   //8 bits

   return(0);
}

int Reduce2 (unsigned int d[5], unsigned char m[3], int i) {
    m[0]=(unsigned char)( (d[0] ) %256); //8 bits
    m[1]=(unsigned char)( (d[1] + i ) %256);   //8 bits
    m[2]=(unsigned char)( (d[2]   ) %256);   //8 bits
}


bool isEqual (unsigned int d1[5], unsigned int d2[5]) {
   return d1[0] == d2[0] && d1[1] == d2[1] && d1[2] == d2[2] && d1[3] == d2[3] && d1[4] == d2[4] ;
}

//------------  Read in the Table ------------------//
//   Store the result in M and D                    //
int ReadT()
{


    unsigned int table1_last = row / 2;

    FILE* table1;
    table1 = fopen("table1.txt", "rb");

    for (unsigned long i = 0; i < table1_last; i++) {
        fread (&(M[i][0]),  sizeof( unsigned char), 1, table1);
        fread (&(M[i][1]),  sizeof( unsigned char), 1, table1);
        fread (&(M[i][2]),  sizeof( unsigned char), 1, table1);
    }

    FILE* table2;
    table2 = fopen("table2.txt", "rb");

    for (unsigned long i = table1_last; i < row; i++) {
        fread (&(M[i][0]),  sizeof( unsigned char), 1, table2);
        fread (&(M[i][1]),  sizeof( unsigned char), 1, table2);
        fread (&(M[i][2]),  sizeof( unsigned char), 1, table2);
    }

    FILE* skipFile = fopen("skiplist.txt", "rb");
    int numPack = (row - 1) / 2 + 1;
    unsigned char pack;
    for (int i = 0; i < numPack - 1; i++) {
        fread(&pack, sizeof(unsigned char), 1, skipFile);
        offset[2 * i] = pack % 16;
        offset[2 * i + 1] = (pack >> 4) % 16;
    }
    fread(&pack, sizeof(unsigned char), 1, skipFile);
    unsigned int leftover = row - (numPack - 1) * 2;
    for (int i = 0; i < leftover; i++) {
        offset[i + 2 * numPack] = (pack >> (4 * i)) % 16;
    }

    return(0);
}


//------------------------------------------------------------------------------------
//      Given a digest,  search for the pre-image   answer_m[3].
//------------------------------------------------------------------------------------
int search( unsigned int target_d[5] ,   unsigned char answer_m[3])
{

    unsigned int table1_last = row / 2;

    for(int i = 0; i < row; i++) {
        if (isEqual(target_d, D[i])) {
                answer_m[0] = M[i][0];
                answer_m[1] = M[i][1];
                answer_m[2] = M[i][2];
                return 1;
        }
    }



    for (int i = L_CHAIN - 2; i >= 0; i--) {
        unsigned char Colour_m[3];
        unsigned int Colour_d[5];
        Colour_d[0] = target_d[0];
        Colour_d[1] = target_d[1];
        Colour_d[2] = target_d[2];
        Colour_d[3] = target_d[3];
        Colour_d[4] = target_d[4];
        for (int j = i; j <= L_CHAIN - 3; j++) {
            Reduce1(Colour_d, Colour_m, j);
            Hash(Colour_m, Colour_d);
        }
        Reduce1(Colour_d, Colour_m, L_CHAIN - 2);

        int converted = Colour_m[0] + (Colour_m[1] << 8) + (Colour_m[2] << 16);
        G = HashTable.find(converted);
        if (G != HashTable.end()) {
            vector<int> indices;
            indices = HashTable[converted];
            int size = indices.size();
            for (int j = 0; j < size; j++) {//loop through the index vector to locate the correct chain
                if (indices[j] < table1_last) {
                    unsigned char find_m[3];
                    find_m[0] = HEAD[indices[j]][0];
                    find_m[1] = HEAD[indices[j]][1];
                    find_m[2] = HEAD[indices[j]][2];
                    unsigned int find_d[5];
                    for (int k = 0; k < i; k++) {//repeat the chain to locate the password
                        Hash(find_m, find_d);
                        Reduce1(find_d, find_m, k);
                    }

                    Hash(find_m, find_d);

                    if (isEqual(find_d, target_d)) {
                        answer_m[0] = find_m[0];
                        answer_m[1] = find_m[1];
                        answer_m[2] = find_m[2];
                        return 1;
                    }
                }

            }
        }

    }

    for (int i = L_CHAIN - 2; i >= 0; i--) {

        unsigned char Colour_m[3];
        unsigned int Colour_d[5];
        Colour_d[0] = target_d[0];
        Colour_d[1] = target_d[1];
        Colour_d[2] = target_d[2];
        Colour_d[3] = target_d[3];
        Colour_d[4] = target_d[4];
        for (int j = i; j <= L_CHAIN - 3; j++) {
            Reduce2(Colour_d, Colour_m, j);
            Hash(Colour_m, Colour_d);
        }
        Reduce2(Colour_d, Colour_m, L_CHAIN - 2);

        int converted = Colour_m[0] + (Colour_m[1] << 8) + (Colour_m[2] << 16);
        G = HashTable.find(converted);
        if (G != HashTable.end()) {
            vector<int> indices;
            indices = HashTable[converted];
            int size = indices.size();
            for (int j = 0; j < size; j++) {//loop through the index vector to locate the correct chain
                if (indices[j] < row && indices[j] >= table1_last) {
                    unsigned char find_m[3];
                    find_m[0] = HEAD[indices[j]][0];
                    find_m[1] = HEAD[indices[j]][1];
                    find_m[2] = HEAD[indices[j]][2];
                    unsigned int find_d[5];
                    for (int k = 0; k < i; k++) {//repeat the chain to locate the password
                        Hash(find_m, find_d);
                        Reduce2(find_d, find_m, k);
                    }
                    Hash(find_m, find_d);
                    if (isEqual(find_d, target_d)) {
                        answer_m[0] = find_m[0];
                        answer_m[1] = find_m[1];
                        answer_m[2] = find_m[2];
                        return 1;
                    }

                }
            }
        }

    }

    return 0;//cannot find
}


//-----------   reading the next digest from the standard input  ----------------//
void readnextd (unsigned  int d[5])
{
   cin.setf(ios::hex,ios::basefield); cin.setf(ios::uppercase);
   cin >> d[0]; cin >> d[1]; cin >> d[2]; cin >> d[3]; cin >> d[4];
}


int main( int argc, char*argv[])
{
    int found;
    int total_found;
    int total_not_found;

    SHA1        sha;
    unsigned int d[5];   // 32 x 5 = 160 bits



    //------------ R E A D     R A I N B O W    T A B L E  --------//
    ReadT();       cout << "READ DONE" << endl;
    unsigned int digest[5];
    unsigned int converted;
    unsigned int rowNum = 0;
    for (int i = 0; i < row; i++) {
        rowNum += offset[i];
        HEAD[i][0] = (unsigned char) (rowNum & createMask(0, 7));
        HEAD[i][1] = (unsigned char) ((rowNum & createMask(8, 15)) >> 8);
        HEAD[i][2] = (unsigned char) ((rowNum & createMask(16, 23)) >> 16);
        Hash(M[i], digest);
        D[i][0] = digest[0];
        D[i][1] = digest[1];
        D[i][2] = digest[2];
        D[i][3] = digest[3];
        D[i][4] = digest[4];
        converted = M[i][0] + (M[i][1] << 8) + (M[i][2] << 16);
        HashTable[converted].push_back(i);
        rowNum ++;
    }


    //--------  PROJECT  INPUT/OUTPUT FORMAT ----------------//

    total_found=0;
    total_not_found=0;

    cout.setf(ios::hex,ios::basefield);       //   setting display to Hexdecimal format.  (this is the irritating part of using C++).
    cout.setf(ios::uppercase);

    unsigned char m[3];

    for (int i=0; i<5000; i++)
      {
        readnextd(d);
          unsigned int hash[5];
          if (search (d,m))
               {
                       total_found++;
                       //------   print the word in hexdecimal format   -----------
                       cout << setw(1) << (unsigned int) m[0] / 16;
                       cout << setw(1) << (unsigned int) m[0] % 8;
                       cout << setw(1) << (unsigned int) m[1] / 16;
                       cout << setw(1) << (unsigned int) m[1] % 8;
                       cout << setw(1) << (unsigned int) m[2] / 16;
                       cout << setw(1) << (unsigned int) m[2] % 8 << endl;
                }
        else
               {  total_not_found ++;
                  cout << setw(6)<< 0 << endl;
               }
      }

    cout.setf(ios::dec);
    cout << "Accuracy       C is: " << total_found/5000.0 << endl;
    cout << "Speedup factor F is: " << (5000.0/TOTAL_SHA)*8388608 << endl;

}

