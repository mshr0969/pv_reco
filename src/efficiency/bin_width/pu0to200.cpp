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
const char* rootFilePath = getenv("REAL_DATA_DIR");
const char* dir = "efficiency/bin_width";
const char* dataDir = "src/efficiency/data";

void pu0to200() {
    string txtDir = string(basePath)+ "/" + string(dataDir) + "/";
    TGraph *g1 = new TGraph((txtDir + "ttbar_efficiency.txt").c_str());
    g1->SetTitle("");
    g1->SetMarkerColor(kBlue);
    g1->SetLineColor(kBlue);
    g1->SetLineWidth(4);

    TGraph *g2 = new TGraph((txtDir + "zmumu_200_efficiency.txt").c_str());
    g2->SetMarkerColor(kOrange);
    g2->SetLineColor(kOrange);
    g2->SetLineWidth(4);

    TGraph *g3 = new TGraph((txtDir + "zmumu_140_efficiency.txt").c_str());
    g3->SetMarkerColor(kGreen+2);
    g3->SetLineColor(kGreen+2);
    g3->SetLineWidth(4);

    TGraph *g4 = new TGraph((txtDir + "zmumu_0_efficiency.txt").c_str());
    g4->SetMarkerColor(kRed);
    g4->SetLineColor(kRed);
    g4->SetLineWidth(4);

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
    g3->Draw("LP same");
    g4->Draw("LP same");

    TLegend *legend = new TLegend(0.6, 0.15, 0.8, 0.45);
    legend->AddEntry(g1, "t#bar{t} (PU200)", "l");
    legend->AddEntry(g2, "Zmumu (PU200)", "l");
    legend->AddEntry(g3, "Zmumu (PU140)", "l");
    legend->AddEntry(g4, "Zmumu (PU0)", "l");
    legend->SetBorderSize(0);

    legend->Draw();

    // ATLASラベルを追加
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.5, 0.7, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.62, 0.7, "Simulation Internal");
    latex.DrawLatex(0.5, 0.65, "#sqrt{s} = 14 TeV");

    c1->Print((string(basePath) + "/output/" + string(dir) + "/pu0to200.pdf").c_str());
}
