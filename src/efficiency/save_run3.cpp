#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* mcTtbarFilePath = getenv("MC_DATA_DIR");
const char* dataDir = "src/efficiency/data";

tuple<vector<vector<double>>, vector<double>, double> pv_reco(TTree *tree, bool is_ftf, int bin_num) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    vector<vector<double>> reco_z0;
    vector<double> primary_vertexies;
    double bin_width;

    const char* ptBranch = is_ftf ? "ftf_id_trk_pt" : "id_trk_pt";
    const char* z0Branch = is_ftf ? "ftf_id_trk_z0" : "id_trk_z0";

    tree->SetBranchAddress(ptBranch, &id_trk_pt);
    tree->SetBranchAddress(z0Branch ,&id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);

    int entries = tree->GetEntries();

    float min_z0 = numeric_limits<float>::max();
    float max_z0 = numeric_limits<float>::lowest();

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            min_z0 = min(min_z0, id_trk_z0->at(i));
            max_z0 = max(max_z0, id_trk_z0->at(i));
        }
    }

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
                primary_vertexies.push_back(primary_vertex_z);
                break;
            }
        }
    }

    return make_tuple(reco_z0, primary_vertexies, bin_width);
}

void save_run3() {
    string fullPath = string(basePath) + "/" + string(mcTtbarFilePath);

    vector<vector<double>> reco_z0;
    vector<double> primary_vertexies;
    double bin_width;

    string outputDir = string(basePath) + "/" + string(dataDir) + "/";
    ofstream outFile_ttbar_ftf(outputDir + "run3_ttbar_ftf.txt");
    ofstream outFile_ttbar_offline(outputDir + "run3_ttbar_offline.txt");

    int max_bin_num = 65536;
    int min_bin_num = 256;

    // FTF=true
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree, true, bin_num);

        int true_count = 0;
        int false_count = 0;

        for (size_t i = 0; i < reco_z0.size(); ++i) {
            if (reco_z0[i][0] < primary_vertexies[i] && primary_vertexies[i] < reco_z0[i][1]) {
                true_count++;
            } else {
                false_count++;
            }
        }

        double efficiency = static_cast<double>(true_count) / (true_count + false_count);
        outFile_ttbar_ftf << bin_width << " " << efficiency << endl;

        cout << "[FTF] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // FTF=false
    TFile *file_offline = new TFile(fullPath.c_str());
    TTree *tree_offline = dynamic_cast<TTree*>(file_offline->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree_offline, false, bin_num);

        int true_count = 0;
        int false_count = 0;

        for (size_t i = 0; i < reco_z0.size(); ++i) {
            if (reco_z0[i][0] < primary_vertexies[i] && primary_vertexies[i] < reco_z0[i][1]) {
                true_count++;
            } else {
                false_count++;
            }
        }

        double efficiency = static_cast<double>(true_count) / (true_count + false_count);
        outFile_ttbar_offline << bin_width << " " << efficiency << endl;

        cout << "[offline] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    outFile_ttbar_ftf.close();
    outFile_ttbar_offline.close();
}
