#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TSystem.h"
#include "TStyle.h"

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data";

void cut_plot() {
    string txtDir = string(basePath) + "/" + string(dataDir) + "/";
    string filePath = txtDir + "cut.txt";

    // ファイルの読み込み
    ifstream inFile(filePath.c_str());
    if (!inFile) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }

    vector<double> ptDiffValues;  // pt差分の値を格納するベクトル
    vector<double> ptValues;      // id_trk_ptの値を格納するベクトル
    double pt_diff, pt_value;

    // ファイルから値を2列読み込み、対応するベクトルに格納
    while (inFile >> pt_diff >> pt_value) {
        ptDiffValues.push_back(pt_diff);
        ptValues.push_back(pt_value);
    }
    inFile.close();

    // 1 次元ヒストグラム (pt_diff の分布) の作成と描画
    int nBins1D = 256;
    TH1D* h1 = new TH1D("h1", "pt_diff Distribution", nBins1D, -1, 1);
    for (size_t i = 0; i < ptDiffValues.size(); ++i) {
        h1->Fill(ptDiffValues[i]);
    }

    TCanvas* c1 = new TCanvas("c1", "Cut Distribution", 800, 600);
    h1->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_1D.pdf").c_str());

    int nBinsX = 256;
    int nBinsY = 256;
    double xMin = -1.0, xMax = 1.0;
    double yMin = 0.0, yMax = 100000.0;

    TH2D* h2 = new TH2D("h2", "", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
    for (size_t i = 0; i < ptDiffValues.size(); ++i) {
        h2->Fill(ptDiffValues[i], ptValues[i]);
    }

    TCanvas* c2 = new TCanvas("c2", "2D Cut Distribution", 800, 600);
    h2->GetXaxis()->SetTitle("pt_diff");
    h2->GetYaxis()->SetTitle("pt");
    h2->Draw("colz");
    c2->Print((string(basePath) + "/output/" + string(dir) + "/cut_2D.pdf").c_str());

    // メモリ解放
    delete c1;
    delete h1;
    delete c2;
    delete h2;
}
