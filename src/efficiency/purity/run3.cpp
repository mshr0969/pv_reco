#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* mcTtbarFilePath = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* dataDir = "src/efficiency/data";

tuple<double, double> pv_reco(TTree *tree, bool is_ftf, int bin_num) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<int> *vxp_nTracks = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    vector<vector<double>> reco_z0;
    double bin_width;

    int num_pv_tracks = 0; // MCのPVから派生したトラックの数
    int num_pv_tracks_within_bin = 0; // 再構成で求めたPVの範囲内にある、PVから派生したトラックの数
    int num_tracks_within_bin_width = 0; // 再構成で求めたPVの範囲内にあるトラックの数

/*
    purity =  num_pv_tracks_within_bin / (num_tracks_within_bin_width + num_pv_tracks_within_bin)
    efficiency = num_pv_tracks_within_bin / num_pv_tracks
*/

    const char* ptBranch = is_ftf ? "ftf_id_trk_pt" : "id_trk_pt";
    const char* z0Branch = is_ftf ? "ftf_id_trk_z0" : "id_trk_z0";

    tree->SetBranchAddress(ptBranch, &id_trk_pt);
    tree->SetBranchAddress(z0Branch ,&id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);
    tree->SetBranchAddress("vxp_nTracks", &vxp_nTracks);

    int entries = tree->GetEntries();

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

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

        float primary_vertex_z = 0;
        for (size_t i = 0; i < vxp_z->size(); ++i) {
            if (vxp_type->at(i) == 1) {
                primary_vertex_z = vxp_z->at(i);
                num_pv_tracks += vxp_nTracks->at(i);

                if (bin_low_edge < primary_vertex_z && primary_vertex_z < bin_up_edge) {
                    num_pv_tracks_within_bin += vxp_nTracks->at(i);
                }
                break;
            }
        }

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            if (bin_low_edge < id_trk_z0->at(i) && id_trk_z0->at(i) < bin_up_edge) {
                num_tracks_within_bin_width++;
            }
        }
    }

    double purity = static_cast<double>(num_pv_tracks_within_bin) / (num_tracks_within_bin_width);
    double efficiency = static_cast<double>(num_pv_tracks_within_bin) / num_pv_tracks;

    cout << "num_pv_tracks_within_bin: " << num_pv_tracks_within_bin << ", num_tracks_within_bin_width: " << num_tracks_within_bin_width << ", num_pv_tracks: " << num_pv_tracks << endl;

    return make_tuple(purity, efficiency);
}

void run3() {
    string fullPath = string(basePath) + "/" + string(mcTtbarFilePath);

    vector<vector<double>> reco_z0;
    vector<double> primary_vertexies;
    double purity;
    double efficiency;

    string outputDir = string(basePath) + "/" + string(dataDir) + "/";
    ofstream outFile_ttbar_ftf(outputDir + "run3_ttbar_purity_ftf.txt");
    ofstream outFile_ttbar_offline(outputDir + "run3_ttbar_purity_offline.txt");

    int max_bin_num = 65536;
    int min_bin_num = 256;

    // FTF=true
    // TFile *file = new TFile(fullPath.c_str());
    // TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    // for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
    //     tie(purity, efficiency) = pv_reco(tree, true, bin_num);
    //     outFile_ttbar_ftf << efficiency << " "  << purity << endl;

    //     cout << "[FTF] efficiency: " << efficiency << ", purity: " << purity << endl;
    // }

    // FTF=false
    TFile *file_offline = new TFile(fullPath.c_str());
    TTree *tree_offline = dynamic_cast<TTree*>(file_offline->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(purity, efficiency) = pv_reco(tree_offline, false, bin_num);
        outFile_ttbar_offline << efficiency << " "  << purity << endl;

        cout << "[offline] efficiency: " << efficiency << ", purity: " << purity << endl;
    }

    outFile_ttbar_ftf.close();
    outFile_ttbar_offline.close();
}
