#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>
#include <TStyle.h>

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
    vector<float> *vxp_z = nullptr;

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);

    int entries =  tree->GetEntries();
    cout << "Total number of events: " << entries << endl;

    tree->GetEntry(10);
    string title =  "z_{0} weighted by p_{T}";
    TH1D *h1 = new TH1D("h1", title.c_str(), 256, -120, 120);
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
    legend->AddEntry(h1, "ID Track", "f");

    gPad->Update();

    Double_t ymax = gPad->GetUymax();

    TH1F *dummy_pv = new TH1F("dummy_pv", "", 256, -120, 120);
    TH1F *dummy_pu = new TH1F("dummy_pu", "", 256, -120, 120);
    dummy_pv->SetLineColor(kRed);
    dummy_pu->SetLineColor(3);
    dummy_pv->SetLineWidth(2);
    dummy_pu->SetLineStyle(2);

    for (int i = 0; i < vxp_z->size(); i++) {
        if (vxp_type->at(i) == 1) {
            double z = vxp_z->at(i);
            TLine *line = new TLine(z, 0, z, ymax);
            line->SetLineColor(kRed);
            line->SetLineWidth(2);
            line->Draw();
        }
    }

    legend->AddEntry(dummy_pv, "Reco PV", "l");
    legend->SetBorderSize(0);
    legend->Draw();

    string outputPath = string(basePath) + "/output/" + string(dir) + "/pileup.pdf";
    c1->Print(outputPath.c_str());
}
