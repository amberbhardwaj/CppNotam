/*
The MIT License (MIT)
Copyright (c) <2021> <Amber Bhardwaj>
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// This file contains the code to crack the first problem

#include <iostream>
#include <vector>
#include  <set>
using namespace std;

class Trace{
public:
    Trace(vector<vector<int>> &im, int s){
        size = s;
        r = 0;
        c = 0;
        d = 0;
        m = im;
        cout << "s = " << m[0][1] << endl;
    }
    ~Trace(){}
    // Is unique
    bool isUnique(vector<int> &v)
    {
        cout << "[";
        for(auto itr : v)
        cout << itr << " ";
        cout << "]\n"; 
        set<int>s(v.begin(), v.end());
        return (s.size()!=v.size());
    }
    int diagonalSum()
    {
        for (int i = 0; i < size; i++)
        {
            for(int j = 0; j < size; j++)
            {
                // finding sum of primary diagonal 
                if (i == j) 
                    d += m[i][j];
            }
        }
        return d;
    }
    
    void hasUniqueMatrixRC()
    {
        r = 0;
        c = 0;
        vector<int> v;
        for (int i = 0; i < size; i++)
        {
            for(int j = 0; j < size; j++)
            {
                v.push_back((m[j][i]));
            }
            
            // Calculate number of rows, have duplicate values
            if (isUnique(m[i]))
                r++;
            cout << "------------\n";
            if (isUnique(v))
                c++;
    
            v.clear();
        }
    }
    
    
    // variable
    int size;
    int r;
    int c;
    int d;
    vector<vector<int>> m;
};


int main ()
{
    int T=0, size=0;
    cin >> T;
    for (int tc = 0; tc < T; tc++)
    {
        vector<vector<int>>latix;
        cin >> size;
        vector<int> v ;
        for (int i = 0; i < size; i++)
        {
            int val = 0;
            for(int j = 0; j < size; j++)
            {
                cin >> val;
                v.push_back(val);
            }
            latix.push_back(v);
            v.clear();
        }
    }
    Trace tobj(latix, size);
    tobj.hasUniqueMatrixRC();

    cout << tobj.diagonalSum() << " " << tobj.r << " " << tobj.c <<endl;
}
