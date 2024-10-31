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
const char* mcFilePath = getenv("MC_DATA_DIR_PU0_Z_MUMU");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data";

void pv_reco(TTree *tree, int bin_num, const string &width) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<float> *id_trk_phi = nullptr;
    vector<float> *id_trk_eta = nullptr;
    vector<float> *id_trk_theta = nullptr;
    vector<float> *id_trk_d0 = nullptr;
    vector<int> *vxp_nTracks = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    vector<double> *truth_pt = nullptr;
    vector<double> *truth_phi = nullptr;
    vector<double> *truth_eta = nullptr;
    vector<double> *truth_charge = nullptr;
    vector<double> *truth_rapidity = nullptr;
    vector<double> *true_vxp_x = nullptr;
    vector<double> *true_vxp_y = nullptr;
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
    tree->SetBranchAddress("id_trk_d0", &id_trk_d0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);
    tree->SetBranchAddress("vxp_nTracks", &vxp_nTracks);
    tree->SetBranchAddress("truth_pt", &truth_pt);
    tree->SetBranchAddress("truth_phi", &truth_phi);
    tree->SetBranchAddress("truth_charge", &truth_charge);
    tree->SetBranchAddress("truth_eta", &truth_eta);
    tree->SetBranchAddress("truth_rapidity", &truth_rapidity);
    tree->SetBranchAddress("true_vxp_x", &true_vxp_x);
    tree->SetBranchAddress("true_vxp_y", &true_vxp_y);
    tree->SetBranchAddress("true_vxp_z", &true_vxp_z);

    int entries = tree->GetEntries();

    // マッチング用のビン幅を設定
    const double phi_bin_width = 0.02; // phiのビン幅
    const double eta_bin_width = 0.02; // etaのビン幅

    TH1D *unmatched_pt_hist = new TH1D("unmatched_pt", "pT of Unmatched Tracks", 128, 0, 10000);
    TH1D *unmatched_eta_hist = new TH1D("unmatched_eta", "Eta of Unmatched Tracks", 128, -5, 5);
    TH1D *unmatched_phi_hist = new TH1D("unmatched_phi", "Phi of Unmatched Tracks", 64, -4, 4);
    TH1D *unmatched_z0_hist = new TH1D("unmathched_z0", "z0 of Unmatched Trakcs", 128, -200, 200);
    TH1D *unmatched_d0_hist = new TH1D("unmatched_d0", "d0 of Unmatched Tracks", 128, -2, 2);

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

        // primary interactionは、最もユニーク数が多いものとする
        double primary_vertex;
        vector<double> truth_z;
        for (size_t i = 0; i < 10; ++i) {
            truth_z.push_back(true_vxp_z->at(i));
        }
        primary_vertex = *max_element(truth_z.begin(), truth_z.end());

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            // pt<1000MeV/cのトラックは除く
            if (id_trk_pt->at(i) < 1000) {
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
                                    matched = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (matched) break;
                }
                if (matched) break;
            }

            // binの幅に入っていて、マッチングが取れなかったトラックをプロット
            if (bin_low_edge < id_trk_z0->at(i) && id_trk_z0->at(i) < bin_up_edge && !matched) {
                unmatched_pt_hist->Fill(id_trk_pt->at(i));
                unmatched_eta_hist->Fill(id_trk_eta->at(i));
                unmatched_phi_hist->Fill(id_trk_phi->at(i));
                unmatched_z0_hist->Fill(id_trk_z0->at(i));
                unmatched_d0_hist->Fill(id_trk_d0->at(i));
            }
        }
    }

    TCanvas *c1 = new TCanvas("c1", "Unmatched Tracks", 1000, 800);
    c1->Divide(3, 2);

    c1->cd(1);
    unmatched_pt_hist->Draw();
    c1->cd(2);
    unmatched_eta_hist->Draw();
    c1->cd(3);
    unmatched_phi_hist->Draw();
    c1->cd(4);
    unmatched_z0_hist->Draw();
    c1->cd(5);
    unmatched_d0_hist->Draw();

    c1->SaveAs(("output/efficiency/purity/unmatched_tracks_distributions_" + width + ".pdf").c_str());

    delete unmatched_pt_hist;
    delete unmatched_eta_hist;
    delete unmatched_phi_hist;
    delete unmatched_z0_hist;
    delete c1;
}

void pu0_cut() {
    string fullPath = string(basePath) + "/" + string(mcFilePath);

    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    // pv_reco(tree, 4096, "narrow");
    pv_reco(tree, 128, "wide");
}
