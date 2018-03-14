/*----------------------------------
This program build the rainbow table  for the other program F.cpp.
------------------------------------*/

#include <iostream>
#include <unordered_map>
#include "sha1.h"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <iomanip>

using namespace std;


//  A table to store all the words and digests. 
//    infeasible to have such large table in practice.   
//    for programming convenient, we store the whole table in memory.


unordered_map<string, unsigned int> lastDigests;
unordered_map <string, unsigned int>::const_iterator G;
unsigned int L_CHAIN = 220;
const unsigned int numEntry = (2.36 * (1 << 24)) / 220;
unsigned char M[numEntry][3];    // array to store the word read from the table (head of chain)
unsigned char offset[numEntry];
unsigned int D[numEntry][5];


int createMask(int a, int b)
{
    int r = 0;
    for (int i = a; i<=b; i++)
        r |= 1 << i;

    return r;
}


//-------   Hash
int Hash (unsigned char m[3], unsigned int d[5] )
{
   SHA1 sha;
   sha.Reset(); sha.Input(m[0]); sha.Input(m[1]); sha.Input(m[2]); 
   sha.Result(d);
   return(0);
}



//-------  Reduce
int Reduce1 (unsigned int d[5], unsigned char m[3],  int i )
{
   m[0]=(unsigned char)( (d[0] + i ) %256); //8 bits
   m[1]=(unsigned char)( (d[1]   ) %256);   //8 bits
   m[2]=(unsigned char)( (d[2]   ) %256);   //8 bits

   return(0);
}

int Reduce2 (unsigned int d[5], unsigned char m[3], int i) {
    m[0]=(unsigned char)( (d[0] ) %256); //8 bits
    m[1]=(unsigned char)( (d[1] + i ) %256);   //8 bits
    m[2]=(unsigned char)( (d[2]   ) %256);   //8 bits
}


int buildT()
{

    unsigned int  d[5];
    unsigned char m[3];


    string word;
    stringstream ss;
    unsigned int row = 0;
    unsigned int table1_last = numEntry / 2;
    unsigned char limit = 0;
    unsigned int counter = 0;

    while (row < table1_last)
    {
        m[0]= (unsigned char) (counter & createMask(0, 7));
        m[1]= (unsigned char) ((counter & createMask(8, 15)) >> 8);
        m[2]= (unsigned char) ((counter & createMask(16, 23)) >> 16);
        counter ++;


        // build the chain.
        // check whether to keep the chain.
        // You may want to drop the chain, for e.g. if the digest is already in the table.
        // This form the main component of your program.
        for (int j = 0; j <= L_CHAIN - 2; j++) {
            Hash(m, d);
            Reduce1(d, m, j);
        }

        Hash(m, d);

        ss << d[0];
        ss << d[1];
        ss << d[2];
        ss << d[3];
        ss << d[4];
        word = ss.str();
        ss.str("");

        G = lastDigests.find(word);
        if (G != lastDigests.end() && limit < 15) {
            limit++;
            continue;
        }
        else {
            lastDigests[word] = row;
        }

        M[row][0] = m[0];
        M[row][1] = m[1];
        M[row][2] = m[2];
        offset[row] = limit;
        limit = 0;
        row++;
    }

    while (row < numEntry)
    {
        m[0]= (unsigned char) (counter & createMask(0, 7));
        m[1]= (unsigned char) ((counter & createMask(8, 15)) >> 8);
        m[2]= (unsigned char) ((counter & createMask(16, 23)) >> 16);
        counter ++;


        // build the chain.
        // check whether to keep the chain.
        // You may want to drop the chain, for e.g. if the digest is already in the table.
        // This form the main component of your program.
        for (int j = 0; j <= L_CHAIN - 2; j++) {
            Hash(m, d);
            Reduce2(d, m, j);
        }

        Hash(m, d);

        ss << d[0];
        ss << d[1];
        ss << d[2];
        ss << d[3];
        ss << d[4];
        word = ss.str();
        ss.str("");

        G = lastDigests.find(word);
        if (G != lastDigests.end() && limit < 15) {
            limit++;
            continue;
        }
        else {
            lastDigests[word] = row;
        }

        M[row][0] = m[0];
        M[row][1] = m[1];
        M[row][2] = m[2];
        offset[row] = limit;
        limit = 0;
        row++;
    }

        //---    Write to the output file
        //note that to reduce the size of the table, it is not neccessary to write the full digest.
    FILE* table1 ;
    table1 = fopen("table1.txt", "wb");

    for (unsigned int i = 0; i < table1_last; i++)
    {
        fwrite(&M[i][0], sizeof(unsigned char), 1, table1);
        fwrite(&M[i][1], sizeof(unsigned char), 1, table1);
        fwrite(&M[i][2], sizeof(unsigned char), 1, table1);
    }


    FILE* table2 ;
    table2 = fopen("table2.txt", "wb");

    for (unsigned long i = table1_last; i < numEntry; i++)
    {
        fwrite(&M[i][0], sizeof(unsigned char), 1, table2);
        fwrite(&M[i][1], sizeof(unsigned char), 1, table2);
        fwrite(&M[i][2], sizeof(unsigned char), 1, table2);

    }


    FILE* skipFile = fopen("skiplist.txt", "wb");
    int numPack = (numEntry - 1) / 2 + 1;
    unsigned char pack;
    for (int i = 0; i < numPack - 1; i++) {
        pack = offset[2*i] + (offset[2*i+1] << 4);
        fwrite(&pack, sizeof(unsigned char), 1, skipFile);
    }

    int leftover = numEntry - (numPack - 1) * 2;
    for (int i = 0; i < leftover; i++) {
        pack += (offset[i + 2 * numPack]) << (4*i);
    }
    fwrite(&pack, sizeof(unsigned char), 1, skipFile);

    cout << "WRITE DONE" << endl;

    return(0);
}

int main( int argc, char*argv[])
{

    SHA1        sha;

    //----  Setting the parameters
   
    //----   Build the table.
    buildT();
}

