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

tuple<double, double, double, int> pv_reco(TTree *tree, int bin_num, double pt_threshold) {
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
    int successful_entries = 0;

    // マッチング用のビン幅を設定
    const double phi_bin_width = 0.02; // phiのビン幅
    const double eta_bin_width = 0.02; // etaのビン幅

    double total_purity = 0.0;
    double total_efficiency = 0.0;

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        // truthトラックをphi-eta空間で索引化
        map<pair<int, int>, vector<size_t>> truth_map;
        for (size_t j = 0; j < truth_pt->size(); ++j) {
            // chargeが+-1でない場合は除く
            if (truth_charge->at(j) != 1 && truth_charge->at(j) != -1) {
                continue;
            }
            int phi_bin = static_cast<int>((truth_phi->at(j) + M_PI) / phi_bin_width);
            int eta_bin = static_cast<int>((truth_eta->at(j) + 5.0) / eta_bin_width); // etaの範囲を-5から5と仮定
            truth_map[make_pair(phi_bin, eta_bin)].push_back(j);
        }

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

            // マッチング
            int phi_bin = static_cast<int>((id_trk_phi->at(i) + M_PI) / phi_bin_width);
            int eta_bin = static_cast<int>((id_trk_eta->at(i) + 5.0) / eta_bin_width);

            bool matched = false;
            for (int dphi = -1; dphi <= 1; ++dphi) {
                for (int deta = -1; deta <= 1; ++deta) {
                    int neighbor_phi_bin = phi_bin + dphi;
                    int neighbor_eta_bin = eta_bin + deta;
                    auto key = make_pair(neighbor_phi_bin, neighbor_eta_bin);
                    if (truth_map.find(key) != truth_map.end()) {
                        for (size_t idx : truth_map[key]) {
                            if (abs(id_trk_phi->at(i) - truth_phi->at(idx)) < 0.0025 &&
                                abs(id_trk_eta->at(i) - truth_eta->at(idx)) < 0.0025 &&
                                (pt_threshold == 0 || abs(1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(idx)) / (1.0 / truth_pt->at(idx)) < pt_threshold)) {
                                num_pv_tracks++;
                                if (bin_low_edge < id_trk_z0->at(i) && id_trk_z0->at(i) < bin_up_edge) {
                                    num_pv_tracks_within_bin++;
                                }
                                matched = true;
                                break;
                            }
                        }
                    }
                    if (matched) break;
                }
                if (matched) break;
            }
        }

        if (num_pv_tracks_within_bin != 0) {
            successful_entries++;
        }

        total_purity += static_cast<double>(num_pv_tracks_within_bin) / num_tracks_within_bin_width;
        total_efficiency += static_cast<double>(num_pv_tracks_within_bin) / num_pv_tracks;
        num_pv_tracks = 0;
        num_pv_tracks_within_bin = 0;
        num_tracks_within_bin_width = 0;
    }

    double purity = total_purity / successful_entries;
    double efficiency = total_efficiency / successful_entries;
    int failed_entries = entries - successful_entries;

    cout << "efficiency: " << efficiency << ", purity: " << purity << ", total_efficiency = " << total_efficiency << ", failed_entries: " << failed_entries << ", bin_width = " << bin_width << endl;

    return make_tuple(purity, efficiency, bin_width, failed_entries);
}

void ttbar_mc200() {
    string fullPath = string(basePath) + "/" + string(mcTtbarFilePath);
    vector<double> primary_vertexies;
    double purity;
    double efficiency;
    double bin_width;
    int failed_entries;

    string outputDir = string(basePath) + "/" + string(dataDir) + "/";

    // 条件なし、0.2、0.1、0.05 のしきい値ごとにファイルを作成
    vector<double> pt_thresholds = {0, 0.2, 0.1, 0.05};
    for (double pt_threshold : pt_thresholds) {
        string fileName = outputDir + "ttbar_mc200_" + to_string(pt_threshold) + ".txt";
        ofstream outFile_ttbar(fileName);

        int max_bin_num = 65536;
        int min_bin_num = 1;

        TFile *file = new TFile(fullPath.c_str());
        TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));

        for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
            tie(purity, efficiency, bin_width, failed_entries) = pv_reco(tree, bin_num, pt_threshold);
            outFile_ttbar << efficiency << " "  << purity << " " << bin_width << " " << failed_entries << endl;
        }
        outFile_ttbar.close();
    }
}
