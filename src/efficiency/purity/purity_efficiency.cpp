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
const char* mcRootFilePath_pu200_ttbar = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* mcRootFilePath_pu200_zmumu = getenv("MC_DATA_DIR_PU200_Z_MUMU");
const char* mcRootFilePath_pu140_zmumu = getenv("MC_DATA_DIR_PU140_Z_MUMU");
const char* mcRootFilePath_pu0_zmumu = getenv("MC_DATA_DIR_PU0_Z_MUMU");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data/purity";

tuple<double, double, double> pv_reco(TTree *tree, int bin_num, bool is_pt2) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<int> *vxp_nTracks = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    vector<double> *truth_pt = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_charge = nullptr;
    vector<double> *true_vxp_z = nullptr;

    double bin_width;

    int num_pv_tracks = 0; // マッチングが取れたトラックの数(MCのPVから派生したトラックの数)
    int num_pv_tracks_within_bin = 0; // 再構成で求めたPVの範囲内にある、かつマッチングが取れたトラックの数
    int num_tracks_within_bin_width = 0; // 再構成で求めたPVの範囲内にあるトラックの数

    /*
        purity =  num_pv_tracks_within_bin / num_tracks_within_bin_width
        efficiency = num_pv_tracks_within_bin / num_pv_tracks
    */

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("id_trk_phi", &id_trk_phi);
    tree->SetBranchAddress("id_trk_eta", &id_trk_eta);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);
    tree->SetBranchAddress("vxp_nTracks", &vxp_nTracks);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);
    tree->SetBranchAddress("truth_eta", &truth_eta);
    tree->SetBranchAddress("true_vxp_z", &true_vxp_z);

    int entries = tree->GetEntries();

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

        TH1D *tempHist = new TH1D("tempHist", "Temporary Histogram", bin_num, -200, 200);

        // primary interactionは、最初の100個のうち、最もユニーク数が多いものとする
        double primary_vertex;
        vector<double> truth_z;
        if (true_vxp_z != nullptr) {
            for (size_t i = 0; i < 100 && i < true_vxp_z->size(); ++i) {
                truth_z.push_back(true_vxp_z->at(i));
            }
        } else {
            cout << "true_vxp_z is nullptr" << endl;
        }
        primary_vertex = *max_element(truth_z.begin(), truth_z.end());

        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            if (is_pt2) {
                tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i) * id_trk_pt->at(i));
            } else {
                tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i));
            }
        }

        int maxBin = tempHist->GetMaximumBin();
        double bin_low_edge = tempHist->GetXaxis()->GetBinLowEdge(maxBin);
        double bin_up_edge = tempHist->GetXaxis()->GetBinUpEdge(maxBin);
        bin_width = bin_up_edge - bin_low_edge;

        delete tempHist;

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            // etaが2.5より大きい場合は除く
            if (abs(id_trk_eta->at(i)) > 2.5) {
                continue;
            }
            if (bin_low_edge < id_trk_z0->at(i) && id_trk_z0->at(i) < bin_up_edge) {
                num_tracks_within_bin_width++;
            }
            // マッチング
            int phi_bin = static_cast<int>((id_trk_phi->at(i) + M_PI) / phi_bin_width);
            int eta_bin = static_cast<int>((id_trk_eta->at(i) + 5.0) / eta_bin_width);

            bool matched = false;
            // 周囲のビンも含めて探索
            for (int dphi = -1; dphi <= 1; ++dphi) {
                for (int deta = -1; deta <= 1; ++deta) {
                    int neighbor_phi_bin = phi_bin + dphi;
                    int neighbor_eta_bin = eta_bin + deta;
                    auto key = make_pair(neighbor_phi_bin, neighbor_eta_bin);
                    if (truth_map.find(key) != truth_map.end()) {
                        for (size_t idx : truth_map[key]) {
                            // マッチング条件
                            if (abs(id_trk_phi->at(i) - truth_phi->at(idx)) < 0.0025 &&
                                abs(id_trk_eta->at(i) - truth_eta->at(idx)) < 0.0025 &&
                                abs(1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(idx)) / (1.0 / truth_pt->at(idx)) < 0.2 &&
                                abs(id_trk_z0->at(i) - primary_vertex) < 0.1) {
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

        if (num_tracks_within_bin_width == 0 || num_pv_tracks == 0) {
            num_pv_tracks = 0;
            num_pv_tracks_within_bin = 0;
            num_tracks_within_bin_width = 0;
            continue;
        }
        total_purity += static_cast<double>(num_pv_tracks_within_bin) / num_tracks_within_bin_width;
        total_efficiency += static_cast<double>(num_pv_tracks_within_bin) / num_pv_tracks;
        num_pv_tracks = 0;
        num_pv_tracks_within_bin = 0;
        num_tracks_within_bin_width = 0;
    }

    double purity = total_purity / entries;
    double efficiency = total_efficiency / entries;

    return make_tuple(purity, efficiency, bin_width);
}

void purity_efficiency() {
    vector<double> primary_vertexies;
    double purity;
    double efficiency;
    double bin_width;

    string outputDir = string(basePath) + "/" + string(dataDir) + "/";
    ofstream outFile_ttbar(outputDir + "ttbar_mc200.txt");
    ofstream outFile_ttbar_pt2(outputDir + "ttbar_mc200_pt2.txt");
    ofstream outFile_zmumu(outputDir + "zmumu_mc200.txt");
    ofstream outFile_zmumu_pt2(outputDir + "zmumu_mc200_pt2.txt");
    ofstream outFile_zmumu140(outputDir + "zmumu_mc140.txt");
    ofstream outFile_zmumu140_pt2(outputDir + "zmumu_mc140_pt2.txt");
    ofstream outFile_zmumu0(outputDir + "zmumu_mc0.txt");
    ofstream outFile_zmumu0_pt2(outputDir + "zmumu_mc0_pt2.txt");

    int max_bin_num = 4096;
    int min_bin_num = 128;

    // ttbar (PU200)
    string fullPath = string(basePath) + "/" + string(mcRootFilePath_pu200_ttbar);
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree, bin_num, false);
        outFile_ttbar << efficiency << " "  << purity << endl;

        cout << "[ttbar PU200] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    // ttbar (PU200) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree, bin_num, true);
        outFile_ttbar_pt2 << efficiency << " "  << purity << endl;

        cout << "[ttbar PU200 pt^2] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    // Zmumu (PU200)
    string fullPath_zmumu = string(basePath) + "/" + string(mcRootFilePath_pu200_zmumu);
    TFile *file_zmumu = new TFile(fullPath_zmumu.c_str());
    TTree *tree_zmumu = dynamic_cast<TTree*>(file_zmumu->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree_zmumu, bin_num, false);
        outFile_zmumu << efficiency << " "  << purity << endl;

        cout << "[Zmumu PU200] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    // Zmumu (PU200) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree_zmumu, bin_num, true);
        outFile_zmumu_pt2 << efficiency << " "  << purity << endl;

        cout << "[Zmumu PU200 pt^2] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    // Zmumu (PU140)
    string fullPath_zmumu_pu140 = string(basePath) + "/" + string(mcRootFilePath_pu140_zmumu);
    TFile *file_zmumu_pu140 = new TFile(fullPath_zmumu_pu140.c_str());
    TTree *tree_zmumu_pu140 = dynamic_cast<TTree*>(file_zmumu_pu140->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree_zmumu_pu140, bin_num, false);
        outFile_zmumu140 << efficiency << " "  << purity << endl;

        cout << "[Zmumu PU140] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    // Zmumu (PU140) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree_zmumu_pu140, bin_num, true);
        outFile_zmumu140_pt2 << efficiency << " "  << purity << endl;

        cout << "[Zmumu PU140 pt^2] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    // Zmumu (PU0)
    string fullPath_zmumu_pu0 = string(basePath) + "/" + string(mcRootFilePath_pu0_zmumu);
    TFile *file_zmumu_pu0 = new TFile(fullPath_zmumu_pu0.c_str());
    TTree *tree_zmumu_pu0 = dynamic_cast<TTree*>(file_zmumu_pu0->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree_zmumu_pu0, bin_num, false);
        outFile_zmumu0 << efficiency << " "  << purity << endl;

        cout << "[Zmumu PU0] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    // Zmumu (PU0) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency, bin_width) = pv_reco(tree_zmumu_pu0, bin_num, true);
        outFile_zmumu0_pt2 << efficiency << " "  << purity << endl;

        cout << "[Zmumu PU0 pt^2] efficiency: " << efficiency << ", purity: " << purity << " bin_num: " << bin_num << " bin_width: " << bin_width << endl;
    }

    outFile_ttbar.close();
    outFile_ttbar_pt2.close();
    outFile_zmumu.close();
    outFile_zmumu_pt2.close();
    outFile_zmumu140.close();
    outFile_zmumu140_pt2.close();
    outFile_zmumu0.close();
    outFile_zmumu0_pt2.close();
}
