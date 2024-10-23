#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TLatex.h>
#include <TLegend.h>
#include <map>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data/purity";


void purity_efficiency_plot() {
    string txtDir = string(basePath)+ "/" + string(dataDir) + "/";
    TGraph *g1 = new TGraph((txtDir + "ttbar_mc200.txt").c_str());
    g1->SetTitle("");
    g1->SetMarkerColor(kBlue);
    g1->SetLineColor(kBlue);
    g1->SetMarkerStyle(21);
    g1->SetLineWidth(4);
    TGraph *g2 = new TGraph((txtDir + "ttbar_mc200_pt2.txt").c_str());
    g2->SetMarkerColor(kBlue);
    g2->SetLineColor(kBlue);
    g2->SetMarkerStyle(21);
    g2->SetLineWidth(4);
    g2->SetLineStyle(2);

    TGraph *g3 = new TGraph((txtDir + "zmumu_mc200.txt").c_str());
    g3->SetMarkerColor(kOrange);
    g3->SetLineColor(kOrange);
    g3->SetMarkerStyle(21);
    g3->SetLineWidth(4);
    TGraph *g4 = new TGraph((txtDir + "zmumu_mc200_pt2.txt").c_str());
    g4->SetMarkerColor(kOrange);
    g4->SetLineColor(kOrange);
    g4->SetMarkerStyle(21);
    g4->SetLineWidth(4);
    g4->SetLineStyle(2);

    TGraph *g5 = new TGraph((txtDir + "zmumu_mc140.txt").c_str());
    g5->SetMarkerColor(kGreen+2);
    g5->SetLineColor(kGreen+2);
    g5->SetMarkerStyle(21);
    g5->SetLineWidth(4);
    TGraph *g6 = new TGraph((txtDir + "zmumu_mc140_pt2.txt").c_str());
    g6->SetMarkerColor(kGreen+2);
    g6->SetLineColor(kGreen+2);
    g6->SetMarkerStyle(21);
    g6->SetLineWidth(4);
    g6->SetLineStyle(2);

    TGraph *g7 = new TGraph((txtDir + "zmumu_mc0.txt").c_str());
    g7->SetMarkerColor(kRed);
    g7->SetLineColor(kRed);
    g7->SetMarkerStyle(21);
    g7->SetLineWidth(4);
    TGraph *g8 = new TGraph((txtDir + "zmumu_mc0_pt2.txt").c_str());
    g8->SetMarkerColor(kRed);
    g8->SetLineColor(kRed);
    g8->SetMarkerStyle(21);
    g8->SetLineWidth(4);
    g8->SetLineStyle(2);

    TCanvas *c1 = new TCanvas("c1", "", 850, 600);
    TH1F *frame = c1->DrawFrame(0.0, 0.0, 1, 1); // x: 0～1, y: 0～1 の範囲を設定
    frame->SetXTitle("efficiency");
    frame->SetYTitle("purity");
    frame->GetXaxis()->SetTitleSize(0.04);
    frame->GetYaxis()->SetTitleSize(0.04);
    frame->GetXaxis()->SetLabelSize(0.04);
    frame->GetYaxis()->SetLabelSize(0.04);

    g1->Draw("LP same");
    g2->Draw("LP same");
    g3->Draw("LP same");
    g4->Draw("LP same");
    g5->Draw("LP same");
    g6->Draw("LP same");
    g7->Draw("LP same");
    g8->Draw("LP same");


    TLegend *legend = new TLegend(0.1, 0.15, 0.34, 0.35);
    legend->AddEntry(g1, "t#bar{t} (PU200) #Sigma p_{T}", "l");
    legend->AddEntry(g2, "t#bar{t} (PU200), #Sigma p_{T}^{2}", "l");
    legend->AddEntry(g3, "Zmumu (PU200) #Sigma p_{T}", "l");
    legend->AddEntry(g4, "Zmumu (PU200), #Sigma p_{T}^{2}", "l");
    legend->SetBorderSize(0);

    TLegend *legend2 = new TLegend(0.35, 0.15, 0.59, 0.35);
    legend2->AddEntry(g5, "Zmumu (PU140) #Sigma p_{T}", "l");
    legend2->AddEntry(g6, "Zmumu (PU140), #Sigma p_{T}^{2}", "l");
    legend2->AddEntry(g7, "Zmumu (PU0) #Sigma p_{T}", "l");
    legend2->AddEntry(g8, "Zmumu (PU0), #Sigma p_{T}^{2}", "l");
    legend2->SetBorderSize(0);

    legend->Draw();
    legend2->Draw();


    // ATLASラベルを追加
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.2, 0.5, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.32, 0.5, "Simulation Work in Progress");
    latex.DrawLatex(0.2, 0.45, "#sqrt{s} = 14 TeV");

    c1->Print((string(basePath) + "/output/" + string(dir) + "/purity_efficiency.pdf").c_str());
}
