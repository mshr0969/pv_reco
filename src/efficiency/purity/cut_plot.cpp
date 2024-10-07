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

void cut_plot_pt() {
    string txtDir = string(basePath) + "/" + string(dataDir) + "/";
    string filePath = txtDir + "cut.txt";
    string failedFilePath = txtDir + "cut_failed.txt";

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

    ifstream failedFile(failedFilePath.c_str());
    if (!failedFile) {
        cerr << "Error opening file: " << failedFilePath << endl;
        return;
    }

    vector<double> failed_z0;  // z0 の値を格納するベクトル
    vector<double> failed_d0;  // d0 の値を格納するベクトル
    vector<double> failed_pt;  // トラックの pt の値を格納するベクトル
    double z0, d0, pt;

        // ファイルから値を3列読み込み、対応するベクトルに格納
    while (failedFile >> z0 >> d0 >> pt) {
        failed_z0.push_back(z0);
        failed_d0.push_back(d0);
        failed_pt.push_back(pt);
    }
    failedFile.close();

    // 1 次元ヒストグラム (pt_diff の分布) の作成と描画
    int nBins1D = 256;
    TH1D* h1 = new TH1D("h1", "", nBins1D, -1, 1);
    for (size_t i = 0; i < ptDiffValues.size(); ++i) {
        h1->Fill(ptDiffValues[i]);
    }

    TCanvas* c1 = new TCanvas("c1", "Cut Distribution", 860, 600);
    h1->GetXaxis()->SetTitle("pT diff");
    h1->GetYaxis()->SetTitle("Entries");
    h1->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_1D.pdf[").c_str());
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_1D.pdf").c_str());

    TH1D *h2_z0 = new TH1D("z0", "", nBins1D, -300, 300);
    for (size_t i = 0; i < failed_z0.size(); ++i) {
        h2_z0->Fill(failed_z0[i]);
    }
    h2_z0->GetXaxis()->SetTitle("offline track z0 [mm]");
    h2_z0->GetYaxis()->SetTitle("Entries");
    h2_z0->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_1D.pdf").c_str());

    TH1D *h2_d0 = new TH1D("d0", "", nBins1D, -5, 5);
    for (size_t i = 0; i < failed_d0.size(); ++i) {
        h2_d0->Fill(failed_d0[i]);
    }
    h2_d0->GetXaxis()->SetTitle("offline track d0 [mm]");
    h2_d0->GetYaxis()->SetTitle("Entries");
    h2_d0->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_1D.pdf").c_str());

    TH1D *h2_pt = new TH1D("pT", "", nBins1D, 0, 120000);
    for (size_t i = 0; i < failed_pt.size(); ++i) {
        h2_pt->Fill(failed_pt[i]);
    }
    h2_pt->GetXaxis()->SetTitle("offline track pT [MeV]");
    h2_pt->GetYaxis()->SetTitle("Entries");
    h2_pt->Draw();
    c1->SetLogy();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_1D.pdf").c_str());
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_1D.pdf]").c_str());

    gStyle->SetOptStat(0);
    int nBinsX = 256;
    int nBinsY = 256;
    double yMin = -1.0, yMax = 1.0;
    double xMin = 0.0, xMax = 120000.0;

    TH2D* h2 = new TH2D("h2", "", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
    for (size_t i = 0; i < ptDiffValues.size(); ++i) {
        h2->Fill(ptValues[i], ptDiffValues[i]);
    }

    TCanvas* c2 = new TCanvas("c2", "2D Cut Distribution", 800, 600);
    h2->GetYaxis()->SetTitle("pT diff");
    h2->GetXaxis()->SetTitle("offline track pT [MeV]");
    h2->Draw("colz");
    c2->Print((string(basePath) + "/output/" + string(dir) + "/cut_2D.pdf").c_str());

    // メモリ解放
    delete c1;
    delete h1;
    delete c2;
    delete h2;
}

void cut_plot_phi() {
    string txtDir = string(basePath) + "/" + string(dataDir) + "/";
    string filePath = txtDir + "cut_phi.txt";
    string failedFilePath = txtDir + "cut_failed_phi.txt";

    // ファイルの読み込み
    ifstream inFile(filePath.c_str());
    if (!inFile) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }

    vector<double> phiDiffValues;
    vector<double> phiValues;
    double phi_diff, phi_value;

    // ファイルから値を2列読み込み、対応するベクトルに格納
    while (inFile >> phi_diff >> phi_value) {
        phiDiffValues.push_back(phi_diff);
        phiValues.push_back(phi_value);
    }
    inFile.close();

    ifstream failedFile(failedFilePath.c_str());
    if (!failedFile) {
        cerr << "Error opening file: " << failedFilePath << endl;
        return;
    }

    vector<double> failed_z0;  // z0 の値を格納するベクトル
    vector<double> failed_d0;  // d0 の値を格納するベクトル
    vector<double> failed_pt;  // トラックの pt の値を格納するベクトル
    double z0, d0, pt;

        // ファイルから値を3列読み込み、対応するベクトルに格納
    while (failedFile >> z0 >> d0 >> pt) {
        failed_z0.push_back(z0);
        failed_d0.push_back(d0);
        failed_pt.push_back(pt);
    }
    failedFile.close();

    // 1 次元ヒストグラム (phi diff の分布) の作成と描画
    int nBins1D = 256;
    TH1D* h1 = new TH1D("h1", "", nBins1D, -0.02, 0.02);
    for (size_t i = 0; i < phiDiffValues.size(); ++i) {
        h1->Fill(phiDiffValues[i]);
    }

    TCanvas* c1 = new TCanvas("c1", "Cut Distribution", 860, 600);
    h1->GetXaxis()->SetTitle("phi diff");
    h1->GetYaxis()->SetTitle("Entries");
    h1->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf[").c_str());
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());

    TH1D *h2_z0 = new TH1D("z0", "", nBins1D, -300, 300);
    for (size_t i = 0; i < failed_z0.size(); ++i) {
        h2_z0->Fill(failed_z0[i]);
    }
    h2_z0->GetXaxis()->SetTitle("offline track z0 [mm]");
    h2_z0->GetYaxis()->SetTitle("Entries");
    h2_z0->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());

    TH1D *h2_d0 = new TH1D("d0", "", nBins1D, -5, 5);
    for (size_t i = 0; i < failed_d0.size(); ++i) {
        h2_d0->Fill(failed_d0[i]);
    }
    h2_d0->GetXaxis()->SetTitle("offline track d0 [mm]");
    h2_d0->GetYaxis()->SetTitle("Entries");
    h2_d0->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());

    TH1D *h2_pt = new TH1D("pT", "", nBins1D, 0, 120000);
    for (size_t i = 0; i < failed_pt.size(); ++i) {
        h2_pt->Fill(failed_pt[i]);
    }
    h2_pt->GetXaxis()->SetTitle("offline track pT [MeV]");
    h2_pt->GetYaxis()->SetTitle("Entries");
    h2_pt->Draw();
    c1->SetLogy();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf]").c_str());

    // メモリ解放
    delete c1;
    delete h1;
}

