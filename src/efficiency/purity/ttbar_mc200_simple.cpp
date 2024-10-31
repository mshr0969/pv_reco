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
// const char* mcTtbarFilePath = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* mcTtbarFilePath = getenv("MC_DATA_DIR_PU0_Z_MUMU");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data";

tuple<double, double> pv_reco(TTree *tree, int bin_num) {
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
    vector<double> *true_vxp_z = nullptr;

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
    tree->SetBranchAddress("true_vxp_z", &true_vxp_z);

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

        TH1D *tempHist = new TH1D("tempHist", "Temporary Histogram", bin_num, -200, 200);
        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i));
        }

        int maxBin = tempHist->GetMaximumBin();
        double bin_low_edge = tempHist->GetXaxis()->GetBinLowEdge(maxBin);
        double bin_up_edge = tempHist->GetXaxis()->GetBinUpEdge(maxBin);
        bin_width = bin_up_edge - bin_low_edge;

        delete tempHist;

        // primary interactionは、最初の100個のうち、最もユニーク数が多いものとする
        // double primary_vertex;
        // vector<double> truth_z;
        // for (size_t i = 0; i < 100; ++i) {
        //     truth_z.push_back(true_vxp_z->at(i));
        // }
        // primary_vertex = *max_element(truth_z.begin(), truth_z.end());

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            // etaが2を超えるトラックは除く
            if (abs(id_trk_eta->at(i)) > 2.0) {
                continue;
            }
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
                                abs(1.0 / id_trk_pt->at(i) - 1.0 / truth_pt->at(idx)) / (1.0 / truth_pt->at(idx)) < 0.2 ) {
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

        if (num_tracks_within_bin_width == 0 || num_pv_tracks_within_bin == 0) {
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
    int failed_entries = entries - successful_entries;

    cout << "efficiency: " << efficiency << ", purity: " << purity << ", total_efficiency = " << total_efficiency << ", failed_entries: " << failed_entries << ", bin_width = " << bin_width << endl;

    return make_tuple(purity, efficiency);
}


void run3_plot() {
    string txtDir = string(basePath)+ "/" + string(dataDir) + "/";
    // TGraph *g1 = new TGraph((txtDir + "run3_ttbar_purity_ftf.txt").c_str());
    TGraph *g1 = new TGraph((txtDir + "zmumu_pu0.txt").c_str());
    g1->SetTitle("");
    g1->SetMarkerColor(kBlue);
    g1->SetLineColor(kBlue);
    g1->SetMarkerStyle(21);
    g1->SetLineWidth(4);

    TCanvas *c1 = new TCanvas("c1", "", 850, 600);
    g1->GetXaxis()->SetTitle("efficiency");
    g1->GetYaxis()->SetTitle("purity");
    g1->GetXaxis()->SetTitleSize(0.04);
    g1->GetYaxis()->SetTitleSize(0.04);
    g1->GetXaxis()->SetLabelSize(0.04);
    g1->GetYaxis()->SetLabelSize(0.04);
    g1->Draw("ALP");

    // ATLASラベルを追加
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.2, 0.5, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.32, 0.5, "Simulation Work In Progress");
    latex.DrawLatex(0.2, 0.45, "#sqrt{s} = 14 TeV");

    c1->Print((string(basePath) + "/output/" + string(dir) + "/zmumu_pu0.pdf").c_str());
}


void ttbar_mc200_simple() {
    string fullPath = string(basePath) + "/" + string(mcTtbarFilePath);

    vector<double> primary_vertexies;
    double purity;
    double efficiency;

    string outputDir = string(basePath) + "/" + string(dataDir) + "/";
    ofstream outFile_ttbar(outputDir + "zmumu_pu0.txt");

    int max_bin_num = 4096;
    int min_bin_num = 128;

    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency) = pv_reco(tree, bin_num);
        outFile_ttbar << efficiency << " "  << purity << endl;
    }

    outFile_ttbar.close();

    run3_plot();
}
