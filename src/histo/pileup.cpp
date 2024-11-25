#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TLatex.h>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* mcRootFilePath = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* dir = "histo";

void pileup() {
    gStyle->SetOptStat(0);
    string fullPath = string(basePath) + "/" + string(mcRootFilePath);
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));

    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *true_vxp_z = nullptr;

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("true_vxp_z", &true_vxp_z);

    int entries =  tree->GetEntries();
    cout << "Total number of events: " << entries << endl;

    tree->GetEntry(4182);
    string title =  "";
    TH1D *h1 = new TH1D("h1", title.c_str(), 256, -200, 200);
    h1->SetFillColor(kBlue - 10);

    std::map<int, double> binValues;

    for (size_t i = 0; i < id_trk_pt->size(); i++) {
        int bin = h1->FindBin(id_trk_z0->at(i));
        binValues[bin] += id_trk_pt->at(i);
    }

    for (const auto& val : binValues) {
        h1->SetBinContent(val.first, val.second);
    }


    TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
    h1->SetXTitle("z_{0} [mm]");
    h1->Draw();

    TLegend *legend = new TLegend(0.72, 0.72, 0.89, 0.89);
    legend->AddEntry(h1, "Sum of  track p_{T}", "f");

    gPad->Update();

    Double_t ymax = gPad->GetUymax();

    TH1F *dummy_pv = new TH1F("dummy_pv", "", 256, -120, 120);
    TH1F *dummy_pu = new TH1F("dummy_pu", "", 256, -120, 120);
    dummy_pv->SetLineColor(kRed);
    dummy_pu->SetLineColor(3);
    dummy_pv->SetLineWidth(2);
    dummy_pu->SetLineStyle(2);

    // true_vxp_zの、初めから100個のうち、最も多いものをprimary vertexとする
    double primary_vertex;
    vector<double> truth_z;
    for (size_t i = 0; i < 100; ++i) {
        truth_z.push_back(true_vxp_z->at(i));
    }
    primary_vertex = *max_element(truth_z.begin(), truth_z.end());
    TLine *line_pv = new TLine(primary_vertex, 0, primary_vertex, ymax);
    line_pv->SetLineColor(kRed);
    line_pv->SetLineWidth(2);
    line_pv->Draw();

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.13, 0.8, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.22, 0.8, "Simulation Work In Progress");
    latex.DrawLatex(0.13, 0.75, "#sqrt{s} = 14 TeV");
    legend->AddEntry(dummy_pv, "True PV", "l");
    legend->SetBorderSize(0);
    legend->Draw();

    string outputPath = string(basePath) + "/output/" + string(dir) + "/pileup.pdf";
    c1->Print(outputPath.c_str());
}
