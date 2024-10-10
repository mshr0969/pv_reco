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
const char* dataDir = "src/efficiency/data/purity";

tuple<vector<double>, vector<double>> pv_reco(TTree *tree, int bin_num, double pt_threshold) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<float> *id_trk_theta = nullptr;
    vector<int> *vxp_nTracks = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    vector<double> *truth_pt = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_charge = nullptr;
    vector<double> *truth_rapidity = nullptr;

    double bin_width;

    int num_pv_tracks = 0; // マッチングが取れたトラックの数(MCのPVから派生したトラックの数)
    int num_pv_tracks_within_bin = 0; // 再構成で求めたPVの範囲内にある、かつマッチングが取れたトラックの数
    int num_tracks_within_bin_width = 0; // 再構成で求めたPVの範囲内にあるトラックの数

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("id_trk_phi", &id_trk_phi);
    tree->SetBranchAddress("id_trk_eta", &id_trk_eta);
    tree->SetBranchAddress("id_trk_theta", &id_trk_theta);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);
    tree->SetBranchAddress("vxp_nTracks", &vxp_nTracks);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);
    tree->SetBranchAddress("truth_eta", &truth_eta);
    tree->SetBranchAddress("truth_rapidity", &truth_rapidity);

    int entries = tree->GetEntries();

    // マッチング用のビン幅を設定
    const double phi_bin_width = 0.02; // phiのビン幅
    const double eta_bin_width = 0.02; // etaのビン幅

    vector<double> purities;
    vector<double> efficiencies;

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        TH1D *tempHist = new TH1D("tempHist", "Temporary Histogram", bin_num, -300, 300);
        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i));
        }

        int maxBin = tempHist->GetMaximumBin();
        double bin_low_edge = tempHist->GetXaxis()->GetBinLowEdge(maxBin);
        double bin_up_edge = tempHist->GetXaxis()->GetBinUpEdge(maxBin);
        bin_width = bin_up_edge - bin_low_edge;

        delete tempHist;

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            if (bin_low_edge < id_trk_z0->at(i) && id_trk_z0->at(i) < bin_up_edge) {
                num_tracks_within_bin_width++;
            }

            bool matched = false;
            for (size_t j = 0; j < truth_pt->size(); ++j) {
                if (abs(id_trk_phi->at(i) - truth_phi->at(j)) < 0.0025 &&
                    abs(id_trk_eta->at(i) - truth_eta->at(j)) < 0.0025 &&
                    (pt_threshold == 0 || abs(1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(j)) / (1.0 / truth_pt->at(j)) < pt_threshold)) {
                    num_pv_tracks++;
                    if (bin_low_edge < id_trk_z0->at(i) && id_trk_z0->at(i) < bin_up_edge) {
                        num_pv_tracks_within_bin++;
                    }
                    matched = true;
                    break;
                }
                if (matched) {
                    break;
                }
            }
        }

        double purity =  static_cast<double>(num_pv_tracks_within_bin) / num_tracks_within_bin_width;
        double efficiency = static_cast<double>(num_pv_tracks_within_bin) / num_pv_tracks;
        cout << "entry: " << entry << ", efficiency: " << efficiency << ", num_pv_tracks_within_bin: " << num_pv_tracks_within_bin << ", num_tracks_within_bin_width: " << num_tracks_within_bin_width << ", num_pv_tracks: " << num_pv_tracks << ", low edge: " << bin_low_edge << ", up edge: " << bin_up_edge << endl;
        purities.push_back(purity);
        efficiencies.push_back(efficiency);
        num_pv_tracks = 0;
        num_pv_tracks_within_bin = 0;
        num_tracks_within_bin_width = 0;
    }

    return make_tuple(purities, efficiencies);
}

void plotPurityVsEfficiency(const vector<double>& purities, const vector<double>& efficiencies) {
    int n = purities.size();
    cout << "n: " << n << endl;

    TCanvas *c1 = new TCanvas("c1", "Purity vs Efficiency", 800, 600);

    TGraph *graph = new TGraph(n);
    for (int i = 0; i < n; ++i) {
        graph->SetPoint(i, efficiencies[i], purities[i]);
    }
    graph->GetXaxis()->SetLimits(0, 1);
    graph->GetYaxis()->SetRangeUser(0, 1);
    graph->SetTitle("");
    graph->GetXaxis()->SetTitle("Efficiency");
    graph->GetYaxis()->SetTitle("Purity");
    graph->SetMarkerStyle(21);
    graph->SetMarkerColor(kBlue);
    graph->Draw("AP");

    c1->SaveAs("output/efficiency/purity/ttbar200_entry.pdf");
}



void ttbar_mc200_entry() {
    string fullPath = string(basePath) + "/" + string(mcTtbarFilePath);
    double pt_threshold = 0.2;

    int bin_num = 512;

    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));

    vector<double> purities;
    vector<double> efficiencies;
    tie(purities, efficiencies) = pv_reco(tree, bin_num, pt_threshold);

    plotPurityVsEfficiency(purities, efficiencies);
}
