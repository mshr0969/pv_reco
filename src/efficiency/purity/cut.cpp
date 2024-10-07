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

// ptの差、ptの配列と、カットから外れたトラックの値(z0, d0, pt)を返す
tuple<vector<pair<double, double>>, vector<tuple<double, double, double>>> pv_reco(TTree *tree) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<float> *id_trk_d0 = nullptr;
    vector<double> *truth_pt = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_charge = nullptr;

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_phi", &id_trk_phi);
    tree->SetBranchAddress("id_trk_eta", &id_trk_eta);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("id_trk_d0", &id_trk_d0);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);
    tree->SetBranchAddress("truth_eta", &truth_eta);

    int entries = tree->GetEntries();

    vector<pair<double, double>> pt_diff;
    vector<tuple<double, double, double>> failed_track;

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);
        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            bool match_found = false;
            for (size_t j = 0; j < truth_pt->size(); ++j) {
                if (abs(id_trk_phi->at(i) - truth_phi->at(j)) < 0.01 &&
                    abs(id_trk_eta->at(i) - truth_eta->at(j)) < 0.01 &&
                    abs(1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(j)) / (1.0 / truth_pt->at(j)) < 0.2){

                    double pt_diff_value = (1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(j)) / (1.0 / truth_pt->at(j));
                    pt_diff.emplace_back(pt_diff_value, id_trk_pt->at(i));
                    match_found = true;
                    break;
                }
            }
            if (!match_found) {
                failed_track.emplace_back(id_trk_z0->at(i), id_trk_d0->at(i), id_trk_pt->at(i));
            }
        }
    }

    return make_tuple(pt_diff, failed_track);
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
    ofstream failedFile(outputDir + "cut_failed.txt");

    vector<pair<double, double>> pt_diff_values;
    vector<tuple<double, double, double>> failed_tracks;
    tie(pt_diff_values, failed_tracks) = pv_reco(tree);

    // カットを満たしたデータを保存
    for (const auto &pt_value : pt_diff_values) {
        outFile << pt_value.first << " " << pt_value.second << endl;
    }

    // カットを満たさなかったデータを保存
    for (const auto &track : failed_tracks) {
        failedFile << get<0>(track) << " " << get<1>(track) << " " << get<2>(track) << endl;
    }

    outFile.close();
    failedFile.close();

}
