#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <limits>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* rootFilePath = getenv("REAL_DATA_DIR");
const char* mcRootFilePath = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* dir = "histo";

void drawHist_beamspot(TTree *tree, bool is_ftf, string outputPath) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    Float_t beam_pos_z;

    const char* ptBranch = is_ftf ? "ftf_id_trk_pt" : "id_trk_pt";
    const char* z0Branch = is_ftf ? "ftf_id_trk_z0" : "id_trk_z0";

    tree->SetBranchAddress(ptBranch, &id_trk_pt);
    tree->SetBranchAddress(z0Branch ,&id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);
    tree->SetBranchAddress("beamPosZ", &beam_pos_z);

    int entries = tree->GetEntries();

    float min_z0 = numeric_limits<float>::max();
    float max_z0 = numeric_limits<float>::lowest();

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            min_z0 = min(min_z0, id_trk_z0->at(i));
            max_z0 = max(max_z0, id_trk_z0->at(i));
        }
    }

    float range = max_z0 - min_z0;
    float margin = 0.1 * range; // 10%のマージンを追加
    min_z0 -= margin;
    max_z0 += margin;

    const char* h1_title = is_ftf ? "before adjust beamspot (FTF)": "before adjust beamspot";
    const char* h2_title = is_ftf ? "after adjust beamspot (FTF)": "after adjust beamspot";

    TH1D *h1 = new TH1D("h1", h1_title, 128, -2, 9);
    h1->SetFillColor(kBlue);
    TH1D *h2 = new TH1D("h2", h2_title, 128, -5, 5);
    h2->SetFillColor(kBlue);

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        TH1D *tempHist = new TH1D("tempHist", "Temporary Histogram", 256, min_z0, max_z0);

        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i));
        }

        int maxBin = tempHist->GetMaximumBin();
        double max_z0 = tempHist->GetXaxis()->GetBinCenter(maxBin);

        delete tempHist;

        float primary_vertex_z = 0;
        for (size_t i = 0; i < vxp_z->size(); ++i) {
            if (vxp_type->at(i) == 1) {
                primary_vertex_z = vxp_z->at(i);
                break;
            }
        }

        double diff = max_z0 - primary_vertex_z;
        int track_num = id_trk_pt->size();
        if (track_num > 0) {
            h1->Fill(diff);
            h2->Fill(diff + static_cast<double>(beam_pos_z));
        }
    }

    TCanvas *c1 = new TCanvas("c1", "", 1400, 600);
    c1->Divide(2, 1);

    c1->cd(1);
    h1->SetXTitle("difference [mm]");
    h1->SetYTitle("number of events");
    h1->Draw();

    c1->cd(2);
    h2->SetXTitle("difference [mm]");
    h2->SetYTitle("number of events");
    h2->Draw();

    c1->Print(outputPath.c_str());

    delete h1;
    delete h2;
    delete c1;
}

void drawHist(TTree *tree, bool is_ftf, string outputPath) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;

    const char* ptBranch = is_ftf ? "ftf_id_trk_pt" : "id_trk_pt";
    const char* z0Branch = is_ftf ? "ftf_id_trk_z0" : "id_trk_z0";

    tree->SetBranchAddress(ptBranch, &id_trk_pt);
    tree->SetBranchAddress(z0Branch ,&id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);

    int entries = tree->GetEntries();

    float min_z0 = numeric_limits<float>::max();
    float max_z0 = numeric_limits<float>::lowest();

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            min_z0 = min(min_z0, id_trk_z0->at(i));
            max_z0 = max(max_z0, id_trk_z0->at(i));
        }
    }

    float range = max_z0 - min_z0;
    float margin = 0.1 * range; // 10%のマージンを追加
    min_z0 -= margin;
    max_z0 += margin;

    const char* h1_title = is_ftf ? "diff (FTF)": "diff (offline)";

    TH1D *h1 = new TH1D("h1", h1_title, 64, -2, 2);
    h1->SetFillColor(kBlue);
    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        TH1D *tempHist = new TH1D("tempHist", "Temporary Histogram", 256, min_z0, max_z0);

        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i));
        }

        int maxBin = tempHist->GetMaximumBin();
        double max_z0 = tempHist->GetXaxis()->GetBinCenter(maxBin);

        delete tempHist;

        float primary_vertex_z = 0;
        for (size_t i = 0; i < vxp_z->size(); ++i) {
            if (vxp_type->at(i) == 1) {
                primary_vertex_z = vxp_z->at(i);
                break;
            }
        }

        double diff = max_z0 - primary_vertex_z;
        int track_num = id_trk_pt->size();
        if (track_num > 0) {
            h1->Fill(diff);
        }

        if (diff > 2 || diff < -2) {
            cout << "entry: " << entry << ", diff: " << diff << endl;
        }
    }

    TCanvas *c1 = new TCanvas("c1", "", 1400, 600);

    c1->cd(1);
    h1->SetXTitle("difference [mm]");
    h1->SetYTitle("number of events");
    h1->Draw();

    c1->Print(outputPath.c_str());

    delete h1;
    delete c1;
}

void diff(bool is_mc=false) {
    gStyle->SetOptStat(111111);
    if (is_mc) {
        string fullPath = string(basePath) + "/" + string(mcRootFilePath);
        TFile *file = new TFile(fullPath.c_str());
        TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
        drawHist(tree, false, string(basePath) + "/output/" + string(dir) + "/diff_mc.pdf");
        // drawHist(tree, true, string(basePath) + "/output/" + string(dir) + "/diff_mc.pdf)");
    } else {
        string fullPath = string(basePath) + "/" + string(rootFilePath);
        TFile *file = new TFile(fullPath.c_str());
        TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
        drawHist_beamspot(tree, false, string(basePath) + "/output/" + string(dir) + "/diff.pdf(");
        drawHist_beamspot(tree, true, string(basePath) + "/output/" + string(dir) + "/diff.pdf)");
    }
}
