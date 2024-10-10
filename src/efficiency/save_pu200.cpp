#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <map>
#include <vector>
#include <cmath>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* mcRootFilePath_pu200_ttbar = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* mcRootFilePath_pu200_zmumu = getenv("MC_DATA_DIR_PU200_Z_MUMU");
const char* mcRootFilePath_pu140_zmumu = getenv("MC_DATA_DIR_PU140_Z_MUMU");
const char* mcRootFilePath_pu0_zmumu = getenv("MC_DATA_DIR_PU0_Z_MUMU");
const char* dataDir = "src/efficiency/data";

tuple<vector<vector<double>>, double, double> pv_reco(TTree *tree, int bin_num) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<float> *id_trk_theta = nullptr;
    vector<double> *truth_pt = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_charge = nullptr;
    vector<double> *truth_rapidity = nullptr;

    vector<vector<double>> reco_z0;
    double bin_width;

    int num_pv_tracks = 0; // マッチングが取れたトラックの数(MCのPVから派生したトラックの数)
    int num_pv_tracks_within_bin = 0; // 再構成で求めたPVの範囲内にある、かつマッチングが取れたトラックの数

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("id_trk_phi", &id_trk_phi);
    tree->SetBranchAddress("id_trk_eta", &id_trk_eta);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);
    tree->SetBranchAddress("truth_eta", &truth_eta);

    int entries = tree->GetEntries();

    // マッチング用のビン幅を設定
    const double phi_bin_width = 0.02; // phiのビン幅
    const double eta_bin_width = 0.02; // etaのビン幅

    double efficiency = 0.0;

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
        reco_z0.push_back({bin_low_edge, bin_up_edge});
        bin_width = bin_up_edge - bin_low_edge;

        delete tempHist;

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {

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
                            if (abs(id_trk_phi->at(i) - truth_phi->at(idx)) < 0.005 &&
                                abs(id_trk_eta->at(i) - truth_eta->at(idx)) < 0.005 &&
                                abs(1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(idx)) / (1.0 / truth_pt->at(idx)) < 0.2) {
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

        efficiency = efficiency + static_cast<double>(num_pv_tracks_within_bin) / num_pv_tracks;
    }

    double avg_efficiency = efficiency / entries;

    return make_tuple(reco_z0, bin_width, avg_efficiency);
}

tuple<vector<vector<double>>, double, double> pv_reco_pt2(TTree *tree, int bin_num) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<float> *id_trk_theta = nullptr;
    vector<double> *truth_pt = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_charge = nullptr;
    vector<double> *truth_rapidity = nullptr;

    vector<vector<double>> reco_z0;
    double bin_width;

    int num_pv_tracks = 0; // マッチングが取れたトラックの数(MCのPVから派生したトラックの数)
    int num_pv_tracks_within_bin = 0; // 再構成で求めたPVの範囲内にある、かつマッチングが取れたトラックの数

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0", &id_trk_z0);
    tree->SetBranchAddress("id_trk_phi", &id_trk_phi);
    tree->SetBranchAddress("id_trk_eta", &id_trk_eta);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);
    tree->SetBranchAddress("truth_eta", &truth_eta);

    int entries = tree->GetEntries();

    // マッチング用のビン幅を設定
    const double phi_bin_width = 0.02; // phiのビン幅
    const double eta_bin_width = 0.02; // etaのビン幅
    double efficiency = 0.0;

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
            tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i) * id_trk_pt->at(i));
        }

        int maxBin = tempHist->GetMaximumBin();
        double bin_low_edge = tempHist->GetXaxis()->GetBinLowEdge(maxBin);
        double bin_up_edge = tempHist->GetXaxis()->GetBinUpEdge(maxBin);
        reco_z0.push_back({bin_low_edge, bin_up_edge});
        bin_width = bin_up_edge - bin_low_edge;

        delete tempHist;

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {

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
                            if (abs(id_trk_phi->at(i) - truth_phi->at(idx)) < 0.005 &&
                                abs(id_trk_eta->at(i) - truth_eta->at(idx)) < 0.005 &&
                                abs(1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(idx)) / (1.0 / truth_pt->at(idx)) < 0.2) {
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

        efficiency = efficiency + static_cast<double>(num_pv_tracks_within_bin) / num_pv_tracks;
    }

    double avg_efficiency = efficiency / entries;

    return make_tuple(reco_z0, bin_width, avg_efficiency);
}

