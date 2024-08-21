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
const char* mcRootFilePath = getenv("MC_DATA_DIR");
const char* dir = "efficiency/bin_width";
const char* dataDir = "src/efficiency/data";

void run3() {
    string txtDir = string(basePath) + "/" + string(dataDir) + "/";
    TGraph *g1 = new TGraph((txtDir + "run3_ttbar_ftf.txt").c_str());
    g1->SetTitle("");
    g1->SetMarkerColor(kBlue);
    g1->SetLineColor(kBlue);
    g1->SetLineWidth(4);

    TGraph *g2 = new TGraph((txtDir + "run3_ttbar_offline.txt").c_str());
    g2->SetMarkerColor(kOrange);
    g2->SetLineColor(kOrange);
    g2->SetLineWidth(4);

    TCanvas *c1 = new TCanvas("c1", "", 800, 600);
    g1->GetXaxis()->SetTitle("bin width [mm]");
    g1->GetYaxis()->SetTitle("efficiency");
    g1->GetXaxis()->SetTitleSize(0.04);
    g1->GetYaxis()->SetTitleSize(0.04);
    g1->GetXaxis()->SetLabelSize(0.04);
    g1->GetYaxis()->SetLabelSize(0.04);
    g1->GetYaxis()->SetRangeUser(0, 1);
    g1->Draw("ALP");
    g2->Draw("LP same");

    TLegend *legend = new TLegend(0.6, 0.4, 0.8, 0.55);
    legend->AddEntry(g1, "t#bar{t} (FTF)", "l");
    legend->AddEntry(g2, "t#bar{t} (Offline)", "l");
    legend->SetBorderSize(0);
    legend->Draw();

    // ATLASラベルを追加
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.5, 0.25, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.62, 0.25, "Simulation Internal");
    latex.DrawLatex(0.5, 0.2, "#sqrt{s} = 13.6 TeV");

    c1->Print((string(basePath) + "/output/" + string(dir) + "/run3.pdf").c_str());
}
