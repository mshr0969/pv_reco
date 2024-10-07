#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TLatex.h>
#include <TLegend.h>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data";

void ttbar_mc200_plot() {
    string txtDir = string(basePath)+ "/" + string(dataDir) + "/";
    // TGraph *g1 = new TGraph((txtDir + "run3_ttbar_purity_ftf.txt").c_str());
    TGraph *g1 = new TGraph((txtDir + "ttbar_mc200.txt").c_str());
    g1->SetTitle("");
    g1->SetMarkerColor(kBlue);
    g1->SetLineColor(kBlue);
    g1->SetLineWidth(4);

    TCanvas *c1 = new TCanvas("c1", "", 800, 600);
    g1->GetXaxis()->SetTitle("efficiency");
    g1->GetYaxis()->SetTitle("purity");
    g1->GetXaxis()->SetTitleSize(0.04);
    g1->GetYaxis()->SetTitleSize(0.04);
    g1->GetXaxis()->SetLabelSize(0.04);
    g1->GetYaxis()->SetLabelSize(0.04);
    g1->Draw("ALP");

    // ATLASラベルを追加
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.2, 0.5, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.32, 0.5, "Simulation Work in Progress");
    latex.DrawLatex(0.2, 0.45, "#sqrt{s} = 14 TeV");

    c1->Print((string(basePath) + "/output/" + string(dir) + "/ttbar200.pdf").c_str());
}