void save_pu200() {
    vector<vector<double>> reco_z0;
    double bin_width;
    double efficiency;

    string outputDir = string(basePath)+ "/" + string(dataDir) + "/";
    ofstream outFile_ttbar(outputDir + "ttbar_efficiency.txt");
    ofstream outFile_ttbar_pt2(outputDir + "ttbar_efficiency_pt2.txt");
    ofstream outFile_zmumu_200(outputDir + "zmumu_200_efficiency.txt");
    ofstream outFile_zmumu_200_pt2(outputDir + "zmumu_200_efficiency_pt2.txt");
    ofstream outFile_zmumu_140(outputDir + "zmumu_140_efficiency.txt");
    ofstream outFile_zmumu_140_pt2(outputDir + "zmumu_140_efficiency_pt2.txt");
    ofstream outFile_zmumu_0(outputDir + "zmumu_0_efficiency.txt");
    ofstream outFile_zmumu_0_pt2(outputDir + "zmumu_0_efficiency_pt2.txt");

    int max_bin_num = 65536;
    int min_bin_num = 256;

    // ttbar (PU200)
    string fullPath = string(basePath) + "/" + string(mcRootFilePath_pu200_ttbar);
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco(tree, bin_num);

        outFile_ttbar << bin_width << " " << efficiency << endl;

        cout << "[ttbar] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // ttbar (PU200) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco_pt2(tree, bin_num);

        outFile_ttbar_pt2 << bin_width << " " << efficiency << endl;

        cout << "[ttbar pt^2] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU200)
    string fullPath_zmumu = string(basePath) + "/" + string(mcRootFilePath_pu200_zmumu);
    TFile *file_zmumu = new TFile(fullPath_zmumu.c_str());
    TTree *tree_zmumu = dynamic_cast<TTree*>(file_zmumu->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco(tree_zmumu, bin_num);

        outFile_zmumu_200 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu 200] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU200) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco_pt2(tree_zmumu, bin_num);

        outFile_zmumu_200_pt2 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu 200 pt^2] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU140)
    string fullPath_zmumu_pu140 = string(basePath) + "/" + string(mcRootFilePath_pu140_zmumu);
    TFile *file_zmumu_pu140 = new TFile(fullPath_zmumu_pu140.c_str());
    TTree *tree_zmumu_pu140 = dynamic_cast<TTree*>(file_zmumu_pu140->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco(tree_zmumu_pu140, bin_num);

        outFile_zmumu_140 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu PU140] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU140) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco_pt2(tree_zmumu_pu140, bin_num);

        outFile_zmumu_140_pt2 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu PU140 pt^2] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU0)
    string fullPath_zmumu_pu0 = string(basePath) + "/" + string(mcRootFilePath_pu0_zmumu);
    TFile *file_zmumu_pu0 = new TFile(fullPath_zmumu_pu0.c_str());
    TTree *tree_zmumu_pu0 = dynamic_cast<TTree*>(file_zmumu_pu0->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco(tree_zmumu_pu0, bin_num);
        cout << "length of reco_z0: " << reco_z0.size() << endl;

        outFile_zmumu_0 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu PU0] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU0) pt^2
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, bin_width, efficiency) = pv_reco_pt2(tree_zmumu_pu0, bin_num);

        outFile_zmumu_0_pt2 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu PU0 pt^2] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    outFile_ttbar.close();
    outFile_ttbar_pt2.close();
    outFile_zmumu_200.close();
    outFile_zmumu_200_pt2.close();
    outFile_zmumu_140.close();
    outFile_zmumu_140_pt2.close();
    outFile_zmumu_0.close();
    outFile_zmumu_0_pt2.close();
}
