// Converter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

//variables

int Num;

vector<double> X;
vector<double> Y;
vector<double> Z;


bool readFile(string _filepath) {
    ifstream myfile;
    myfile.open(_filepath);

    string line;
    string oneWord;

    if (myfile.is_open())
    {
        cout << "Reading File" << _filepath << "..." << endl;
        while (getline(myfile, line)) {
            std::stringstream sStream;
            sStream << line;
            oneWord = "";
            sStream >> oneWord;
            X.push_back(stod(oneWord));
            sStream >> oneWord;
            Y.push_back(stod(oneWord));
            sStream >> oneWord;
            Z.push_back(stod(oneWord));

            Num++;
        }

        myfile.close();
        return true;
    }
    else cout << "ERROR!";
    return false;
}
bool writeFile(string _filepath) {




    ofstream myfile;
    myfile.open(_filepath);
    if (myfile.is_open())
    {
        cout << "Writing File" << _filepath << "... ";
        myfile << Num << endl;
        for (int i = 0; i < Num; i++) {  
            myfile << fixed << setprecision(2) << X[i] << " " << Y[i] << " " << Z[i] << endl;
        }
        myfile.close();
        cout << "Done!" << endl;
        return true;
    }
    else
        cout << "error writing file..";
    return false;
}

int main()
{
    readFile("../../lasdata1.txt");
    readFile("../../lasdata2.txt");
    writeFile("../../VertexData.txt");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
