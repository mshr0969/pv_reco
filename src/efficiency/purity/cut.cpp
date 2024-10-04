#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TLatex.h>
#include <map>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* mcTtbarFilePath = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data";

// ptの差、ptの配列を返す
vector<pair<double, double>> pv_reco(TTree *tree) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<double> *truth_pt = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_charge = nullptr;

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_phi", &id_trk_phi);
    tree->SetBranchAddress("id_trk_eta", &id_trk_eta);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);
    tree->SetBranchAddress("truth_eta", &truth_eta);

    int entries = tree->GetEntries();

    vector<pair<double, double>> pt_diff;

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);
        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            for (size_t j = 0; j < truth_pt->size(); ++j) {
                if (abs(id_trk_phi->at(i) - truth_phi->at(j)) < 0.01 &&
                    abs(id_trk_eta->at(i) - truth_eta->at(j)) < 0.01 ){

                    double pt_diff_value = (1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(j)) / (1.0 / truth_pt->at(j));
                    pt_diff.emplace_back(pt_diff_value, id_trk_pt->at(i));
                }
            }
        }
    }

    return pt_diff;
}

void cut() {
    string fullPath = string(basePath) + "/" + string(mcTtbarFilePath);
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));

    double pt_diff;
    double pt_value;

    // 保存
    string outputDir = string(basePath) + "/" + string(dataDir) + "/";
    ofstream outFile(outputDir + "cut.txt");

    vector<pair<double, double>> pt_diff_values = pv_reco(tree);
    for (size_t i = 0; i < pt_diff_values.size(); ++i) {
        outFile << pt_diff_values[i].first << " " << pt_diff_values[i].second << endl;
    }

    outFile.close();

}
