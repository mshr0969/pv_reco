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

void event_display() {
    gStyle->SetOptStat(0);
    string fullPath = string(basePath) + "/" + string(mcRootFilePath);
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));

    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<float> *id_trk_d0 = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *true_vxp_z = nullptr;
    vector<float> *truth_pt = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_charge = nullptr;

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("id_trk_d0", &id_trk_d0);
    tree->SetBranchAddress("id_trk_eta", &id_trk_eta);
    tree->SetBranchAddress("id_trk_phi", &id_trk_phi);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("true_vxp_z", &true_vxp_z);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_eta", &truth_eta);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);

    int entries =  tree->GetEntries();
    cout << "Total number of events: " << entries << endl;

    tree->GetEntry(1506);
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

    // ヒストグラムで最も高いビン幅を取得
    int maxBin = h1->GetMaximumBin();
    double binLowEdge = h1->GetBinLowEdge(maxBin);
    double binUpEdge = binLowEdge + h1->GetBinWidth(maxBin);

    cout << "Max bin range: [" << binLowEdge << ", " << binUpEdge << "]" << endl;


    vector<tuple<double, double, double, double, double>> tracksInMaxBin; // {pT, z0, d0, eta, phi}
    for (size_t i = 0; i < id_trk_pt->size(); i++) {
        if (abs(id_trk_eta->at(i)) > 3) {
            continue;
        }
        if (id_trk_z0->at(i) >= binLowEdge && id_trk_z0->at(i) < binUpEdge) {
            tracksInMaxBin.emplace_back(id_trk_pt->at(i), id_trk_z0->at(i), id_trk_d0->at(i), id_trk_eta->at(i), id_trk_phi->at(i));
        }
    }
    sort(tracksInMaxBin.begin(), tracksInMaxBin.end(), [](const auto &a, const auto &b) {
        return get<0>(a) > get<0>(b); // Sort by pT descending
    });

    cout << "Top 10 tracks in max bin:" << endl;
    for (size_t i = 0; i < min(size_t(10), tracksInMaxBin.size()); i++) {
        cout << "pT: " << get<0>(tracksInMaxBin[i])
            << ", z0: " << get<1>(tracksInMaxBin[i])
            << ", d0: " << get<2>(tracksInMaxBin[i])
            << ", eta: " << get<3>(tracksInMaxBin[i])
            << ", phi: " << get<4>(tracksInMaxBin[i]) << endl;
    }

    // chargeが+-1で、truth_ptの上位10個を取得 {pT, eta, phi}
    vector<tuple<double, double, double>> topTruthTracks; // {pT, eta, phi}
    for (size_t i = 0; i < truth_pt->size(); i++) {
        if (truth_charge->at(i) == 1 || truth_charge->at(i) == -1) {
            topTruthTracks.emplace_back(truth_pt->at(i), truth_eta->at(i), truth_phi->at(i));
        }
    }

    // pTで降順ソート
    sort(topTruthTracks.begin(), topTruthTracks.end(), [](const auto &a, const auto &b) {
        return get<0>(a) > get<0>(b);
    });

    // 上位10個を出力
    cout << "Top 10 truth tracks with max pT:" << endl;
    for (size_t i = 0; i < min(size_t(10), topTruthTracks.size()); i++) {
        cout << "pT: " << get<0>(topTruthTracks[i])
            << ", eta: " << get<1>(topTruthTracks[i])
            << ", phi: " << get<2>(topTruthTracks[i]) << endl;
    }
}
