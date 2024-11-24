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
    vector<int> *vxp_type = nullptr;
    vector<float> *true_vxp_z = nullptr;

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("true_vxp_z", &true_vxp_z);

    int entries =  tree->GetEntries();
    cout << "Total number of events: " << entries << endl;

    tree->GetEntry(119);
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

    // 該当ビンに入っているトラックを収集し、pTが高い順にソート
    vector<pair<double, double>> tracksInMaxBin; // {pT, z0}
    for (size_t i = 0; i < id_trk_pt->size(); i++) {
        if (id_trk_z0->at(i) >= binLowEdge && id_trk_z0->at(i) < binUpEdge) {
            tracksInMaxBin.emplace_back(id_trk_pt->at(i), id_trk_z0->at(i));
        }
    }
    sort(tracksInMaxBin.begin(), tracksInMaxBin.end(), greater<>()); // pTで降順ソート

    // 上位10個を出力
    cout << "Top 10 tracks in max bin:" << endl;
    for (size_t i = 0; i < min(size_t(10), tracksInMaxBin.size()); i++) {
        cout << "pT: " << tracksInMaxBin[i].first << ", z0: " << tracksInMaxBin[i].second << endl;
    }
}