void cut_plot() {
    string txtDir = string(basePath) + "/" + string(dataDir) + "/";
    string filePath = txtDir + "cut_phi.txt";
    string failedFilePath = txtDir + "cut_failed_phi.txt";

    // ファイルの読み込み
    ifstream inFile(filePath.c_str());
    if (!inFile) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }

    vector<double> phiDiffValues;
    vector<double> phiValues;
    double phi_diff, phi_value;

    // ファイルから値を2列読み込み、対応するベクトルに格納
    while (inFile >> phi_diff >> phi_value) {
        phiDiffValues.push_back(phi_diff);
        phiValues.push_back(phi_value);
    }
    inFile.close();

    ifstream failedFile(failedFilePath.c_str());
    if (!failedFile) {
        cerr << "Error opening file: " << failedFilePath << endl;
        return;
    }

    vector<double> failed_z0;  // z0 の値を格納するベクトル
    vector<double> failed_d0;  // d0 の値を格納するベクトル
    vector<double> failed_pt;  // トラックの pt の値を格納するベクトル
    double z0, d0, pt;

        // ファイルから値を3列読み込み、対応するベクトルに格納
    while (failedFile >> z0 >> d0 >> pt) {
        failed_z0.push_back(z0);
        failed_d0.push_back(d0);
        failed_pt.push_back(pt);
    }
    failedFile.close();

    // 1 次元ヒストグラム (phi diff の分布) の作成と描画
    int nBins1D = 256;
    TH1D* h1 = new TH1D("h1", "", nBins1D, -0.02, 0.02);
    for (size_t i = 0; i < phiDiffValues.size(); ++i) {
        h1->Fill(phiDiffValues[i]);
    }

    TCanvas* c1 = new TCanvas("c1", "Cut Distribution", 860, 600);
    h1->GetXaxis()->SetTitle("phi diff");
    h1->GetYaxis()->SetTitle("Entries");
    h1->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf[").c_str());
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());

    TH1D *h2_z0 = new TH1D("z0", "", nBins1D, -300, 300);
    for (size_t i = 0; i < failed_z0.size(); ++i) {
        h2_z0->Fill(failed_z0[i]);
    }
    h2_z0->GetXaxis()->SetTitle("offline track z0 [mm]");
    h2_z0->GetYaxis()->SetTitle("Entries");
    h2_z0->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());

    TH1D *h2_d0 = new TH1D("d0", "", nBins1D, -5, 5);
    for (size_t i = 0; i < failed_d0.size(); ++i) {
        h2_d0->Fill(failed_d0[i]);
    }
    h2_d0->GetXaxis()->SetTitle("offline track d0 [mm]");
    h2_d0->GetYaxis()->SetTitle("Entries");
    h2_d0->Draw();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());

    TH1D *h2_pt = new TH1D("pT", "", nBins1D, 0, 120000);
    for (size_t i = 0; i < failed_pt.size(); ++i) {
        h2_pt->Fill(failed_pt[i]);
    }
    h2_pt->GetXaxis()->SetTitle("offline track pT [MeV]");
    h2_pt->GetYaxis()->SetTitle("Entries");
    h2_pt->Draw();
    c1->SetLogy();
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf").c_str());
    c1->Print((string(basePath) + "/output/" + string(dir) + "/cut_phi.pdf]").c_str());

    // メモリ解放
    delete c1;
    delete h1;
}
